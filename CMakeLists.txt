cmake_minimum_required(VERSION 3.20)
project(spps_raylib C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(EMSCRIPTEN)
    option(SPPS_WEB_LITE "Build browser version without preloaded model assets" OFF)
    set(PLATFORM Web CACHE STRING "" FORCE)
endif()

set(SPPS_MAIN_SOURCES
    src/main.c
    src/platform_input.c
    src/player.c
    src/world.c
    src/ui.c
    src/combat.c
    src/npc.c
    src/ship_minigame.c
    spps-voyage/src/objects.c
    spps-voyage/src/ocean.c
    spps-voyage/src/spawner.c
    spps-voyage/src/collision.c
    spps-voyage/src/crew.c
    spps-voyage/src/interior.c
)

set(SPPS_VOYAGE_SOURCES
    src/platform_input.c
    spps-voyage/src/main.c
    spps-voyage/src/objects.c
    spps-voyage/src/ocean.c
    spps-voyage/src/spawner.c
    spps-voyage/src/collision.c
    spps-voyage/src/crew.c
    spps-voyage/src/interior.c
)

set(RAYLIB_VERSION 5.5)
set(SPPS_LOCAL_WEB_RAYLIB_DIR "${CMAKE_SOURCE_DIR}/build_web/raylib")

function(spps_ensure_raylib)
    if(TARGET raylib OR TARGET raylib_static)
        return()
    endif()

    if(EMSCRIPTEN AND EXISTS "${SPPS_LOCAL_WEB_RAYLIB_DIR}/CMakeLists.txt")
        set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
        set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
        add_subdirectory("${SPPS_LOCAL_WEB_RAYLIB_DIR}" "${CMAKE_BINARY_DIR}/raylib")
        return()
    endif()

    find_package(raylib ${RAYLIB_VERSION} QUIET)
    if(raylib_FOUND OR TARGET raylib OR TARGET raylib_static)
        return()
    endif()

    include(FetchContent)
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(
        raylib
        DOWNLOAD_EXTRACT_TIMESTAMP OFF
        URL https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz
    )
    FetchContent_MakeAvailable(raylib)
endfunction()

spps_ensure_raylib()

if(TARGET raylib)
    set(SPPS_RAYLIB_TARGET raylib)
elseif(TARGET raylib_static)
    set(SPPS_RAYLIB_TARGET raylib_static)
else()
    message(FATAL_ERROR "raylib target was not created")
endif()

add_executable(main ${SPPS_MAIN_SOURCES})
target_include_directories(main PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/spps-voyage/src
)
target_link_libraries(main PRIVATE ${SPPS_RAYLIB_TARGET})

if(EMSCRIPTEN)
    target_compile_definitions(main PRIVATE PLATFORM_WEB)
    target_link_options(main PRIVATE
        "SHELL:-sALLOW_MEMORY_GROWTH=1"
        "SHELL:-sEXPORTED_RUNTIME_METHODS=[requestFullscreen]"
        "SHELL:-sEXPORTED_FUNCTIONS=['_main','_SPPS_WebSetKeyState','_SPPS_WebPulseKey','_SPPS_WebResetInputs']"
        "SHELL:-sUSE_GLFW=3"
        "SHELL:-sFORCE_FILESYSTEM=1"
        "SHELL:-sMIN_WEBGL_VERSION=2"
        "SHELL:-sMAX_WEBGL_VERSION=2"
        "SHELL:--shell-file ${CMAKE_SOURCE_DIR}/web/shell.html"
    )
    if(NOT SPPS_WEB_LITE)
        target_link_options(main PRIVATE
            "SHELL:--preload-file ${CMAKE_SOURCE_DIR}/assets/snake.glb@/assets/snake.glb"
            "SHELL:--preload-file ${CMAKE_SOURCE_DIR}/assets/column.glb@/assets/column.glb"
            "SHELL:--preload-file ${CMAKE_SOURCE_DIR}/assets/roman_temple.glb@/assets/roman_temple.glb"
            "SHELL:--preload-file ${CMAKE_SOURCE_DIR}/assets/desert_house.glb@/assets/desert_house.glb"
            "SHELL:--preload-file ${CMAKE_SOURCE_DIR}/assets/ancient_character.glb@/assets/ancient_character.glb"
            "SHELL:--preload-file ${CMAKE_SOURCE_DIR}/assets/viking_boat.glb@/assets/viking_boat.glb"
            "SHELL:--preload-file ${CMAKE_SOURCE_DIR}/assets/al_capone.glb@/assets/al_capone.glb"
            "SHELL:--preload-file ${CMAKE_SOURCE_DIR}/assets/paul.glb@/assets/paul.glb"
            "SHELL:--preload-file ${CMAKE_SOURCE_DIR}/assets/roman_character.glb@/assets/roman_character.glb"
            "SHELL:--preload-file ${CMAKE_SOURCE_DIR}/assets/roman_soldier.glb@/assets/roman_soldier.glb"
            "SHELL:--preload-file ${CMAKE_SOURCE_DIR}/spps-voyage/models/boat.glb@/spps-voyage/models/boat.glb"
            "SHELL:--preload-file ${CMAKE_SOURCE_DIR}/spps-voyage/models/malta_island.glb@/spps-voyage/models/malta_island.glb"
            "SHELL:--preload-file ${CMAKE_SOURCE_DIR}/spps-voyage/models/rock.glb@/spps-voyage/models/rock.glb"
            "SHELL:--preload-file ${CMAKE_SOURCE_DIR}/spps-voyage/models/gold_coin.glb@/spps-voyage/models/gold_coin.glb"
            "SHELL:--preload-file ${CMAKE_SOURCE_DIR}/spps-voyage/models/roman_character.glb@/spps-voyage/models/roman_character.glb"
        )
    endif()
    set_target_properties(main PROPERTIES SUFFIX ".html")
else()
    add_executable(voyage ${SPPS_VOYAGE_SOURCES})
    target_include_directories(voyage PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/spps-voyage/src
    )
    target_link_libraries(voyage PRIVATE ${SPPS_RAYLIB_TARGET})

    if(WIN32)
        target_link_libraries(main PRIVATE opengl32 gdi32 winmm)
        target_link_libraries(voyage PRIVATE opengl32 gdi32 winmm)
    elseif(APPLE)
        target_link_libraries(main PRIVATE "-framework OpenGL" "-framework Cocoa" "-framework IOKit")
        target_link_libraries(voyage PRIVATE "-framework OpenGL" "-framework Cocoa" "-framework IOKit")
    elseif(UNIX)
        target_link_libraries(main PRIVATE m pthread dl GL)
        target_link_libraries(voyage PRIVATE m pthread dl GL)
    endif()
endif()
