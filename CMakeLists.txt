cmake_minimum_required(VERSION 3.10)
project(Home_controller_C C)


set(CMAKE_C_STANDARD 11)

add_executable(Home_controller_C  "main.c" basicComm.c xmlget.c)
target_link_libraries(Home_controller_C pthread)
