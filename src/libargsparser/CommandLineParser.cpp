#include "CommandLineParser.h"
#include "EndpointValidator.h"

#include <common/Endpoint.h>

#include <iostream>

std::pair<std::string, std::string> CommandLineParser::handle_command_line_args(char* argv[], const std::vector<std::string>& local_addresses) {
    std::string listen_arg = argv[1];
    
    if (!EndpointValidator::validate_listen_address(listen_arg, local_addresses)) {
        Endpoint ep = Endpoint::from_string(listen_arg);
        EndpointValidator::display_validation_error(ep.address, local_addresses);
        exit(1);
    }
    
    return {argv[1], argv[2]};
}

std::string CommandLineParser::extract_host_from_endpoint(const std::string& endpoint) {
    Endpoint ep = Endpoint::from_string(endpoint);
    return ep.address;
} 