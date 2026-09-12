cmake_minimum_required(VERSION 3.20)
project(finch VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable strict compiler warnings
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -O3)
endif()

# Source files
add_executable(finch
    src/main.cxx
    src/Bit-Flipper/Bit.cxx
    src/Bit-Flipper/HexDump.cxx
)

# Link platform socket libraries
if(WIN32)
    target_link_libraries(finch PRIVATE ws2_32)
endif()