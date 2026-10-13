#include <gtest/gtest.h>

#include <iostream>
#include <limits>
#include <string>

#include "demo.h"

static void printMenu() {
    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "  Smart Pointers - Lab 1\n";
    std::cout << "========================================\n";
    std::cout << "  1. Run auto tests (Google Test)\n";
    std::cout << "  2. Demo UnqPtr\n";
    std::cout << "  3. Demo ShrdPtr\n";
    std::cout << "  4. Demo SmrtPtr\n";
    std::cout << "  5. Demo MemorySpan\n";
    std::cout << "  6. Demo MsPtr\n";
    std::cout << "  7. Demo Sequence\n";
    std::cout << "  8. Demo UnqPtr array\n";
    std::cout << "  9. Demo ShrdPtr array\n";
    std::cout << "  0. Exit\n";
    std::cout << "----------------------------------------\n";
    std::cout << "  Choice: ";
}

static void runAutoTests() {
    int argc = 1;
    char arg0[] = "smart_pointers";
    char* argv[] = { arg0, nullptr };

    ::testing::InitGoogleTest(&argc, argv);
    const int result = RUN_ALL_TESTS();
    (void)result;
}

int main() {
    while (true) {
        printMenu();

        int choice = -1;
        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input. Please enter a number.\n";
            continue;
        }

        switch (choice) {
        case 1:
            std::cout << "\n=== Running auto tests ===\n\n";
            runAutoTests();
            break;
        case 2:
            demoUnqPtr();
            break;
        case 3:
            demoShrdPtr();
            break;
        case 4:
            demoSmrtPtr();
            break;
        case 5:
            demoMemorySpan();
            break;
        case 6:
            demoMsPtr();
            break;
        case 7:
            demoSequence();
            break;
        case 8:
            demoUnqPtrArr();
            break;
        case 9:
            demoShrdPtrArr();
            break;
        case 0:
            std::cout << "Bye.\n";
            return 0;
        default:
            std::cout << "Unknown option: " << choice << "\n";
            break;
        }
    }

    return 0;
}