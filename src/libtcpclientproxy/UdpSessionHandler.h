#pragma once

#include <common/OwnedBuffer.h>
#include <liblogger/ILogger.h>
#include <libudp/server/IUdpSession.h>

#include <memory>

class UdpSessionHandler final {
public:
    explicit UdpSessionHandler(logger::ILogger& logger);

    ConstBuffer readUdpMessage(udp::IUdpSession& client_session, Endpoint& udp_sender);
    void sendResponseToUdp(udp::IUdpSession& client_session, ConstBuffer response, const Endpoint& udp_sender) const;

private:
    logger::ILogger& logger_;
    OwnedBuffer buffer_{2048};
    static constexpr std::chrono::milliseconds timeout_ms{1000};
};