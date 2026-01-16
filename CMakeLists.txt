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

add_compile_options(-g -Wall -Wextra -Wpedantic)

add_executable(fred-tester
    src/main.cpp
    src/CommandTest.cpp
    src/MapiHandler.cpp
    src/TrackingTest.cpp
    src/FredTesterRpc.cpp
    src/Logger.cpp
    src/utils.cpp
    src/tests/Configurations.cpp
    src/tests/CounterRates.cpp
    src/tests/FredTester.cpp
    src/tests/HistogramsSingle.cpp
    src/tests/HistogramsTracking.cpp
    src/tests/Parameters.cpp
    src/tests/ResetErrors.cpp
    src/tests/ResetSystem.cpp
    src/tests/Status.cpp

    src/tests/ConfigBuilder.cpp
    src/tests/TesterConfig.cpp
)

target_include_directories(fred-tester PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)
target_include_directories(fred-tester PUBLIC /opt/dim/dim)
target_include_directories(fred-tester PUBLIC /usr/local/include/dim)

# External
target_include_directories(fred-tester PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/external/argparse/include/
    ${CMAKE_CURRENT_SOURCE_DIR}/external/tomlplusplus/include/
)

target_precompile_headers(fred-tester PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/external/argparse/include/argparse/argparse.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/external/tomlplusplus/include/toml++/toml.hpp
)

target_compile_definitions(fred-tester PUBLIC TOML_EXCEPTIONS=0)

target_link_libraries(fred-tester ${DIM_LIBRARY} pthread)

# Address sanitizer
add_compile_options(-fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer)
target_compile_definitions(fred-tester PRIVATE _GLIBCXX_ASSERTIONS)
target_link_options(fred-tester PRIVATE -fsanitize=address -fsanitize=undefined)

