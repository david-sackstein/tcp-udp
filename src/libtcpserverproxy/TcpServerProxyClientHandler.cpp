#include "TcpServerProxyClientHandler.h"
#include "TcpServerProxyUdpHandler.h"

#include <common/OwnedBuffer.h>
#include <common/StringUtils.h>
#include <common/task/RunningTask.h>

#include <memory>
#include <stdexcept>

TcpServerProxyClientHandler::TcpServerProxyClientHandler(
    logger::ILogger& logger,
    Endpoint udp_proxy,
    udp::IUdpClient &udp_client,
    TcpServerProxyUdpHandler* udp_handler)
    : logger_(logger),
      udp_proxy_(std::move(udp_proxy)),
      udp_client_(udp_client),
      udp_handler_(udp_handler) {}

std::unique_ptr<ITask> TcpServerProxyClientHandler::handle_client(std::unique_ptr<tcp::ITcpSession> client_session) {
    // Convert to shared_ptr for safe sharing
    std::shared_ptr<tcp::ITcpSession> shared_session = std::move(client_session);
    Endpoint tcp_client = shared_session->get_peer();
    std::string client_key = tcp_client.to_string();

    logger_.log("TcpServerProxyClientHandler: New TCP client connected: %s", tcp_client.to_string().c_str());

    // Register this TCP session with the UDP handler so responses can be routed back
    udp_handler_->register_tcp_session(client_key, shared_session);

    // Create a background task to handle this client's TCP messages
    return std::make_unique<RunningTask>([shared_session, this, client_key](std::atomic<bool>& cancelled) {
        try {
            while (!cancelled) {
                // Read from TCP client
                IOResult result = shared_session->read(tcp_buffer_.view(), std::chrono::milliseconds(100));
                
                if (result.code == IOResultCode::Error || result.code == IOResultCode::ConnectionClosed) {
                    logger_.log("TcpServerProxyClientHandler: Client %s disconnected", client_key.c_str());
                    break;
                }
                
                if (result.code == IOResultCode::Timeout) {
                    continue; // No data, keep waiting
                }
                
                if (result.count > 0) {
                    // Forward TCP data to UDP proxy immediately
                    ConstBuffer tcp_data{tcp_buffer_.view().data, result.count};
                    if (!udp_client_.send_to(udp_proxy_, tcp_data)) {
                        logger_.log("TcpServerProxyClientHandler: Failed to send to UDP proxy for %s", client_key.c_str());
                    } else {
                        logger_.log("TcpServerProxyClientHandler: Forwarded TCP data from %s to UDP proxy", client_key.c_str());
                    }
                }
            }
        } catch (std::runtime_error& e) {
            logger_.log("TcpServerProxyClientHandler: Exception for client %s: %s", client_key.c_str(), e.what());
        }
        
        // Unregister when done
        udp_handler_->unregister_tcp_session(client_key);
        logger_.log("TcpServerProxyClientHandler: Cleaned up client %s", client_key.c_str());
    });
}