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

    // Validation methods
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

    logger::ILogger& validation_logger_;    // Always verbose for validation messages
    logger::ILogger& communication_logger_; // Depends on verbose flag for communication logs
    std::unique_ptr<tcp::ITcpClient> client_;
    std::shared_ptr<tcp::ITcpSession> session_;
    std::atomic<bool> running_{false};
    std::thread sender_thread_;
    std::thread receiver_thread_;

    // Validation tracking
    std::map<std::string, int> client_sequence_numbers_; // Track last seen sequence number per client
    std::mutex validation_mutex_;
    int current_sequence_number_{0};
    std::string current_client_id_; // Track the current client's ID for validation
};