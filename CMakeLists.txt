cmake_minimum_required(VERSION 3.21)
project(SimpleVector)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra -Wstrict-overflow -Werror=vla -fsanitize=address -g -O1 -fno-omit-frame-pointer -fno-optimize-sibling-calls")

add_executable(SimpleVector main.cpp tests.h simple_vector.h array_ptr.h)
