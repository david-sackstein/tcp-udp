#include "BindingFactory.h"

#include <libtcp/Exports.h>

BindingFactory::BindingFactory(logger::ILogger& logger, Endpoint tcp_server_endpoint, std::shared_ptr<ACE_Reactor> reactor)
    : logger_(logger),
      tcp_server_endpoint_(std::move(tcp_server_endpoint)),
      reactor_(std::move(reactor)) {}

std::unique_ptr<UdpTcpBinding> BindingFactory::createBinding(
    const std::string& source_key,
    udp::IUdpSession& udp_session,
    const Endpoint& udp_sender) const {
    
    (void)source_key; // Suppress unused parameter warning
    
    auto client = std::unique_ptr(tcp::create_tcp_client(logger_));
    auto session = client->connect(Endpoint::any_loop_back(), tcp_server_endpoint_);

    if (!session) {
        throw std::runtime_error("Failed to connect to TCP server");
    }

    auto binding = std::make_unique<UdpTcpBinding>(std::move(client), session, udp_session, udp_sender);
    binding->register_with_reactor(reactor_.get());
    
    return binding;
}

std::shared_ptr<tcp::ITcpSession> BindingFactory::createTcpConnection() const {
    auto client = std::unique_ptr(tcp::create_tcp_client(logger_));
    return client->connect(Endpoint::any_loop_back(), tcp_server_endpoint_);
} 