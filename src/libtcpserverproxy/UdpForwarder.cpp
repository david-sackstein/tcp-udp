#include "UdpForwarder.h"

#include <common/Buffer.h>

UdpForwarder::UdpForwarder(
    logger::ILogger& logger,
    Endpoint udp_proxy,
    udp::IUdpClient& udp_client)
    : logger_(logger),
      udp_proxy_(std::move(udp_proxy)),
      udp_client_(udp_client) {}

void UdpForwarder::forwardTcpToUdp(
    tcp::ITcpSession& tcp_session,
    const std::string& client_key,
    std::atomic<bool>& cancelled) {
    
    while (!cancelled) {
        IOResult result = tcp_session.read(tcp_buffer_.view(), timeout_ms);
        
        if (result.code == IOResultCode::Error || result.code == IOResultCode::ConnectionClosed) {
            logger_.log(logger::LogLevel::INFO, "UdpForwarder: Client %s disconnected", client_key.c_str());
            break;
        }
        
        if (result.code == IOResultCode::Timeout) {
            continue;
        }
        
        if (result.count > 0) {
            forwardToUdpProxy(result, client_key);
        }
    }
}

void UdpForwarder::forwardToUdpProxy(const IOResult& tcp_data, const std::string& client_key) {
    ConstBuffer data_to_forward{tcp_buffer_.view().data, tcp_data.count};
    
    if (!udp_client_.send_to(udp_proxy_, data_to_forward)) {
        logger_.log(logger::LogLevel::ERROR, "UdpForwarder: Failed to send to UDP proxy for %s", client_key.c_str());
    } else {
        logger_.log(logger::LogLevel::INFO, "UdpForwarder: Forwarded TCP data from %s to UDP proxy", client_key.c_str());
    }
} 