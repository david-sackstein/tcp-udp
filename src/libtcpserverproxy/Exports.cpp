#include "TcpServerProxy.h"

#include <background/BackgroundRunner.h>
#include <libacetools/ACEInitializer.h>
#include <libtcpserverproxy/Exports.h>
#include "background/BackgroundServer.h"


namespace server_proxy {
    static std::vector<std::string> ip_addresses = ACEInitializer::init();

    EXPORTED const std::vector<std::string>& get_local_ipv4_addresses(){
        return ip_addresses;
    }

    EXPORTED std::unique_ptr<IBlockingServer> create_tcp_server_proxy(const Endpoint& local_endpoint, const Endpoint &udp_proxy) {
        return std::make_unique<TcpServerProxy>(local_endpoint, udp_proxy);
    }

    EXPORTED std::unique_ptr<IBackgroundServer>
    start_tcp_server_proxy(const Endpoint& local_endpoint, const Endpoint &udp_proxy) {
        return startServer(
                [&] { return create_tcp_server_proxy(local_endpoint, udp_proxy); },
                [](auto &proxy) { proxy.start(); }
        );
    }
}