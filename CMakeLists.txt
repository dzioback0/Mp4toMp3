cmake_minimum_required(VERSION 3.21)
project(mp4tomp3code)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_PREFIX_PATH "C:/Qt/6.10.2/msvc2022_64")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
add_executable(mp4tomp3code
        WIN32
        main.cpp
        app.rc
        Mainwindow.cpp
        Mainwindow.h)
target_link_libraries(mp4tomp3code PRIVATE Qt6::Widgets)