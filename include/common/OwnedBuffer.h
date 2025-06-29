#pragma once

#include "Buffer.h"

#include <vector>

class OwnedBuffer {
public:
    explicit OwnedBuffer(size_t size)
        : vec_(size) {}

    [[nodiscard]] Buffer view(size_t size = 0) {
        return { vec_.data(), size == 0 ? vec_.size() : size };
    }

    [[nodiscard]] ConstBuffer view(size_t size = 0) const {
        return { vec_.data(), size == 0 ? vec_.size() : size };
    }

private:
    std::vector<char> vec_;
};