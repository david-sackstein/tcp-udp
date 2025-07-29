#include <liblogger/Exports.h>
#include "ConsoleLogger.h"
#include "FileLogger.h"

namespace logger {

EXPORTED std::unique_ptr<ILogger> create_console_logger(LogLevel level) {
    return std::make_unique<ConsoleLogger>(level);
}

EXPORTED std::unique_ptr<ILogger> create_file_logger(LogLevel level, const std::string& filename) {
    return std::make_unique<FileLogger>(level, filename);
}

} // namespace logger