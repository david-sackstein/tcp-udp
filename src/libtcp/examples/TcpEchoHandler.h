#pragma once

#include <libtcp/server/ITcpClientHandler.h>
#include <liblogger/ILogger.h>

class TcpEchoHandler final : public tcp::ITcpClientHandler {
public:
    explicit TcpEchoHandler(logger::ILogger& logger);
    std::unique_ptr<ITask> handle_client(std::unique_ptr<tcp::ITcpSession> session) override;

private:
    logger::ILogger& logger_;
};
