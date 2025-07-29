#include "ConsoleLogger.h"
#include "FileLogger.h"
#include <liblogger/Exports.h>

namespace logger {

EXPORTED std::unique_ptr<ILogger> create_console_logger(LogLevel level) {
    return std::make_unique<ConsoleLogger>(level);
}

EXPORTED std::unique_ptr<ILogger> create_file_logger(LogLevel level, const std::string& filename) {
    return std::make_unique<FileLogger>(level, filename);
}

} 