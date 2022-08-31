cmake_minimum_required(VERSION 3.23)
project(bmp2gba C)

set(CMAKE_C_STANDARD 99)

IF(WIN32)
    add_definitions(-DB2G_WIN32)
ENDIF()

include_directories(lib/)

set(SOURCE_FILES
        # Program
        src/b2g_main.c

        # Libraries
        lib/stb_image.c)

add_executable(bmp2gba ${SOURCE_FILES})
