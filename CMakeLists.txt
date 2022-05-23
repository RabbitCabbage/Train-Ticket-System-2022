cmake_minimum_required(VERSION 3.16)
project(TrainTicketSystem)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-Ofast")
# set(CMAKE_CXX_FLAGS_DEBUG "-g -DDEBUG -Wall -fsanitize=undefined -fsanitize=address")

include_directories(BackEnd/db)
include_directories(BackEnd/lib)
include_directories(BackEnd/src)

add_executable(code BackEnd/src/main.cpp)
