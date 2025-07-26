#pragma once

#include "libudp/server/IUdpClientHandler.h"
#include <liblogger/ILogger.h>

class UdpEchoHandler final : public udp::IUdpClientHandler {
public:
    explicit UdpEchoHandler(logger::ILogger& logger);
    std::unique_ptr<ITask> handle_client(udp::IUdpSession& client_session) override;

private:
    logger::ILogger& logger_;
};
