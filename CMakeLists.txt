cmake_minimum_required(VERSION 3.16)
project(Project2)

set(CMAKE_CXX_STANDARD 11)

include_directories(.)

add_executable(Project2
        main.cpp
        node.cpp
        node.h
        record.h
        bptree.cpp
        bptree.h)

add_executable(Test
        node.cpp
        node.h
        record.h
        bptree.cpp
        bptree.h
        test/unitTest.h
        test/unitTest.cpp
        test/test.cpp)

