#pragma once

#include <liblogger/ILogger.h>
#include <string>

namespace argsparser {

class PortCleanup {
public:
    explicit PortCleanup(logger::ILogger& logger);
    bool check_and_cleanup_port(const std::string& endpoint, bool force = false);

private:
    bool is_port_in_use(int port);
    bool kill_process_on_port(int port);
    bool ask_user_for_confirmation(int port);
    void log_port_status(int port, bool in_use);

    logger::ILogger& logger_;
};
} // namespace argsparser
