cmake_minimum_required(VERSION 2.6)
project(distribuidos)


add_executable(dbtest
        db_test.cpp # main()
        )

target_link_libraries(dbtest
        cinema-common
        dl
        pthread
        )
