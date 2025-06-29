#include <libtcp/Exports.h>
#include <libargsparser/Exports.h>
#include <iostream>

int main(int argc, char* argv[]) {
    const auto& local_addresses = tcp::get_local_ipv4_addresses();
    
    std::string listen_endpoint_str = get_listen_endpoint(argc, argv, local_addresses);
    Endpoint listen_ep = Endpoint::from_string(listen_endpoint_str);

    std::unique_ptr<tcp::ITcpClientHandler> echo_handler = tcp::create_tcp_echo_handler();
    auto server = tcp::create_tcp_server(listen_ep, *echo_handler);

    std::cout << "Server running on " << server->get_local_endpoint().to_string() << std::endl;
    std::cout << "Press Ctrl+C to stop the server..." << std::endl;

    // Start the server in a blocking manner (ACE reactor handles signals internally)
    server->start();

    // Stop when ctrl-c aborts start
    server->stop();
    
    std::cout << "tcp_echo_server shut down" << std::endl;
    return 0;
}
