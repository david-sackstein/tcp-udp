#pragma once

#include <common/task/RunningTask.h>
#include <liblogger/ILogger.h>
#include <libtcp/server/ITcpClientHandler.h>

#include <memory>
#include <mutex>
#include <vector>

// Custom handler for multi-client notification test
class NotificationHandler final : public tcp::ITcpClientHandler {
public:
    explicit NotificationHandler(logger::ILogger& logger);

    std::unique_ptr<ITask> handle_client(std::unique_ptr<tcp::ITcpSession> client_session) override;

private:
    static constexpr size_t BUFFER_SIZE = 1024;
    static constexpr auto TIMEOUT_MS = std::chrono::milliseconds{100};

    void process_message(const std::string& message, const std::shared_ptr<tcp::ITcpSession>& sender_session);
    void send_echo_response(const std::string& message, const std::shared_ptr<tcp::ITcpSession>& sender_session) const;
    void send_notifications_to_others(const std::string& message,
        const std::shared_ptr<tcp::ITcpSession>& sender_session);
    void remove_session(const std::shared_ptr<tcp::ITcpSession>& session);

    logger::ILogger& logger_;
    std::vector<std::shared_ptr<tcp::ITcpSession>> active_sessions_;
    std::mutex sessions_mutex_;
};