cmake_minimum_required(VERSION 3.0)
project(clow LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Include directories
include_directories("../include")

# Tests
add_executable(freelist_tests freelist_test.c)
target_include_directories(freelist_tests PUBLIC "../include")

# Tests
add_executable(gpalloc_tests gpalloc_test.c)
target_include_directories(gpalloc_tests PUBLIC "../include")