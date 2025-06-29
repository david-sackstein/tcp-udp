#include <libtcpserverproxy/Exports.h>
#include <libargsparser/Exports.h>
#include <libtcpserverproxy/Exports.h>
#include <iostream>
#include <vector>

int main(int argc, char* argv[]) {
    const auto& local_addresses = server_proxy::get_local_ipv4_addresses();
    
    auto endpoint_strings = get_endpoint_configuration(argc, argv, local_addresses);

    Endpoint listen_ep = Endpoint::from_string(endpoint_strings.first);
    Endpoint forward_ep = Endpoint::from_string(endpoint_strings.second);

    std::cout << "TCP Server Proxy running on " << listen_ep.to_string() << std::endl;
    std::cout << "Forwarding connections to UDP proxy at " << forward_ep.to_string() << std::endl;
    std::cout << "Press 'q' and Enter to stop the server..." << std::endl;

    auto server = server_proxy::create_tcp_server_proxy(listen_ep, forward_ep);

    // Start the server in a blocking manner (ACE reactor handles signals internally)
    server->start();

    // Stop when ctrl-c aborts start
    server->stop();

    std::cout << "tcpserverproxy shut down" << std::endl;

    return 0;
}
