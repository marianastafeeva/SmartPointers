#pragma once

#include "MemorySpan.h"

#include <cstddef>
#include <stdexcept>

namespace lab {

template <typename T>
class MsPtr {
public:
    MsPtr() noexcept : span_(nullptr), index_(0) {}

    MsPtr(MemorySpan<T>* span, std::size_t index)
        : span_(span), index_(index) {
        if (span_ && index_ > span_->size_) {
            throw std::out_of_range("MsPtr: index out of range");
        }
    }

    [[nodiscard]] T& operator*() const {
        checkDereference();
        return span_->data_[index_];
    }

    [[nodiscard]] T* operator->() const {
        checkDereference();
        return &span_->data_[index_];
    }

    [[nodiscard]] T& operator[](std::size_t offset) const {
        return *(*this + offset);
    }

    MsPtr& operator++() {
        checkIncrement();
        ++index_;
        return *this;
    }

    MsPtr operator++(int) {
        MsPtr tmp = *this;
        ++(*this);
        return tmp;
    }

    MsPtr& operator--() {
        checkDecrement();
        --index_;
        return *this;
    }

    MsPtr operator--(int) {
        MsPtr tmp = *this;
        --(*this);
        return tmp;
    }

    MsPtr& operator+=(std::ptrdiff_t offset) {
        const auto newIndex = static_cast<std::ptrdiff_t>(index_) + offset;
        checkBounds(newIndex);
        index_ = static_cast<std::size_t>(newIndex);
        return *this;
    }

    MsPtr& operator-=(std::ptrdiff_t offset) {
        return (*this += -offset);
    }

    [[nodiscard]] MsPtr operator+(std::ptrdiff_t offset) const {
        MsPtr tmp = *this;
        tmp += offset;
        return tmp;
    }

    [[nodiscard]] MsPtr operator-(std::ptrdiff_t offset) const {
        MsPtr tmp = *this;
        tmp -= offset;
        return tmp;
    }

    [[nodiscard]] std::ptrdiff_t operator-(const MsPtr& other) const {
        return static_cast<std::ptrdiff_t>(index_)
             - static_cast<std::ptrdiff_t>(other.index_);
    }

    [[nodiscard]] bool operator==(const MsPtr& other) const noexcept {
        return span_ == other.span_ && index_ == other.index_;
    }

    [[nodiscard]] bool operator!=(const MsPtr& other) const noexcept {
        return !(*this == other);
    }

    [[nodiscard]] bool operator<(const MsPtr& other) const {
        checkSameSpan(other);
        return index_ < other.index_;
    }

    [[nodiscard]] bool operator<=(const MsPtr& other) const {
        return !(other < *this);
    }

    [[nodiscard]] bool operator>(const MsPtr& other) const {
        return other < *this;
    }

    [[nodiscard]] bool operator>=(const MsPtr& other) const {
        return !(*this < other);
    }

    [[nodiscard]] std::size_t index() const noexcept { return index_; }
    [[nodiscard]] MemorySpan<T>* span() const noexcept { return span_; }
    [[nodiscard]] bool valid() const noexcept { return span_ != nullptr; }

    [[nodiscard]] explicit operator bool() const noexcept {
        return valid() && index_ < span_->size_;
    }

    void reset() noexcept {
        span_ = nullptr;
        index_ = 0;
    }

private:
    void checkDereference() const {
        if (!span_) {
            throw std::runtime_error("MsPtr: dereferencing null pointer");
        }
        if (index_ >= span_->size_) {
            throw std::out_of_range("MsPtr: dereference out of range");
        }
    }

    void checkIncrement() const {
        if (!span_) {
            throw std::runtime_error("MsPtr: incrementing null pointer");
        }
        if (index_ >= span_->size_) {
            throw std::out_of_range("MsPtr: increment past end");
        }
    }

    void checkDecrement() const {
        if (!span_) {
            throw std::runtime_error("MsPtr: decrementing null pointer");
        }
        if (index_ == 0) {
            throw std::out_of_range("MsPtr: decrement before begin");
        }
    }

    void checkBounds(std::ptrdiff_t newIndex) const {
        if (!span_) {
            throw std::runtime_error("MsPtr: arithmetic on null pointer");
        }
        if (newIndex < 0
            || newIndex > static_cast<std::ptrdiff_t>(span_->size_)) {
            throw std::out_of_range("MsPtr: arithmetic out of range");
        }
    }

    void checkSameSpan(const MsPtr& other) const {
        if (span_ != other.span_) {
            throw std::runtime_error("MsPtr: comparing pointers to different spans");
        }
    }

    MemorySpan<T>* span_;
    std::size_t index_;

    template <typename U>
    friend class MemorySpan;
};

template <typename T>
MsPtr<T> MemorySpan<T>::Locate(std::size_t index) {
    at(index);
    return MsPtr<T>(this, index);
}

}