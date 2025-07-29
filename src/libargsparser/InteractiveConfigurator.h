#pragma once

#include <string>
#include <vector>

class InteractiveConfigurator final {
public:
    static std::pair<std::string, std::string> handle_interactive_config(const std::vector<std::string>& local_addresses);
    static std::string select_host(const std::vector<std::string>& addresses);
    static void display_address_options(const std::vector<std::string>& addresses);
    static std::string build_endpoint_string(const std::string& host, int port);
}; 