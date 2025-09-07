cmake_minimum_required(VERSION 3.12)
project(VideoServer)

set(CMAKE_CXX_STANDARD 17)

find_package(Boost REQUIRED COMPONENTS system)

add_executable(server server.cpp)
target_link_libraries(server PRIVATE Boost::system pthread)
