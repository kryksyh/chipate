cmake_minimum_required(VERSION 4.0.0)

project(
  chipate
  VERSION 0.0.1
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
FetchContent_Declare(
  raylib
  GIT_REPOSITORY https://github.com/raysan5/raylib.git
  GIT_TAG 5.5)
FetchContent_MakeAvailable(raylib)

add_executable(chipate src/main.cpp src/chip8.cpp src/asm.cpp)
target_link_libraries(chipate PRIVATE raylib)

enable_testing()

# Fetch Catch2 for unit tests
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.4.0)
FetchContent_MakeAvailable(Catch2)

add_executable(chip8_tests tests/test_chip8.cpp tests/test_chip8_opcodes.cpp
                           tests/test_asm.cpp src/chip8.cpp src/asm.cpp)

target_include_directories(chip8_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(chip8_tests PRIVATE Catch2::Catch2WithMain raylib)
add_test(NAME chip8_tests COMMAND chip8_tests)
