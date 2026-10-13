#pragma once

#include "UnqPtr.h"

#include <utility>
#include <type_traits>
#include <cstddef>
#include <stdexcept>

namespace lab {

template <typename T>
class ShrdPtr {
public:
    ShrdPtr() noexcept : ptr_(nullptr), count_(nullptr) {}

    explicit ShrdPtr(UnqPtr<T>&& owner)
        : ptr_(owner.release()), count_(new std::size_t(1)) {}

    template <typename U>
        requires std::is_convertible_v<U*, T*>
    explicit ShrdPtr(UnqPtr<U>&& owner)
        : ptr_(owner.release()), count_(new std::size_t(1)) {}

    ShrdPtr(const ShrdPtr& other) noexcept
        : ptr_(other.ptr_), count_(other.count_) {
        if (count_) ++(*count_);
    }

    template <typename U>
        requires std::is_convertible_v<U*, T*>
    ShrdPtr(const ShrdPtr<U>& other) noexcept
        : ptr_(other.ptr_), count_(other.count_) {
        if (count_) ++(*count_);
    }

    ShrdPtr(ShrdPtr&& other) noexcept
        : ptr_(other.ptr_), count_(other.count_) {
        other.ptr_ = nullptr;
        other.count_ = nullptr;
    }

    template <typename U>
        requires std::is_convertible_v<U*, T*>
    ShrdPtr(ShrdPtr<U>&& other) noexcept
        : ptr_(other.ptr_), count_(other.count_) {
        other.ptr_ = nullptr;
        other.count_ = nullptr;
    }

    ShrdPtr& operator=(const ShrdPtr& other) noexcept {
        if (this != &other) {
            release();
            ptr_ = other.ptr_;
            count_ = other.count_;
            if (count_) ++(*count_);
        }
        return *this;
    }

    template <typename U>
        requires std::is_convertible_v<U*, T*>
    ShrdPtr& operator=(const ShrdPtr<U>& other) noexcept {
        if (static_cast<const void*>(this) != static_cast<const void*>(&other)) {
            release();
            ptr_ = other.ptr_;
            count_ = other.count_;
            if (count_) ++(*count_);
        }
        return *this;
    }

    ShrdPtr& operator=(ShrdPtr&& other) noexcept {
        if (this != &other) {
            release();
            ptr_ = other.ptr_;
            count_ = other.count_;
            other.ptr_ = nullptr;
            other.count_ = nullptr;
        }
        return *this;
    }

    template <typename U>
        requires std::is_convertible_v<U*, T*>
    ShrdPtr& operator=(ShrdPtr<U>&& other) noexcept {
        if (static_cast<const void*>(this) != static_cast<const void*>(&other)) {
            release();
            ptr_ = other.ptr_;
            count_ = other.count_;
            other.ptr_ = nullptr;
            other.count_ = nullptr;
        }
        return *this;
    }

    ~ShrdPtr() {
        release();
    }

    [[nodiscard]] T* get() const noexcept { return ptr_; }
    [[nodiscard]] T& operator*()  const { return *ptr_; }
    [[nodiscard]] T* operator->() const noexcept { return ptr_; }

    [[nodiscard]] explicit operator bool() const noexcept {
        return ptr_ != nullptr;
    }

    [[nodiscard]] std::size_t use_count() const noexcept {
        return count_ ? *count_ : 0;
    }

    void reset() noexcept {
        release();
    }

    void reset(UnqPtr<T>&& owner) {
        release();
        ptr_ = owner.release();
        count_ = new std::size_t(1);
    }

    void swap(ShrdPtr& other) noexcept {
        std::swap(ptr_, other.ptr_);
        std::swap(count_, other.count_);
    }

private:
    void release() noexcept {
        if (count_) {
            if (--(*count_) == 0) {
                delete ptr_;
                delete count_;
            }
            ptr_ = nullptr;
            count_ = nullptr;
        }
    }

    template <typename U>
    friend class ShrdPtr;

    T* ptr_;
    std::size_t* count_;
};

template <typename T, typename... Args>
ShrdPtr<T> makeShrd(Args&&... args) {
    return ShrdPtr<T>(makeUnq<T>(std::forward<Args>(args)...));
}

template <typename T>
void swap(ShrdPtr<T>& a, ShrdPtr<T>& b) noexcept {
    a.swap(b);
}

template <typename T>
class ShrdPtr<T[]> {
public:
    ShrdPtr() noexcept : ptr_(nullptr), size_(0), count_(nullptr) {}

    explicit ShrdPtr(UnqPtr<T[]>&& owner)
        : ptr_(nullptr),
          size_(owner.size()),
          count_(nullptr) {
        ptr_ = owner.release();
        count_ = new std::size_t(1);
    }

    ShrdPtr(const ShrdPtr& other) noexcept
        : ptr_(other.ptr_), size_(other.size_), count_(other.count_) {
        if (count_) ++(*count_);
    }

    ShrdPtr(ShrdPtr&& other) noexcept
        : ptr_(other.ptr_), size_(other.size_), count_(other.count_) {
        other.ptr_ = nullptr;
        other.size_ = 0;
        other.count_ = nullptr;
    }

    ShrdPtr& operator=(const ShrdPtr& other) noexcept {
        if (this != &other) {
            release();
            ptr_ = other.ptr_;
            size_ = other.size_;
            count_ = other.count_;
            if (count_) ++(*count_);
        }
        return *this;
    }

    ShrdPtr& operator=(ShrdPtr&& other) noexcept {
        if (this != &other) {
            release();
            ptr_ = other.ptr_;
            size_ = other.size_;
            count_ = other.count_;
            other.ptr_ = nullptr;
            other.size_ = 0;
            other.count_ = nullptr;
        }
        return *this;
    }

    ~ShrdPtr() {
        release();
    }

    [[nodiscard]] T* get() const noexcept { return ptr_; }

    [[nodiscard]] std::size_t size() const noexcept { return size_; }

    [[nodiscard]] T& operator[](std::size_t index) {
        if (index >= size_) {
            throw std::out_of_range("ShrdPtr<T[]>::operator[]: index out of range");
        }
        return ptr_[index];
    }

    [[nodiscard]] const T& operator[](std::size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("ShrdPtr<T[]>::operator[]: index out of range");
        }
        return ptr_[index];
    }

    [[nodiscard]] explicit operator bool() const noexcept {
        return ptr_ != nullptr;
    }

    [[nodiscard]] std::size_t use_count() const noexcept {
        return count_ ? *count_ : 0;
    }

    void reset() noexcept {
        release();
    }

    void reset(UnqPtr<T[]>&& owner) {
        release();
        size_ = owner.size();
        ptr_ = owner.release();
        count_ = new std::size_t(1);
    }

    void swap(ShrdPtr& other) noexcept {
        std::swap(ptr_, other.ptr_);
        std::swap(size_, other.size_);
        std::swap(count_, other.count_);
    }

private:
    void release() noexcept {
        if (count_) {
            if (--(*count_) == 0) {
                delete[] ptr_;
                delete count_;
            }
            ptr_ = nullptr;
            size_ = 0;
            count_ = nullptr;
        }
    }

    T* ptr_;
    std::size_t size_;
    std::size_t* count_;
};

template <typename T>
ShrdPtr<T[]> makeShrdArr(std::size_t count) {
    return ShrdPtr<T[]>(makeUnqArr<T>(count));
}

template <typename T>
void swap(ShrdPtr<T[]>& a, ShrdPtr<T[]>& b) noexcept {
    a.swap(b);
}

}