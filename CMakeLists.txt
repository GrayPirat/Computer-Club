cmake_minimum_required(VERSION 3.10)
project(computer_club)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

add_executable(computer_club 
    src/main.cpp 
    src/computer_club.cpp
)