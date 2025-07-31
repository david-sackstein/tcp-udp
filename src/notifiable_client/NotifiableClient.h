#pragma once

#include <libtcp/Exports.h>

#include <atomic>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

class NotifiableClientArgs;

class NotifiableClient final {
public:
    explicit NotifiableClient(logger::ILogger& validation_logger, logger::ILogger& communication_logger);
    ~NotifiableClient();

    bool connect(const std::string& endpoint);
    void start(const NotifiableClientArgs& args);
    void stop();

private:
    void sender_loop(const NotifiableClientArgs& args);
    void receiver_loop();

    void validate_echo_response(const std::string& response,
        const std::string& expected_client_id,
        int expected_sequence_number);
    void validate_notification(const std::string& notification);
    std::string create_request_message(const std::string& client_id, int sequence_number, const std::string& message);
    bool parse_echo_response(const std::string& response,
        std::string& client_id,
        int& sequence_number,
        std::string& message);
    bool parse_notification(const std::string& notification,
        std::string& client_id,
        int& sequence_number,
        std::string& message);

    bool parse_inner_content(const std::string& inner_content,
        std::string& client_id,
        int& sequence_number,
        std::string& message);
    void handle_echo_response(const std::string& response);
    void handle_notification_response(const std::string& response);
    void process_response(const std::string& response);

    logger::ILogger& validation_logger_;
    logger::ILogger& communication_logger_;
    std::unique_ptr<tcp::ITcpClient> client_;
    std::shared_ptr<tcp::ITcpSession> session_;
    std::atomic<bool> running_{false};
    std::thread sender_thread_;
    std::thread receiver_thread_;

    std::map<std::string, int> client_sequence_numbers_;
    std::mutex validation_mutex_;
    int current_sequence_number_{0};
    std::string current_client_id_;
};