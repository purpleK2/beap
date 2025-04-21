cmake_minimum_required(VERSION 3.30)

project(beap_test VERSION 0.1)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED true)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(beap_test "src/test.c" "src/beap_wrappers.c")

add_subdirectory(beap)
target_link_libraries(beap_test PUBLIC beap)
target_include_directories(beap_test PUBLIC "${PROJECT_BINARY_DIR}" "${PROJECT_SOURCE_DIR}/beap")
