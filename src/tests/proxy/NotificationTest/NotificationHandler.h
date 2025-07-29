#pragma once

#include <libtcp/server/ITcpClientHandler.h>
#include <liblogger/ILogger.h>
#include <common/task/RunningTask.h>

#include <memory>
#include <vector>
#include <mutex>

// Custom handler for multi-client notification test
class NotificationHandler final : public tcp::ITcpClientHandler {
public:
    explicit NotificationHandler(logger::ILogger& logger);

    std::unique_ptr<ITask> handle_client(std::unique_ptr<tcp::ITcpSession> client_session) override;

private:
    logger::ILogger& logger_;
    std::vector<std::shared_ptr<tcp::ITcpSession>> active_sessions_;
    std::mutex sessions_mutex_;
}; 