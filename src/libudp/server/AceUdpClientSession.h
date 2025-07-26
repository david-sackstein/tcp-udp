#pragma once

#include <libudp/server/IUdpSession.h>
#include <liblogger/ILogger.h>

#include <ace/SOCK_Dgram.h>
#include <ace/INET_Addr.h>

class AceUdpClientSession final : public udp::IUdpSession {
public:
    AceUdpClientSession(
        logger::ILogger& logger,
        Endpoint  local_endpoint,
        ACE_SOCK_Dgram& server_socket);

    IOResult read_from(
        Buffer buffer,
        Endpoint& sender,
        std::chrono::milliseconds timeout) override;

    IOResult write_to(
        ConstBuffer buffer,
        const Endpoint& remote,
        std::chrono::milliseconds timeout) override;

private:
    logger::ILogger& logger_;
    Endpoint local_endpoint_;
    ACE_SOCK_Dgram& server_socket_;
};