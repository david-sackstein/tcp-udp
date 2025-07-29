#pragma once

#include "libacetools/IOResultCode.h"

#include <string>

class IOResult {
public:
    IOResultCode code;
    size_t count;              // Valid when code == IOResultCode::Success
    std::string error_message; // Valid when code == IOResultCode::Error

    static IOResult result_success(size_t bytes) {
        return {IOResultCode::Success, bytes, ""};
    }

    static IOResult result_connection_closed() {
        return {IOResultCode::ConnectionClosed, 0, ""};
    }

    static IOResult result_timeout() {
        return {IOResultCode::Timeout, 0, ""};
    }

    static IOResult result_error(const std::string& message) {
        return {IOResultCode::Error, 0, message};
    }

private:
    IOResult(IOResultCode result, size_t bytes_read, std::string error_message)
        : code(result), count(bytes_read), error_message(std::move(error_message)) {}
};
