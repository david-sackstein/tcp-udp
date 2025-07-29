#include "TcpClientProxyUdpHandler.h"

#include <common/Constants.h>
#include <common/StringUtils.h>
#include <libacetools/IOResult.h>

TcpClientProxyUdpHandler::TcpClientProxyUdpHandler(
    logger::ILogger& logger, Endpoint local_endpoint, Endpoint tcp_server, std::shared_ptr<ACE_Reactor> reactor)
    : logger_(logger),
      local_endpoint_(std::move(local_endpoint)),
      udp_session_handler_(std::make_unique<UdpSessionHandler>(logger)),
      binding_manager_(std::make_unique<TcpBindingManager>(logger)) {
    
    auto factory = std::make_unique<BindingFactory>(logger, std::move(tcp_server), std::move(reactor));
    binding_manager_->setBindingFactory(std::move(factory));
}

std::unique_ptr<ITask> TcpClientProxyUdpHandler::handle_client(udp::IUdpSession& client_session) {
    try {
        Endpoint udp_sender;
        ConstBuffer received_from_udp = udp_session_handler_->readUdpMessage(client_session, udp_sender);
        std::string source_key = udp_sender.to_string();

        auto& binding = binding_manager_->getOrCreateBinding(source_key, client_session, udp_sender);
        sendToTcpServer(*binding, received_from_udp, udp_sender);

        return nullptr;
    } catch (std::runtime_error& e) {
        logger_.log(logger::LogLevel::ERROR, "TcpClientProxyUdpHandler: Exception: %s", e.what());
        return nullptr;
    }
}

void TcpClientProxyUdpHandler::sendToTcpServer(const UdpTcpBinding& binding, ConstBuffer data, const Endpoint& udp_sender) const {
    auto& tcp_session = binding.get_tcp_session();
    auto tcp_send_result = tcp_session.write(data, common::STANDARD_TIMEOUT);
    
    if (tcp_send_result.code == IOResultCode::Error) {
        throw std::runtime_error(format_string(
            "TcpClientProxyUdpHandler: %s failed to send to TCP server: %s",
            udp_sender.to_string().c_str(),
            tcp_send_result.error_message.c_str()));
    }
    
    if (tcp_send_result.code == IOResultCode::ConnectionClosed) {
        logger_.log(logger::LogLevel::INFO, "TcpClientProxyUdpHandler: %s write TCP ConnectionClosed", udp_sender.to_string().c_str());
        return;
    }
    
    if (tcp_send_result.code == IOResultCode::Timeout) {
        return;
    }
}



