#include "TcpServerProxy.h"

#include <libtcp/Exports.h>
#include <libtcp/InternalExports.h>
#include <libtcpserverproxy/Exports.h>
#include <libudp/Exports.h>
#include <libudp/InternalExports.h>
#include <libudp/client/AceUdpClient.h>

#include <utility>

TcpServerProxy::TcpServerProxy(logger::ILogger& logger, Endpoint local_endpoint, Endpoint udp_proxy)
    : local_endpoint_(std::move(local_endpoint)),
      udp_proxy_(std::move(udp_proxy)),
      reactor_(std::make_shared<ACE_Reactor>()),
      logger_(logger) {
    // Create UDP client on any available port (ephemeral) for sending to client proxy
    udp_client_ = udp::create_udp_client(logger_, Endpoint::any_loop_back());

    // Create UDP handler for receiving responses from client proxy
    udp_handler_ = std::make_unique<ServerProxyUdpHandler>(logger_);

    // Create TCP handler for accepting client connections
    tcp_handler_ = std::make_unique<ServerProxyClientHandler>(logger_, udp_proxy_, *udp_client_, udp_handler_.get());
}

const Endpoint& TcpServerProxy::get_local_endpoint() const {
    return local_endpoint_;
}

void TcpServerProxy::start() {
    // Register UDP client socket with reactor for receiving responses
    if (reactor_->register_handler(this, ACE_Event_Handler::READ_MASK) == 0) {
        reactor_registered_ = true;
        logger_.log(logger::LogLevel::INFO, "TcpServerProxy: Registered UDP client socket with reactor");
    } else {
        logger_.log(logger::LogLevel::ERROR, "TcpServerProxy: Failed to register UDP client socket with reactor");
    }

    // Create TCP server with shared reactor
    tcp_server_ = tcp::create_tcp_server(logger_, local_endpoint_, *tcp_handler_, reactor_);
    tcp_server_->start();
}

void TcpServerProxy::stop() {
    if (reactor_registered_ && reactor_) {
        reactor_->remove_handler(this, ACE_Event_Handler::READ_MASK);
        reactor_registered_ = false;
    }

    if (tcp_server_) {
        tcp_server_->stop();
    }
}

int TcpServerProxy::handle_input(ACE_HANDLE) {
    // Handle incoming UDP response from client proxy
    return udp_handler_->handle_udp_input(*udp_client_);
}

ACE_HANDLE TcpServerProxy::get_handle() const {
    // Cast to AceUdpClient to get socket handle
    auto* ace_client = static_cast<const AceUdpClient*>(udp_client_.get());
    return ace_client->get_socket();
}
