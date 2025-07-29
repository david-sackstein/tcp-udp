#pragma once

#include <common/Endpoint.h>
#include <common/OwnedBuffer.h>
#include <libacetools/IOResult.h>
#include <liblogger/ILogger.h>
#include <libtcp/ITcpSession.h>
#include <libudp/client/IUdpClient.h>

#include <atomic>
#include <memory>

class UdpForwarder final {
public:
    UdpForwarder(
        logger::ILogger& logger,
        Endpoint udp_proxy,
        udp::IUdpClient& udp_client);

    void forwardTcpToUdp(
        tcp::ITcpSession& tcp_session,
        const std::string& client_key,
        std::atomic<bool>& cancelled);

private:
    void forwardToUdpProxy(const IOResult& tcp_data, const std::string& client_key);

    logger::ILogger& logger_;
    const Endpoint udp_proxy_;
    udp::IUdpClient& udp_client_;
    OwnedBuffer tcp_buffer_{2048};
    static constexpr std::chrono::milliseconds timeout_ms{100};
}; 