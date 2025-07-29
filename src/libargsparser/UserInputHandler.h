#pragma once

#include <string>

class UserInputHandler final {
public:
    static void clear_cin_buffer();
    static int get_validated_user_input(int min_value, int max_value, const std::string& prompt);
    static std::string get_user_string_input(const std::string& prompt);
    static int select_port();
    static std::string select_remote_endpoint();
};