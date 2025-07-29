#include "ConsoleLogger.h"
#include <common/StringUtils.h>
#include <iostream>

namespace logger {

ConsoleLogger::ConsoleLogger(LogLevel level) : level_(level) {}

void ConsoleLogger::log(LogLevel level, const char* format, ...) {
    if (level < level_) {
        return;
    }

    std::lock_guard lock (mutex_);

    va_list args;
    va_start(args, format);
    std::string msg = format_string_va(format, args);
    va_end(args);

    std::cout << msg << std::endl;
}

} 