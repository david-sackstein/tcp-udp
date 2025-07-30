#include "ArgsParser.h"
#include "CommandLineParser.h"
#include "EndpointValidator.h"
#include "InteractiveConfigurator.h"
#include "UserInputHandler.h"

#include <common/Endpoint.h>

#include <algorithm>
#include <iostream>

std::pair<std::string, std::string> ArgsParser::get_endpoint_configuration(int argc,
    char* argv[],
    const std::vector<std::string>& local_addresses) {
    // If exactly 2 arguments provided, use command line arguments
    if (argc == 3) {
        return CommandLineParser::handle_command_line_args(argv, local_addresses);
    }

    // Otherwise, provide interactive interface
    return InteractiveConfigurator::handle_interactive_config(local_addresses);
}

std::string ArgsParser::get_listen_endpoint(int argc, char* argv[], const std::vector<std::string>& local_addresses) {
    // If at least 2 arguments provided, use command line argument (first non-flag argument)
    if (argc >= 2) {
        std::string listen_arg;
        // Find the first argument that's not a flag (doesn't start with --)
        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg.substr(0, 2) != "--") {
                listen_arg = arg;
                break;
            }
        }

        if (!listen_arg.empty()) {
            if (!EndpointValidator::validate_listen_address(listen_arg, local_addresses)) {
                Endpoint ep = Endpoint::from_string(listen_arg);
                EndpointValidator::display_validation_error(ep.address, local_addresses);
                exit(1);
            }
            return listen_arg;
        }
    }

    // Otherwise, use interactive selection
    std::string local_host = InteractiveConfigurator::select_host(local_addresses);
    int local_port = UserInputHandler::select_port();
    return InteractiveConfigurator::build_endpoint_string(local_host, local_port);
}