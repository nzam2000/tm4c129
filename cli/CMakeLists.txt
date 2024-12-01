cmake_minimum_required(VERSION 3.29)
project(tm4c129_cli)
set(CMAKE_CXX_STANDARD 20)

set(SOURCES
        src/main.cpp
)

include_directories(
        src
)

add_executable(${PROJECT_NAME} ${SOURCES})
