#pragma once

#include <liblogger/ILogger.h>

#include <mutex>

namespace logger {

class ConsoleLogger final : public ILogger {
public:
    void log(const char* format, ...) override;
private:
    std::mutex mutex_;
};

} 