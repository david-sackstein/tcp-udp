#include "TcpClientProxy.h"

#include <background/BackgroundRunner.h>
#include <libacetools/ACEInitializer.h>
#include <libtcpclientproxy/Exports.h>

namespace client_proxy {

static std::vector<std::string> ip_addresses = ACEInitializer::init();

EXPORTED const std::vector<std::string>& get_local_ipv4_addresses() {
    return ip_addresses;
}

EXPORTED std::unique_ptr<IBlockingServer> create_tcp_client_proxy(logger::ILogger& logger,
    const Endpoint& local_endpoint,
    const Endpoint& tcp_server) {
    return std::make_unique<TcpClientProxy>(logger, local_endpoint, tcp_server);
}

EXPORTED std::unique_ptr<IBackgroundServer> start_tcp_client_proxy(logger::ILogger& logger,
    const Endpoint& local_endpoint,
    const Endpoint& tcp_server) {
    return startServer([&] { return create_tcp_client_proxy(logger, local_endpoint, tcp_server); },
        [](auto& proxy) { proxy.start(); });
}

} // namespace client_proxy
