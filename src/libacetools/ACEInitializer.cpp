#include <libacetools/ACEInitializer.h>
#include "LocalAddresses.h"

#include <ace/Init_ACE.h>

ACEInitializer::ACEInitializer() {
    ACE::init();

    local_addresses_ = {"0.0.0.0", "127.0.0.1"};
    auto ipv4_addresses = get_local_ipv4_addresses();
    local_addresses_.insert(local_addresses_.end(), ipv4_addresses.begin(), ipv4_addresses.end());
}

ACEInitializer::~ACEInitializer() {
    ACE::fini();
}

EXPORTED const std::vector<std::string>& ACEInitializer::init() {
    static ACEInitializer instance;
    return instance.local_addresses_;
}
