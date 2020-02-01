cmake_minimum_required(VERSION 3.14)
project(Client C)

set(CMAKE_C_STANDARD 11)

add_executable(Client main.c cJSON.h cJSON.c client.c client.h UI.c UI.h)

target_link_libraries(Client ws2_32)