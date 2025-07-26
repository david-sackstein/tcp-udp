#pragma once

#include <common/Endpoint.h>
#include <common/OwnedBuffer.h>
#include <libtcp/server/ITcpClientHandler.h>
#include <libudp/client/IUdpClient.h>
#include <liblogger/ILogger.h>

#include <functional>

class TcpServerProxyClientHandler : public tcp::ITcpClientHandler {
public:
    TcpServerProxyClientHandler(
        logger::ILogger& logger,
        Endpoint udp_proxy,
        udp::IUdpClient &udp_client
    );

    std::unique_ptr<ITask> handle_client(std::unique_ptr<tcp::ITcpSession> client_session) override;

private:
    std::function<void(std::atomic<bool>&)> make_work(const std::shared_ptr<tcp::ITcpSession>& shared_session);

    // Extracted methods for each operation
    ConstBuffer read_from_tcp_client(const std::shared_ptr<tcp::ITcpSession>& tcp_session, const Endpoint& tcp_client);
    void send_to_udp_proxy(ConstBuffer data, const Endpoint& tcp_client);
    ConstBuffer receive_from_udp_proxy(const Endpoint& tcp_client);
    void send_response_to_tcp_client(const std::shared_ptr<tcp::ITcpSession>& tcp_session, ConstBuffer response, const Endpoint& tcp_client);
    
    // Cancellation check method
    static void check_cancellation(std::atomic<bool>& cancelled);

    logger::ILogger& logger_;
    const Endpoint udp_proxy_;
    udp::IUdpClient &udp_client_;
    
    // Member buffers to avoid lifetime issues
    OwnedBuffer tcp_buffer_{2048};
    OwnedBuffer udp_buffer_{2048};
}; 