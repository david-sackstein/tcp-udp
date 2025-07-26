#pragma once

#include <liblogger/ILogger.h>
#include <fstream>

namespace logger {

class FileLogger final : public ILogger {
public:
    explicit FileLogger(const std::string& filename);
    void log(const char* format, ...) override;

private:
    std::ofstream out;
};

} 