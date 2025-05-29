cmake_minimum_required(VERSION 3.10)
project(fred-tester)

# Prevent in-source builds
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
  message(FATAL_ERROR "In-source builds are not allowed. Please create a separate build directory.")
endif()

# Find DIM
set(DIM_ROOT "" CACHE PATH "Root directory of DIM library")

# Optionally add include and lib directories from DIM_ROOT
if(DIM_ROOT)
    set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "${DIM_ROOT}/dim")
    set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "${DIM_ROOT}/linux")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(fred-tester
    src/main.cpp
    src/FredTester.cpp
)

target_include_directories(fred-tester PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)
target_include_directories(fred-tester PUBLIC /usr/local/include/dim/)

