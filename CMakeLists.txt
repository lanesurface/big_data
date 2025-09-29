cmake_minimum_required(VERSION 3.25)
project(data_structs LANGUAGES C)

add_executable(hash)
target_sources(hash PRIVATE hmap.c main.c)
target_compile_options(hash PRIVATE "-g" "-Wall" "-Wextra")
target_compile_features(hash PRIVATE c_std_11)
