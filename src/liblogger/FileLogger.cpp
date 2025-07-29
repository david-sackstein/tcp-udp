#include "FileLogger.h"
#include <common/StringUtils.h>

namespace logger {

FileLogger::FileLogger(LogLevel level, const std::string& filename) : out(filename, std::ios::app), level_(level) {}

void FileLogger::log(LogLevel level, const char* format, ...) {
    if (level < level_) {
        return;
    }

    std::lock_guard lock(mutex_);

    va_list args;
    va_start(args, format);
    std::string msg = format_string_va(format, args);
    va_end(args);

    out << msg << std::endl;
}

} // namespace logger