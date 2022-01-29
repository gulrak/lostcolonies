include(CMakeDependentOption)

if(NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()

# avoid masses of targets for CDash
set_property(GLOBAL PROPERTY CTEST_TARGETS_ADDED 1)

# Organize unrelated targets to clean IDE hierarchy.
set(DEPENDENCY_FOLDER "thirdparty")

# This will also clean up the CMake ALL_BUILD, INSTALL, RUN_TESTS and ZERO_CHECK projects.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER ${DEPENDENCY_FOLDER})

option(GHC_FLAT_OUTPUT
        "Combine outputs (binaries, libs and archives) in top level bin and lib directories."
        OFF)
if(GHC_FLAT_OUTPUT)
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

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'RelWithDebInfo' as none was specified.")
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose the type of build." FORCE)
    # Set the possible values of build type for cmake-gui, ccmake
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()


find_program(CCACHE ccache)
if(CCACHE)
    message("using ccache")
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE})
else()
    message("ccache not found and not used")
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

# Generate compile_commands.json to make it easier to work with clang based
# tools
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(GHC_ENABLE_IPO "Enable inter-procedural optimization, aka Link Time Optimization (LTO)" OFF)
if(GHC_ENABLE_IPO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT result OUTPUT output)
    if(result)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    else()
        message(SEND_ERROR "IPO is not supported: ${output}")
    endif()
endif()

