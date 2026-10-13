#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <new>
#include <string>
#include <vector>

#include "UnqPtr.h"
#include "ShrdPtr.h"
#include "SmrtPtr.h"
#include "MemorySpan.h"
#include "MsPtr.h"
#include "Sequence.h"

namespace {

std::atomic<std::size_t> g_allocCount{0};
std::atomic<std::size_t> g_allocBytes{0};

void resetAllocCounters() {
    g_allocCount.store(0);
    g_allocBytes.store(0);
}

std::size_t currentAllocCount() {
    return g_allocCount.load();
}

} // namespace

void* operator new(std::size_t size) {
    void* p = std::malloc(size);
    if (!p) throw std::bad_alloc();
    g_allocCount.fetch_add(1);
    g_allocBytes.fetch_add(size);
    return p;
}

void* operator new[](std::size_t size) {
    void* p = std::malloc(size);
    if (!p) throw std::bad_alloc();
    g_allocCount.fetch_add(1);
    g_allocBytes.fetch_add(size);
    return p;
}

void operator delete(void* p) noexcept {
    if (p) {
        g_allocCount.fetch_sub(1);
        std::free(p);
    }
}

void operator delete[](void* p) noexcept {
    if (p) {
        g_allocCount.fetch_sub(1);
        std::free(p);
    }
}

void operator delete(void* p, std::size_t) noexcept {
    ::operator delete(p);
}

void operator delete[](void* p, std::size_t) noexcept {
    ::operator delete[](p);
}

struct BenchResult {
    std::string name;
    std::size_t N;
    double time_ms;
    std::size_t peak_allocs;
};

template <typename F>
BenchResult runBench(const std::string& name, std::size_t N, F&& fn) {
    resetAllocCounters();

    const auto t0 = std::chrono::high_resolution_clock::now();
    fn();
    const auto t1 = std::chrono::high_resolution_clock::now();

    const double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
    const std::size_t peak = currentAllocCount();

    return BenchResult{name, N, ms, peak};
}

void benchUnqPtr(std::size_t N, std::vector<BenchResult>& out) {
    out.push_back(runBench("UnqPtr", N, [N]() {
        for (std::size_t i = 0; i < N; ++i) {
            auto p = lab::makeUnq<int>(static_cast<int>(i));
            (void)*p;
        }
    }));
}

void benchRawUnique(std::size_t N, std::vector<BenchResult>& out) {
    out.push_back(runBench("RawUnique", N, [N]() {
        for (std::size_t i = 0; i < N; ++i) {
            int* p = new int(static_cast<int>(i));
            (void)*p;
            delete p;
        }
    }));
}

void benchStdUnique(std::size_t N, std::vector<BenchResult>& out) {
    out.push_back(runBench("StdUnique", N, [N]() {
        for (std::size_t i = 0; i < N; ++i) {
            auto p = std::make_unique<int>(static_cast<int>(i));
            (void)*p;
        }
    }));
}

void benchShrdPtr(std::size_t N, std::vector<BenchResult>& out) {
    out.push_back(runBench("ShrdPtr", N, [N]() {
        for (std::size_t i = 0; i < N; ++i) {
            auto a = lab::makeShrd<int>(static_cast<int>(i));
            auto b = a;
            (void)*a;
            (void)*b;
        }
    }));
}

void benchStdShared(std::size_t N, std::vector<BenchResult>& out) {
    out.push_back(runBench("StdShared", N, [N]() {
        for (std::size_t i = 0; i < N; ++i) {
            auto a = std::make_shared<int>(static_cast<int>(i));
            auto b = a;
            (void)*a;
            (void)*b;
        }
    }));
}

void benchSmrtPtr(std::size_t N, std::vector<BenchResult>& out) {
    out.push_back(runBench("SmrtPtr", N, [N]() {
        for (std::size_t i = 0; i < N; ++i) {
            auto a = lab::makeSmrt<int>(static_cast<int>(i));
            auto b = a;
            (void)*a;
            (void)*b;
        }
    }));
}

void printResults(const std::vector<BenchResult>& results) {
    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "  Benchmark results\n";
    std::cout << "========================================\n";
    std::cout << std::left
              << std::setw(16) << "Type"
              << std::setw(12) << "N"
              << std::setw(16) << "Time (ms)"
              << std::setw(16) << "Peak allocs"
              << "\n";
    std::cout << std::string(60, '-') << "\n";

    for (const auto& r : results) {
        std::cout << std::left
                  << std::setw(16) << r.name
                  << std::setw(12) << r.N
                  << std::setw(16) << std::fixed << std::setprecision(4) << r.time_ms
                  << std::setw(16) << r.peak_allocs
                  << "\n";
    }
}

void saveCsv(const std::vector<BenchResult>& results, const std::string& path) {
    std::ofstream f(path);
    if (!f) {
        std::cerr << "Cannot open file: " << path << "\n";
        return;
    }

    f << "type,N,time_ms,peak_allocs\n";
    for (const auto& r : results) {
        f << r.name << ","
          << r.N << ","
          << std::fixed << std::setprecision(6) << r.time_ms << ","
          << r.peak_allocs << "\n";
    }

    std::cout << "\nSaved CSV: " << path << "\n";
}

int main() {
    const std::vector<std::size_t> sizes = {
        10, 100, 1000, 10000, 100000, 1000000
    };

    std::vector<BenchResult> results;

    {
        auto warm = lab::makeSmrt<int>(0);
        (void)*warm;
    }

    for (std::size_t N : sizes) {
        std::cout << "Running benchmarks for N = " << N << "...\n";

        benchUnqPtr(N, results);
        benchRawUnique(N, results);
        benchStdUnique(N, results);

        benchShrdPtr(N, results);
        benchStdShared(N, results);

        benchSmrtPtr(N, results);
    }

    printResults(results);
    saveCsv(results, "../results/tables/bench.csv");

    return 0;
}