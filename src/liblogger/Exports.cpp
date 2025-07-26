#include "ConsoleLogger.h"
#include "FileLogger.h"
#include <liblogger/Exports.h>

namespace logger {

EXPORTED std::unique_ptr<ILogger> create_console_logger() {
    return std::make_unique<ConsoleLogger>();
}

EXPORTED std::unique_ptr<ILogger> create_file_logger(const std::string& filename) {
    return std::make_unique<FileLogger>(filename);
}

} 