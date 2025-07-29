#include "InteractiveConfigurator.h"
#include "UserInputHandler.h"

#include <common/Endpoint.h>

#include <iostream>

std::pair<std::string, std::string> InteractiveConfigurator::handle_interactive_config(
    const std::vector<std::string>& local_addresses) {
    std::cout << "=== TCP Client Proxy Configuration ===" << std::endl;
    std::cout << "No command line arguments provided. Using interactive configuration." << std::endl;

    // Get local address and port
    std::string local_host = select_host(local_addresses);
    int local_port = UserInputHandler::select_port();
    std::string listen_endpoint = build_endpoint_string(local_host, local_port);

    // Get remote endpoint
    std::string forward_endpoint = UserInputHandler::select_remote_endpoint();

    // Show final configuration
    std::cout << "\n=== Final Configuration ===" << std::endl;
    std::cout << "Listen endpoint: " << listen_endpoint << std::endl;
    std::cout << "Forward endpoint: " << forward_endpoint << std::endl;
    std::cout << "Press Enter to continue...";
    std::cin.get();

    return {listen_endpoint, forward_endpoint};
}

std::string InteractiveConfigurator::select_host(const std::vector<std::string>& addresses) {
    std::cout << "\n=== Local Address Selection ===\n";

    display_address_options(addresses);

    std::string prompt = "\nSelect local address (0-" + std::to_string(addresses.size() - 1) + "): ";
    int selection = UserInputHandler::get_validated_user_input(0, static_cast<int>(addresses.size() - 1), prompt);

    return addresses[selection];
}

void InteractiveConfigurator::display_address_options(const std::vector<std::string>& addresses) {
    std::cout << "Available local addresses:\n";
    for (size_t i = 0; i < addresses.size(); ++i) {
        std::cout << "  " << i << ". " << addresses[i];
        if (i == 0) {
            std::cout << " (any/all interfaces)";
        }
        std::cout << "\n";
    }
}

std::string InteractiveConfigurator::build_endpoint_string(const std::string& host, int port) {
    Endpoint endpoint(host, static_cast<uint16_t>(port));
    return endpoint.to_string();
}