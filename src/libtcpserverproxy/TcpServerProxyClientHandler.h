#pragma once

#include <common/Endpoint.h>
#include <libtcp/server/ITcpClientHandler.h>
#include <libudp/client/IUdpClient.h>

#include <functional>

class TcpServerProxyClientHandler : public tcp::ITcpClientHandler {
public:
    TcpServerProxyClientHandler(
        Endpoint udp_proxy,
        udp::IUdpClient &udp_client
    );

    std::unique_ptr<ITask> handle_client(std::unique_ptr<tcp::ITcpSession> client_session) override;

private:
    std::function<void(std::atomic<bool>&)> make_work(const std::shared_ptr<tcp::ITcpSession>& shared_session);

    // Extracted methods for each operation
    static ConstBuffer read_from_tcp_client(const std::shared_ptr<tcp::ITcpSession>& tcp_session, const Endpoint& tcp_client);
    void send_to_udp_proxy(ConstBuffer data, const Endpoint& tcp_client);
    ConstBuffer receive_from_udp_proxy(const Endpoint& tcp_client);
    static void send_response_to_tcp_client(const std::shared_ptr<tcp::ITcpSession>& tcp_session, ConstBuffer response, const Endpoint& tcp_client);
    
    // Cancellation check method
    static void check_cancellation(std::atomic<bool>& cancelled);

    const Endpoint udp_proxy_;
    udp::IUdpClient &udp_client_;
}; 