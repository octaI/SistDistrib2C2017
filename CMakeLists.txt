cmake_minimum_required(VERSION 2.6)
project(distribuidos)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories("include")

add_library(common
        src/ipc/queue.cpp include/ipc/queue.h
        src/ipc/communicationqueue.cpp include/ipc/communicationqueue.h
        include/constants.h include/messages/message.h
        )

add_library(cinema-common
        src/db/sqlite3.c include/db/sqlite3.h include/db/sqlite3ext.h
        )

add_subdirectory("src/utils")
add_subdirectory("src/cinema")
add_subdirectory("src/client")
add_subdirectory("test")


