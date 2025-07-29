#pragma once

#include <string>
#include <vector>

class CommandLineParser final {
public:
    static std::pair<std::string, std::string> handle_command_line_args(char* argv[], const std::vector<std::string>& local_addresses);
    static std::string extract_host_from_endpoint(const std::string& endpoint);
}; 