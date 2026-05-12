cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME stm32g4-uart)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Build_APP: ${BUILD_APP}")
message(STATUS "Build_TEST: ${BUILD_TEST}")
message(STATUS "System: ${CMAKE_SYSTEM_NAME}")

option(BUILD_APP "Building for app firmware" ON)
option(BUILD_TEST "Building for unit test" OFF)


if(BUILD_APP)

    # Create an executable object type
    add_executable(${CMAKE_PROJECT_NAME})
    # Add STM32CubeMX generated sources
    add_subdirectory(cmake/stm32cubemx)
    add_subdirectory(app)

    # Link directories setup
    target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
        # Add user defined library search paths
    )

    # Add project symbols (macros)
    target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
        # Add user defined symbols
    )

    # Remove wrong libob.a library dependency when using cpp files
    list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

    # Add linked libraries
    target_link_libraries(${CMAKE_PROJECT_NAME}
        stm32cubemx

        # Add user defined libraries
    )

endif()

if(BUILD_TEST)
    include(CTest)
    enable_testing()
    add_subdirectory(tests)
    
endif()