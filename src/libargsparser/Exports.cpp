#include "ArgsParser.h"

#include <libargsparser/Exports.h>

static ArgsParser& get_args_parser() {
    static ArgsParser parser;
    return parser;
}

EXPORTED std::pair<std::string, std::string> get_endpoint_configuration(int argc, char* argv[], const std::vector<std::string>& local_addresses) {
    return get_args_parser().get_endpoint_configuration(argc, argv, local_addresses);
}

EXPORTED std::string get_listen_endpoint(int argc, char* argv[], const std::vector<std::string>& local_addresses) {
    return get_args_parser().get_listen_endpoint(argc, argv, local_addresses);
} 