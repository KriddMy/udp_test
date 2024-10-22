message("Compiling ${CMAKE_CURRENT_SOURCE_DIR} folder...")

add_library(core UdpServer.cpp PacketReciever.cpp FileSender.cpp)
target_include_directories(core PRIVATE ${CMAKE_SOURCE_DIR}/include)