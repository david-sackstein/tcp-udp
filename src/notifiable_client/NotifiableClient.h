#pragma once

#include <libtcp/Exports.h>

#include <atomic>
#include <memory>
#include <string>
#include <thread>

class NotifiableClientArgs;

class NotifiableClient final {
public:
    explicit NotifiableClient(logger::ILogger& logger);
    ~NotifiableClient();

    bool connect(const std::string& endpoint);
    void start(const NotifiableClientArgs& args);
    void stop();

private:
    void sender_loop(const NotifiableClientArgs& args);
    void receiver_loop();

    logger::ILogger& logger_;
    std::unique_ptr<tcp::ITcpClient> client_;
    std::shared_ptr<tcp::ITcpSession> session_;
    std::atomic<bool> running_{false};
    std::thread sender_thread_;
    std::thread receiver_thread_;
};