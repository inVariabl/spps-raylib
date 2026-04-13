cmake_minimum_required(VERSION 3.20)
project(raylib_game C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Where raylib is located
set(RAYLIB_PATH "C:/raylib/raylib/src")

include_directories(${RAYLIB_PATH})
include_directories(${CMAKE_SOURCE_DIR}/src)
link_directories(${RAYLIB_PATH})

add_executable(game
    src/main.c
    src/objects.c
    src/ocean.c
    src/spawner.c
    src/collision.c
    src/crew.c
    src/interior.c
)

target_link_libraries(game
    raylib
    opengl32
    gdi32
    winmm
)