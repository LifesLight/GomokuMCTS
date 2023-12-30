cmake_minimum_required(VERSION 3.20)
project(GomokuMCTS)

set(CMAKE_CXX_STANDARD 23)

set(SOURCE_FILES
    src/Dev.cpp
    src/Randomizer.cpp
    src/State.cpp
    src/Statistics.cpp
    src/Node.cpp
    src/main.cpp
)

add_executable(GomokuMCTS ${SOURCE_FILES})

set(CMAKE_CXX_FLAGS "-O3 -flto -march=native -fno-stack-protector -Wall -Wextra -pedantic")
