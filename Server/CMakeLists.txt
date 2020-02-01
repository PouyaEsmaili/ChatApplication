cmake_minimum_required(VERSION 3.14)
project(Server C)

set(CMAKE_C_STANDARD 11)

add_executable(Server main.c cJSON.h cJSON.c sha256.h sha256.c base64.h base64.c server.c server.h file.c file.h json.c json.h)

target_link_libraries(Server ws2_32)