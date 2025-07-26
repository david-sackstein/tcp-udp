#pragma once

#include <string>

enum class IOResultCode {
    Success,           // Data received (n > 0)
    ConnectionClosed,  // Connection closed by peer (n = 0)
    Timeout,           // Timeout occurred, no data (n = -1, EAGAIN/EWOULDBLOCK)
    Error              // Error occurred (n = -1, other errors)
};
