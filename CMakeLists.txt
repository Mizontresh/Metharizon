cmake_minimum_required(VERSION 3.16)
project(Metharizon VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Vulkan
find_package(Vulkan REQUIRED)

# Set vcpkg paths manually
set(VCPKG_ROOT "C:/vcpkg")
set(GLFW_INCLUDE_DIR "${VCPKG_ROOT}/installed/x64-windows/include")
set(GLFW_LIBRARY "${VCPKG_ROOT}/installed/x64-windows/lib/glfw3dll.lib")
set(GLM_INCLUDE_DIR "${VCPKG_ROOT}/installed/x64-windows/include")

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/core)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/rendering)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/physics)

# Source files
file(GLOB_RECURSE SOURCES 
    "src/*.cpp"
    "src/vulkan/*.cpp"
    "src/core/*.cpp"
    "src/rendering/*.cpp"
    "src/physics/*.cpp"
)

# Explicitly add Camera.cpp to ensure it's compiled
list(APPEND SOURCES "src/core/Camera.cpp")

# Shader files
file(GLOB_RECURSE SHADERS 
    "shaders/*.comp"
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link libraries
target_link_libraries(${PROJECT_NAME} 
    Vulkan::Vulkan
    ${GLFW_LIBRARY}
)

# Include directories for libraries
target_include_directories(${PROJECT_NAME} PRIVATE
    ${GLFW_INCLUDE_DIR}
    ${GLM_INCLUDE_DIR}
)

# Compile shaders to SPIR-V
set(SPIRV_FILES "")
foreach(SHADER ${SHADERS})
    get_filename_component(SHADER_NAME ${SHADER} NAME_WE)
    get_filename_component(SHADER_EXT ${SHADER} EXT)
    set(SPIRV_FILE "${CMAKE_CURRENT_BINARY_DIR}/${SHADER_NAME}${SHADER_EXT}.spv")
    
    add_custom_command(
        OUTPUT ${SPIRV_FILE}
        COMMAND glslc ${SHADER} -o ${SPIRV_FILE}
        DEPENDS ${SHADER}
        COMMENT "Compiling ${SHADER} to SPIR-V"
    )
    
    list(APPEND SPIRV_FILES ${SPIRV_FILE})
endforeach()

# Create a custom target for shader compilation
add_custom_target(Shaders ALL DEPENDS ${SPIRV_FILES})

# Make the executable depend on the shader compilation target
add_dependencies(${PROJECT_NAME} Shaders)

# Copy shaders to build directory
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders
) 