#include "PortCleanup.h"

#include <common/Endpoint.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>

namespace argsparser {
constexpr bool force = true;

PortCleanup::PortCleanup(logger::ILogger& logger) : logger_(logger) {}

bool PortCleanup::check_and_cleanup_port(const std::string& endpoint) {
    Endpoint ep = Endpoint::from_string(endpoint);
    int port = ep.port;

    if (!is_port_in_use(port)) {
        return true;
    }

    log_port_status(port, true);

    if (force) {
        return kill_process_on_port(port);
    }

    if (ask_user_for_confirmation(port)) {
        return kill_process_on_port(port);
    }

    logger_.log(logger::LogLevel::ERROR, "Port cleanup cancelled by user");
    return false;
}

bool PortCleanup::is_port_in_use(int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        logger_.log(logger::LogLevel::ERROR, "Failed to create socket for port check");
        return false;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    bool in_use = (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) != 0);
    close(sock);

    log_port_status(port, in_use);
    return in_use;
}

bool PortCleanup::kill_process_on_port(int port) {
    std::string cmd = "lsof -ti:" + std::to_string(port) + " | xargs kill -9";
    int result = system(cmd.c_str());

    if (result == 0) {
        logger_.log(logger::LogLevel::INFO, "Killed process on port %d", port);
        return true;
    }

    logger_.log(logger::LogLevel::ERROR, "Failed to kill process on port %d", port);
    return false;
}

bool PortCleanup::ask_user_for_confirmation(int port) {
    std::cout << "Port " << port << " is already in use. Kill existing process? (y/N): ";

    std::string response;
    std::getline(std::cin, response);

    return (response == "y" || response == "Y");
}

void PortCleanup::log_port_status(int port, bool in_use) {
    if (in_use) {
        logger_.log(logger::LogLevel::ERROR, "Port %d is already in use", port);
    } else {
        logger_.log(logger::LogLevel::INFO, "Port %d is available", port);
    }
}
} // namespace argsparser
