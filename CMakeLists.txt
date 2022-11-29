cmake_minimum_required(VERSION 3.22)
project(untitled C)

set(CMAKE_C_STANDARD 17)

add_executable(untitled shell.c shellCode.c shellCode.h)
