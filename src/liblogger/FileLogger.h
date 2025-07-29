#pragma once

#include <liblogger/ILogger.h>
#include <fstream>
#include <mutex>

namespace logger {

class FileLogger final : public ILogger {
public:
    explicit FileLogger(const std::string& filename);
    void log(const char* format, ...) override;

private:
    std::mutex mutex_;
    std::ofstream out;
};

} 