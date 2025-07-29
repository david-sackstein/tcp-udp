#include "EndpointValidator.h"
#include "UserInputHandler.h"

#include <common/Constants.h>

#include <iostream>
#include <limits>

void UserInputHandler::clear_cin_buffer() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

int UserInputHandler::get_validated_user_input(int min_value, int max_value, const std::string& prompt) {
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

std::string UserInputHandler::get_user_string_input(const std::string& prompt) {
    std::cout << prompt;
    std::string input;
    clear_cin_buffer(); // Clear newline from previous input
    std::getline(std::cin, input);
    return input;
}

int UserInputHandler::select_port() {
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
        
        if (EndpointValidator::is_valid_port(port)) {
            break;
        }
        
        std::cout << "Invalid port. Please enter a number between 0 and " << common::MAX_PORT << ".\n";
    }
    return port;
}

std::string UserInputHandler::select_remote_endpoint() {
    std::cout << "\n=== Remote TCP Server Configuration ===\n";
    
    std::string prompt = "Enter remote TCP server address (host:port): ";
    std::string remote_endpoint = get_user_string_input(prompt);

    if (!EndpointValidator::is_valid_endpoint_format(remote_endpoint)) {
        std::cout << "Invalid format. Please use host:port format.\n";
        return select_remote_endpoint(); // Recursive call for retry
    }
    
    return remote_endpoint;
} 