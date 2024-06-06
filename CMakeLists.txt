cmake_minimum_required(VERSION 3.15.0)
project(micro_os C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(micro_os
        fat_helper.c
        main.c
        lru_cache.c
ansi_repl.c)
