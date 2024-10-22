cmake_minimum_required(VERSION 3.28)

project(UdpPackets)
message("Starting Building the project ${CMAKE_PROJECT_NAME}...")

add_compile_options(
    -Wall
    -Wextra
)

add_subdirectory(src/client)
add_subdirectory(src/server)
add_subdirectory(src/core)
