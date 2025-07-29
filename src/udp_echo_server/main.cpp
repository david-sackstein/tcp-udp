#include <libudp/Exports.h>
#include <libargsparser/Exports.h>
#include <liblogger/Exports.h>
#include <iostream>

void wait_for_quit_command();

int main(int argc, char* argv[]) {
    auto logger = logger::create_console_logger(logger::LogLevel::ERROR);
    
    const auto& local_addresses = udp::get_local_ipv4_addresses();
    
    std::string listen_endpoint_str = get_listen_endpoint(argc, argv, local_addresses);
    Endpoint listen_ep = Endpoint::from_string(listen_endpoint_str);

    std::unique_ptr<udp::IUdpClientHandler> handler = udp::create_udp_echo_handler(*logger);
    
    auto server = udp::start_udp_server(*logger, listen_ep, *handler);

    logger->log(logger::LogLevel::INFO, "UDP Server running on %s", server->get_local_endpoint().to_string().c_str());
    logger->log(logger::LogLevel::INFO, "Press 'q' and Enter to stop the server...");

    wait_for_quit_command();

    server->stop();

    logger->log(logger::LogLevel::INFO, "Server stopped");

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
