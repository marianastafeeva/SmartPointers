#pragma once

#include <cstddef>
#include <type_traits>
#include <utility>
#include <unordered_map>
#include <stdexcept>

namespace lab {

template <typename T>
class SmrtStorage {
public:
    static SmrtStorage& instance() {
        static SmrtStorage storage;
        return storage;
    }

    T* add(T* raw) {
        map_[raw] = 1;
        return raw;
    }

    void addRef(T* raw) {
        auto it = map_.find(raw);
        if (it != map_.end()) {
            ++(it->second);
        }
    }

    void release(T* raw) {
        if (raw == nullptr) return;

        auto it = map_.find(raw);
        if (it == map_.end()) return;

        if (--(it->second) == 0) {
            map_.erase(it);
            delete raw;
        }
    }

    std::size_t count(T* raw) const {
        auto it = map_.find(raw);
        return it == map_.end() ? 0 : it->second;
    }

    std::size_t size() const {
        return map_.size();
    }

private:
    SmrtStorage() = default;
    SmrtStorage(const SmrtStorage&) = delete;
    SmrtStorage& operator=(const SmrtStorage&) = delete;

    std::unordered_map<T*, std::size_t> map_;
};

template <typename T>

class SmrtPtr {
public:
    SmrtPtr() noexcept : ptr_(nullptr) {}

    explicit SmrtPtr(T* raw) : ptr_(nullptr) {
        if (raw != nullptr) {
            SmrtStorage<T>::instance().add(raw);
            ptr_ = raw;
        }
    }

    SmrtPtr(const SmrtPtr& other) noexcept : ptr_(other.ptr_) {
        if (ptr_) SmrtStorage<T>::instance().addRef(ptr_);
    }

    template <typename U>
        requires std::is_convertible_v<U*, T*>
    SmrtPtr(const SmrtPtr<U>& other) noexcept
        : ptr_(static_cast<T*>(other.get())) {
        if (ptr_) SmrtStorage<T>::instance().addRef(ptr_);
    }

    SmrtPtr(SmrtPtr&& other) noexcept : ptr_(other.ptr_) {
        other.ptr_ = nullptr;
    }

    SmrtPtr& operator=(const SmrtPtr& other) noexcept {
        if (this != &other) {
            release();
            ptr_ = other.ptr_;
            if (ptr_) SmrtStorage<T>::instance().addRef(ptr_);
        }
        return *this;
    }

    SmrtPtr& operator=(SmrtPtr&& other) noexcept {
        if (this != &other) {
            release();
            ptr_ = other.ptr_;
            other.ptr_ = nullptr;
        }
        return *this;
    }

    ~SmrtPtr() {
        release();
    }

    [[nodiscard]] T* get() const noexcept { return ptr_; }
    [[nodiscard]] T& operator*()  const { return *ptr_; }
    [[nodiscard]] T* operator->() const noexcept { return ptr_; }

    [[nodiscard]] explicit operator bool() const noexcept {
        return ptr_ != nullptr;
    }

    [[nodiscard]] std::size_t use_count() const noexcept {
        return ptr_ ? SmrtStorage<T>::instance().count(ptr_) : 0;
    }

    void reset() noexcept {
        release();
    }

    void reset(T* raw) {
        release();
        if (raw != nullptr) {
            SmrtStorage<T>::instance().add(raw);
            ptr_ = raw;
        }
    }

    void swap(SmrtPtr& other) noexcept {
        std::swap(ptr_, other.ptr_);
    }

private:
    void release() noexcept {
        if (ptr_) {
            SmrtStorage<T>::instance().release(ptr_);
            ptr_ = nullptr;
        }
    }

    template <typename U>
    friend class SmrtPtr;

    T* ptr_;
};

template <typename T, typename... Args>
SmrtPtr<T> makeSmrt(Args&&... args) {
    return SmrtPtr<T>(new T(std::forward<Args>(args)...));
}

template <typename T>
void swap(SmrtPtr<T>& a, SmrtPtr<T>& b) noexcept {
    a.swap(b);
}

}