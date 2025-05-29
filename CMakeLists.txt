cmake_minimum_required(VERSION 3.10)
project(fred-tester)

# Prevent in-source builds
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
  message(FATAL_ERROR "In-source builds are not allowed. Please create a separate build directory.")
endif()

find_library(DIM_LIBRARY NAMES dim REQUIRED)

if(NOT DIM_LIBRARY)
    message(FATAL_ERROR "DIM library not found. Please install DIM library and try again.")
else()
    message(STATUS "DIM library found: ${DIM_LIBRARY}")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(fred-tester
    src/main.cpp
    src/FredTester.cpp
)

target_include_directories(fred-tester PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)
target_include_directories(fred-tester PUBLIC /opt/dim/dim)

target_link_libraries(fred-tester ${DIM_LIBRARY} pthread)

