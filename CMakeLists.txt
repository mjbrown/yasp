# -- Setup --

# Declare the minimum CMake requirement
cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

# Enforce separation of build directory and source directory
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "Prevented in-tree build.")
endif(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})

# Check the build type - if it's not defined by the user then set it to DEBUG
if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type specified - setting build type to DEBUG")
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING
        "Choose the type of build, options are: Debug Release"
        FORCE)
endif()

# -- Main Library --

# Declare the name and languages of this project
# Add CXX to include C++ support
project(YASP C)

# List all source files here, one per line.  Changes to header files are
# detected automatically, so headers do not need to be added here.
set(SRC_FILES
    yasp.c
)

# Define directories that contain local headers.
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Create a static library out of the YASP code
add_library(yasp STATIC ${SRC_FILES})

# -- Unit Tests --

# Include the static Unity Unit Test library. This will automatically create the
# variables "Unity_INCLUDE_DIRS" and "Unity_LIBRARIES"
add_subdirectory(unity)

# Add Unity header directories.
include_directories(
    ${Unity_INCLUDE_DIRS}
)

# List files that will generate unit tests. Again, headers do not need to be
# listed here.
set(UT_SRC_FILES
    test.c
)

# Add an executable with our unit test code, and link it to the YASP and Unity
# static libraries
add_executable(ut_yasp ${UT_SRC_FILES})
target_link_libraries(ut_yasp yasp ${Unity_LIBRARIES})

# Enable CMake unit test runner
enable_testing()

# Add ut_yasp as a test to be run with "make test"
add_test(test_yasp ut_yasp)
