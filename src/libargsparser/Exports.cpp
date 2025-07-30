#include "ArgsParser.h"

#include <libargsparser/Exports.h>

#include <iostream>
#include <memory>

EXPORTED std::pair<std::string, std::string> get_endpoint_configuration(int argc,
    char* argv[],
    const std::vector<std::string>& local_addresses) {
    return ArgsParser::get_endpoint_configuration(argc, argv, local_addresses);
}

EXPORTED std::string get_listen_endpoint(int argc, char* argv[], const std::vector<std::string>& local_addresses) {
    return ArgsParser::get_listen_endpoint(argc, argv, local_addresses);
}

EXPORTED bool parse_custom_arguments(int argc,
    char* argv[],
    const ArgumentHandlerMap& argument_handlers,
    const ErrorHandler& error_handler) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        auto it = argument_handlers.find(arg);
        if (it == argument_handlers.end()) {
            continue;
        }

        if (i + 1 >= argc) {
            if (error_handler) {
                error_handler("Missing value for argument: " + arg);
            } else {
                std::cerr << "Missing value for argument: " << arg << std::endl;
            }
            return false;
        }

        if (std::string value = argv[i + 1]; !it->second(value, value)) {
            return false;
        }

        ++i; // Skip
    }

    return true;
}