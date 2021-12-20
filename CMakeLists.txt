cmake_minimum_required(VERSION 2.8)
project(MultiEvent)

include_directories(./src)

aux_source_directory(./src src)

add_executable(server ${src} server.cpp)
add_executable(client ${src} client.cpp)

target_link_libraries(server event_core event_pthreads pthread)
target_link_libraries(client event_core event_pthreads pthread)