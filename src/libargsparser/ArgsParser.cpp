#include "ArgsParser.h"

#include <iostream>
#include <limits>
#include <algorithm>

std::pair<std::string, std::string> ArgsParser::get_endpoint_configuration(int argc, char* argv[], const std::vector<std::string>& local_addresses) {

    // If exactly 2 arguments provided, use command line arguments
    if (argc == 3) {
        return handle_command_line_args(argv, local_addresses);
    }

    // Otherwise, provide interactive interface
    return handle_interactive_config(local_addresses);
}

std::string ArgsParser::get_listen_endpoint(int argc, char* argv[], const std::vector<std::string>& local_addresses) {
    // If exactly 1 argument provided, use command line argument
    if (argc == 2) {
        std::string listen_arg = argv[1];
        if (!validate_listen_address(listen_arg, local_addresses)) {
            Endpoint ep = Endpoint::from_string(listen_arg);
            display_validation_error(ep.address, local_addresses);
            exit(1);
        }
        return listen_arg;
    }
    // Otherwise, use interactive selection
    std::string local_host = select_host(local_addresses);
    int local_port = select_port();
    return build_endpoint_string(local_host, local_port);
}

std::pair<std::string, std::string> ArgsParser::handle_command_line_args(char* argv[], const std::vector<std::string>& local_addresses) {
    std::string listen_arg = argv[1];
    
    if (!validate_listen_address(listen_arg, local_addresses)) {
        Endpoint ep = Endpoint::from_string(listen_arg);
        display_validation_error(ep.address, local_addresses);
        exit(1);
    }
    
    return {argv[1], argv[2]};
}

std::pair<std::string, std::string> ArgsParser::handle_interactive_config(const std::vector<std::string>& local_addresses) {
    std::cout << "=== TCP Client Proxy Configuration ===" << std::endl;
    std::cout << "No command line arguments provided. Using interactive configuration." << std::endl;

    // Get local address and port
    std::string local_host = select_host(local_addresses);
    int local_port = select_port();
    std::string listen_endpoint = build_endpoint_string(local_host, local_port);

    // Get remote endpoint
    std::string forward_endpoint = select_remote_endpoint();

    // Show final configuration
    std::cout << "\n=== Final Configuration ===" << std::endl;
    std::cout << "Listen endpoint: " << listen_endpoint << std::endl;
    std::cout << "Forward endpoint: " << forward_endpoint << std::endl;
    std::cout << "Press Enter to continue...";
    std::cin.get();
    
    return {listen_endpoint, forward_endpoint};
}

bool ArgsParser::is_valid_port(int port) {
    return port >= 0 && port <= 65535;
}

bool ArgsParser::is_valid_endpoint_format(const std::string& endpoint) {
    try {
        Endpoint::from_string(endpoint);
        return true;
    } catch (...) {
        return false;
    }
}

bool ArgsParser::validate_listen_address(const std::string& listen_arg, const std::vector<std::string>& local_addresses) {
    Endpoint ep = Endpoint::from_string(listen_arg);
    return std::find(local_addresses.begin(), local_addresses.end(), ep.address) != local_addresses.end();
}

void ArgsParser::clear_cin_buffer() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

int ArgsParser::get_validated_user_input(int min_value, int max_value, const std::string& prompt) {
    int input;
    while (true) {
        std::cout << prompt;
        std::cin >> input;
        
        if (std::cin.fail()) {
            clear_cin_buffer();
            std::cout << "Invalid input. Please enter a number.\n";
            continue;
        }
        
        if (input >= min_value && input <= max_value) {
            break;
        }
        
        std::cout << "Invalid selection. Please choose between " << min_value << " and " << max_value << ".\n";
    }
    return input;
}

std::string ArgsParser::get_user_string_input(const std::string& prompt) {
    std::cout << prompt;
    std::string input;
    clear_cin_buffer(); // Clear newline from previous input
    std::getline(std::cin, input);
    return input;
}

void ArgsParser::display_address_options(const std::vector<std::string>& addresses) {
    std::cout << "Available local addresses:\n";
    for (size_t i = 0; i < addresses.size(); ++i) {
        std::cout << "  " << i << ". " << addresses[i];
        if (i == 0) {
            std::cout << " (any/all interfaces)";
        }
        std::cout << "\n";
    }
}

void ArgsParser::display_validation_error(const std::string& host, const std::vector<std::string>& local_addresses) {
    std::cerr << "Error: The specified listen address '" << host << "' is not available on this machine." << std::endl;
    std::cerr << "Available addresses are:" << std::endl;
    for (const auto& addr : local_addresses) {
        std::cerr << "  " << addr << std::endl;
    }
}

std::string ArgsParser::select_host(const std::vector<std::string>& addresses) {
    std::cout << "\n=== Local Address Selection ===\n";

    display_address_options(addresses);

    std::string prompt = "\nSelect local address (0-" + std::to_string(addresses.size() - 1) + "): ";
    int selection = get_validated_user_input(0, static_cast<int>(addresses.size() - 1), prompt);
    
    return addresses[selection];
}

int ArgsParser::select_port() {
    std::cout << "\n=== Port Selection ===\n";
    std::cout << "Enter port number (0 for any available port): ";

    int port;
    while (true) {
        std::cin >> port;
        
        if (std::cin.fail()) {
            clear_cin_buffer();
            std::cout << "Invalid input. Please enter a number.\n";
            continue;
        }
        
        if (is_valid_port(port)) {
            break;
        }
        
        std::cout << "Invalid port. Please enter a number between 0 and 65535.\n";
    }
    return port;
}

std::string ArgsParser::select_remote_endpoint() {
    std::cout << "\n=== Remote TCP Server Configuration ===\n";
    
    std::string prompt = "Enter remote TCP server address (host:port): ";
    std::string remote_endpoint = get_user_string_input(prompt);

    if (!is_valid_endpoint_format(remote_endpoint)) {
        std::cout << "Invalid format. Please use host:port format.\n";
        return select_remote_endpoint(); // Recursive call for retry
    }
    
    return remote_endpoint;
}

std::string ArgsParser::build_endpoint_string(const std::string& host, int port) {
    Endpoint endpoint(host, static_cast<uint16_t>(port));
    return endpoint.to_string();
}

std::string ArgsParser::extract_host_from_endpoint(const std::string& endpoint) {
    Endpoint ep = Endpoint::from_string(endpoint);
    return ep.address;
} 