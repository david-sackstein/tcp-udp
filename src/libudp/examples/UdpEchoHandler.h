#pragma once

#include "libudp/server/IUdpClientHandler.h"

class UdpEchoHandler final : public udp::IUdpClientHandler {
public:
    std::unique_ptr<ITask> handle_client(udp::IUdpSession& client_session) override;
};
