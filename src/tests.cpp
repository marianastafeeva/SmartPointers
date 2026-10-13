#include <gtest/gtest.h>

#include <type_traits>
#include <utility>
#include <stdexcept>

#include "UnqPtr.h"
#include "ShrdPtr.h"
#include "SmrtPtr.h"
#include "MemorySpan.h"
#include "MsPtr.h"
#include "Sequence.h"

struct Base {
    virtual ~Base() = default;
    virtual int value() const { return 1; }
};

struct Derived : Base {
    int value() const override { return 2; }
};

static_assert(!std::is_copy_constructible_v<lab::UnqPtr<int>>,
              "UnqPtr must not be copy-constructible");
static_assert(!std::is_copy_assignable_v<lab::UnqPtr<int>>,
              "UnqPtr must not be copy-assignable");

// ============================================================
//  UnqPtr
// ============================================================

TEST(UnqPtr, DefaultConstructor) {
    lab::UnqPtr<int> p;
    EXPECT_EQ(p.get(), nullptr);
}

TEST(UnqPtr, FromRawPointer) {
    lab::UnqPtr<int> p(new int(42));
    EXPECT_EQ(*p, 42);
    EXPECT_TRUE(p);
}

TEST(UnqPtr, MoveConstructor) {
    lab::UnqPtr<int> p(new int(10));
    lab::UnqPtr<int> q = std::move(p);
    EXPECT_FALSE(p);
    EXPECT_EQ(*q, 10);
}

TEST(UnqPtr, MoveAssignment) {
    lab::UnqPtr<int> p;
    p = lab::UnqPtr<int>(new int(20));
    EXPECT_EQ(*p, 20);
}

TEST(UnqPtr, SubtypingMoveConstructor) {
    lab::UnqPtr<Derived> d(new Derived());
    lab::UnqPtr<Base> b = std::move(d);
    EXPECT_FALSE(d);
    EXPECT_EQ(b->value(), 2);
}

TEST(UnqPtr, SubtypingMoveAssignment) {
    lab::UnqPtr<Derived> d(new Derived());
    lab::UnqPtr<Base> b;
    b = std::move(d);
    EXPECT_EQ(b->value(), 2);
}

TEST(UnqPtr, Release) {
    lab::UnqPtr<int> p(new int(99));
    int* raw = p.release();
    EXPECT_FALSE(p);
    delete raw;
}

TEST(UnqPtr, ResetWithPointer) {
    lab::UnqPtr<int> p(new int(11));
    p.reset(new int(22));
    EXPECT_EQ(*p, 22);
}

TEST(UnqPtr, ResetWithoutPointer) {
    lab::UnqPtr<int> p(new int(11));
    p.reset();
    EXPECT_FALSE(p);
}

TEST(UnqPtr, Swap) {
    lab::UnqPtr<int> p(new int(1));
    lab::UnqPtr<int> q(new int(2));
    p.swap(q);
    EXPECT_EQ(*p, 2);
    EXPECT_EQ(*q, 1);
}

TEST(UnqPtr, MakeUnq) {
    auto p = lab::makeUnq<int>(77);
    EXPECT_EQ(*p, 77);
}

// ============================================================
//  ShrdPtr
// ============================================================

TEST(ShrdPtr, DefaultConstructor) {
    lab::ShrdPtr<int> s;
    EXPECT_EQ(s.get(), nullptr);
    EXPECT_EQ(s.use_count(), 0);
}

TEST(ShrdPtr, FromUnqPtr) {
    lab::ShrdPtr<int> s = lab::makeShrd<int>(42);
    EXPECT_EQ(*s, 42);
    EXPECT_EQ(s.use_count(), 1);
}

TEST(ShrdPtr, CopyIncrementsCounter) {
    lab::ShrdPtr<int> a = lab::makeShrd<int>(10);
    lab::ShrdPtr<int> b = a;
    EXPECT_EQ(a.use_count(), 2);
    EXPECT_EQ(b.use_count(), 2);
    EXPECT_EQ(*a, 10);
    EXPECT_EQ(*b, 10);
}

TEST(ShrdPtr, DestructorDecrementsCounter) {
    lab::ShrdPtr<int> a = lab::makeShrd<int>(5);
    {
        lab::ShrdPtr<int> b = a;
        EXPECT_EQ(a.use_count(), 2);
    }
    EXPECT_EQ(a.use_count(), 1);
}

TEST(ShrdPtr, AssignmentDecrementsOldCounter) {
    lab::ShrdPtr<int> a = lab::makeShrd<int>(3);
    lab::ShrdPtr<int> b = lab::makeShrd<int>(4);
    a = b;
    EXPECT_EQ(a.use_count(), 2);
    EXPECT_EQ(b.use_count(), 2);
    EXPECT_EQ(*a, 4);
}

TEST(ShrdPtr, MoveConstructor) {
    lab::ShrdPtr<int> a = lab::makeShrd<int>(7);
    lab::ShrdPtr<int> b = std::move(a);
    EXPECT_FALSE(a);
    EXPECT_EQ(a.use_count(), 0);
    EXPECT_EQ(b.use_count(), 1);
    EXPECT_EQ(*b, 7);
}

TEST(ShrdPtr, SubtypingMoveConstructor) {
    lab::ShrdPtr<Derived> d = lab::makeShrd<Derived>();
    lab::ShrdPtr<Base> b = std::move(d);
    EXPECT_FALSE(d);
    EXPECT_EQ(b.use_count(), 1);
    EXPECT_EQ(b->value(), 2);
}

TEST(ShrdPtr, SubtypingCopyConstructor) {
    lab::ShrdPtr<Derived> d = lab::makeShrd<Derived>();
    lab::ShrdPtr<Base> b = d;
    EXPECT_EQ(d.use_count(), 2);
    EXPECT_EQ(b.use_count(), 2);
    EXPECT_EQ(b->value(), 2);
}

TEST(ShrdPtr, Reset) {
    lab::ShrdPtr<int> a = lab::makeShrd<int>(1);
    a.reset();
    EXPECT_FALSE(a);
    EXPECT_EQ(a.use_count(), 0);
}

TEST(ShrdPtr, ResetWithUnqPtr) {
    lab::ShrdPtr<int> a = lab::makeShrd<int>(1);
    a.reset(lab::makeUnq<int>(2));
    EXPECT_EQ(*a, 2);
    EXPECT_EQ(a.use_count(), 1);
}

TEST(ShrdPtr, Swap) {
    lab::ShrdPtr<int> a = lab::makeShrd<int>(1);
    lab::ShrdPtr<int> b = lab::makeShrd<int>(2);
    a.swap(b);
    EXPECT_EQ(*a, 2);
    EXPECT_EQ(*b, 1);
}

// ============================================================
//  SmrtPtr
// ============================================================

TEST(SmrtPtr, DefaultConstructor) {
    lab::SmrtPtr<int> s;
    EXPECT_EQ(s.get(), nullptr);
    EXPECT_EQ(s.use_count(), 0);
}

TEST(SmrtPtr, FromRawPointer) {
    lab::SmrtPtr<int> s(new int(42));
    EXPECT_EQ(*s, 42);
    EXPECT_EQ(s.use_count(), 1);
}

TEST(SmrtPtr, CopyIncrementsCounter) {
    lab::SmrtPtr<int> a(new int(10));
    lab::SmrtPtr<int> b = a;
    EXPECT_EQ(a.use_count(), 2);
    EXPECT_EQ(b.use_count(), 2);
    EXPECT_EQ(*a, 10);
    EXPECT_EQ(*b, 10);
}

TEST(SmrtPtr, DestructorDecrementsCounter) {
    lab::SmrtPtr<int> a(new int(5));
    {
        lab::SmrtPtr<int> b = a;
        EXPECT_EQ(a.use_count(), 2);
    }
    EXPECT_EQ(a.use_count(), 1);
}

TEST(SmrtPtr, MoveConstructor) {
    lab::SmrtPtr<int> a(new int(7));
    lab::SmrtPtr<int> b = std::move(a);
    EXPECT_FALSE(a);
    EXPECT_EQ(b.use_count(), 1);
    EXPECT_EQ(*b, 7);
}

TEST(SmrtPtr, MoveAssignment) {
    lab::SmrtPtr<int> a(new int(1));
    lab::SmrtPtr<int> b(new int(2));
    a = std::move(b);
    EXPECT_FALSE(b);
    EXPECT_EQ(*a, 2);
}

TEST(SmrtPtr, ResetWithoutPointer) {
    lab::SmrtPtr<int> a(new int(1));
    a.reset();
    EXPECT_FALSE(a);
    EXPECT_EQ(a.use_count(), 0);
}

TEST(SmrtPtr, ResetWithPointer) {
    lab::SmrtPtr<int> a(new int(1));
    a.reset(new int(2));
    EXPECT_EQ(*a, 2);
    EXPECT_EQ(a.use_count(), 1);
}

TEST(SmrtPtr, Swap) {
    lab::SmrtPtr<int> a(new int(1));
    lab::SmrtPtr<int> b(new int(2));
    a.swap(b);
    EXPECT_EQ(*a, 2);
    EXPECT_EQ(*b, 1);
}

TEST(SmrtPtr, MakeSmrt) {
    auto s = lab::makeSmrt<int>(77);
    EXPECT_EQ(*s, 77);
}

TEST(SmrtPtr, StorageEmptiesAfterRelease) {
    {
        lab::SmrtPtr<int> s(new int(42));
        EXPECT_EQ(s.use_count(), 1);
    }
    EXPECT_EQ(lab::SmrtStorage<int>::instance().size(), 0);
}

TEST(SmrtPtr, SubtypingCopy) {
    lab::SmrtPtr<Derived> d(new Derived());
    lab::SmrtPtr<Base> b = d;
    EXPECT_EQ(b->value(), 2);
}

// ============================================================
//  MemorySpan
// ============================================================

TEST(MemorySpan, DefaultConstructor) {
    lab::MemorySpan<int> span;
    EXPECT_EQ(span.size(), 0);
    EXPECT_TRUE(span.empty());
}

TEST(MemorySpan, FromSize) {
    lab::MemorySpan<int> span(5);
    EXPECT_EQ(span.size(), 5);
    EXPECT_FALSE(span.empty());
}

TEST(MemorySpan, FromInitializerList) {
    lab::MemorySpan<int> span{10, 20, 30};
    EXPECT_EQ(span.size(), 3);
    EXPECT_EQ(span.at(0), 10);
    EXPECT_EQ(span.at(1), 20);
    EXPECT_EQ(span.at(2), 30);
}

TEST(MemorySpan, FromArray) {
    int src[] = {1, 2, 3, 4};
    lab::MemorySpan<int> span(src, 4);
    EXPECT_EQ(span.size(), 4);
    EXPECT_EQ(span.at(3), 4);
}

TEST(MemorySpan, SetAndAt) {
    lab::MemorySpan<int> span{1, 2, 3};
    span.set(1, 99);
    EXPECT_EQ(span.at(1), 99);
}

TEST(MemorySpan, AtThrowsOutOfRange) {
    lab::MemorySpan<int> span{1, 2, 3};
    EXPECT_THROW(span.at(3), std::out_of_range);
    EXPECT_THROW(span.at(100), std::out_of_range);
}

TEST(MemorySpan, SetThrowsOutOfRange) {
    lab::MemorySpan<int> span{1, 2, 3};
    EXPECT_THROW(span.set(100, 0), std::out_of_range);
}

TEST(MemorySpan, CopyReturnsSharedCopy) {
    lab::MemorySpan<int> span{1, 2, 3};
    auto p = span.Copy(1);
    EXPECT_EQ(*p, 2);
    EXPECT_EQ(p.use_count(), 1);
}

TEST(MemorySpan, GetReturnsUniqueCopy) {
    lab::MemorySpan<int> span{1, 2, 3};
    auto p = span.Get(2);
    EXPECT_EQ(*p, 3);
}

TEST(MemorySpan, LocateReturnsMsPtr) {
    lab::MemorySpan<int> span{1, 2, 3};
    auto p = span.Locate(0);
    EXPECT_EQ(*p, 1);
    EXPECT_EQ(p.index(), 0);
}

TEST(MemorySpan, MoveConstructor) {
    lab::MemorySpan<int> span{1, 2, 3};
    lab::MemorySpan<int> moved = std::move(span);
    EXPECT_EQ(moved.size(), 3);
    EXPECT_EQ(span.size(), 0);
}

TEST(MemorySpan, DataAccess) {
    lab::MemorySpan<int> span{5, 6, 7};
    EXPECT_EQ(span.data()[1], 6);
}

// ============================================================
//  MsPtr
// ============================================================

TEST(MsPtr, Dereference) {
    lab::MemorySpan<int> span{10, 20, 30};
    auto p = span.Locate(0);
    EXPECT_EQ(*p, 10);
}

TEST(MsPtr, ArrowOperator) {
    lab::MemorySpan<Base> span;
    (void)span;
    lab::MemorySpan<int> s{42};
    auto p = s.Locate(0);
    EXPECT_EQ(*p, 42);
}

TEST(MsPtr, Increment) {
    lab::MemorySpan<int> span{10, 20, 30};
    auto p = span.Locate(0);
    ++p;
    EXPECT_EQ(*p, 20);
    p++;
    EXPECT_EQ(*p, 30);
}

TEST(MsPtr, Decrement) {
    lab::MemorySpan<int> span{10, 20, 30};
    auto p = span.Locate(2);
    --p;
    EXPECT_EQ(*p, 20);
    p--;
    EXPECT_EQ(*p, 10);
}

TEST(MsPtr, PlusEquals) {
    lab::MemorySpan<int> span{10, 20, 30, 40};
    auto p = span.Locate(0);
    p += 2;
    EXPECT_EQ(*p, 30);
}

TEST(MsPtr, MinusEquals) {
    lab::MemorySpan<int> span{10, 20, 30, 40};
    auto p = span.Locate(3);
    p -= 2;
    EXPECT_EQ(*p, 20);
}

TEST(MsPtr, PlusOperator) {
    lab::MemorySpan<int> span{10, 20, 30};
    auto p = span.Locate(0) + 2;
    EXPECT_EQ(*p, 30);
}

TEST(MsPtr, MinusOperator) {
    lab::MemorySpan<int> span{10, 20, 30};
    auto p = span.Locate(2) - 1;
    EXPECT_EQ(*p, 20);
}

TEST(MsPtr, Difference) {
    lab::MemorySpan<int> span{10, 20, 30, 40, 50};
    auto a = span.Locate(0);
    auto b = span.Locate(4);
    EXPECT_EQ(b - a, 4);
}

TEST(MsPtr, Subscript) {
    lab::MemorySpan<int> span{10, 20, 30, 40};
    auto p = span.Locate(0);
    EXPECT_EQ(p[2], 30);
}

TEST(MsPtr, Equality) {
    lab::MemorySpan<int> span{10, 20, 30};
    auto a = span.Locate(1);
    auto b = span.Locate(1);
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);
}

TEST(MsPtr, Comparison) {
    lab::MemorySpan<int> span{10, 20, 30};
    auto a = span.Locate(0);
    auto b = span.Locate(2);
    EXPECT_TRUE(a < b);
    EXPECT_TRUE(a <= b);
    EXPECT_TRUE(b > a);
    EXPECT_TRUE(b >= a);
}

TEST(MsPtr, IndexAndSpan) {
    lab::MemorySpan<int> span{10, 20, 30};
    auto p = span.Locate(1);
    EXPECT_EQ(p.index(), 1);
    EXPECT_EQ(p.span(), &span);
}

TEST(MsPtr, Valid) {
    lab::MemorySpan<int> span{10, 20};
    auto p = span.Locate(0);
    EXPECT_TRUE(p.valid());
}

TEST(MsPtr, DereferenceEndThrows) {
    lab::MemorySpan<int> span{10, 20};
    auto end = span.Locate(0);
    ++end;
    ++end;
    EXPECT_THROW((void)*end, std::out_of_range);
}

TEST(MsPtr, IncrementPastEndThrows) {
    lab::MemorySpan<int> span{10, 20};
    auto end = span.Locate(0);
    ++end;
    ++end;
    EXPECT_THROW(++end, std::out_of_range);
}

TEST(MsPtr, DecrementBeforeBeginThrows) {
    lab::MemorySpan<int> span{10, 20};
    auto p = span.Locate(0);
    EXPECT_THROW(--p, std::out_of_range);
}

TEST(MsPtr, ArithmeticPastEndThrows) {
    lab::MemorySpan<int> span{10, 20};
    auto p = span.Locate(0);
    EXPECT_THROW(p += 100, std::out_of_range);
}

TEST(MsPtr, DefaultIsInvalid) {
    lab::MsPtr<int> p;
    EXPECT_FALSE(p.valid());
}

// ============================================================
//  Sequence
// ============================================================

TEST(DynamicArray, DefaultConstructor) {
    lab::DynamicArray<int> arr;
    EXPECT_EQ(arr.size(), 0);
    EXPECT_TRUE(arr.empty());
}

TEST(DynamicArray, FromSize) {
    lab::DynamicArray<int> arr(5);
    EXPECT_EQ(arr.size(), 5);
}

TEST(DynamicArray, FromInitializerList) {
    lab::DynamicArray<int> arr{1, 2, 3};
    EXPECT_EQ(arr.size(), 3);
    EXPECT_EQ(arr.at(0), 1);
    EXPECT_EQ(arr.at(2), 3);
}

TEST(DynamicArray, PushBack) {
    lab::DynamicArray<int> arr;
    arr.push_back(10);
    arr.push_back(20);
    EXPECT_EQ(arr.size(), 2);
    EXPECT_EQ(arr.at(1), 20);
}

TEST(DynamicArray, PushBackGrowth) {
    lab::DynamicArray<int> arr;
    for (int i = 0; i < 100; ++i) {
        arr.push_back(i);
    }
    EXPECT_EQ(arr.size(), 100);
    EXPECT_EQ(arr.at(0), 0);
    EXPECT_EQ(arr.at(99), 99);
}

TEST(DynamicArray, PopBack) {
    lab::DynamicArray<int> arr{1, 2, 3};
    arr.pop_back();
    EXPECT_EQ(arr.size(), 2);
    EXPECT_EQ(arr.at(1), 2);
}

TEST(DynamicArray, PopBackEmptyThrows) {
    lab::DynamicArray<int> arr;
    EXPECT_THROW(arr.pop_back(), std::out_of_range);
}

TEST(DynamicArray, AtThrowsOutOfRange) {
    lab::DynamicArray<int> arr{1, 2, 3};
    EXPECT_THROW(arr.at(3), std::out_of_range);
}

TEST(DynamicArray, Clear) {
    lab::DynamicArray<int> arr{1, 2, 3};
    arr.clear();
    EXPECT_EQ(arr.size(), 0);
}

TEST(DynamicArray, Reserve) {
    lab::DynamicArray<int> arr;
    arr.reserve(100);
    EXPECT_GE(arr.capacity(), 100);
}

TEST(DynamicArray, CopyReturnsShared) {
    lab::DynamicArray<int> arr{10, 20, 30};
    auto p = arr.Copy(1);
    EXPECT_EQ(*p, 20);
}

TEST(DynamicArray, GetReturnsUnique) {
    lab::DynamicArray<int> arr{10, 20, 30};
    auto p = arr.Get(2);
    EXPECT_EQ(*p, 30);
}

TEST(DynamicArray, LocateReturnsSmrt) {
    lab::DynamicArray<int> arr{10, 20, 30};
    auto p = arr.Locate(0);
    EXPECT_EQ(*p, 10);
}

TEST(LinkedList, DefaultConstructor) {
    lab::LinkedList<int> list;
    EXPECT_EQ(list.size(), 0);
    EXPECT_TRUE(list.empty());
}

TEST(LinkedList, FromInitializerList) {
    lab::LinkedList<int> list{1, 2, 3};
    EXPECT_EQ(list.size(), 3);
    EXPECT_EQ(list.at(0), 1);
    EXPECT_EQ(list.at(2), 3);
}

TEST(LinkedList, PushFront) {
    lab::LinkedList<int> list{2, 3};
    list.push_front(1);
    EXPECT_EQ(list.size(), 3);
    EXPECT_EQ(list.at(0), 1);
}

TEST(LinkedList, PushBack) {
    lab::LinkedList<int> list{1, 2};
    list.push_back(3);
    EXPECT_EQ(list.size(), 3);
    EXPECT_EQ(list.at(2), 3);
}

TEST(LinkedList, PopFront) {
    lab::LinkedList<int> list{1, 2, 3};
    list.pop_front();
    EXPECT_EQ(list.size(), 2);
    EXPECT_EQ(list.at(0), 2);
}

TEST(LinkedList, PopFrontEmptyThrows) {
    lab::LinkedList<int> list;
    EXPECT_THROW(list.pop_front(), std::out_of_range);
}

TEST(LinkedList, AtThrowsOutOfRange) {
    lab::LinkedList<int> list{1, 2, 3};
    EXPECT_THROW(list.at(3), std::out_of_range);
}

TEST(LinkedList, Clear) {
    lab::LinkedList<int> list{1, 2, 3};
    list.clear();
    EXPECT_EQ(list.size(), 0);
}

TEST(LinkedList, CopyReturnsShared) {
    lab::LinkedList<int> list{10, 20, 30};
    auto p = list.Copy(1);
    EXPECT_EQ(*p, 20);
}

TEST(LinkedList, GetReturnsUnique) {
    lab::LinkedList<int> list{10, 20, 30};
    auto p = list.Get(2);
    EXPECT_EQ(*p, 30);
}

TEST(LinkedList, LocateReturnsSmrt) {
    lab::LinkedList<int> list{10, 20, 30};
    auto p = list.Locate(1);
    EXPECT_EQ(*p, 20);
}

TEST(UnqPtrArr, DefaultConstructor) {
    lab::UnqPtr<int[]> p;
    EXPECT_EQ(p.get(), nullptr);
    EXPECT_EQ(p.size(), 0);
}

TEST(UnqPtrArr, FromRawPointerAndSize) {
    lab::UnqPtr<int[]> p(new int[5], 5);
    EXPECT_NE(p.get(), nullptr);
    EXPECT_EQ(p.size(), 5);
}

TEST(UnqPtrArr, OperatorSubscript) {
    lab::UnqPtr<int[]> p(new int[3], 3);
    p[0] = 10;
    p[1] = 20;
    p[2] = 30;
    EXPECT_EQ(p[0], 10);
    EXPECT_EQ(p[1], 20);
    EXPECT_EQ(p[2], 30);
}

TEST(UnqPtrArr, OperatorSubscriptThrowsOutOfRange) {
    lab::UnqPtr<int[]> p(new int[3], 3);
    EXPECT_THROW((void)p[3], std::out_of_range);
    EXPECT_THROW((void)p[100], std::out_of_range);
}

TEST(UnqPtrArr, MoveConstructor) {
    lab::UnqPtr<int[]> a(new int[3], 3);
    a[0] = 42;

    lab::UnqPtr<int[]> b = std::move(a);
    EXPECT_EQ(a.get(), nullptr);
    EXPECT_EQ(a.size(), 0);
    EXPECT_EQ(b.size(), 3);
    EXPECT_EQ(b[0], 42);
}

TEST(UnqPtrArr, MoveAssignment) {
    lab::UnqPtr<int[]> a(new int[2], 2);
    lab::UnqPtr<int[]> b(new int[3], 3);
    b[0] = 99;

    a = std::move(b);
    EXPECT_EQ(a.size(), 3);
    EXPECT_EQ(a[0], 99);
    EXPECT_EQ(b.get(), nullptr);
    EXPECT_EQ(b.size(), 0);
}

TEST(UnqPtrArr, Release) {
    lab::UnqPtr<int[]> p(new int[3], 3);
    int* raw = p.release();
    EXPECT_NE(raw, nullptr);
    EXPECT_EQ(p.get(), nullptr);
    EXPECT_EQ(p.size(), 0);
    delete[] raw;
}

TEST(UnqPtrArr, ResetWithPointer) {
    lab::UnqPtr<int[]> p(new int[2], 2);
    p.reset(new int[5], 5);
    EXPECT_EQ(p.size(), 5);
}

TEST(UnqPtrArr, ResetWithoutPointer) {
    lab::UnqPtr<int[]> p(new int[3], 3);
    p.reset();
    EXPECT_EQ(p.get(), nullptr);
    EXPECT_EQ(p.size(), 0);
}

TEST(UnqPtrArr, Swap) {
    lab::UnqPtr<int[]> a(new int[2], 2);
    lab::UnqPtr<int[]> b(new int[5], 5);

    a.swap(b);
    EXPECT_EQ(a.size(), 5);
    EXPECT_EQ(b.size(), 2);
}

TEST(UnqPtrArr, MakeUnqArr) {
    auto arr = lab::makeUnqArr<int>(10);
    EXPECT_NE(arr.get(), nullptr);
    EXPECT_EQ(arr.size(), 10);
}

TEST(UnqPtrArr, MakeUnqArrZeroSize) {
    auto arr = lab::makeUnqArr<int>(0);
    EXPECT_EQ(arr.get(), nullptr);
    EXPECT_EQ(arr.size(), 0);
}

TEST(ShrdPtrArr, DefaultConstructor) {
    lab::ShrdPtr<int[]> s;
    EXPECT_EQ(s.get(), nullptr);
    EXPECT_EQ(s.size(), 0);
    EXPECT_EQ(s.use_count(), 0);
}

TEST(ShrdPtrArr, FromUnqPtrArr) {
    auto arr = lab::makeShrdArr<int>(5);
    EXPECT_NE(arr.get(), nullptr);
    EXPECT_EQ(arr.size(), 5);
    EXPECT_EQ(arr.use_count(), 1);
}

TEST(ShrdPtrArr, CopyIncrementsCounter) {
    auto a = lab::makeShrdArr<int>(3);
    auto b = a;
    EXPECT_EQ(a.use_count(), 2);
    EXPECT_EQ(b.use_count(), 2);
    EXPECT_EQ(b.size(), 3);
}

TEST(ShrdPtrArr, DestructorDecrementsCounter) {
    auto a = lab::makeShrdArr<int>(3);
    {
        auto b = a;
        EXPECT_EQ(a.use_count(), 2);
    }
    EXPECT_EQ(a.use_count(), 1);
}

TEST(ShrdPtrArr, MoveConstructor) {
    auto a = lab::makeShrdArr<int>(5);
    auto b = std::move(a);
    EXPECT_EQ(a.get(), nullptr);
    EXPECT_EQ(a.size(), 0);
    EXPECT_EQ(a.use_count(), 0);
    EXPECT_EQ(b.size(), 5);
    EXPECT_EQ(b.use_count(), 1);
}

TEST(ShrdPtrArr, OperatorSubscript) {
    auto arr = lab::makeShrdArr<int>(3);
    arr[0] = 10;
    arr[1] = 20;
    arr[2] = 30;
    EXPECT_EQ(arr[0], 10);
    EXPECT_EQ(arr[1], 20);
    EXPECT_EQ(arr[2], 30);
}

TEST(ShrdPtrArr, OperatorSubscriptThrowsOutOfRange) {
    auto arr = lab::makeShrdArr<int>(3);
    EXPECT_THROW((void)arr[3], std::out_of_range);
    EXPECT_THROW((void)arr[100], std::out_of_range);
}

TEST(ShrdPtrArr, Reset) {
    auto arr = lab::makeShrdArr<int>(3);
    arr.reset();
    EXPECT_EQ(arr.get(), nullptr);
    EXPECT_EQ(arr.size(), 0);
    EXPECT_EQ(arr.use_count(), 0);
}

TEST(ShrdPtrArr, ResetWithUnqPtrArr) {
    auto arr = lab::makeShrdArr<int>(3);
    arr.reset(lab::makeUnqArr<int>(7));
    EXPECT_EQ(arr.size(), 7);
    EXPECT_EQ(arr.use_count(), 1);
}

TEST(ShrdPtrArr, Swap) {
    auto a = lab::makeShrdArr<int>(3);
    auto b = lab::makeShrdArr<int>(7);

    a.swap(b);
    EXPECT_EQ(a.size(), 7);
    EXPECT_EQ(b.size(), 3);
}

TEST(ShrdPtrArr, MakeShrdArr) {
    auto arr = lab::makeShrdArr<double>(4);
    EXPECT_EQ(arr.size(), 4);
    EXPECT_EQ(arr.use_count(), 1);
}

TEST(ShrdPtrArr, CopyConstructorKeepsSize) {
    auto a = lab::makeShrdArr<int>(5);
    auto b = a;
    EXPECT_EQ(b.size(), 5);
    EXPECT_EQ(a.size(), 5);
}

TEST(ShrdPtrArr, MoveAssignment) {
    auto a = lab::makeShrdArr<int>(3);
    auto b = lab::makeShrdArr<int>(7);

    a = std::move(b);
    EXPECT_EQ(a.size(), 7);
    EXPECT_EQ(b.get(), nullptr);
    EXPECT_EQ(b.size(), 0);
}