#pragma once

#include <string>
#include <cstdint>

struct Endpoint {
    static constexpr auto localhost = "127.0.0.1";
    static constexpr auto any_address = "0.0.0.0";
    static constexpr uint16_t any_port = 0;

    std::string address = any_address;
    uint16_t port = any_port;

    explicit Endpoint() = default;

    explicit Endpoint(std::string addr, uint16_t p)
        : address(std::move(addr)),
          port(p) {}

    bool operator==(const Endpoint &other) const {
        return address == other.address && port == other.port;
    }

    bool operator!=(const Endpoint &other) const {
        return !(*this == other);
    }

    [[nodiscard]] std::string to_string() const;

    static Endpoint from_string(const std::string &addr_port);

    static Endpoint any() { return Endpoint(); }

    static Endpoint any_loop_back() { return loop_back(any_port); }

    static Endpoint loop_back(uint16_t p) { return Endpoint(localhost, p); }

private:
    static std::string get_address_string(const std::string &);
};

inline std::string Endpoint::to_string() const {
    return get_address_string(address) + ":" + std::to_string(port);
}

inline std::string Endpoint::get_address_string(const std::string& address) {
    if (address == localhost) {
        return "localhost";
    }
    if (address == any_address) {
        return "any";
    }
    return address;
}

inline Endpoint Endpoint::from_string(const std::string &addr_port) {
    auto pos = addr_port.find(':');

    if (pos == std::string::npos) {
        return Endpoint(addr_port, any_port);
    }

    const auto &addr = addr_port.substr(0, pos);
    const auto &port_str = addr_port.substr(pos + 1);
    const auto port = static_cast<uint16_t>(std::stoi(port_str));

    return Endpoint(addr, port);
}

