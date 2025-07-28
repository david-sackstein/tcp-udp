#include "UdpTcpBinding.h"
#include "TcpClientProxyUdpHandler.h"

#include <common/OwnedBuffer.h>
#include <common/task/RunningTask.h>
#include <common/StringUtils.h>
#include <libtcp/Exports.h>

#include <stdexcept>

std::chrono::milliseconds block = std::chrono::milliseconds(1000);

TcpClientProxyUdpHandler::TcpClientProxyUdpHandler(
    logger::ILogger& logger, Endpoint local_endpoint, Endpoint tcp_server, std::shared_ptr<ACE_Reactor> reactor)
    : logger_(logger),
      local_endpoint_{std::move(local_endpoint)},
      tcp_server_endpoint_{std::move(tcp_server)},
      reactor_(std::move(reactor)) {}

std::unique_ptr<ITask> TcpClientProxyUdpHandler::handle_client(udp::IUdpSession& client_session) {
    try {
        Endpoint udp_sender;
        ConstBuffer received_from_udp = read_udp_message(client_session, udp_sender);
        std::string source_key = udp_sender.to_string();

        BindingPtr& binding = get_or_create_binding(source_key, client_session, udp_sender);
        send_to_tcp_server(*binding, received_from_udp, udp_sender);

        return nullptr;
    } catch (std::runtime_error& e) {
        logger_.log("TcpClientProxyUdpHandler: Exception: %s", e.what());
        return nullptr;
    }
}

TcpClientProxyUdpHandler::BindingPtr&
TcpClientProxyUdpHandler::get_or_create_binding(const std::string &source_key, udp::IUdpSession& udp_session, const Endpoint& udp_sender) {
    auto it = bindings_.find(source_key);
    if (it != bindings_.end()) {
        return it->second;
    }
    return create_binding(source_key, udp_session, udp_sender);
}

TcpClientProxyUdpHandler::BindingPtr& TcpClientProxyUdpHandler::create_binding(const std::string &source_key, udp::IUdpSession& udp_session, const Endpoint& udp_sender) {
    auto client = std::unique_ptr(tcp::create_tcp_client(logger_));
    auto session = client->connect(local_endpoint_, tcp_server_endpoint_);

    if (!session) {
        throw std::runtime_error("Failed to connect to TCP server");
    }

    auto binding = std::make_unique<UdpTcpBinding>(std::move(client), session, udp_session, udp_sender);
    binding->register_with_reactor(reactor_.get());
    
    auto [it, inserted] = bindings_.emplace(source_key, std::move(binding));
    if (!inserted) {
        throw std::runtime_error("Failed to store binding");
    }
    
    return it->second;
}

ConstBuffer TcpClientProxyUdpHandler::read_udp_message(
    udp::IUdpSession& client_session, Endpoint& udp_sender) {

    IOResult udp_read_result = client_session.read_from(buffer_.view(), udp_sender, block);

    if (udp_read_result.code == IOResultCode::Error) {
        throw std::runtime_error(format_string(
            "TcpClientProxyUdpHandler: %s failed to read UDP: %s",
            udp_sender.to_string().c_str(),
            udp_read_result.error_message.c_str()));
    }

    if (udp_read_result.code == IOResultCode::ConnectionClosed) {
        logger_.log("TcpClientProxyUdpHandler: %s read UDP ConnectionClosed", udp_sender.to_string().c_str());
        return {};
    }

    if (udp_read_result.code == IOResultCode::Timeout) {
        return {};
    }

    return buffer_.view(udp_read_result.count);
}

void TcpClientProxyUdpHandler::send_to_tcp_server(
    const UdpTcpBinding& binding, ConstBuffer data, const Endpoint& udp_sender) const {
    auto& tcp_session = binding.get_tcp_session();
    auto tcp_send_result = tcp_session.write(data, block);
    
    if (tcp_send_result.code == IOResultCode::Error) {
        throw std::runtime_error(format_string(
            "TcpClientProxyUdpHandler: %s failed to send to TCP server: %s",
            udp_sender.to_string().c_str(),
            tcp_send_result.error_message.c_str()));
    }
    
    if (tcp_send_result.code == IOResultCode::ConnectionClosed) {
        logger_.log("TcpClientProxyUdpHandler: %s write TCP ConnectionClosed", udp_sender.to_string().c_str());
        return;
    }
    
    if (tcp_send_result.code == IOResultCode::Timeout) {
        return;
    }
}

void TcpClientProxyUdpHandler::send_response_to_udp(
    udp::IUdpSession& client_session, ConstBuffer response, const Endpoint& udp_sender) const {

    IOResult udp_write_result = client_session.write_to(response, udp_sender, block);
    
    if (udp_write_result.code == IOResultCode::Error) {
        throw std::runtime_error(format_string(
            "TcpClientProxyUdpHandler: %s failed to send UDP response: %s",
            udp_sender.to_string().c_str(),
            udp_write_result.error_message.c_str()));
    }
    
    if (udp_write_result.code == IOResultCode::ConnectionClosed) {
        logger_.log("TcpClientProxyUdpHandler: %s write UDP ConnectionClosed", udp_sender.to_string().c_str());
        return;
    }
    
    if (udp_write_result.code == IOResultCode::Timeout) {
        return;
    }
}



