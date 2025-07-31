#include "NotifiableClient.h"
#include "NotifiableClientArgs.h"

#include <common/Constants.h>
#include <common/OwnedBuffer.h>

#include <chrono>
#include <iostream>
#include <thread>

NotifiableClient::NotifiableClient(logger::ILogger& validation_logger, logger::ILogger& communication_logger)
    : validation_logger_(validation_logger), communication_logger_(communication_logger) {}

NotifiableClient::~NotifiableClient() {
    stop();
}

bool NotifiableClient::connect(const std::string& endpoint) {
    client_ = tcp::create_tcp_client(communication_logger_);
    if (!client_) {
        communication_logger_.log(logger::LogLevel::ERROR, "Failed to create client");
        return false;
    }

    Endpoint ep = Endpoint::from_string(endpoint);
    session_ = client_->connect(Endpoint::any_loop_back(), ep);
    if (!session_) {
        communication_logger_.log(logger::LogLevel::ERROR, "Failed to connect to server");
        return false;
    }

    communication_logger_.log(logger::LogLevel::INFO, "Connected to server successfully");
    return true;
}

void NotifiableClient::start(const NotifiableClientArgs& args) {
    current_client_id_ = args.get_client_id(); // Store the client ID for validation
    running_ = true;

    // Start receiver thread
    receiver_thread_ = std::thread([this]() { receiver_loop(); });

    // Start sender thread
    sender_thread_ = std::thread([this, &args]() { sender_loop(args); });
}

void NotifiableClient::stop() {
    running_ = false;

    if (sender_thread_.joinable()) {
        sender_thread_.join();
    }

    if (receiver_thread_.joinable()) {
        receiver_thread_.join();
    }

    if (client_) {
        client_->disconnect();
    }
}

std::string NotifiableClient::create_request_message(const std::string& client_id,
    int sequence_number,
    const std::string& message) {
    return "request:" + client_id + ":" + std::to_string(sequence_number) + ":" + message;
}

bool NotifiableClient::parse_echo_response(const std::string& response,
    std::string& client_id,
    int& sequence_number,
    std::string& message) {
    // Expected format: "echo [request:client_id:sequence_number:message]"
    if (response.length() < 6 || response.substr(0, 6) != "echo [") {
        return false;
    }

    std::string inner_content = response.substr(6, response.length() - 7); // Remove "echo [" and "]"

    // Parse "request:client_id:sequence_number:message"
    size_t pos1 = inner_content.find(':');
    if (pos1 == std::string::npos || inner_content.substr(0, pos1) != "request") {
        return false;
    }

    size_t pos2 = inner_content.find(':', pos1 + 1);
    if (pos2 == std::string::npos) {
        return false;
    }

    client_id = inner_content.substr(pos1 + 1, pos2 - pos1 - 1);

    size_t pos3 = inner_content.find(':', pos2 + 1);
    if (pos3 == std::string::npos) {
        return false;
    }

    try {
        sequence_number = std::stoi(inner_content.substr(pos2 + 1, pos3 - pos2 - 1));
    } catch (const std::exception&) {
        return false;
    }

    message = inner_content.substr(pos3 + 1);
    return true;
}

bool NotifiableClient::parse_notification(const std::string& notification,
    std::string& client_id,
    int& sequence_number,
    std::string& message) {
    // Expected format: "notify [request:client_id:sequence_number:message]"
    if (notification.length() < 8 || notification.substr(0, 8) != "notify [") {
        return false;
    }

    std::string inner_content = notification.substr(8, notification.length() - 9); // Remove "notify [" and "]"

    // Parse "request:client_id:sequence_number:message"
    size_t pos1 = inner_content.find(':');
    if (pos1 == std::string::npos || inner_content.substr(0, pos1) != "request") {
        return false;
    }

    size_t pos2 = inner_content.find(':', pos1 + 1);
    if (pos2 == std::string::npos) {
        return false;
    }

    client_id = inner_content.substr(pos1 + 1, pos2 - pos1 - 1);

    size_t pos3 = inner_content.find(':', pos2 + 1);
    if (pos3 == std::string::npos) {
        return false;
    }

    try {
        sequence_number = std::stoi(inner_content.substr(pos2 + 1, pos3 - pos2 - 1));
    } catch (const std::exception&) {
        return false;
    }

    message = inner_content.substr(pos3 + 1);
    return true;
}

void NotifiableClient::validate_echo_response(const std::string& response,
    const std::string& expected_client_id,
    int expected_sequence_number) {
    std::string response_client_id;
    int response_sequence_number;
    std::string response_message;

    if (!parse_echo_response(response, response_client_id, response_sequence_number, response_message)) {
        validation_logger_.log(
            logger::LogLevel::ERROR, "VALIDATION ERROR: Failed to parse echo response: %s", response.c_str());
        return;
    }

    if (response_client_id != expected_client_id) {
        validation_logger_.log(logger::LogLevel::ERROR,
            "VALIDATION ERROR: Echo response client ID mismatch. Expected: %s, Got: %s", expected_client_id.c_str(),
            response_client_id.c_str());
        return;
    }

    if (response_sequence_number != expected_sequence_number) {
        validation_logger_.log(logger::LogLevel::ERROR,
            "VALIDATION ERROR: Echo response sequence number mismatch. Expected: %d, Got: %d", expected_sequence_number,
            response_sequence_number);
        return;
    }

    validation_logger_.log(logger::LogLevel::INFO, "VALIDATION SUCCESS: Echo response validated for %s, sequence %d",
        expected_client_id.c_str(), expected_sequence_number);
}

void NotifiableClient::validate_notification(const std::string& notification) {
    std::string client_id;
    int sequence_number;
    std::string message;

    if (!parse_notification(notification, client_id, sequence_number, message)) {
        validation_logger_.log(
            logger::LogLevel::ERROR, "VALIDATION ERROR: Failed to parse notification: %s", notification.c_str());
        return;
    }

    std::lock_guard<std::mutex> lock(validation_mutex_);

    auto it = client_sequence_numbers_.find(client_id);
    if (it == client_sequence_numbers_.end()) {
        // First notification from this client, record the sequence number
        client_sequence_numbers_[client_id] = sequence_number;
        validation_logger_.log(logger::LogLevel::INFO, "VALIDATION INFO: First notification from %s, sequence %d",
            client_id.c_str(), sequence_number);
    } else {
        // Verify sequence number is increasing
        int expected_sequence = it->second + 1;
        if (sequence_number != expected_sequence) {
            validation_logger_.log(logger::LogLevel::ERROR,
                "VALIDATION ERROR: Notification sequence number mismatch for %s. Expected: %d, Got: %d",
                client_id.c_str(), expected_sequence, sequence_number);
        } else {
            validation_logger_.log(logger::LogLevel::INFO,
                "VALIDATION SUCCESS: Notification sequence validated for %s, sequence %d", client_id.c_str(),
                sequence_number);
        }
        it->second = sequence_number;
    }
}

void NotifiableClient::sender_loop(const NotifiableClientArgs& args) {
    static std::chrono::milliseconds block = std::chrono::milliseconds::max();
    int request_count = 0;

    while (running_) {
        int sequence_number = ++current_sequence_number_;
        std::string base_message = "request from " + args.get_client_id() + ": " + std::to_string(request_count++);
        std::string message = create_request_message(args.get_client_id(), sequence_number, base_message);

        auto result = session_->write(ConstBuffer(message.data(), message.size()), block);
        if (result.code != IOResultCode::Success) {
            communication_logger_.log(
                logger::LogLevel::ERROR, "Failed to send message: %s", result.error_message.c_str());
            break;
        }

        communication_logger_.log(logger::LogLevel::INFO, "Sent: %s", message.c_str());

        std::this_thread::sleep_for(std::chrono::milliseconds(args.get_request_interval_ms()));
    }
}

void NotifiableClient::receiver_loop() {
    OwnedBuffer buffer(common::STANDARD_BUFFER_SIZE);

    while (running_) {
        auto result = session_->read(buffer.view(), std::chrono::milliseconds(100));

        if (result.code == IOResultCode::Success && result.count > 0) {
            std::string response(buffer.view().data, result.count);
            communication_logger_.log(logger::LogLevel::INFO, "Received: %s", response.c_str());

            // Validate the response
            if (response.substr(0, 5) == "echo ") {
                // This is an echo response to our request
                std::string response_client_id;
                int response_sequence_number;
                std::string response_message;
                if (parse_echo_response(response, response_client_id, response_sequence_number, response_message)) {
                    // Validate that the echo response matches what we sent
                    validate_echo_response(response, current_client_id_, response_sequence_number);
                }
            } else if (response.substr(0, 7) == "notify ") {
                // This is a notification about another client's request
                validate_notification(response);
            }
            continue;
        }

        if (result.code == IOResultCode::ConnectionClosed) {
            communication_logger_.log(logger::LogLevel::INFO, "Server disconnected");
            break;
        }
    }
}