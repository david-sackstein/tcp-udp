#include "FileLogger.h"
#include <common/StringUtils.h>

namespace logger {

FileLogger::FileLogger(const std::string& filename)
    : out(filename, std::ios::app) {}

void FileLogger::log(const char* format, ...) {
    va_list args;
    va_start(args, format);
    std::string msg = format_string_va(format, args);
    va_end(args);

    out << msg << std::endl;
}

} 