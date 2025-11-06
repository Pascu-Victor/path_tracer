cmake_minimum_required(VERSION 3.20)
project(PathTracer VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

# Find SDL3 package
find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3)

# Find TBB package
find_package(TBB REQUIRED CONFIG REQUIRED COMPONENTS tbb)

# Find Vulkan package
find_package(Vulkan REQUIRED)

# Find GLM package
find_package(glm REQUIRED)

# Source files
set(SOURCES
    src/main_vulkan.cpp
    src/Vec3.cpp
    src/Ray.cpp
    src/Quaternion.cpp
    src/Material.cpp
    src/Object.cpp
    src/Sphere.cpp
    src/Ellipsoid.cpp
    src/Camera.cpp
    src/Light.cpp
    src/Volumetric.cpp
    src/VulkanRenderer.cpp
    src/ShaderCompiler.cpp
    src/SceneWrappers.cpp
)

# Header files
set(HEADERS
    include/Vec3.h
    include/Ray.h
    include/Quaternion.h
    include/Material.h
    include/Object.h
    include/Sphere.h
    include/Ellipsoid.h
    include/Camera.h
    include/Light.h
    include/Volumetric.h
    include/VulkanRenderer.h
    include/ShaderCompiler.h
    include/SceneWrappers.h
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_compile_options(${PROJECT_NAME} PRIVATE -ffast-math -flto -O3 -g)

target_link_options(${PROJECT_NAME} PRIVATE -flto)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${Vulkan_INCLUDE_DIR}
)

# Link SDL3
target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3)

# Link TBB
target_link_libraries(${PROJECT_NAME} PRIVATE TBB::tbb)

# Link Vulkan
target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan::Vulkan)

# Link GLM
target_link_libraries(${PROJECT_NAME} PRIVATE glm::glm)

# Set output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Copy shader files to build directory
add_custom_target(copy_shaders ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory 
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders 
    ${CMAKE_BINARY_DIR}/shaders
    COMMENT "Copying shader files to build directory"
)

add_dependencies(${PROJECT_NAME} copy_shaders)

# Copy volume files to build directory
add_custom_target(copy_volume ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory 
    ${CMAKE_CURRENT_SOURCE_DIR}/volume 
    ${CMAKE_BINARY_DIR}/volume
    COMMENT "Copying volume files to build directory"
)

add_dependencies(${PROJECT_NAME} copy_volume)
