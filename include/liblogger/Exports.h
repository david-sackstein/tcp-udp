#pragma once

#include <liblogger/ILogger.h>
#include <common/Exported.h>
#include <memory>
#include <string>

namespace logger {

EXPORTED std::unique_ptr<ILogger> create_console_logger();
EXPORTED std::unique_ptr<ILogger> create_file_logger(const std::string& filename);

} 