cmake_minimum_required(VERSION 3.23)
project(bmp2gba C)

set(CMAKE_C_STANDARD 99)

IF(WIN32)
    add_definitions(-DB2G_WIN32)
ELSEIF (APPLE)
    add_definitions(-DB2G_MACOS)
ELSEIF (UNIX)
    add_definitions(-DB2G_LINUX)
ENDIF ()

include_directories(lib/ lib/STC/include/)

set(SOURCE_FILES
        # Program
        src/b2g_main.c
        src/b2g_platform.c
        src/b2g_platform.h)

add_executable(bmp2gba ${SOURCE_FILES})
