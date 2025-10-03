# Minimum required version of CMake
cmake_minimum_required(VERSION 3.0)

# Project name and language
project(clow C)

# Set the library source files
set(SOURCES
    include/clow/freelist.c
    include/clow/gpalloc.c
    include/clow/slice.c
)

# Add the library target
# Use STATIC for a static library, SHARED for a shared library
add_library(clow STATIC ${SOURCES})

# Specify the include directory for this library
target_include_directories(clow PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Optionally set the C standard (e.g., C11)
set_target_properties(clow PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
)