cmake_minimum_required(VERSION 3.21)
project(cpp-simple-vector)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra -Wstrict-overflow -Werror=vla -fsanitize=address -g -O1 -fno-omit-frame-pointer -fno-optimize-sibling-calls")

add_executable(cpp-simple-vector simple-vector/main.cpp simple-vector/tests.h simple-vector/simple_vector.h simple-vector/array_ptr.h)
