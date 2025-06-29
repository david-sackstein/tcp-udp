#include <libudp/Exports.h>
#include <libargsparser/Exports.h>
#include <iostream>
#include <vector>

void wait_for_quit_command();

int main(int argc, char* argv[]) {
    const auto& local_addresses = udp::get_local_ipv4_addresses();
    
    std::string listen_endpoint_str = get_listen_endpoint(argc, argv, local_addresses);
    Endpoint listen_ep = Endpoint::from_string(listen_endpoint_str);

    std::unique_ptr<udp::IUdpClientHandler> handler = udp::create_udp_echo_handler();
    
    auto server = udp::start_udp_server(listen_ep, *handler);

    std::cout << "UDP Server running on " << server->get_local_endpoint().to_string() << std::endl;
    std::cout << "Press 'q' and Enter to stop the server..." << std::endl;

    wait_for_quit_command();

    server->stop();

    std::cout << "Server stopped" << std::endl;

    return 0;
}

void wait_for_quit_command() {
    std::string input;
    while (std::getline(std::cin, input)) {
        if (input == "q" || input == "Q") {
            break;
        }
    }
}
