#pragma once

namespace logger {
    enum class LogLevel {
        DEBUG = 0,
        INFO = 1,
        ERROR = 2
    };

    class ILogger {
    public:
        virtual ~ILogger() = default;
        virtual void log(LogLevel level, const char* format, ...) = 0;
    };
}
