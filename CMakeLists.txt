cmake_minimum_required(VERSION 3.20)
project(GomokuMCTS)

set(CMAKE_CXX_STANDARD 23)

set(SOURCE_FILES
    src/State.cpp
    src/Statistics.cpp
    src/Node.cpp
    src/MCTS.cpp
    src/Config.cpp
)

add_executable(AlphaGomoku ${SOURCE_FILES})

set(CMAKE_CXX_FLAGS "-O3")
