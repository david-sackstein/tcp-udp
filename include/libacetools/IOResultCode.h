#pragma once

enum class IOResultCode {
    Success,          // Data received (n > 0)
    ConnectionClosed, // Connection closed by peer (n = 0)
    Timeout,          // Timeout occurred, no data (n = -1, EAGAIN/EWOULDBLOCK)
    Error             // Error occurred (n = -1, other errors)
};

inline const char* to_string(IOResultCode code) {
    switch (code) {
        case IOResultCode::Success:
            return "Success";
        case IOResultCode::ConnectionClosed:
            return "ConnectionClosed";
        case IOResultCode::Timeout:
            return "Timeout";
        case IOResultCode::Error:
            return "Error";
    }
}
