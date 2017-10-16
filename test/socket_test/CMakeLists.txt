cmake_minimum_required(VERSION 2.6)
project(distribuidos)


add_executable(socket_test
        socket_test.cpp # main()
        )

target_link_libraries(socket_test
        common
        dl
        pthread
        )
