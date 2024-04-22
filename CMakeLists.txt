cmake_minimum_required(VERSION 3.20)

# ---- Project ----
project(qac VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)

# ---- Include guards ----
if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
    message(FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there.")
endif()

# ---- File inclusion ----
file(GLOB_RECURSE headers CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp")
file(GLOB_RECURSE sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

# ---- Add executable ----
add_executable(${PROJECT_NAME} ${headers} ${sources})