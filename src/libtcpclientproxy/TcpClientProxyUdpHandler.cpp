#include "UdpTcpBinding.h"
#include "TcpClientProxyUdpHandler.h"

#include <common/OwnedBuffer.h>
#include <common/task/RunningTask.h>
#include <libtcp/Exports.h>

#include <stdexcept>

std::chrono::milliseconds block = std::chrono::milliseconds(1000);

TcpClientProxyUdpHandler::TcpClientProxyUdpHandler(Endpoint local_endpoint, Endpoint tcp_server)
    :
    local_endpoint_{std::move(local_endpoint)},
    tcp_server_endpoint_{std::move(tcp_server)} {}

std::unique_ptr<ITask> TcpClientProxyUdpHandler::handle_client(udp::IUdpSession& client_session) {
    return std::make_unique<RunningTask>([&client_session, this](std::atomic<bool>& cancelled) {
        try{
            handle_client(client_session, cancelled);
        } catch (std::runtime_error& e) {
            printf("%s", e.what());
            return;
        }
    });
}

void TcpClientProxyUdpHandler::handle_client(udp::IUdpSession& client_session, std::atomic<bool>& cancelled) {

    // Received a message over udp
    Endpoint udp_sender;

    // Read the udp message
    ConstBuffer received_from_udp = read_udp_message(client_session, udp_sender);

    std::string source_key = udp_sender.to_string();

    check_cancellation(cancelled);

    // Find or create the connection to the tcp server
    BindingPtr& binding = get_or_create_binding(source_key);

    check_cancellation(cancelled);

    // Send to the tcp server
    send_to_tcp_server(*binding, received_from_udp, udp_sender);

    check_cancellation(cancelled);

    // Read response from the tcp server
    ConstBuffer read_from_tcp = read_from_tcp_server(*binding, udp_sender);

    check_cancellation(cancelled);

    // Send response to the udp sender
    send_response_to_udp(client_session, read_from_tcp, udp_sender);
}

TcpClientProxyUdpHandler::BindingPtr&
TcpClientProxyUdpHandler::get_or_create_binding(const std::string &source_key) {
    auto it = bindings_.find(source_key);
    if (it != bindings_.end()) {
        return it->second;
    }
    return create_binding(source_key);
}

TcpClientProxyUdpHandler::BindingPtr& TcpClientProxyUdpHandler::create_binding(const std::string &source_key) {
    auto client = std::unique_ptr(tcp::create_tcp_client());
    auto session = client->connect(local_endpoint_, tcp_server_endpoint_);
    if (!session) {
        throw std::runtime_error("Failed to connect to TCP server");
    }

    auto binding = std::make_unique<UdpTcpBinding>(UdpTcpBinding{std::move(client), session});
    auto [it, inserted] = bindings_.emplace(source_key, std::move(binding));
    return it->second;
}

ConstBuffer TcpClientProxyUdpHandler::read_udp_message(
    udp::IUdpSession& client_session, Endpoint& udp_sender) {

    IOResult udp_read_result = client_session.read_from(buffer_.view(), udp_sender, block);

    if (udp_read_result.code == IOResultCode::Error) {
        printf("TcpClientProxyUdpHandler: %s failed to read UDP: %s\n", udp_sender.to_string().c_str(), udp_read_result.error_message.c_str());
        throw std::runtime_error("Failed to read UDP message: " + udp_read_result.error_message);
    }

    if (udp_read_result.code == IOResultCode::ConnectionClosed || udp_read_result.code == IOResultCode::Timeout) {
        return {};
    }

    return buffer_.view(udp_read_result.count); // Success case
}

void TcpClientProxyUdpHandler::send_to_tcp_server(const UdpTcpBinding& binding, const ConstBuffer data, const Endpoint& udp_sender) {
    auto tcp_session = binding.tcp_session;
    auto tcp_send_result = tcp_session->write(data, block);
    
    if (tcp_send_result.code == IOResultCode::Error) {
        printf("TcpClientProxyUdpHandler: %s failed to send to TCP server: %s\n", udp_sender.to_string().c_str(), tcp_send_result.error_message.c_str());
        throw std::runtime_error("Failed to send to TCP server: " + tcp_send_result.error_message);
    }
    
    if (tcp_send_result.code == IOResultCode::ConnectionClosed || tcp_send_result.code == IOResultCode::Timeout) {
        return;
    }


ConstBuffer TcpClientProxyUdpHandler::read_from_tcp_server(const UdpTcpBinding& binding, const Endpoint& udp_sender) {
    auto tcp_session = binding.tcp_session;
    auto tcp_read_result = tcp_session->read(buffer_.view(), block);
    
    if (tcp_read_result.code == IOResultCode::Error) {
        printf("TcpClientProxyUdpHandler: %s failed to read from TCP server: %s\n", udp_sender.to_string().c_str(), tcp_read_result.error_message.c_str());
        throw std::runtime_error("Failed to read from TCP server: " + tcp_read_result.error_message);
    }
    
    if (tcp_read_result.code == IOResultCode::ConnectionClosed || tcp_read_result.code == IOResultCode::Timeout) {
        return {};
    }
    
    return buffer_.view(tcp_read_result.count);
}

void TcpClientProxyUdpHandler::send_response_to_udp(
    udp::IUdpSession& client_session, ConstBuffer response, const Endpoint& udp_sender) {

    IOResult udp_write_result = client_session.write_to(response, udp_sender, block);
    
    if (udp_write_result.code == IOResultCode::Error) {
        printf("TcpClientProxyUdpHandler: %s failed to send UDP response: %s\n", udp_sender.to_string().c_str(), udp_write_result.error_message.c_str());
        throw std::runtime_error("Failed to send response to UDP client: " + udp_write_result.error_message);
    }

    if (udp_write_result.code == IOResultCode::ConnectionClosed || udp_write_result.code == IOResultCode::Timeout) {
        return; // Graceful cleanup
    }
}

void TcpClientProxyUdpHandler::check_cancellation(std::atomic<bool>& cancelled) {
    if (cancelled) {
        throw std::runtime_error("TcpClientProxyUdpHandler was cancelled");
    }
}

