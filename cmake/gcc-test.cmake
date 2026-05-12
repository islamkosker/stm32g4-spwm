# Host system (Windows + GCC)
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(CMAKE_C_COMPILER_ID GNU)
set(CMAKE_CXX_COMPILER_ID GNU)

if(WIN32)
    set(COMPILER_PATH   "C:/compiler/gcc/bin/")
    set(EXT             ".exe")

else()
    set(COMPILER_PATH   "") 
    set(EXT             "")
    set(CMAKE_SYSTEM_NAME Linux)

endif()

set(TOOLCHAIN_PREFIX        ${COMPILER_PATH})


set(CMAKE_C_COMPILER        ${TOOLCHAIN_PREFIX}gcc${EXT})
set(CMAKE_CXX_COMPILER      ${TOOLCHAIN_PREFIX}g++${EXT})


# Native executable
set(CMAKE_EXECUTABLE_SUFFIX ${EXT})

# Try compile fix
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(COMMON_FLAGS "-Wall -Wextra -Wpedantic")

# Debug / Release / Test
set(CMAKE_C_FLAGS_DEBUG   "-O0 -g3")
set(CMAKE_C_FLAGS_RELEASE "-O2")

set(CMAKE_CXX_FLAGS_DEBUG   "-O0 -g3")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

# Apply common flags
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} ${COMMON_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_FLAGS}")


set(CMAKE_C_FLAGS_DEBUG   "${CMAKE_C_FLAGS_DEBUG} ${SANITIZER_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${SANITIZER_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${SANITIZER_FLAGS}")

# Coverage (opsiyonel)
option(ENABLE_COVERAGE "Enable coverage reporting" OFF)

if(ENABLE_COVERAGE)
    message(STATUS "Coverage enabled")
    set(COVERAGE_FLAGS "--coverage")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COVERAGE_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${COVERAGE_FLAGS}")
endif()