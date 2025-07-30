#pragma once

#include <common/Endpoint.h>
#include <liblogger/ILogger.h>
#include <libtcp/server/ITcpClientHandler.h>
#include <libudp/client/IUdpClient.h>
#include "ClientKeyManager.h"
#include "SessionManager.h"
#include "UdpForwarder.h"

#include <memory>

// Forward declaration
class ServerProxyUdpHandler;

class ServerProxyClientHandler final : public tcp::ITcpClientHandler {
public:
    ServerProxyClientHandler(logger::ILogger& logger,
        Endpoint udp_proxy,
        udp::IUdpClient& udp_client,
        ServerProxyUdpHandler* udp_handler);

    std::unique_ptr<ITask> handle_client(std::unique_ptr<tcp::ITcpSession> client_session) override;

private:
    ServerProxyUdpHandler* udp_handler_;
    std::unique_ptr<SessionManager> session_manager_;
    std::unique_ptr<UdpForwarder> udp_forwarder_;
    std::unique_ptr<ClientKeyManager> client_key_manager_;
};