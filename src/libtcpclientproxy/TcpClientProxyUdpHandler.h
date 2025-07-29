#pragma once

#include "BindingFactory.h"
#include "TcpBindingManager.h"
#include "UdpSessionHandler.h"

#include <common/Endpoint.h>
#include <liblogger/ILogger.h>
#include <libudp/server/IUdpClientHandler.h>

#include <memory>

class TcpClientProxyUdpHandler final : public udp::IUdpClientHandler {
public:
    TcpClientProxyUdpHandler(logger::ILogger& logger, Endpoint local_endpoint, Endpoint tcp_server, std::shared_ptr<ACE_Reactor> reactor);

    std::unique_ptr<ITask> handle_client(udp::IUdpSession& client_session) override;

private:
    void sendToTcpServer(const UdpTcpBinding& binding, ConstBuffer data, const Endpoint& udp_sender) const;

    logger::ILogger& logger_;
    const Endpoint local_endpoint_;
    std::unique_ptr<UdpSessionHandler> udp_session_handler_;
    std::unique_ptr<TcpBindingManager> binding_manager_;
}; 