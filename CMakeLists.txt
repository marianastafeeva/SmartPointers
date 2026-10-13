cmake_minimum_required(VERSION 3.16)
project(SmartPointers LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

include_directories(${CMAKE_SOURCE_DIR}/include)

find_package(GTest REQUIRED)

add_executable(smart_pointers
    src/main.cpp
    src/demo.cpp
    src/tests.cpp
)

target_link_libraries(smart_pointers
    PRIVATE
    GTest::gtest
)

add_executable(smart_pointers_bench
    src/bench.cpp
)