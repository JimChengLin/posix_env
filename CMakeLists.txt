cmake_minimum_required(VERSION 3.10)
project(posix_env)

set(CMAKE_CXX_STANDARD 17)

add_executable(posix_env main.cpp
        src/defs.h
        src/env.cpp src/env.h
        src/mmap_file.cpp src/mmap_file.h
        src/random_access_file.cpp src/random_access_file.h
        src/sequential_file.cpp src/sequential_file.h
        src/slice.h
        src/writable_file.cpp src/writable_file.h
        )