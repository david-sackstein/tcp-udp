#include "NotifiableClientArgs.h"

#include <libargsparser/Exports.h>
#include <libtcp/Exports.h>

bool NotifiableClientArgs::parse(int argc, char* argv[]) {
    const auto& local_addresses = tcp::get_local_ipv4_addresses();
    endpoint_ = get_listen_endpoint(argc, argv, local_addresses);

    // Use get_flag_value for simple flag parsing
    std::string client_id_value = get_flag_value(argc, argv, "--client-id", client_id_);
    if (client_id_value != client_id_) {
        client_id_ = client_id_value;
    }

    std::string interval_value = get_flag_value(argc, argv, "--interval", std::to_string(request_interval_ms_));
    if (interval_value != std::to_string(request_interval_ms_)) {
        if (!handle_interval_argument(interval_value)) {
            return false;
        }
    }

    return true;
}

bool NotifiableClientArgs::handle_client_id_argument(const std::string& value) {
    client_id_ = value;
    return true;
}

bool NotifiableClientArgs::handle_interval_argument(const std::string& value) {
    try {
        request_interval_ms_ = std::stoi(value);
        if (request_interval_ms_ <= 0) {
            error_message_ = "Request interval must be positive";
            return false;
        }
    } catch (const std::exception& e) {
        error_message_ = "Invalid request interval: " + std::string(e.what());
        return false;
    }
    return true;
}

void NotifiableClientArgs::handle_error(const std::string& error) {
    error_message_ = error;
}
