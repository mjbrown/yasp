cmake_minimum_required(VERSION 3.10)
project(yasp C)

set(CMAKE_C_STANDARD 99)

add_executable(yasp main.c fifo.c fifo.h packetize.c packetize.h util.c util.h)