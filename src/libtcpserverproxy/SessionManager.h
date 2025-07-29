#pragma once

#include <liblogger/ILogger.h>
#include <libtcp/server/ITcpClientHandler.h>

#include <atomic>
#include <functional>
#include <memory>

class SessionManager final {
public:
    explicit SessionManager(logger::ILogger& logger);
    
    std::unique_ptr<ITask> createSessionTask(
        std::shared_ptr<tcp::ITcpSession> client_session,
        const std::string& client_key,
        std::function<void(tcp::ITcpSession&, const std::string&, std::atomic<bool>&)> session_handler,
        std::function<void(const std::string&)> cleanup_handler) const;

private:
    logger::ILogger& logger_;
}; 