cmake_minimum_required(VERSION 2.6)
project(distribuidos)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_executable(initializer
        initializer.cpp # main()
        )
target_link_libraries(initializer common)

add_executable(destructor
        destructor.cpp # main()
        )

target_link_libraries(destructor common)

add_executable(database_data_init database_data_init.cpp
        )

target_link_libraries(database_data_init cinema-common
        dl
        pthread)