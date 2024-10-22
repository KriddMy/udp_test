message("Compiling ${CMAKE_CURRENT_SOURCE_DIR} folder...")

add_executable(server main.cpp)
target_link_libraries(server PRIVATE core)
target_include_directories(server PRIVATE ${CMAKE_SOURCE_DIR}/include)