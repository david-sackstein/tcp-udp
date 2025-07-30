#include "NotifiableClientArgs.h"

#include <libargsparser/Exports.h>
#include <libtcp/Exports.h>

bool NotifiableClientArgs::parse(int argc, char *argv[]) {
    const auto &local_addresses = tcp::get_local_ipv4_addresses();
    endpoint_ = get_listen_endpoint(argc, argv, local_addresses);

    ArgumentHandlerMap handlers;
    handlers["--client-id"] = [this](
        const std::string &value, std::string &) {
                return handle_client_id_argument(value);
            };
    handlers["--interval"] = [this](const std::string &value, std::string &) {
        return handle_interval_argument(value);
    };

    ErrorHandler error_handler = [this](const std::string &error) { handle_error(error); };

    return parse_custom_arguments(argc, argv, handlers, error_handler);
}

bool NotifiableClientArgs::handle_client_id_argument(const std::string &value) {
    client_id_ = value;
    return true;
}

bool NotifiableClientArgs::handle_interval_argument(const std::string &value) {
    try {
        request_interval_ms_ = std::stoi(value);
        if (request_interval_ms_ <= 0) {
            error_message_ = "Request interval must be positive";
            return false;
        }
    } catch (const std::exception &e) {
        error_message_ = "Invalid request interval: " + std::string(e.what());
        return false;
    }
    return true;
}

void NotifiableClientArgs::handle_error(const std::string &error) {
    error_message_ = error;
}
