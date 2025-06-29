#pragma once

#include <common/Exported.h>
#include <vector>
#include <string>

class ACEInitializer {
    ACEInitializer();
    ~ACEInitializer();

    std::vector<std::string> local_addresses_;

public:

    EXPORTED static const std::vector<std::string>& init();
};
