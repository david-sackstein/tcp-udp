#pragma once

#include "ClientKeyManager.h"
#include "TcpSessionManager.h"
#include "UdpForwarder.h"
#include <common/Endpoint.h>
#include <liblogger/ILogger.h>
#include <libtcp/server/ITcpClientHandler.h>
#include <libudp/client/IUdpClient.h>

#include <memory>

// Forward declaration
class TcpServerProxyUdpHandler;

class TcpServerProxyClientHandler final : public tcp::ITcpClientHandler {
public:
    TcpServerProxyClientHandler(
        logger::ILogger& logger,
        Endpoint udp_proxy,
        udp::IUdpClient &udp_client,
        TcpServerProxyUdpHandler* udp_handler
    );

    std::unique_ptr<ITask> handle_client(std::unique_ptr<tcp::ITcpSession> client_session) override;

private:
    logger::ILogger& logger_;
    TcpServerProxyUdpHandler* udp_handler_;
    std::unique_ptr<TcpSessionManager> session_manager_;
    std::unique_ptr<UdpForwarder> udp_forwarder_;
    std::unique_ptr<ClientKeyManager> client_key_manager_;
}; 