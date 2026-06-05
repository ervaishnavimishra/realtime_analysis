cmake_minimum_required(VERSION 3.20)

project(realtime_analysis)

set(CMAKE_CXX_STANDARD 20)

add_executable(
    realtime_analysis
    src/main.cpp
    src/websocket_client.cpp
)

target_link_libraries(
    realtime_analysis
    ws2_32
    ssl
    crypto
)