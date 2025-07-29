#include "TcpSessionManager.h"

#include <common/task/RunningTask.h>

#include <stdexcept>

TcpSessionManager::TcpSessionManager(logger::ILogger& logger)
    : logger_(logger) {}

std::unique_ptr<ITask> TcpSessionManager::createSessionTask(
    std::shared_ptr<tcp::ITcpSession> client_session,
    const std::string& client_key,
    std::function<void(tcp::ITcpSession&, const std::string&, std::atomic<bool>&)> session_handler,
    std::function<void(const std::string&)> cleanup_handler) const {
    
    return std::make_unique<RunningTask>([client_session, this, client_key, session_handler, cleanup_handler](std::atomic<bool>& cancelled) {
        try {
            session_handler(*client_session, client_key, cancelled);
        } catch (std::runtime_error& e) {
            logger_.log(logger::LogLevel::ERROR, "TcpSessionManager: Exception for client %s: %s", client_key.c_str(), e.what());
        }
        
        cleanup_handler(client_key);
        logger_.log(logger::LogLevel::INFO, "TcpSessionManager: Cleaned up client %s", client_key.c_str());
    });
} 