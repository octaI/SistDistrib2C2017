cmake_minimum_required(VERSION 2.6)
project(distribuidos C CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories("include")

add_library(common
        src/ipc/queue.cpp include/ipc/queue.h
        src/ipc/communicationqueue.cpp include/ipc/communicationqueue.h
        include/constants.h include/messages/message.h
        src/ipc/sharedmemory.cpp include/ipc/sharedmemory.h)

add_library(cinema-common
        src/db/sqlite3.c
        include/db/sqlite3.h
        include/db/sqlite3ext.h
        include/db/db_api.h
        src/db/db_api.cpp
        include/admin/admin.h)

add_subdirectory("src/utils")
add_subdirectory("src/cinema")
add_subdirectory("src/client")
#add_subdirectory("src/admin")
add_subdirectory("test")
add_subdirectory("test/db_test")

