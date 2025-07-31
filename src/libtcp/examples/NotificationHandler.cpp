#include "NotificationHandler.h"

#include <common/OwnedBuffer.h>
#include <libacetools/IOResultCode.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <memory>
#include <string>

NotificationHandler::NotificationHandler(logger::ILogger& logger) : logger_(logger) {}

std::unique_ptr<ITask> NotificationHandler::handle_client(std::unique_ptr<tcp::ITcpSession> client_session) {
    std::shared_ptr shared_session = std::move(client_session);

    logger_.log(logger::LogLevel::INFO, "NotificationHandler: New client connected");

    // Add this session to our tracked sessions
    {
        std::lock_guard<std::mutex> lock(sessions_mutex_);
        active_sessions_.push_back(shared_session);
        logger_.log(logger::LogLevel::INFO, "NotificationHandler: Total active sessions: %zu", active_sessions_.size());
    }

    return std::make_unique<RunningTask>([shared_session, this](std::atomic<bool>& cancelled) {
        OwnedBuffer buffer_in(BUFFER_SIZE);

        while (!cancelled) {
            auto read_result = shared_session->read(buffer_in.view(), TIMEOUT_MS);

            if (read_result.code == IOResultCode::Error || read_result.code == IOResultCode::ConnectionClosed) {
                logger_.log(logger::LogLevel::INFO, "NotificationHandler: Client disconnected");
                break;
            }

            if (read_result.code == IOResultCode::Timeout) {
                continue;
            }

            std::string received_message(buffer_in.view().data, read_result.count);
            logger_.log(
                logger::LogLevel::INFO, "NotificationHandler: Received message: '%s'", received_message.c_str());

            process_message(received_message, shared_session);
        }

        remove_session(shared_session);
    });
}

void NotificationHandler::process_message(const std::string& message,
    const std::shared_ptr<tcp::ITcpSession>& sender_session) {
    send_echo_response(message, sender_session);
    send_notifications_to_others(message, sender_session);
}

void NotificationHandler::send_echo_response(const std::string& message,
    const std::shared_ptr<tcp::ITcpSession>& sender_session) const {
    std::string echo_response = "echo [" + message + "]";
    ConstBuffer echo_buffer(echo_response.data(), echo_response.size());
    auto echo_result = sender_session->write(echo_buffer, TIMEOUT_MS);
    logger_.log(logger::LogLevel::INFO, "NotificationHandler: Sent echo response to sender");
}

void NotificationHandler::send_notifications_to_others(const std::string& message,
    const std::shared_ptr<tcp::ITcpSession>& sender_session) {
    std::string notification = "notify [" + message + "]";
    ConstBuffer notify_buffer(notification.data(), notification.size());

    std::lock_guard<std::mutex> lock(sessions_mutex_);
    logger_.log(logger::LogLevel::INFO, "NotificationHandler: Sending notifications to %zu other clients",
        active_sessions_.size() - 1);

    for (auto& session : active_sessions_) {
        if (session.get() != sender_session.get()) {
            auto notify_result = session->write(notify_buffer, TIMEOUT_MS);
            logger_.log(logger::LogLevel::INFO, "NotificationHandler: Sent notification to other client");
        }
    }
}

void NotificationHandler::remove_session(const std::shared_ptr<tcp::ITcpSession>& session) {
    std::lock_guard<std::mutex> lock(sessions_mutex_);
    auto it = std::find(active_sessions_.begin(), active_sessions_.end(), session);
    if (it != active_sessions_.end()) {
        active_sessions_.erase(it);
        logger_.log(
            logger::LogLevel::INFO, "NotificationHandler: Removed session, remaining: %zu", active_sessions_.size());
    }
}