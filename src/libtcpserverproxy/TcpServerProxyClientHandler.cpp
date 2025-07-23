#include "TcpServerProxyClientHandler.h"

#include <common/OwnedBuffer.h>
#include <common/task/RunningTask.h>
#include <common/task/CompletedTask.h>

#include <memory>
#include <stdexcept>

std::chrono::milliseconds block = std::chrono::milliseconds(100);

TcpServerProxyClientHandler::TcpServerProxyClientHandler(
    Endpoint udp_proxy,
    udp::IUdpClient &udp_client)
    : udp_proxy_(std::move(udp_proxy)),
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
                    printf("TcpServerProxyClientHandler: client disconnected, exiting\n");
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
                    printf("TcpServerProxyClientHandler: no response from UDP proxy, exiting\n");
                    break;
                }
                
                check_cancellation(cancelled);
                
                // Send response to TCP client
                send_response_to_tcp_client(shared_session, response_from_udp, tcp_client);
                
                // Continue to next request-response cycle
            }
        } catch (std::runtime_error& e) {
            printf("TcpServerProxyClientHandler: %s\n", e.what());
            return;
        }
    };
}

ConstBuffer TcpServerProxyClientHandler::read_from_tcp_client(const std::shared_ptr<tcp::ITcpSession>& tcp_session, const Endpoint& tcp_client) {
    IOResult result = tcp_session->read(tcp_buffer_.view(), block);
    
    if (result.code == IOResultCode::Success) {
        if (result.count > 0) {
            return tcp_buffer_.view(result.count);
        } else {
            // No data received
            return ConstBuffer(nullptr, 0);
        }
    } else {
        std::string msg = "TcpServerProxyClientHandler: " + tcp_client.to_string() + " failed to read from " + tcp_client.to_string() + ": " + result.error_message;
        printf("%s\n", msg.c_str());
        throw std::runtime_error("Failed to read from TCP client: " + result.error_message);
    }
}

void TcpServerProxyClientHandler::send_to_udp_proxy(ConstBuffer data, const Endpoint& tcp_client) {
    if (!udp_client_.send_to(udp_proxy_, data)) {
        std::string msg = "TcpServerProxyClientHandler: " + tcp_client.to_string() + " failed to send to UDP proxy at " + udp_proxy_.to_string();
        printf("%s\n", msg.c_str());
        throw std::runtime_error("Failed to send to UDP proxy: unknown error");
    }
}

ConstBuffer TcpServerProxyClientHandler::receive_from_udp_proxy(const Endpoint& tcp_client) {
    Endpoint sender;
    auto received = udp_client_.receive_from(udp_buffer_.view(), sender);
    
    if (received == -1) {
        std::string msg = "TcpServerProxyClientHandler: " + tcp_client.to_string() + " failed to receive from UDP proxy at " + udp_proxy_.to_string();
        printf("%s\n", msg.c_str());
        throw std::runtime_error("Failed to receive from UDP proxy: unknown error");
    }
    
    return udp_buffer_.view(received);
}

void TcpServerProxyClientHandler::send_response_to_tcp_client(
    const std::shared_ptr<tcp::ITcpSession>& tcp_session, ConstBuffer response, const Endpoint& tcp_client) {

    IOResult result = tcp_session->write(response, block);
    
    if (result.code != IOResultCode::Success) {
        std::string msg = "TcpServerProxyClientHandler: " + tcp_client.to_string() + " failed to send response to " + tcp_client.to_string() + ": " + result.error_message;
        printf("%s\n", msg.c_str());
        throw std::runtime_error("Failed to send response to TCP client: " + result.error_message);
    }
}

void TcpServerProxyClientHandler::check_cancellation(std::atomic<bool>& cancelled) {
    if (cancelled) {
        throw std::runtime_error("TcpServerProxyClientHandler was cancelled");
    }
}