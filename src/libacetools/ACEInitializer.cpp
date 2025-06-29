#include <libacetools/ACEInitializer.h>
#include "LocalAddresses.h"

#include <ace/Init_ACE.h>

ACEInitializer::ACEInitializer() {
    ACE::init();
    
    local_addresses_ = get_local_ipv4_addresses();
    local_addresses_.insert(local_addresses_.begin(), "0.0.0.0");
}

ACEInitializer::~ACEInitializer() {
    ACE::fini();
}

EXPORTED const std::vector<std::string>& ACEInitializer::init() {
    static ACEInitializer instance;
    return instance.local_addresses_;
}
