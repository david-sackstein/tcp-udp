#pragma once

#include <string>
#include <vector>

class EndpointValidator final {
public:
    static bool is_valid_port(int port);
    static bool is_valid_endpoint_format(const std::string& endpoint);
    static bool validate_listen_address(const std::string& listen_arg, const std::vector<std::string>& local_addresses);
    static void display_validation_error(const std::string& host, const std::vector<std::string>& local_addresses);
}; 