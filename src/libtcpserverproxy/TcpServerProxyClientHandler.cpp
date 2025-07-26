#include "TcpServerProxyClientHandler.h"

#include <common/OwnedBuffer.h>
#include <common/task/RunningTask.h>
#include <common/StringUtils.h>

#include <memory>
#include <stdexcept>

std::chrono::milliseconds block = std::chrono::milliseconds(100);

TcpServerProxyClientHandler::TcpServerProxyClientHandler(
    logger::ILogger& logger,
    Endpoint udp_proxy,
    udp::IUdpClient &udp_client)
    : logger_(logger),
      udp_proxy_(std::move(udp_proxy)),
      udp_client_(udp_client) {}

std::unique_ptr<ITask> TcpServerProxyClientHandler::handle_client(std::unique_ptr<tcp::ITcpSession> client_session) {
    return std::make_unique<RunningTask>(make_work(std::move(client_session)));
}

std::function<void(std::atomic<bool>&)> TcpServerProxyClientHandler::make_work(const std::shared_ptr<tcp::ITcpSession>& shared_session) {
    return [shared_session, this](std::atomic<bool>& cancelled) {
        try {
            while (!cancelled) {
                Endpoint tcp_client = shared_session->get_peer();
                
                // Read from TCP client
                auto received_from_tcp = read_from_tcp_client(shared_session, tcp_client);
                if (received_from_tcp.size == 0) {
                    // No data received, client likely disconnected
                    logger_.log("TcpServerProxyClientHandler: client disconnected, exiting");
                    break;
                }
                
                check_cancellation(cancelled);
                
                // Send to UDP proxy
                send_to_udp_proxy(received_from_tcp, tcp_client);
                
                check_cancellation(cancelled);
                
                // Receive from UDP proxy
                auto response_from_udp = receive_from_udp_proxy(tcp_client);
                if (response_from_udp.size == 0) {
                    // No response received, exit
                    logger_.log("TcpServerProxyClientHandler: no response from UDP proxy, exiting");
                    break;
                }
                
                check_cancellation(cancelled);
                
                // Send response to TCP client
                send_response_to_tcp_client(shared_session, response_from_udp, tcp_client);
                
                // Continue to next request-response cycle
            }
        } catch (std::runtime_error& e) {
            logger_.log("TcpServerProxyClientHandler: %s", e.what());
            return;
        }
    };
}

ConstBuffer TcpServerProxyClientHandler::read_from_tcp_client(const std::shared_ptr<tcp::ITcpSession>& tcp_session, const Endpoint& tcp_client) {
    IOResult result = tcp_session->read(tcp_buffer_.view(), block);
    
    if (result.code == IOResultCode::Error) {
        throw std::runtime_error(format_string("TcpServerProxyClientHandler: %s failed to read: %s", tcp_client.to_string().c_str(), result.error_message.c_str()));
    }
    
    if (result.code == IOResultCode::ConnectionClosed) {
        logger_.log("TcpServerProxyClientHandler: %s read ConnectionClosed", tcp_client.to_string().c_str());
        return {};
    }
    
    if (result.code == IOResultCode::Timeout) {
        return {};
    }
    
    if (result.count == 0) {
        return {};
    }
    
    return tcp_buffer_.view(result.count);
}

void TcpServerProxyClientHandler::send_to_udp_proxy(ConstBuffer data, const Endpoint& tcp_client) {
    if (!udp_client_.send_to(udp_proxy_, data)) {
        throw std::runtime_error(format_string("TcpServerProxyClientHandler: %s failed to send to UDP proxy at %s", tcp_client.to_string().c_str(), udp_proxy_.to_string().c_str()));
    }
}

ConstBuffer TcpServerProxyClientHandler::receive_from_udp_proxy(const Endpoint& tcp_client) {
    Endpoint sender;
    auto received = udp_client_.receive_from(udp_buffer_.view(), sender);
    
    if (received == -1) {
        throw std::runtime_error(format_string("TcpServerProxyClientHandler: %s failed to receive from UDP proxy at %s", tcp_client.to_string().c_str(), udp_proxy_.to_string().c_str()));
    }
    
    return udp_buffer_.view(received);
}

void TcpServerProxyClientHandler::send_response_to_tcp_client(
    const std::shared_ptr<tcp::ITcpSession>& tcp_session, ConstBuffer response, const Endpoint& tcp_client) {

    IOResult result = tcp_session->write(response, block);
    
    if (result.code == IOResultCode::Error) {
        throw std::runtime_error(format_string("TcpServerProxyClientHandler: %s failed to send response: %s", tcp_client.to_string().c_str(), result.error_message.c_str()));
    }
    
    if (result.code == IOResultCode::ConnectionClosed) {
        logger_.log("TcpServerProxyClientHandler: %s write ConnectionClosed", tcp_client.to_string().c_str());
        return;
    }
    
    if (result.code == IOResultCode::Timeout) {
        return;
    }
}

void TcpServerProxyClientHandler::check_cancellation(std::atomic<bool>& cancelled) {
    if (cancelled) {
        throw std::runtime_error("TcpServerProxyClientHandler was cancelled");
    }
}