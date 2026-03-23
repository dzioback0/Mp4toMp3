cmake_minimum_required(VERSION 3.21)
project(mp4tomp3code)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_PREFIX_PATH "C:/Qt/6.10.2/msvc2022_64")

find_package(Qt6 REQUIRED COMPONENTS Widgets)
add_executable(mp4tomp3code main.cpp app.rc)
target_link_libraries(mp4tomp3code PRIVATE Qt6::Widgets)