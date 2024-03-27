cmake_minimum_required(VERSION 3.26)
project(OrangeKV)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_executable(OrangeKV src/main.cpp
        include/OrangeKV/LRU.hpp
        utility/hash.hpp)