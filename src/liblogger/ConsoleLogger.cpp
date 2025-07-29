#include "ConsoleLogger.h"
#include <common/StringUtils.h>
#include <iostream>

namespace logger {

void ConsoleLogger::log(const char* format, ...) {
    std::lock_guard lock (mutex_);

    va_list args;
    va_start(args, format);
    std::string msg = format_string_va(format, args);
    va_end(args);

    std::cout << msg << std::endl;
}

} 