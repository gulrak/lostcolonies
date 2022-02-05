cmake_minimum_required(VERSION 3.16)

project(lostcolonies VERSION 1.1 LANGUAGES C CXX)

if(EMSCRIPTEN)
    set(PLATFORM "Web")
endif()

set(WITH_FLAT_OUTPUT ON CACHE BOOL "Build with collecting output directories bin&lib")

include(cmake/CompilerOptions.cmake)

include(FetchContent)
FetchContent_Declare(
    raylib
    URL https://github.com/raysan5/raylib/archive/refs/tags/4.0.0.tar.gz
)
FetchContent_GetProperties(raylib)
if (NOT raylib_POPULATED)
    set(FETCHCONTENT_QUIET NO)
    FetchContent_Populate(raylib)
    if(NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
        set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
        set(SUPPORT_SCREEN_CAPTURE OFF CACHE BOOL "" FORCE)
        set(SUPPORT_GIF_RECORDING OFF CACHE BOOL "" FORCE)
        set(SUPPORT_FILEFORMAT_DDS OFF CACHE BOOL "" FORCE)
        set(SUPPORT_FILEFORMAT_HDR OFF CACHE BOOL "" FORCE)
        set(SUPPORT_FILEFORMAT_KTX OFF CACHE BOOL "" FORCE)
        set(SUPPORT_FILEFORMAT_ASTC OFF CACHE BOOL "" FORCE)
        set(SUPPORT_FILEFORMAT_FNT OFF CACHE BOOL "" FORCE)
        set(SUPPORT_FILEFORMAT_TTF OFF CACHE BOOL "" FORCE)
        set(SUPPORT_MESH_GENERATION OFF CACHE BOOL "" FORCE)
        set(SUPPORT_FILEFORMAT_OBJ OFF CACHE BOOL "" FORCE)
        set(SUPPORT_FILEFORMAT_MTL OFF CACHE BOOL "" FORCE)
        set(SUPPORT_FILEFORMAT_IQM OFF CACHE BOOL "" FORCE)
        set(SUPPORT_FILEFORMAT_GLTF OFF CACHE BOOL "" FORCE)
        set(SUPPORT_FILEFORMAT_VOX OFF CACHE BOOL "" FORCE)
        set(SUPPORT_FILEFORMAT_OGG OFF CACHE BOOL "" FORCE)
        set(SUPPORT_FILEFORMAT_XM OFF CACHE BOOL "" FORCE)
        set(SUPPORT_FILEFORMAT_MOD OFF CACHE BOOL "" FORCE)
        set(SUPPORT_FILEFORMAT_MP3 OFF CACHE BOOL "" FORCE)
        set(SUPPORT_FILEFORMAT_FLAC OFF CACHE BOOL "" FORCE)
        #set(SUPPORT_TRACELOG OFF CACHE BOOL "" FORCE)
    endif()
    add_subdirectory(${raylib_SOURCE_DIR} ${raylib_BINARY_DIR})
endif()

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug" OR WITH_RECORDING)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBAV IMPORTED_TARGET
        libavdevice
        libavfilter
        libavformat
        libavcodec
        libswresample
        libswscale
        libavutil
    )
endif()


set(GAME_SOURCES
    src/main.cpp
    src/gradient.cpp
    src/scene_base.hpp
    src/scene_setup.hpp
    src/scene_ingame.hpp
    src/scene_generators.hpp
    src/scene.cpp
    src/sprite.hpp
    src/sound.hpp
    src/sound.cpp
    src/random.hpp
    src/random.cpp
    src/config.hpp
    src/math.hpp
    src/particles.hpp
    src/hash.hpp
    src/cli.hpp
    src/procedural.hpp
    src/procedural.cpp
    src/spritemanager.hpp
    src/spritemanager.cpp
    src/vectorfont.hpp
    src/vectorfont.cpp
    src/font8x8.h)

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    list(APPEND GAME_SOURCES src/scene_generators.cpp src/scene_generators.hpp)
endif()
if(LIBAV_FOUND)
    list(APPEND GAME_SOURCES src/ffmpeg_encode.hpp src/ffmpeg_encode.cpp)
endif()

add_executable(lostcolonies ${GAME_SOURCES})
generate_version_info(lostcolonies)
target_link_libraries(lostcolonies PRIVATE compiler_options raylib)
if(LIBAV_FOUND)
    target_link_libraries(lostcolonies PRIVATE PkgConfig::LIBAV)
    target_compile_definitions(lostcolonies PRIVATE WITH_FFMPEG)
endif()
if (${PLATFORM} MATCHES "Web")
    set_target_properties(lostcolonies PROPERTIES LINK_FLAGS "-Os -s USE_GLFW=3 -s USE_WEBGL2=1 -s WASM=1 -s ALLOW_MEMORY_GROWTH=1 --no-heap-copy --shell-file ${CMAKE_CURRENT_SOURCE_DIR}/resource_web/minishell.html")
    set_target_properties(lostcolonies PROPERTIES OUTPUT_NAME "index")
    set_target_properties(lostcolonies PROPERTIES SUFFIX ".html")
endif ()
