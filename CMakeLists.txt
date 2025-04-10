cmake_minimum_required(VERSION 3.30)

project(beap_test VERSION 0.1)


set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED true)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(${CMAKE_PROJECT_NAME} "src/test.c" "src/beap_wrappers.c")

if (HEAP_DEBUG)
  target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE "HEAP_DEBUG")
endif()

add_subdirectory(beap)
target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC beap)
target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC "${PROJECT_BINARY_DIR}" "${PROJECT_SOURCE_DIR}/beap")
