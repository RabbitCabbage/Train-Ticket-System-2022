cmake_minimum_required(VERSION 3.16)
project(TrainTicketSystem)

set(CMAKE_CXX_STANDARD 14)
# set(CMAKE_CXX_FLAGS_DEBUG "-g -DDEBUG -Wall -fsanitize=undefined -fsanitize=address")

include_directories(db)
include_directories(lib)
include_directories(src)

add_executable(TrainTicketSystem
        lib/linked_hashmap.hpp
        lib/map.hpp
        lib/vector.hpp
        lib/utility.hpp
        lib/exceptions.hpp
        src/Commander.hpp
        src/main.cpp
        src/RollBack.hpp
        src/System.hpp
        lib/tools.hpp)