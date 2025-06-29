#pragma once

#include <string>
#include <vector>
#include <common/Endpoint.h>

class ArgsParser {
public:
    std::pair<std::string, std::string> get_endpoint_configuration(int argc, char* argv[], const std::vector<std::string>& local_addresses);
    std::string get_listen_endpoint(int argc, char* argv[], const std::vector<std::string>& local_addresses);

private:
    std::pair<std::string, std::string> handle_command_line_args(char* argv[], const std::vector<std::string>& local_addresses);
    std::pair<std::string, std::string> handle_interactive_config(const std::vector<std::string>& local_addresses);

    bool is_valid_port(int port);
    bool is_valid_endpoint_format(const std::string& endpoint);
    bool validate_listen_address(const std::string& listen_arg, const std::vector<std::string>& local_addresses);

    void clear_cin_buffer();
    int get_validated_user_input(int min_value, int max_value, const std::string& prompt);
    std::string get_user_string_input(const std::string& prompt);

    void display_address_options(const std::vector<std::string>& addresses);
    void display_validation_error(const std::string& host, const std::vector<std::string>& local_addresses);

    std::string select_host(const std::vector<std::string>& addresses);
    int select_port();
    std::string select_remote_endpoint();

    std::string build_endpoint_string(const std::string& host, int port);
    std::string extract_host_from_endpoint(const std::string& endpoint);
}; 