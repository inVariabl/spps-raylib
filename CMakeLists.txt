# cmake -S . -B out-gcc -G "MinGW Makefiles"
# cmake --build out-gcc
# ./out-gcc/game.exe 


cmake_minimum_required(VERSION 3.20)
project(spps_raylib C)

set(CMAKE_C_STANDARD 99)

set(RAYLIB_PATH "C:/raylib/raylib/src")

include_directories(${RAYLIB_PATH})
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/spps-voyage/src)

link_directories(${RAYLIB_PATH})

file(GLOB SRC_FILES src/*.c)
list(APPEND SRC_FILES
    spps-voyage/src/objects.c
    spps-voyage/src/ocean.c
    spps-voyage/src/spawner.c
    spps-voyage/src/collision.c
    spps-voyage/src/crew.c
    spps-voyage/src/interior.c
)

set(VOYAGE_SRC_FILES
    spps-voyage/src/main.c
    spps-voyage/src/objects.c
    spps-voyage/src/ocean.c
    spps-voyage/src/spawner.c
    spps-voyage/src/collision.c
    spps-voyage/src/crew.c
    spps-voyage/src/interior.c
)

add_executable(main ${SRC_FILES})
add_executable(voyage ${VOYAGE_SRC_FILES})

target_link_libraries(main raylib)
target_link_libraries(voyage raylib)

# Platform-specific stuff
if (WIN32)
    target_link_libraries(main opengl32 gdi32 winmm)
    target_link_libraries(voyage opengl32 gdi32 winmm)
elseif (APPLE)
    target_link_libraries(main "-framework OpenGL" "-framework Cocoa" "-framework IOKit")
    target_link_libraries(voyage "-framework OpenGL" "-framework Cocoa" "-framework IOKit")
elseif (UNIX)
    target_link_libraries(main m pthread dl GL)
    target_link_libraries(voyage m pthread dl GL)
endif()
