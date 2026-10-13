#include "demo.h"

#include <iostream>
#include <limits>
#include <string>
#include <utility>
#include <vector>

#include "UnqPtr.h"
#include "ShrdPtr.h"
#include "SmrtPtr.h"
#include "MemorySpan.h"
#include "MsPtr.h"
#include "Sequence.h"

static void pause() {
    std::cout << "\nPress Enter to continue...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

static int readInt(const std::string& prompt) {
    int value = 0;
    while (true) {
        std::cout << prompt;
        if (std::cin >> value) {
            return value;
        }
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Invalid input. Please enter a number.\n";
    }
}

void demoUnqPtr() {
    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "  Demo: UnqPtr\n";
    std::cout << "========================================\n";

    std::cout << "\n[1] Creating UnqPtr with a value\n";
    const int initial = readInt("    Enter a number: ");
    lab::UnqPtr<int> p(new int(initial));
    std::cout << "    *p = " << *p << "\n";
    std::cout << "    p is valid? " << (p ? "yes" : "no") << "\n";

    pause();

    std::cout << "\n[2] Moving ownership to another UnqPtr\n";
    lab::UnqPtr<int> q = std::move(p);
    std::cout << "    After move:\n";
    std::cout << "    p is valid? " << (p ? "yes" : "no") << "\n";
    std::cout << "    q is valid? " << (q ? "yes" : "no") << "\n";
    std::cout << "    *q = " << *q << "\n";

    pause();

    std::cout << "\n[3] Replacing the value with reset()\n";
    const int replacement = readInt("    Enter a new number: ");
    q.reset(new int(replacement));
    std::cout << "    *q = " << *q << "\n";

    pause();

    std::cout << "\n[4] Releasing ownership\n";
    int* raw = q.release();
    std::cout << "    After release:\n";
    std::cout << "    q is valid? " << (q ? "yes" : "no") << "\n";
    std::cout << "    raw value = " << *raw << "\n";
    std::cout << "    Deleting raw manually...\n";
    delete raw;

    pause();

    std::cout << "\n[5] Creating with makeUnq\n";
    const int factoryValue = readInt("    Enter a number: ");
    auto ptr = lab::makeUnq<int>(factoryValue);
    std::cout << "    *ptr = " << *ptr << "\n";

    std::cout << "\n=== Demo UnqPtr finished ===\n";
    pause();
}

void demoShrdPtr() {
    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "  Demo: ShrdPtr\n";
    std::cout << "========================================\n";

    std::cout << "\n[1] Creating ShrdPtr from UnqPtr\n";
    const int initial = readInt("    Enter a number: ");
    lab::ShrdPtr<int> a = lab::makeShrd<int>(initial);
    std::cout << "    *a = " << *a << "\n";
    std::cout << "    a.use_count() = " << a.use_count() << "\n";

    pause();

    std::cout << "\n[2] Copying (shared ownership)\n";
    lab::ShrdPtr<int> b = a;
    std::cout << "    After copy:\n";
    std::cout << "    a.use_count() = " << a.use_count() << "\n";
    std::cout << "    b.use_count() = " << b.use_count() << "\n";
    std::cout << "    *a = " << *a << ", *b = " << *b << "\n";

    pause();

    std::cout << "\n[3] More copies\n";
    const int extra = readInt("    How many more copies? ");
    std::vector<lab::ShrdPtr<int>> copies;
    for (int i = 0; i < extra; ++i) {
        copies.push_back(a);
    }
    std::cout << "    a.use_count() = " << a.use_count() << "\n";

    pause();

    std::cout << "\n[4] Destroying extra copies\n";
    copies.clear();
    std::cout << "    After clear:\n";
    std::cout << "    a.use_count() = " << a.use_count() << "\n";

    pause();

    std::cout << "\n[5] Leaving inner scope\n";
    {
        lab::ShrdPtr<int> c = a;
        std::cout << "    Inside scope: a.use_count() = " << a.use_count() << "\n";
    }
    std::cout << "    Outside scope: a.use_count() = " << a.use_count() << "\n";

    pause();

    std::cout << "\n[6] Moving ownership\n";
    lab::ShrdPtr<int> d = std::move(a);
    std::cout << "    After move:\n";
    std::cout << "    a is valid? " << (a ? "yes" : "no") << "\n";
    std::cout << "    a.use_count() = " << a.use_count() << "\n";
    std::cout << "    d.use_count() = " << d.use_count() << "\n";
    std::cout << "    *d = " << *d << "\n";

    std::cout << "\n=== Demo ShrdPtr finished ===\n";
    pause();
}

void demoSmrtPtr() {
    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "  Demo: SmrtPtr\n";
    std::cout << "========================================\n";

    auto& storage = lab::SmrtStorage<int>::instance();

    std::cout << "\n[1] Creating SmrtPtr from raw pointer\n";
    const int initial = readInt("    Enter a number: ");
    lab::SmrtPtr<int> a(new int(initial));
    std::cout << "    *a = " << *a << "\n";
    std::cout << "    a.use_count() = " << a.use_count() << "\n";
    std::cout << "    storage.size() = " << storage.size() << "\n";

    pause();

    std::cout << "\n[2] Copying (shared ownership)\n";
    lab::SmrtPtr<int> b = a;
    std::cout << "    After copy:\n";
    std::cout << "    a.use_count() = " << a.use_count() << "\n";
    std::cout << "    b.use_count() = " << b.use_count() << "\n";
    std::cout << "    storage.size() = " << storage.size() << "\n";

    pause();

    std::cout << "\n[3] More copies stored in a vector\n";
    const int extra = readInt("    How many more copies? ");
    std::vector<lab::SmrtPtr<int>> copies;
    for (int i = 0; i < extra; ++i) {
        copies.push_back(a);
    }
    std::cout << "    a.use_count() = " << a.use_count() << "\n";
    std::cout << "    storage.size() = " << storage.size() << "\n";

    pause();

    std::cout << "\n[4] Destroying extra copies\n";
    copies.clear();
    std::cout << "    After clear:\n";
    std::cout << "    a.use_count() = " << a.use_count() << "\n";
    std::cout << "    storage.size() = " << storage.size() << "\n";

    pause();

    std::cout << "\n[5] Leaving inner scope\n";
    {
        lab::SmrtPtr<int> c = a;
        std::cout << "    Inside scope: a.use_count() = " << a.use_count() << "\n";
        std::cout << "    Inside scope: storage.size() = " << storage.size() << "\n";
    }
    std::cout << "    Outside scope: a.use_count() = " << a.use_count() << "\n";
    std::cout << "    Outside scope: storage.size() = " << storage.size() << "\n";

    pause();

    std::cout << "\n[6] Moving ownership\n";
    lab::SmrtPtr<int> d = std::move(a);
    std::cout << "    After move:\n";
    std::cout << "    a is valid? " << (a ? "yes" : "no") << "\n";
    std::cout << "    a.use_count() = " << a.use_count() << "\n";
    std::cout << "    d.use_count() = " << d.use_count() << "\n";
    std::cout << "    storage.size() = " << storage.size() << "\n";

    pause();

    std::cout << "\n[7] Destroying the last owner\n";
    std::cout << "    Before inner scope: storage.size() = " << storage.size() << "\n";
    {
        lab::SmrtPtr<int> last = d;
        lab::SmrtPtr<int> another = b;
        std::cout << "    Inside: use_count = " << d.use_count()
                  << ", storage.size() = " << storage.size() << "\n";
    }
    std::cout << "    After inner scope: d.use_count() = " << d.use_count() << "\n";
    std::cout << "    After inner scope: storage.size() = " << storage.size() << "\n";

    std::cout << "\n    Now resetting d and b...\n";
    d.reset();
    b.reset();
    std::cout << "    After reset: d is valid? " << (d ? "yes" : "no") << "\n";
    std::cout << "    After reset: b is valid? " << (b ? "yes" : "no") << "\n";
    std::cout << "    After reset: storage.size() = " << storage.size() << "\n";

    std::cout << "\n=== Demo SmrtPtr finished ===\n";
    pause();
}

void demoMemorySpan() {
    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "  Demo: MemorySpan\n";
    std::cout << "========================================\n";

    std::cout << "\n[1] Creating MemorySpan from initializer list\n";
    lab::MemorySpan<int> span{10, 20, 30, 40, 50};
    std::cout << "    span.size() = " << span.size() << "\n";
    std::cout << "    span.capacity() = " << span.capacity() << "\n";
    std::cout << "    span.empty()? " << (span.empty() ? "yes" : "no") << "\n";
    std::cout << "    Elements: ";
    for (std::size_t i = 0; i < span.size(); ++i) {
        std::cout << span.at(i) << " ";
    }
    std::cout << "\n";

    pause();

    std::cout << "\n[2] Changing an element with set()\n";
    const std::size_t idx = static_cast<std::size_t>(
        readInt("    Enter index to change (0-4): "));
    const int newValue = readInt("    Enter new value: ");
    try {
        span.set(idx, newValue);
        std::cout << "    After set: span.at(" << idx << ") = " << span.at(idx) << "\n";
    } catch (const std::out_of_range& e) {
        std::cout << "    Error: " << e.what() << "\n";
    }

    pause();

    std::cout << "\n[3] Accessing element out of range\n";
    try {
        const auto bad = span.at(100);
        std::cout << "    span.at(100) = " << bad << "\n";
    } catch (const std::out_of_range& e) {
        std::cout << "    Caught exception: " << e.what() << "\n";
    }

    pause();

    std::cout << "\n[4] Copy returns ShrdPtr (shared copy of the element)\n";
    const std::size_t copyIdx = static_cast<std::size_t>(
        readInt("    Enter index to copy: "));
    try {
        auto cp = span.Copy(copyIdx);
        std::cout << "    *cp = " << *cp << "\n";
        std::cout << "    cp.use_count() = " << cp.use_count() << "\n";
        {
            auto cp2 = cp;
            std::cout << "    After another copy: cp.use_count() = " << cp.use_count() << "\n";
        }
        std::cout << "    After scope: cp.use_count() = " << cp.use_count() << "\n";
    } catch (const std::out_of_range& e) {
        std::cout << "    Error: " << e.what() << "\n";
    }

    pause();

    std::cout << "\n[5] Get returns UnqPtr (unique copy)\n";
    const std::size_t getIdx = static_cast<std::size_t>(
        readInt("    Enter index to get: "));
    try {
        auto un = span.Get(getIdx);
        std::cout << "    *un = " << *un << "\n";
    } catch (const std::out_of_range& e) {
        std::cout << "    Error: " << e.what() << "\n";
    }

    pause();

    std::cout << "\n[6] Locate returns MsPtr (pointer into the span)\n";
    const std::size_t locateIdx = static_cast<std::size_t>(
        readInt("    Enter index to locate: "));
    try {
        auto mp = span.Locate(locateIdx);
        std::cout << "    *mp = " << *mp << "\n";
        std::cout << "    mp.index() = " << mp.index() << "\n";
        std::cout << "    mp.valid()? " << (mp.valid() ? "yes" : "no") << "\n";
        std::cout << "    Changing span via set()...\n";
        span.set(locateIdx, 999);
        std::cout << "    *mp is now = " << *mp
                  << "  (changes because MsPtr points INTO the span)\n";
    } catch (const std::out_of_range& e) {
        std::cout << "    Error: " << e.what() << "\n";
    }

    pause();

    std::cout << "\n[7] Raw data access\n";
    std::cout << "    span.data()[0] = " << span.data()[0] << "\n";
    std::cout << "    span.data()[1] = " << span.data()[1] << "\n";

    std::cout << "\n=== Demo MemorySpan finished ===\n";
    pause();
}

void demoMsPtr() {
    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "  Demo: MsPtr\n";
    std::cout << "========================================\n";

    std::cout << "\n[1] Creating a MemorySpan and locating the first element\n";
    lab::MemorySpan<int> span{10, 20, 30, 40, 50};
    std::cout << "    span = {10, 20, 30, 40, 50}\n";
    std::cout << "    span.size() = " << span.size() << "\n";

    auto p = span.Locate(0);
    std::cout << "    p = span.Locate(0)\n";
    std::cout << "    *p = " << *p << "\n";
    std::cout << "    p.index() = " << p.index() << "\n";

    pause();

    std::cout << "\n[2] Increment and decrement\n";
    ++p;
    std::cout << "    After ++p: *p = " << *p << ", index = " << p.index() << "\n";
    ++p;
    std::cout << "    After ++p: *p = " << *p << ", index = " << p.index() << "\n";
    --p;
    std::cout << "    After --p: *p = " << *p << ", index = " << p.index() << "\n";

    pause();

    std::cout << "\n[3] Arithmetic with offset\n";
    auto shifted = span.Locate(0) + 3;
    std::cout << "    span.Locate(0) + 3: *shifted = " << *shifted
              << ", index = " << shifted.index() << "\n";

    shifted -= 2;
    std::cout << "    shifted -= 2: *shifted = " << *shifted
              << ", index = " << shifted.index() << "\n";

    pause();

    std::cout << "\n[4] Subscript operator\n";
    auto base = span.Locate(0);
    std::cout << "    base[1] = " << base[1] << "\n";
    std::cout << "    base[3] = " << base[3] << "\n";

    pause();

    std::cout << "\n[5] Difference between two MsPtr\n";
    auto first = span.Locate(0);
    auto last = span.Locate(4);
    std::cout << "    last - first = " << (last - first) << "\n";

    pause();

    std::cout << "\n[6] Comparisons\n";
    std::cout << "    first == first? " << (first == first ? "yes" : "no") << "\n";
    std::cout << "    first != last?  " << (first != last ? "yes" : "no") << "\n";
    std::cout << "    first < last?   " << (first < last ? "yes" : "no") << "\n";
    std::cout << "    first <= last?  " << (first <= last ? "yes" : "no") << "\n";
    std::cout << "    last > first?   " << (last > first ? "yes" : "no") << "\n";

    pause();

    std::cout << "\n[7] Modifying via MsPtr (points INTO the span)\n";
    auto mid = span.Locate(2);
    std::cout << "    Before: *mid = " << *mid << "\n";
    *mid = 999;
    std::cout << "    After *mid = 999: *mid = " << *mid << "\n";
    std::cout << "    span.at(2) = " << span.at(2)
              << "  (same, because MsPtr points into the span)\n";

    pause();

    std::cout << "\n[8] Boundary checks\n";

    std::cout << "    Trying to decrement past begin...\n";
    auto begin = span.Locate(0);
    try {
        --begin;
        std::cout << "    Hmm, no exception?!\n";
    } catch (const std::out_of_range& e) {
        std::cout << "    Caught: " << e.what() << "\n";
    }

    std::cout << "    Trying to increment past end...\n";
    auto end = span.Locate(0);
    for (std::size_t i = 0; i < span.size(); ++i) {
        ++end;
    }
    try {
        ++end;
        std::cout << "    Hmm, no exception?!\n";
    } catch (const std::out_of_range& e) {
        std::cout << "    Caught: " << e.what() << "\n";
    }

    std::cout << "    Trying to dereference end...\n";
    auto end2 = span.Locate(0);
    for (std::size_t i = 0; i < span.size(); ++i) {
        ++end2;
    }
    try {
        (void)*end2;
        std::cout << "    Hmm, no exception?!\n";
    } catch (const std::out_of_range& e) {
        std::cout << "    Caught: " << e.what() << "\n";
    }

    std::cout << "    Trying arithmetic past end...\n";
    auto p2 = span.Locate(0);
    try {
        p2 += 100;
        std::cout << "    Hmm, no exception?!\n";
    } catch (const std::out_of_range& e) {
        std::cout << "    Caught: " << e.what() << "\n";
    }

    std::cout << "\n=== Demo MsPtr finished ===\n";
    pause();
}

void demoSequence() {
    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "  Demo: Sequence (DynamicArray + LinkedList)\n";
    std::cout << "========================================\n";

    std::cout << "\n########## DynamicArray ##########\n";

    std::cout << "\n[1] Creating a DynamicArray from initializer list\n";
    lab::DynamicArray<int> arr{10, 20, 30};
    std::cout << "    arr = {10, 20, 30}\n";
    std::cout << "    arr.size() = " << arr.size() << "\n";
    std::cout << "    arr.capacity() = " << arr.capacity() << "\n";
    std::cout << "    arr.empty()? " << (arr.empty() ? "yes" : "no") << "\n";

    pause();

    std::cout << "\n[2] push_back several values\n";
    const int n = readInt("    How many values to add? ");
    for (int i = 0; i < n; ++i) {
        const int v = readInt("    Enter value to push_back: ");
        arr.push_back(v);
    }
    std::cout << "    After push_back x" << n << ":\n";
    std::cout << "    size = " << arr.size()
              << ", capacity = " << arr.capacity() << "\n";
    std::cout << "    Elements: ";
    for (std::size_t i = 0; i < arr.size(); ++i) {
        std::cout << arr.at(i) << " ";
    }
    std::cout << "\n";

    pause();

    std::cout << "\n[3] pop_back\n";
    if (!arr.empty()) {
        arr.pop_back();
        std::cout << "    After pop_back: size = " << arr.size() << "\n";
    }

    pause();

    std::cout << "\n[4] Accessing with at() and operator[]\n";
    const std::size_t idx = static_cast<std::size_t>(
        readInt("    Enter index to access: "));
    try {
        std::cout << "    arr.at(" << idx << ") = " << arr.at(idx) << "\n";
        std::cout << "    arr[" << idx << "] = " << arr[idx] << "\n";
    } catch (const std::out_of_range& e) {
        std::cout << "    Caught: " << e.what() << "\n";
    }

    pause();

    std::cout << "\n[5] Copy / Get / Locate\n";
    if (!arr.empty()) {
        const std::size_t copyIdx = static_cast<std::size_t>(
            readInt("    Enter index to Copy/Get/Locate: "));
        try {
            auto cp = arr.Copy(copyIdx);
            std::cout << "    *arr.Copy(" << copyIdx << ") = " << *cp << "\n";

            auto un = arr.Get(copyIdx);
            std::cout << "    *arr.Get(" << copyIdx << ") = " << *un << "\n";

            auto sm = arr.Locate(copyIdx);
            std::cout << "    *arr.Locate(" << copyIdx << ") = " << *sm << "\n";
        } catch (const std::out_of_range& e) {
            std::cout << "    Caught: " << e.what() << "\n";
        }
    }

    pause();

    std::cout << "\n[6] clear\n";
    arr.clear();
    std::cout << "    After clear: size = " << arr.size()
              << ", empty? " << (arr.empty() ? "yes" : "no") << "\n";

    pause();

    std::cout << "\n########## LinkedList ##########\n";

    std::cout << "\n[7] Creating a LinkedList from initializer list\n";
    lab::LinkedList<int> list{100, 200, 300};
    std::cout << "    list = {100, 200, 300}\n";
    std::cout << "    list.size() = " << list.size() << "\n";
    std::cout << "    list.empty()? " << (list.empty() ? "yes" : "no") << "\n";

    pause();

    std::cout << "\n[8] push_front and push_back\n";
    const int frontValue = readInt("    Enter value for push_front: ");
    list.push_front(frontValue);

    const int backValue = readInt("    Enter value for push_back: ");
    list.push_back(backValue);

    std::cout << "    After push_front + push_back:\n";
    std::cout << "    size = " << list.size() << "\n";
    std::cout << "    Elements: ";
    for (std::size_t i = 0; i < list.size(); ++i) {
        std::cout << list.at(i) << " ";
    }
    std::cout << "\n";

    pause();

    std::cout << "\n[9] pop_front\n";
    if (!list.empty()) {
        list.pop_front();
        std::cout << "    After pop_front: size = " << list.size() << "\n";
        std::cout << "    First element now: " << list.at(0) << "\n";
    }

    pause();

    std::cout << "\n[10] Accessing with at()\n";
    const std::size_t listIdx = static_cast<std::size_t>(
        readInt("    Enter index to access: "));
    try {
        std::cout << "    list.at(" << listIdx << ") = " << list.at(listIdx) << "\n";
    } catch (const std::out_of_range& e) {
        std::cout << "    Caught: " << e.what() << "\n";
    }

    pause();

    std::cout << "\n[11] Copy / Get / Locate on LinkedList\n";
    if (!list.empty()) {
        auto cp = list.Copy(0);
        std::cout << "    *list.Copy(0) = " << *cp << "\n";

        auto un = list.Get(0);
        std::cout << "    *list.Get(0) = " << *un << "\n";

        auto sm = list.Locate(0);
        std::cout << "    *list.Locate(0) = " << *sm << "\n";
    }

    pause();

    std::cout << "\n[12] Boundary check\n";
    lab::LinkedList<int> emptyList;
    std::cout << "    Trying pop_front on empty list...\n";
    try {
        emptyList.pop_front();
        std::cout << "    Hmm, no exception?!\n";
    } catch (const std::out_of_range& e) {
        std::cout << "    Caught: " << e.what() << "\n";
    }

    std::cout << "\n=== Demo Sequence finished ===\n";
    pause();
}

void demoUnqPtrArr() {
    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "  Demo: UnqPtr<T[]> (array)\n";
    std::cout << "========================================\n";

    std::cout << "\n[1] Creating an array via makeUnqArr\n";
    const int n = readInt("    Enter array size: ");
    auto arr = lab::makeUnqArr<int>(n);
    std::cout << "    arr.size() = " << arr.size() << "\n";
    std::cout << "    arr is valid? " << (arr ? "yes" : "no") << "\n";

    pause();

    std::cout << "\n[2] Filling the array\n";
    for (int i = 0; i < n; ++i) {
        const int value = readInt("    Enter value for arr[" + std::to_string(i) + "]: ");
        arr[i] = value;
    }
    std::cout << "    Array: ";
    for (std::size_t i = 0; i < arr.size(); ++i) {
        std::cout << arr[i] << " ";
    }
    std::cout << "\n";

    pause();

    std::cout << "\n[3] Out-of-range access\n";
    try {
        (void)arr[n];
        std::cout << "    Hmm, no exception?!\n";
    } catch (const std::out_of_range& e) {
        std::cout << "    Caught: " << e.what() << "\n";
    }

    pause();

    std::cout << "\n[4] Moving the array\n";
    auto moved = std::move(arr);
    std::cout << "    After move:\n";
    std::cout << "    arr is valid? " << (arr ? "yes" : "no") << "\n";
    std::cout << "    arr.size() = " << arr.size() << "\n";
    std::cout << "    moved.size() = " << moved.size() << "\n";

    pause();

    std::cout << "\n[5] Releasing ownership\n";
    int* raw = moved.release();
    std::cout << "    After release:\n";
    std::cout << "    moved is valid? " << (moved ? "yes" : "no") << "\n";
    std::cout << "    raw[0] = " << raw[0] << "\n";
    std::cout << "    Deleting raw manually...\n";
    delete[] raw;

    std::cout << "\n=== Demo UnqPtr<T[]> finished ===\n";
    pause();
}

void demoShrdPtrArr() {
    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "  Demo: ShrdPtr<T[]> (array)\n";
    std::cout << "========================================\n";

    std::cout << "\n[1] Creating an array via makeShrdArr\n";
    const int n = readInt("    Enter array size: ");
    auto a = lab::makeShrdArr<int>(n);
    std::cout << "    a.size() = " << a.size() << "\n";
    std::cout << "    a.use_count() = " << a.use_count() << "\n";

    pause();

    std::cout << "\n[2] Filling via operator[]\n";
    for (int i = 0; i < n; ++i) {
        const int value = readInt("    Enter value for a[" + std::to_string(i) + "]: ");
        a[i] = value;
    }
    std::cout << "    Array: ";
    for (std::size_t i = 0; i < a.size(); ++i) {
        std::cout << a[i] << " ";
    }
    std::cout << "\n";

    pause();

    std::cout << "\n[3] Copying (shared ownership)\n";
    auto b = a;
    std::cout << "    After copy:\n";
    std::cout << "    a.use_count() = " << a.use_count() << "\n";
    std::cout << "    b.use_count() = " << b.use_count() << "\n";
    std::cout << "    b.size() = " << b.size() << "\n";

    pause();

    std::cout << "\n[4] Leaving inner scope\n";
    {
        auto c = a;
        std::cout << "    Inside scope: a.use_count() = " << a.use_count() << "\n";
    }
    std::cout << "    Outside scope: a.use_count() = " << a.use_count() << "\n";

    pause();

    std::cout << "\n[5] Out-of-range access\n";
    try {
        (void)a[n];
        std::cout << "    Hmm, no exception?!\n";
    } catch (const std::out_of_range& e) {
        std::cout << "    Caught: " << e.what() << "\n";
    }

    pause();

    std::cout << "\n[6] Moving ownership\n";
    auto d = std::move(a);
    std::cout << "    After move:\n";
    std::cout << "    a is valid? " << (a ? "yes" : "no") << "\n";
    std::cout << "    a.use_count() = " << a.use_count() << "\n";
    std::cout << "    d.use_count() = " << d.use_count() << "\n";
    std::cout << "    d.size() = " << d.size() << "\n";

    std::cout << "\n=== Demo ShrdPtr<T[]> finished ===\n";
    pause();
}