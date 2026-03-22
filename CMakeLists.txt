# cmake -S . -B out-gcc -G "MinGW Makefiles"
# cmake --build out-gcc
# ./out-gcc/game.exe 


cmake_minimum_required(VERSION 3.20)
project(spps_raylib C)

set(CMAKE_C_STANDARD 99)

set(RAYLIB_PATH "C:/raylib/raylib/src")

include_directories(${RAYLIB_PATH})
include_directories(${CMAKE_SOURCE_DIR}/src)

link_directories(${RAYLIB_PATH})

file(GLOB SRC_FILES src/*.c)

add_executable(game ${SRC_FILES})

target_link_libraries(game raylib)

# Platform-specific stuff
if (WIN32)
    target_link_libraries(game opengl32 gdi32 winmm)
elseif (APPLE)
    target_link_libraries(game "-framework OpenGL" "-framework Cocoa" "-framework IOKit")
elseif (UNIX)
    target_link_libraries(game m pthread dl GL)
endif()