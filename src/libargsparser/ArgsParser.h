#pragma once

#include <string>
#include <vector>
#include <common/Endpoint.h>

class ArgsParser final {
public:
    std::pair<std::string, std::string> get_endpoint_configuration(int argc, char* argv[], const std::vector<std::string>& local_addresses);
    std::string get_listen_endpoint(int argc, char* argv[], const std::vector<std::string>& local_addresses);

private:
    static std::pair<std::string, std::string> handle_command_line_args(char* argv[], const std::vector<std::string>& local_addresses);
    static std::pair<std::string, std::string> handle_interactive_config(const std::vector<std::string>& local_addresses);

    static bool is_valid_port(int port);
    static bool is_valid_endpoint_format(const std::string& endpoint);
    static bool validate_listen_address(const std::string& listen_arg, const std::vector<std::string>& local_addresses);

    static void clear_cin_buffer();
    static int get_validated_user_input(int min_value, int max_value, const std::string& prompt);
    static std::string get_user_string_input(const std::string& prompt);

    static void display_address_options(const std::vector<std::string>& addresses);
    static void display_validation_error(const std::string& host, const std::vector<std::string>& local_addresses);

    static std::string select_host(const std::vector<std::string>& addresses);
    static int select_port();
    static std::string select_remote_endpoint();

    static std::string build_endpoint_string(const std::string& host, int port);
    static std::string extract_host_from_endpoint(const std::string& endpoint);
}; 