#pragma once

#include "UnqPtr.h"
#include "ShrdPtr.h"

#include <cstddef>
#include <utility>
#include <stdexcept>

namespace lab {

template <typename T>
class MsPtr;

template <typename T>
class MemorySpan {
public:
    MemorySpan() noexcept : data_(nullptr), size_(0), capacity_(0) {}

    explicit MemorySpan(std::size_t capacity)
        : data_(capacity ? new T[capacity]() : nullptr),
          size_(capacity),
          capacity_(capacity) {}

    MemorySpan(const T* src, std::size_t count)
        : data_(count ? new T[count] : nullptr),
          size_(count),
          capacity_(count) {
        for (std::size_t i = 0; i < count; ++i) {
            data_[i] = src[i];
        }
    }

    MemorySpan(std::initializer_list<T> init)
        : data_(init.size() ? new T[init.size()] : nullptr),
          size_(init.size()),
          capacity_(init.size()) {
        std::size_t i = 0;
        for (const auto& v : init) {
            data_[i++] = v;
        }
    }

    ~MemorySpan() {
        delete[] data_;
    }

    MemorySpan(const MemorySpan&)= delete;
    MemorySpan& operator=(const MemorySpan&) = delete;

    MemorySpan(MemorySpan&& other) noexcept
        : data_(other.data_), size_(other.size_), capacity_(other.capacity_) {
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }

    MemorySpan& operator=(MemorySpan&& other) noexcept {
        if (this != &other) {
            delete[] data_;
            data_ = other.data_;
            size_ = other.size_;
            capacity_ = other.capacity_;
            other.data_ = nullptr;
            other.size_ = 0;
            other.capacity_ = 0;
        }
        return *this;
    }

    [[nodiscard]] std::size_t size() const noexcept { return size_; }
    [[nodiscard]] std::size_t capacity() const noexcept { return capacity_; }
    [[nodiscard]] bool empty() const noexcept { return size_ == 0; }

    T& at(std::size_t index) {
        if (index >= size_) {
            throw std::out_of_range("MemorySpan::at: index out of range");
        }
        return data_[index];
    }

    const T& at(std::size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("MemorySpan::at: index out of range");
        }
        return data_[index];
    }

    void set(std::size_t index, const T& value) {
        at(index) = value;
    }

    ShrdPtr<T> Copy(std::size_t index) {
        return makeShrd<T>(at(index));
    }

    UnqPtr<T> Get(std::size_t index) {
        return makeUnq<T>(at(index));
    }

    MsPtr<T> Locate(std::size_t index);

    T* data() noexcept { return data_; }
    const T* data() const noexcept { return data_; }

private:
    T* data_;
    std::size_t size_;
    std::size_t capacity_;

    template <typename U>
    friend class MsPtr;
};

}