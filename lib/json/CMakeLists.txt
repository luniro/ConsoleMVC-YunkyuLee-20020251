cmake_minimum_required(VERSION 3.16)
project(json_lib VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(json_lib
    src/json_value.cpp
    src/lexer.cpp
    src/parser.cpp
    src/serializer.cpp
)

target_include_directories(json_lib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

if(MSVC)
    target_compile_options(json_lib PRIVATE /W4)
else()
    target_compile_options(json_lib PRIVATE -Wall -Wextra -Wpedantic)
endif()

option(JSON_BUILD_TESTS "Build tests" ON)
if(JSON_BUILD_TESTS)
    add_executable(json_tests tests/test_main.cpp)
    target_link_libraries(json_tests PRIVATE json_lib)
    if(MSVC)
        target_compile_options(json_tests PRIVATE /W4)
    else()
        target_compile_options(json_tests PRIVATE -Wall -Wextra)
    endif()
endif()
