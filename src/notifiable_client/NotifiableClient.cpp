#include "NotifiableClient.h"
#include "NotifiableClientArgs.h"

#include <common/Constants.h>
#include <common/OwnedBuffer.h>

#include <chrono>
#include <iostream>
#include <thread>

NotifiableClient::NotifiableClient(logger::ILogger& logger) : logger_(logger) {}

NotifiableClient::~NotifiableClient() {
    stop();
}

bool NotifiableClient::connect(const std::string& endpoint) {
    client_ = tcp::create_tcp_client(logger_);
    if (!client_) {
        logger_.log(logger::LogLevel::ERROR, "Failed to create client");
        return false;
    }

    Endpoint ep = Endpoint::from_string(endpoint);
    session_ = client_->connect(Endpoint::any_loop_back(), ep);
    if (!session_) {
        logger_.log(logger::LogLevel::ERROR, "Failed to connect to server");
        return false;
    }

    logger_.log(logger::LogLevel::INFO, "Connected to server successfully");
    return true;
}

void NotifiableClient::start(const NotifiableClientArgs& args) {
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

void NotifiableClient::sender_loop(const NotifiableClientArgs& args) {
    static std::chrono::milliseconds block = std::chrono::milliseconds::max();
    int request_count = 0;

    while (running_) {
        std::string message = "request from " + args.get_client_id() + ": " + std::to_string(request_count++);

        auto result = session_->write(ConstBuffer(message.data(), message.size()), block);
        if (result.code != IOResultCode::Success) {
            logger_.log(logger::LogLevel::ERROR, "Failed to send message: %s", result.error_message.c_str());
            break;
        }

        logger_.log(logger::LogLevel::INFO, "Sent: %s", message.c_str());

        std::this_thread::sleep_for(std::chrono::milliseconds(args.get_request_interval_ms()));
    }
}

void NotifiableClient::receiver_loop() {
    OwnedBuffer buffer(common::STANDARD_BUFFER_SIZE);

    while (running_) {
        auto result = session_->read(buffer.view(), std::chrono::milliseconds(100));

        if (result.code == IOResultCode::Success && result.count > 0) {
            std::string response(buffer.view().data, result.count);
            logger_.log(logger::LogLevel::INFO, "Received: %s", response.c_str());
            continue;
        }

        if (result.code == IOResultCode::ConnectionClosed) {
            logger_.log(logger::LogLevel::INFO, "Server disconnected");
            break;
        }
    }
}