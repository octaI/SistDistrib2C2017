cmake_minimum_required(VERSION 2.6)
project(distribuidos)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_library(common
        src/ipc/queue.cpp src/ipc/communicationqueue.cpp src/db/sqlite3.c src/db/db_api.cpp)

add_subdirectory("src/utils")
add_subdirectory("src/cinema")
add_subdirectory("src/client")
add_subdirectory("test")
add_subdirectory("src/admin")

