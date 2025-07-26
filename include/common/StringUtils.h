#pragma once

#include <cstdarg>
#include <cstdio>
#include <string>
#include <vector>

inline std::string format_string_va(const char* format, va_list args) {
    va_list args_copy;
    va_copy(args_copy, args);
    int size = vsnprintf(nullptr, 0, format, args_copy);
    va_end(args_copy);

    std::vector<char> buffer(size + 1);
    vsnprintf(buffer.data(), buffer.size(), format, args);
    return buffer.data();
}

inline std::string format_string(const char* format, ...) {
    va_list args;
    va_start(args, format);
    std::string result = format_string_va(format, args);
    va_end(args);
    return result;
}
