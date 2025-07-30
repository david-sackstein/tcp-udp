#pragma once

#include <libargsparser/Exports.h>

#include <string>
#include <vector>

class NotifiableClientArgs final {
public:
    NotifiableClientArgs() = default;

    bool parse(int argc, char* argv[]);

    [[nodiscard]] const std::string& get_endpoint() const {
        return endpoint_;
    }

    [[nodiscard]] const std::string& get_client_id() const {
        return client_id_;
    }

    [[nodiscard]] int get_request_interval_ms() const {
        return request_interval_ms_;
    }

    [[nodiscard]] const std::string& get_error_message() const {
        return error_message_;
    }

private:
    bool handle_client_id_argument(const std::string& value, std::string&);
    bool handle_interval_argument(const std::string& value, std::string&);
    void handle_error(const std::string& error);

    std::string endpoint_;
    std::string client_id_ = "client1";
    int request_interval_ms_ = 1000;
    std::string error_message_;
};