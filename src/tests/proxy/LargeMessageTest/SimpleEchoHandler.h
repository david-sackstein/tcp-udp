#pragma once

#include <libtcp/server/ITcpClientHandler.h>
#include <liblogger/ILogger.h>
#include <common/task/ITask.h>

#include <memory>

// Simple echo handler that echoes back the exact message without any prefix
class SimpleEchoHandler final : public tcp::ITcpClientHandler {
public:
    explicit SimpleEchoHandler(logger::ILogger& logger);

    std::unique_ptr<ITask> handle_client(std::unique_ptr<tcp::ITcpSession> client_session) override;

private:
    static constexpr size_t BUFFER_SIZE = 1024;
    static constexpr auto TIMEOUT_MS = std::chrono::milliseconds{100};
    
    logger::ILogger& logger_;
}; 