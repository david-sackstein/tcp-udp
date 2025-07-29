#include "EndpointValidator.h"

#include <common/Constants.h>
#include <common/Endpoint.h>

#include <algorithm>
#include <iostream>

bool EndpointValidator::is_valid_port(int port) {
    return port >= 0 && port <= common::MAX_PORT;
}

bool EndpointValidator::is_valid_endpoint_format(const std::string& endpoint) {
    try {
        Endpoint::from_string(endpoint);
        return true;
    } catch (...) {
        return false;
    }
}

bool EndpointValidator::validate_listen_address(const std::string& listen_arg,
    const std::vector<std::string>& local_addresses) {
    Endpoint ep = Endpoint::from_string(listen_arg);
    return std::find(local_addresses.begin(), local_addresses.end(), ep.address) != local_addresses.end();
}

void EndpointValidator::display_validation_error(const std::string& host,
    const std::vector<std::string>& local_addresses) {
    std::cerr << "Error: The specified listen address '" << host << "' is not available on this machine." << std::endl;
    std::cerr << "Available addresses are:" << std::endl;
    for (const auto& addr : local_addresses) {
        std::cerr << "  " << addr << std::endl;
    }
}