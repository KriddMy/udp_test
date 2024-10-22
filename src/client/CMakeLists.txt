message("Compiling ${CMAKE_CURRENT_SOURCE_DIR} folder...")

add_executable(client main.cpp)
target_link_libraries(client PRIVATE core)
target_include_directories(client PRIVATE ${CMAKE_SOURCE_DIR}/include)