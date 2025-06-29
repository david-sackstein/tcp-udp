#pragma once

#include <libtcp/server/ITcpClientHandler.h>

class TcpEchoHandler : public tcp::ITcpClientHandler {
public:
    std::unique_ptr<ITask> handle_client(std::unique_ptr<tcp::ITcpSession> session) override;
};
