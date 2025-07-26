#include "TcpServerProxy.h"

#include <libtcpserverproxy/Exports.h>
#include <libtcp/Exports.h>
#include <libudp/Exports.h>

#include <utility>

TcpServerProxy::TcpServerProxy(logger::ILogger &logger, Endpoint local_endpoint, Endpoint udp_proxy)
    : local_endpoint_(std::move(local_endpoint)),
      udp_proxy_(std::move(udp_proxy)),
      logger_(logger) {
    // Create UDP client on any available port (ephemeral)
    udp_client_ = udp::create_udp_client(logger_, Endpoint::any_loop_back());
    handler_ = std::make_unique<TcpServerProxyClientHandler>(
        logger_,
        udp_proxy_,
        *udp_client_);
}

const Endpoint &TcpServerProxy::get_local_endpoint() const {
    return local_endpoint_;
}

void TcpServerProxy::start() {
    tcp_server_ = create_tcp_server(logger_, local_endpoint_, *handler_);
    tcp_server_->start();
}

void TcpServerProxy::stop() {
    tcp_server_->stop();
}
