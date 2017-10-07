cmake_minimum_required(VERSION 2.6)
project(distribuidos)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_executable(client
        client.cpp # main()
        ../../include/utils/TextTable.h ../../include/client/client_utils.h client_utils.cpp ../../include/middleware/client_interface.h ../middleware/client_interface.cpp)

target_link_libraries(client common)