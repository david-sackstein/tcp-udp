#include <libtcpserverproxy/Exports.h>
#include <libargsparser/Exports.h>
#include <libtcpserverproxy/Exports.h>
#include <liblogger/Exports.h>
#include <iostream>
#include <vector>

int main(int argc, char* argv[]) {
    auto logger = logger::create_console_logger();
    
    const auto& local_addresses = server_proxy::get_local_ipv4_addresses();
    
    auto endpoint_strings = get_endpoint_configuration(argc, argv, local_addresses);

    Endpoint listen_ep = Endpoint::from_string(endpoint_strings.first);
    Endpoint forward_ep = Endpoint::from_string(endpoint_strings.second);

    logger->log("TCP Server Proxy running on %s", listen_ep.to_string().c_str());
    logger->log("Forwarding connections to UDP proxy at %s", forward_ep.to_string().c_str());
    logger->log("Press 'q' and Enter to stop the server...");

    auto server = server_proxy::create_tcp_server_proxy(*logger, listen_ep, forward_ep);

    server->start();
    server->stop();

    logger->log("tcpserverproxy shut down");

    return 0;
}
