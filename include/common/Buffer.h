#pragma once

#include <cstddef>
#include <vector>
#include <string>

template<typename T>
struct BufferT {
    T* data;
    size_t size;

    BufferT() : data(nullptr), 0 {}

    BufferT(T* ptr, size_t len) : data(ptr), size(len) {}

    explicit BufferT(std::vector<T>& v) : data(v.data()), size(v.size()) {}

    [[nodiscard]] BufferT<T> slice(size_t new_size) const {
        return BufferT<T>(data, new_size);
    }

    operator BufferT<const T>() const { // NOLINT(*-explicit-constructor)
        return BufferT<const T>(data, size);
    }
};

using Buffer = BufferT<char>;
using ConstBuffer = BufferT<const char>;
