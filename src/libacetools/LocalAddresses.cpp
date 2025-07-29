#include "LocalAddresses.h"

#include <common/Constants.h>

#include <ace/INET_Addr.h>
#include <ifaddrs.h>

bool is_ipv4_addr(const ifaddrs* ifa);
std::string sockaddr_to_string(const sockaddr* sa);
std::string strip_port(const std::string& s);

std::vector<std::string> get_local_ipv4_addresses() {
    std::vector<std::string> addresses;
    struct ifaddrs* if_addresses{};

    if (::getifaddrs(&if_addresses) != 0) {
        return {};
    }

    for (struct ifaddrs* ifa = if_addresses; ifa; ifa = ifa->ifa_next) {
        if (is_ipv4_addr(ifa)) {
            auto address = sockaddr_to_string(ifa->ifa_addr);
            if (!address.empty()) {
                addresses.emplace_back(address);
            }
        }
    }

    ::freeifaddrs(if_addresses);
    return addresses;
}

std::string sockaddr_to_string(const sockaddr* sa) {
    ACE_INET_Addr addr{};
    char buf[common::SMALL_BUFFER_SIZE] = {};

    bool set_ok = addr.set(reinterpret_cast<const sockaddr_in*>(sa), sizeof(struct sockaddr_in)) == 0;
    bool to_string_ok = addr.addr_to_string(buf, sizeof(buf), 1) == 0;

    if (set_ok && to_string_ok) {
        return strip_port(buf);
    }

    return {};
}

bool is_ipv4_addr(const ifaddrs* ifa) {
    return ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET;
}

std::string strip_port(const std::string& s) {
    auto pos = s.find(':');
    return (pos != std::string::npos) ? s.substr(0, pos) : s;
}
