#pragma once

#include <utility>
#include <type_traits>
#include <cstddef>
#include <stdexcept>

namespace lab {

template <typename T>
class UnqPtr {
public:
    constexpr UnqPtr() noexcept : ptr_(nullptr) {}

    explicit UnqPtr(T* raw) noexcept : ptr_(raw) {}

    UnqPtr(UnqPtr&& other) noexcept : ptr_(other.ptr_) {
        other.ptr_ = nullptr;
    }

    template <typename U>
        requires std::is_convertible_v<U*, T*>
    UnqPtr(UnqPtr<U>&& other) noexcept : ptr_(other.release()) {}

    UnqPtr(const UnqPtr&)            = delete;
    UnqPtr& operator=(const UnqPtr&) = delete;

    UnqPtr& operator=(UnqPtr&& other) noexcept {
        if (this != &other) {
            reset(other.release());
        }
        return *this;
    }

    template <typename U>
        requires std::is_convertible_v<U*, T*>
    UnqPtr& operator=(UnqPtr<U>&& other) noexcept {
        reset(other.release());
        return *this;
    }

    ~UnqPtr() {
        delete ptr_;
    }

    [[nodiscard]] T* get() const noexcept { return ptr_; }

    [[nodiscard]] T& operator*()  const { return *ptr_; }
    [[nodiscard]] T* operator->() const noexcept { return ptr_; }

    [[nodiscard]] explicit operator bool() const noexcept {
        return ptr_ != nullptr;
    }

    T* release() noexcept {
        T* tmp = ptr_;
        ptr_ = nullptr;
        return tmp;
    }

    void reset(T* raw = nullptr) noexcept {
        if (ptr_ != raw) {
            delete ptr_;
            ptr_ = raw;
        }
    }

    void swap(UnqPtr& other) noexcept {
        std::swap(ptr_, other.ptr_);
    }

    template <typename U>
    friend class ShrdPtr;

private:
    T* ptr_;
};

template <typename T, typename... Args>
UnqPtr<T> makeUnq(Args&&... args) {
    return UnqPtr<T>(new T(std::forward<Args>(args)...));
}

template <typename T>
void swap(UnqPtr<T>& a, UnqPtr<T>& b) noexcept {
    a.swap(b);
}

template <typename T>
class UnqPtr<T[]> {
public:
    constexpr UnqPtr() noexcept : ptr_(nullptr), size_(0) {}

    UnqPtr(T* raw, std::size_t size) noexcept
        : ptr_(raw), size_(size) {}

    UnqPtr(UnqPtr&& other) noexcept
        : ptr_(other.ptr_), size_(other.size_) {
        other.ptr_ = nullptr;
        other.size_ = 0;
    }

    UnqPtr(const UnqPtr&)            = delete;
    UnqPtr& operator=(const UnqPtr&) = delete;

    UnqPtr& operator=(UnqPtr&& other) noexcept {
        if (this != &other) {
            std::size_t otherSize = other.size_;
            T* otherPtr = other.release();
            reset(otherPtr, otherSize);
        }
        return *this;
    }

    ~UnqPtr() {
        delete[] ptr_;
    }

    [[nodiscard]] T* get() const noexcept { return ptr_; }

    [[nodiscard]] std::size_t size() const noexcept { return size_; }

    [[nodiscard]] T& operator[](std::size_t index) {
        if (index >= size_) {
            throw std::out_of_range("UnqPtr<T[]>::operator[]: index out of range");
        }
        return ptr_[index];
    }

    [[nodiscard]] const T& operator[](std::size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("UnqPtr<T[]>::operator[]: index out of range");
        }
        return ptr_[index];
    }

    [[nodiscard]] explicit operator bool() const noexcept {
        return ptr_ != nullptr;
    }

    T* release() noexcept {
        T* tmp = ptr_;
        ptr_ = nullptr;
        size_ = 0;
        return tmp;
    }

    void reset(T* raw, std::size_t size) noexcept {
        if (ptr_ != raw) {
            delete[] ptr_;
            ptr_ = raw;
            size_ = size;
        }
    }

    void reset() noexcept {
        if (ptr_) {
            delete[] ptr_;
            ptr_ = nullptr;
            size_ = 0;
        }
    }

    void swap(UnqPtr& other) noexcept {
        std::swap(ptr_, other.ptr_);
        std::swap(size_, other.size_);
    }

    template <typename U>
    friend class ShrdPtr;

private:
    T* ptr_;
    std::size_t size_;
};

template <typename T>
UnqPtr<T[]> makeUnqArr(std::size_t count) {
    return UnqPtr<T[]>(count ? new T[count]() : nullptr, count);
}

template <typename T>
void swap(UnqPtr<T[]>& a, UnqPtr<T[]>& b) noexcept {
    a.swap(b);
}

}