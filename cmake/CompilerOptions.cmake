if(compiler_included)
    return()
endif()
set(compiler_included true)

include(CMakeDependentOption)

if(NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS ON)
endif()

# avoid masses of targets for CDash
set_property(GLOBAL PROPERTY CTEST_TARGETS_ADDED 1)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    if(${PLATFORM} MATCHES "Web")
        message(STATUS "Setting build type to 'MinSizeRel' as none was specified.")
        set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose the type of build." FORCE)
        set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
    else()
        message(STATUS "Setting build type to 'RelWithDebInfo' as none was specified.")
        set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose the type of build." FORCE)
        set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
    endif()
endif()

option(WITH_FLAT_OUTPUT
    "Combine outputs (binaries, libs and archives) in top level bin and lib directories."
    OFF)

if(WITH_FLAT_OUTPUT)
    link_directories(${CMAKE_BINARY_DIR}/lib)
    set(BINARY_OUT_DIR ${CMAKE_BINARY_DIR}/bin)
    set(LIB_OUT_DIR ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BINARY_OUT_DIR})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${LIB_OUT_DIR})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${LIB_OUT_DIR})

    foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
        string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${BINARY_OUT_DIR})
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${LIB_OUT_DIR})
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${LIB_OUT_DIR})
    endforeach()
    message("using combined output directories ${BINARY_OUT_DIR} and ${LIB_OUT_DIR}")
else()
    message("not combining output")
endif()

add_library(compiler_options INTERFACE)


option(WITH_WARNING_AS_ERROR "Treats compiler warnings as errors" OFF)
if(MSVC)
    target_compile_options(compiler_options INTERFACE /W4 "/permissive-")
    if(WITH_WARNING_AS_ERROR)
        target_compile_options(compiler_options INTERFACE /WX /wd"4996")
    endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(compiler_options INTERFACE
        -Wall
        -Wextra
        -Wno-shadow
        -Wnon-virtual-dtor
        -Wcast-align
        -Wunused
        -Woverloaded-virtual
        -Wpedantic
        -Wconversion
        -Wno-sign-conversion
        -Wnull-dereference
        -Wno-double-promotion
        -Wno-missing-field-initializers
        -Wno-sign-compare
        -Wno-float-conversion
        -Wno-unused-parameter
        -Wformat=2
        )
    if(WITH_WARNING_AS_ERROR)
        target_compile_options(compiler_options INTERFACE -Werror -Wno-error=deprecated-declarations)
    endif()

    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        target_compile_options(compiler_options INTERFACE
            -Wmisleading-indentation
            -Wduplicated-cond
            -Wduplicated-branches
            -Wlogical-op
            -Wuseless-cast
            )
    else()
        target_compile_options(compiler_options INTERFACE
            -Wno-implicit-int-conversion
            -Wno-gnu-binary-literal
            -Wno-newline-eof
            -Wno-shorten-64-to-32
        )
    endif()
endif()

option(WITH_PCH "Enable Precompiled Headers" OFF)
if(WITH_PCH)
    target_precompile_headers(compiler_options INTERFACE
        <algorithm>
        <array>
        <vector>
        <string>
        <utility>
        <functional>
        <memory>
        <memory_resource>
        <string_view>
        <cmath>
        <cstddef>
        <type_traits>
        )
endif()

option(WITH_ASAN "Enable the Address Sanitizers" OFF)
if(WITH_ASAN)
    message("Enable Address Sanitizer")
    target_compile_options(compiler_options INTERFACE -fsanitize=address -fno-omit-frame-pointer)
    target_link_libraries(compiler_options INTERFACE -fsanitize=address)
endif()

option(WITH_TSAN "Enable the Thread Sanitizers" OFF)
if(WITH_TSAN)
    message("Enable Thread Sanitizer")
    target_compile_options(compiler_options INTERFACE -fsanitize=thread)
    target_link_libraries(compiler_options INTERFACE -fsanitize=thread)
endif()

option(WITH_MSAN "Enable the Memory Sanitizers" OFF)
if(WITH_MSAN)
    message("Enable Memory Sanitizer")
    target_compile_options(compiler_options INTERFACE -fsanitize=memory -fno-omit-frame-pointer)
    target_link_libraries(compiler_options INTERFACE -fsanitize=memory)
endif()

option(WITH_UBSAN "Enable the Undefined Behavior Sanitizers" OFF)
if(WITH_UBSAN)
    message("Enable Undefined Behavior Sanitizer")
    target_compile_options(compiler_options INTERFACE -fsanitize=undefined)
    target_link_libraries(compiler_options INTERFACE -fsanitize=undefined)
endif()

if (${PLATFORM} MATCHES "Web")
    target_link_options(compiler_options INTERFACE -Os -s USE_GLFW=3 -s USE_WEBGL2=1 -s WASM=1 -s ALLOW_MEMORY_GROWTH=1 --no-heap-copy --shell-file ${CMAKE_CURRENT_SOURCE_DIR}/minishell.html)
endif()

find_program(CCACHE ccache)
if(CCACHE)
    message("using ccache")
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE})
else()
    message("ccache not found and not used")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(WITH_IPO "Enable inter-procedural optimization, aka Link Time Optimization (LTO)" OFF)
if(WITH_IPO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT result OUTPUT output)
    if(result)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    else()
        message(SEND_ERROR "IPO is not supported: ${output}")
    endif()
endif()

find_package(Git)

macro(generate_version_info targetName)
    if(GIT_FOUND)
        string(TOLOWER ${targetName} PROJECT_LOWERCASE_NAME)
        string(TOUPPER ${targetName} PROJECT_UPPERCASE_NAME)
        execute_process(
            COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE GIT_BRANCH
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        execute_process(
            COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE GIT_COMMIT_HASH
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        set(GIT_COMMIT_HASH_PREFIXED "-${GIT_COMMIT_HASH}")
        if(NOT PROJECT_VERSION_PATCH)
            execute_process(
                COMMAND ${GIT_EXECUTABLE} rev-list HEAD --count
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                OUTPUT_VARIABLE PROJECT_VERSION_PATCH
                OUTPUT_STRIP_TRAILING_WHITESPACE
            )
        endif()
    endif()

    configure_file(${CMAKE_SOURCE_DIR}/cmake/version.hpp.in ${CMAKE_BINARY_DIR}/${targetName}/version.hpp)
    include_directories(${CMAKE_BINARY_DIR})
endmacro()
