cmake_minimum_required(VERSION 3.20)
project(ConsoleMVC CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ENABLE_TESTS "Enable test build with coverage (Clang)" OFF)

if(ENABLE_TESTS)
    add_compile_options("-fprofile-instr-generate" "-fcoverage-mapping")
    add_link_options("-fprofile-instr-generate")
    add_subdirectory(lib/googletest-main)
endif()

set(JSON_BUILD_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(lib/json)

add_subdirectory(src/mvc)

add_executable(${PROJECT_NAME}
    src/main.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE mvc)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /utf-8)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)
endif()

