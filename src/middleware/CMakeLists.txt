cmake_minimum_required(VERSION 2.6)
project(distribuidos)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")


add_executable(mom
        mom.cpp
        )

target_link_libraries(mom common)
