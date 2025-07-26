#pragma once

#include "UdpTcpBinding.h"

#include <common/OwnedBuffer.h>
#include <libudp/server/IUdpClientHandler.h>
#include <liblogger/ILogger.h>

#include <unordered_map>
#include <memory>

class TcpClientProxyUdpHandler : public udp::IUdpClientHandler {
public:
    TcpClientProxyUdpHandler(logger::ILogger& logger, Endpoint  local_endpoint, Endpoint tcp_server);

    std::unique_ptr<ITask> handle_client(udp::IUdpSession& client_session) override;

private:
    using BindingPtr = std::unique_ptr<UdpTcpBinding>;
    using BindingMap = std::unordered_map<std::string, BindingPtr>;

    void handle_client(udp::IUdpSession& client_session, std::atomic<bool>& cancelled);

    ConstBuffer read_udp_message(udp::IUdpSession& client_session, Endpoint& udp_sender);
    BindingPtr& get_or_create_binding(const std::string& source_key);
    void send_to_tcp_server(const UdpTcpBinding& binding, ConstBuffer data, const Endpoint& udp_sender);
    ConstBuffer read_from_tcp_server(const UdpTcpBinding& binding, const Endpoint& udp_sender);
    void send_response_to_udp(udp::IUdpSession& client_session, ConstBuffer response, const Endpoint& udp_sender);

    void check_cancellation(std::atomic<bool>& cancelled);
    BindingPtr& create_binding(const std::string& source_key);

    logger::ILogger& logger_;
    const Endpoint local_endpoint_;
    const Endpoint tcp_server_endpoint_;
    BindingMap bindings_;
    OwnedBuffer buffer_{2048}; // larger than MTU
}; 