cmake_minimum_required(VERSION 2.6)
project(distribuidos)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")


add_executable(cinema
        cinema.cpp # main()
        cinema_handler.cpp
        )

target_link_libraries(cinema common)
