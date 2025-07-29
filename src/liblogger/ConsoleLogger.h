#pragma once

#include <liblogger/ILogger.h>

#include <mutex>

namespace logger {

class ConsoleLogger final : public ILogger {
public:
    explicit ConsoleLogger(LogLevel level);
    void log(LogLevel level, const char* format, ...) override;
private:
    std::mutex mutex_;
    LogLevel level_;
};

} 