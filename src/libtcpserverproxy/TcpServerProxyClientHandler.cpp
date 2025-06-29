#include "TcpServerProxyClientHandler.h"

#include <common/OwnedBuffer.h>
#include <common/task/RunningTask.h>
#include <common/task/CompletedTask.h>

#include <memory>
#include <stdexcept>

std::chrono::milliseconds block = std::chrono::milliseconds(200);

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
                ConstBuffer received_from_tcp = read_from_tcp_client(shared_session, tcp_client);
                
                check_cancellation(cancelled);
                
                // Send to UDP proxy
                send_to_udp_proxy(received_from_tcp, tcp_client);
                
                check_cancellation(cancelled);
                
                // Receive from UDP proxy
                ConstBuffer response_from_udp = receive_from_udp_proxy(tcp_client);
                
                check_cancellation(cancelled);
                
                // Send response to TCP client
                send_response_to_tcp_client(shared_session, response_from_udp, tcp_client);
            }
        } catch (std::runtime_error& e) {
            printf("TcpServerProxyClientHandler: %s\n", e.what());
            return;
        }
    };
}

ConstBuffer TcpServerProxyClientHandler::read_from_tcp_client(const std::shared_ptr<tcp::ITcpSession>& tcp_session, const Endpoint& tcp_client) {
    OwnedBuffer tcp_buf(1024);
    IOResult result = tcp_session->read(tcp_buf.view(), block);
    
    if (result.code != IOResultCode::Success) {
        printf("TcpServerProxyClientHandler: %s failed to read from %s\n",
               tcp_client.to_string().c_str(), tcp_client.to_string().c_str());
        throw std::runtime_error("Failed to read from TCP client");
    }
    
    return tcp_buf.view(result.count);
}

void TcpServerProxyClientHandler::send_to_udp_proxy(ConstBuffer data, const Endpoint& tcp_client) {
    if (!udp_client_.send_to(udp_proxy_, data)) {
        printf("TcpServerProxyClientHandler: %s failed to send to UDP proxy at %s\n",
               tcp_client.to_string().c_str(), udp_proxy_.to_string().c_str());
        throw std::runtime_error("Failed to send to UDP proxy");
    }
}

ConstBuffer TcpServerProxyClientHandler::receive_from_udp_proxy(const Endpoint& tcp_client) {
    OwnedBuffer udp_buf(1024);
    Endpoint sender;
    auto received = udp_client_.receive_from(udp_buf.view(), sender);
    
    if (received == -1) {
        printf("TcpServerProxyClientHandler: %s failed to receive from UDP proxy at %s\n",
               tcp_client.to_string().c_str(), udp_proxy_.to_string().c_str());
        throw std::runtime_error("Failed to receive from UDP proxy");
    }
    
    return udp_buf.view(received);
}

void TcpServerProxyClientHandler::send_response_to_tcp_client(
    const std::shared_ptr<tcp::ITcpSession>& tcp_session, ConstBuffer response, const Endpoint& tcp_client) {

    IOResult result = tcp_session->write(response, block);
    
    if (result.code != IOResultCode::Success) {
        printf("TcpServerProxyClientHandler: %s failed to send response to %s\n",
               tcp_client.to_string().c_str(), tcp_client.to_string().c_str());
        throw std::runtime_error("Failed to send response to TCP client");
    }
}

void TcpServerProxyClientHandler::check_cancellation(std::atomic<bool>& cancelled) {
    if (cancelled) {
        throw std::runtime_error("TcpServerProxyClientHandler was cancelled");
    }
}