#include "ServerProxyClientHandler.h"
#include "ClientKeyManager.h"
#include "ServerProxyUdpHandler.h"
#include "SessionManager.h"
#include "UdpForwarder.h"

#include <common/Endpoint.h>
#include <libudp/client/IUdpClient.h>

ServerProxyClientHandler::ServerProxyClientHandler(logger::ILogger& logger,
    Endpoint udp_proxy,
    udp::IUdpClient& udp_client,
    ServerProxyUdpHandler* udp_handler)
    : logger_(logger),
      udp_handler_(udp_handler),
      session_manager_(std::make_unique<SessionManager>(logger)),
      udp_forwarder_(std::make_unique<UdpForwarder>(logger, std::move(udp_proxy), udp_client)),
      client_key_manager_(std::make_unique<ClientKeyManager>(logger)) {}

std::unique_ptr<ITask> ServerProxyClientHandler::handle_client(std::unique_ptr<tcp::ITcpSession> client_session) {
    std::string client_key = client_key_manager_->generateClientKey(*client_session);
    client_key_manager_->logClientConnection(client_key);

    std::shared_ptr<tcp::ITcpSession> shared_session = std::move(client_session);
    udp_handler_->register_tcp_session(client_key, shared_session);

    return session_manager_->createSessionTask(
        shared_session, client_key,
        [this](tcp::ITcpSession& session, const std::string& key, std::atomic<bool>& cancelled) {
            udp_forwarder_->forwardTcpToUdp(session, key, cancelled);
        },
        [this](const std::string& key) { udp_handler_->unregister_tcp_session(key); });
}