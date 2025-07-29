#pragma once

#include <common/Endpoint.h>
#include <common/OwnedBuffer.h>
#include <libtcp/server/ITcpClientHandler.h>
#include <libudp/client/IUdpClient.h>
#include <liblogger/ILogger.h>
#include <libacetools/IOResult.h>

#include <memory>
#include <atomic>

// Forward declaration
class TcpServerProxyUdpHandler;

class TcpServerProxyClientHandler : public tcp::ITcpClientHandler {
public:
    TcpServerProxyClientHandler(
        logger::ILogger& logger,
        Endpoint udp_proxy,
        udp::IUdpClient &udp_client,
        TcpServerProxyUdpHandler* udp_handler
    );

    std::unique_ptr<ITask> handle_client(std::unique_ptr<tcp::ITcpSession> client_session) override;

private:
    void handleTcpToUdpForwarding(
        tcp::ITcpSession& tcp_session, 
        const std::string& client_key, 
        std::atomic<bool>& cancelled);
    
    void forwardToUdpProxy(const IOResult& tcp_data, const std::string& client_key);

    logger::ILogger& logger_;
    const Endpoint udp_proxy_;
    udp::IUdpClient &udp_client_;
    TcpServerProxyUdpHandler* udp_handler_;
    OwnedBuffer tcp_buffer_{2048};
}; 