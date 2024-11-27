cmake_minimum_required(VERSION 3.29)
project(my_malloc C)

set(CMAKE_C_STANDARD 11)

add_executable(my_malloc
        test_malloc.c
        my_malloc.h
        my_malloc.c)