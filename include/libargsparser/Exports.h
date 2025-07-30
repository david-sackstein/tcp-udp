#pragma once

#include <common/Exported.h>
#include <liblogger/ILogger.h>

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

using ArgumentHandler = std::function<bool(const std::string&, std::string&)>;
using ErrorHandler = std::function<void(const std::string&)>;
using ArgumentHandlerMap = std::unordered_map<std::string, ArgumentHandler>;

EXPORTED bool parse_custom_arguments(int argc,
    char* argv[],
    const ArgumentHandlerMap& argument_handlers,
    const ErrorHandler& error_handler);

EXPORTED std::pair<std::string, std::string> get_endpoint_configuration(int argc,
    char* argv[],
    const std::vector<std::string>& local_addresses);

EXPORTED std::string get_listen_endpoint(int argc, char* argv[], const std::vector<std::string>& local_addresses);

EXPORTED std::string get_flag_value(int argc,
    char* argv[],
    const std::string& flag_name,
    const std::string& default_value = "");

EXPORTED bool check_and_cleanup_port(logger::ILogger& logger, const std::string& endpoint, bool force = false);
