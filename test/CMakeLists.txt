cmake_minimum_required(VERSION 2.6)
project(distribuidos)


add_executable(commqueuetest
        commqueue_test.cpp # main()
        )

target_link_libraries(commqueuetest common)
