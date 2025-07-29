#pragma once

#include <string>
#include <vector>

class ArgsParser final {
public:
    static std::pair<std::string, std::string> get_endpoint_configuration(int argc, char* argv[], const std::vector<std::string>& local_addresses);

    static std::string get_listen_endpoint(int argc, char* argv[], const std::vector<std::string>& local_addresses);
}; 