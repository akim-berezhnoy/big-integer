cmake_minimum_required(VERSION 3.21)
project(bigint)

set(CMAKE_CXX_STANDARD 20)

find_package(GTest REQUIRED)

add_executable(tests tests.cpp big_integer.cpp)

if(MSVC)
    target_compile_options(tests PRIVATE /W4 /permissive-)
    if(TREAT_WARNINGS_AS_ERRORS)
        target_compile_options(tests PRIVATE /WX)
    endif()
else()
    target_compile_options(tests PRIVATE -Wall -Wextra -pedantic -Wno-sign-compare -Wold-style-cast)
    if(TREAT_WARNINGS_AS_ERRORS)
        target_compile_options(tests PRIVATE -Werror)
    endif()
endif()

# Compiler specific warnings
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(tests PRIVATE -Wshadow=compatible-local)
    target_compile_options(tests PRIVATE -Wduplicated-branches)
    target_compile_options(tests PRIVATE -Wduplicated-cond)
    target_compile_options(tests PRIVATE -Wnull-dereference)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(tests PRIVATE -Wshadow-uncaptured-local)
    target_compile_options(tests PRIVATE -Wloop-analysis)
    target_compile_options(tests PRIVATE -Wno-self-assign-overloaded)
endif()

option(USE_SANITIZERS "Enable to build with undefined,leak and address sanitizers" OFF)
if(USE_SANITIZERS)
    message(STATUS "Enabling sanitizers...")
    target_compile_options(tests PUBLIC -fsanitize=address,undefined,leak -fno-sanitize-recover=all)
    target_link_options(tests PUBLIC -fsanitize=address,undefined,leak)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(STATUS "Enabling libc++...")
    target_compile_options(tests PUBLIC -stdlib=libc++)
    target_link_options(tests PUBLIC -stdlib=libc++)
endif()

if(CMAKE_BUILD_TYPE MATCHES "Debug")
    message(STATUS "Enabling _GLIBCXX_DEBUG...")
    target_compile_options(tests PUBLIC -D_GLIBCXX_DEBUG)
endif()

target_link_libraries(tests GTest::gtest GTest::gtest_main)
