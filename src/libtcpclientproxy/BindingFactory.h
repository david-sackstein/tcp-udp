#pragma once

#include "UdpTcpBinding.h"

#include <common/Endpoint.h>
#include <liblogger/ILogger.h>
#include <libtcp/ITcpSession.h>
#include <libudp/server/IUdpSession.h>

#include <memory>

class BindingFactory final {
public:
    BindingFactory(logger::ILogger& logger, Endpoint tcp_server_endpoint, std::shared_ptr<ACE_Reactor> reactor);

    std::unique_ptr<UdpTcpBinding> createBinding(const std::string& source_key,
        udp::IUdpSession& udp_session,
        const Endpoint& udp_sender) const;

private:
    std::shared_ptr<tcp::ITcpSession> createTcpConnection() const;

    logger::ILogger& logger_;
    const Endpoint tcp_server_endpoint_;
    std::shared_ptr<ACE_Reactor> reactor_;
};