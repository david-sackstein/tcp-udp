#pragma once

#include <libtcp/server/ITcpClientHandler.h>
#include <liblogger/ILogger.h>
#include <common/task/ITask.h>

#include <memory>

// Simple echo handler that echoes back the exact message without any prefix
class SimpleEchoHandler : public tcp::ITcpClientHandler {
public:
    explicit SimpleEchoHandler(logger::ILogger& logger);

    std::unique_ptr<ITask> handle_client(std::unique_ptr<tcp::ITcpSession> client_session) override;

private:
    logger::ILogger& logger_;
}; 