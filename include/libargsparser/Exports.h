#pragma once

#include <common/Exported.h>

#include <string>
#include <vector>

EXPORTED std::pair<std::string, std::string> get_endpoint_configuration(int argc,
    char* argv[],
    const std::vector<std::string>& local_addresses);
EXPORTED std::string get_listen_endpoint(int argc, char* argv[], const std::vector<std::string>& local_addresses);