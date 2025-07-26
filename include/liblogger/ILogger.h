#pragma once

namespace logger {
    class ILogger {
    public:
        virtual ~ILogger() = default;
        virtual void log(const char* format, ...) = 0;
    };
}
