#pragma once

#include <liblogger/ILogger.h>

namespace logger {

class ConsoleLogger final : public ILogger {
public:
    void log(const char* format, ...) override;
};

} 