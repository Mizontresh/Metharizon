# Tell CMake to use vcpkg’s installed libraries
set(CMAKE_TOOLCHAIN_FILE
    "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake"
    CACHE STRING "")

cmake_minimum_required(VERSION 3.10)
project(Metharizon LANGUAGES CXX)

# Require C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find dependencies via vcpkg
find_package(SDL2 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

# List all sources and headers
add_executable(Metharizon
    src/main.cpp

    src/Window.cpp
    src/Time.cpp
    src/Input.cpp
    src/Raymarcher.cpp

    src/Window.hpp
    src/Time.hpp
    src/Input.hpp
    src/Raymarcher.hpp
)

# Include your src/ directory
target_include_directories(Metharizon PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

# Link libraries
target_link_libraries(Metharizon PRIVATE
    SDL2::SDL2main
    SDL2::SDL2
    glad::glad
    glm::glm
)

# After building, copy shaders/ into the same folder as Metharizon.exe
add_custom_command(TARGET Metharizon POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/shaders"
    "$<TARGET_FILE_DIR:Metharizon>/shaders"
)
