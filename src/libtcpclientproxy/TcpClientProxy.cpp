#include "TcpClientProxy.h"
#include "ClientProxyUdpHandler.h"

#include <libudp/InternalExports.h>

TcpClientProxy::TcpClientProxy(logger::ILogger& logger, const Endpoint& local_endpoint, Endpoint tcp_server)
    : local_endpoint_(local_endpoint),
      tcp_server_(std::move(tcp_server)),
      reactor_(std::make_shared<ACE_Reactor>()),
      logger_(logger) {
    udp_handler_ = std::make_unique<ClientProxyUdpHandler>(logger, local_endpoint, tcp_server_, reactor_);
    udp_server_ = udp::create_udp_server(logger_, local_endpoint, *udp_handler_, reactor_);
}

TcpClientProxy::~TcpClientProxy() {
    stop();
}

const Endpoint& TcpClientProxy::get_local_endpoint() const {
    return local_endpoint_;
}

void TcpClientProxy::start() {
    if (udp_server_) {
        udp_server_->start();
    }
}

void TcpClientProxy::stop() {
    if (udp_server_) {
        udp_server_->stop();
    }
}
