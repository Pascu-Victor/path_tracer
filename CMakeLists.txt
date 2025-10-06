cmake_minimum_required(VERSION 3.20)
project(PathTracer VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SDL3 package
find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3)

# Find TBB package
find_package(TBB REQUIRED CONFIG REQUIRED COMPONENTS tbb)

# Source files
set(SOURCES
    src/main.cpp
    src/Vec3.cpp
    src/Ray.cpp
    src/Material.cpp
    src/Object.cpp
    src/Sphere.cpp
    src/Camera.cpp
    src/Light.cpp
)

# Header files
set(HEADERS
    include/Vec3.h
    include/Ray.h
    include/Material.h
    include/Object.h
    include/Sphere.h
    include/Camera.h
    include/Light.h
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link SDL3
target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3)

# Link TBB
target_link_libraries(${PROJECT_NAME} PRIVATE TBB::tbb)

# Set output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
