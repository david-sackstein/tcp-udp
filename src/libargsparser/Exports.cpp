#include "ArgsParser.h"
#include "PortCleanup.h"

#include <libargsparser/Exports.h>

bool parse_custom_arguments(int argc,
    char* argv[],
    const ArgumentHandlerMap& argument_handlers,
    const ErrorHandler& error_handler) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        auto it = argument_handlers.find(arg);
        if (it != argument_handlers.end()) {
            std::string value;
            if (!it->second(arg, value)) {
                return false;
            }
        } else if (error_handler) {
            error_handler(arg);
        }
    }
    return true;
}

std::pair<std::string, std::string> get_endpoint_configuration(int argc,
    char* argv[],
    const std::vector<std::string>& local_addresses) {
    ArgsParser parser;
    return parser.get_endpoint_configuration(argc, argv, local_addresses);
}

std::string get_listen_endpoint(int argc, char* argv[], const std::vector<std::string>& local_addresses) {
    ArgsParser parser;
    return parser.get_listen_endpoint(argc, argv, local_addresses);
}

std::string get_flag_value(int argc, char* argv[], const std::string& flag_name, const std::string& default_value) {
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == flag_name) {
            if (i + 1 < argc) {
                return std::string(argv[i + 1]);
            }
            return "true"; // Flag present without value
        }
    }
    return default_value;
}

bool check_and_cleanup_port(logger::ILogger& logger, const std::string& endpoint) {
    argsparser::PortCleanup port_cleanup(logger);
    return port_cleanup.check_and_cleanup_port(endpoint);
}