#pragma once

#include "ShrdPtr.h"
#include "UnqPtr.h"
#include "SmrtPtr.h"

#include <cstddef>
#include <stdexcept>
#include <initializer_list>
#include <utility>

namespace lab {

template <typename T>
class DynamicArray {
public:
    DynamicArray() noexcept : data_(nullptr), size_(0), capacity_(0) {}

    explicit DynamicArray(std::size_t count)
        : data_(count ? new T[count]() : nullptr),
          size_(count),
          capacity_(count) {}

    DynamicArray(std::initializer_list<T> init)
        : data_(init.size() ? new T[init.size()] : nullptr),
          size_(init.size()),
          capacity_(init.size()) {
        std::size_t i = 0;
        for (const auto& v : init) {
            data_[i++] = v;
        }
    }

    ~DynamicArray() {
        delete[] data_;
    }

    DynamicArray(const DynamicArray&) = delete;
    DynamicArray& operator=(const DynamicArray&) = delete;

    DynamicArray(DynamicArray&& other) noexcept
        : data_(other.data_), size_(other.size_), capacity_(other.capacity_) {
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }

    DynamicArray& operator=(DynamicArray&& other) noexcept {
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
        checkIndex(index);
        return data_[index];
    }

    const T& at(std::size_t index) const {
        checkIndex(index);
        return data_[index];
    }

    T& operator[](std::size_t index) noexcept { return data_[index]; }
    const T& operator[](std::size_t index) const noexcept { return data_[index]; }

    void push_back(const T& value) {
        if (size_ == capacity_) {
            grow(capacity_ ? capacity_ * 2 : 1);
        }
        data_[size_++] = value;
    }

    void push_back(T&& value) {
        if (size_ == capacity_) {
            grow(capacity_ ? capacity_ * 2 : 1);
        }
        data_[size_++] = std::move(value);
    }

    void pop_back() {
        if (size_ == 0) {
            throw std::out_of_range("DynamicArray::pop_back: empty");
        }
        --size_;
    }

    void clear() noexcept {
        size_ = 0;
    }

    void reserve(std::size_t newCapacity) {
        if (newCapacity > capacity_) {
            grow(newCapacity);
        }
    }

    ShrdPtr<T> Copy(std::size_t index) {
        return makeShrd<T>(at(index));
    }

    UnqPtr<T> Get(std::size_t index) {
        return makeUnq<T>(at(index));
    }

    SmrtPtr<T> Locate(std::size_t index) {
        return makeSmrt<T>(at(index));
    }

    T* data() noexcept { return data_; }
    const T* data() const noexcept { return data_; }

private:
    void checkIndex(std::size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("DynamicArray: index out of range");
        }
    }

    void grow(std::size_t newCapacity) {
        T* newData = new T[newCapacity];
        for (std::size_t i = 0; i < size_; ++i) {
            newData[i] = std::move(data_[i]);
        }
        delete[] data_;
        data_ = newData;
        capacity_ = newCapacity;
    }

    T* data_;
    std::size_t size_;
    std::size_t capacity_;
};

template <typename T>
class LinkedList {
public:
    LinkedList() noexcept : head_(nullptr), size_(0) {}

    LinkedList(std::initializer_list<T> init) : LinkedList() {
        for (const auto& v : init) {
            push_back(v);
        }
    }

    ~LinkedList() {
        clear();
    }

    LinkedList(const LinkedList&) = delete;
    LinkedList& operator=(const LinkedList&) = delete;

    LinkedList(LinkedList&& other) noexcept
        : head_(other.head_), size_(other.size_) {
        other.head_ = nullptr;
        other.size_ = 0;
    }

    LinkedList& operator=(LinkedList&& other) noexcept {
        if (this != &other) {
            clear();
            head_ = other.head_;
            size_ = other.size_;
            other.head_ = nullptr;
            other.size_ = 0;
        }
        return *this;
    }

    [[nodiscard]] std::size_t size() const noexcept { return size_; }
    [[nodiscard]] bool empty() const noexcept { return size_ == 0; }

    void push_front(const T& value) {
        head_ = new Node(value, head_);
        ++size_;
    }

    void push_back(const T& value) {
        Node* newNode = new Node(value, nullptr);
        if (!head_) {
            head_ = newNode;
        } else {
            Node* cur = head_;
            while (cur->next) {
                cur = cur->next;
            }
            cur->next = newNode;
        }
        ++size_;
    }

    void pop_front() {
        if (!head_) {
            throw std::out_of_range("LinkedList::pop_front: empty");
        }
        Node* old = head_;
        head_ = head_->next;
        delete old;
        --size_;
    }

    void clear() noexcept {
        while (head_) {
            Node* next = head_->next;
            delete head_;
            head_ = next;
        }
        size_ = 0;
    }

    T& at(std::size_t index) {
        return nodeAt(index)->value;
    }

    const T& at(std::size_t index) const {
        return nodeAt(index)->value;
    }

    ShrdPtr<T> Copy(std::size_t index) {
        return makeShrd<T>(at(index));
    }

    UnqPtr<T> Get(std::size_t index) {
        return makeUnq<T>(at(index));
    }

    SmrtPtr<T> Locate(std::size_t index) {
        return makeSmrt<T>(at(index));
    }

private:
    struct Node {
        T value;
        Node* next;
        Node(const T& v, Node* n) : value(v), next(n) {}
    };

    Node* nodeAt(std::size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("LinkedList: index out of range");
        }
        Node* cur = head_;
        for (std::size_t i = 0; i < index; ++i) {
            cur = cur->next;
        }
        return cur;
    }

    Node* head_;
    std::size_t size_;
};

}