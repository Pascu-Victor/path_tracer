# Path Tracer

A simple ray tracer implemented in C++ using SDL3 for rendering.

## Features

### Core Ray Tracing

- **Vec3 Math Library**: Full 3D vector operations (add, subtract, multiply, dot/cross product, normalization)
- **Ray Casting**: Cast rays from camera through each pixel into the scene
- **Object System**: Base object class with virtual hit detection method
- **Sphere Primitives**: Ray-sphere intersection testing

### Lighting System

- **Point Light Sources**: Support for multiple colored point lights with position, color, and intensity
- **Phong/Blinn-Phong Shading**: Realistic shading with ambient, diffuse, and specular components
- **Shadow Casting**: Objects cast shadows based on light occlusion
- **Light Attenuation**: Distance-based light falloff for realistic lighting

### Material System

- **Material Properties**:
  - `color`: Base albedo color (RGB, 0-1 range)
  - `ambient`: Ambient reflection coefficient
  - `diffuse`: Diffuse (Lambertian) reflection coefficient
  - `specular`: Specular highlight intensity
  - `shininess`: Specular highlight sharpness
  - `reflectivity`: Mirror-like reflection amount (0-1)

### Bounce Lighting (Reflections)

- **Recursive Ray Tracing**: Support for reflective materials
- **Depth Limiting**: Configurable maximum recursion depth (default: 5 bounces)
- **Blended Reflections**: Materials can be partially reflective

### Scene Setup

The demo scene includes:

- 3 colored spheres (red, green, blue) with standard materials
- 1 reflective sphere (mirror-like) demonstrating bounce lighting
- 1 large yellow ground sphere
- 3 colored point lights:
  - Warm white light (main light)
  - Blue light (accent)
  - Red light (accent)

## Building

### Requirements

- CMake 3.20 or higher
- C++17 compiler
- SDL3 library

### Build Instructions

```bash
mkdir build
cd build
cmake ..
cmake --build .
./bin/PathTracer
```

## Usage

- The scene renders automatically when the application starts
- Press **ESC** or close the window to exit

## Technical Details

### Lighting Model

The ray tracer uses the Blinn-Phong shading model:

- **Ambient**: Constant base lighting
- **Diffuse**: Lambertian reflection based on surface normal and light direction
- **Specular**: Blinn-Phong highlights using half-vector between view and light directions

### Shadow Rays

For each intersection point, shadow rays are cast toward each light source to determine occlusion. If an object blocks the path to the light, that light's contribution is skipped.

### Reflection Rays

When a material has reflectivity > 0, a reflection ray is cast in the mirror direction. The final color is blended between the directly lit color and the reflected color based on the reflectivity coefficient.

### Recursion Depth

The maximum recursion depth is set to 5, preventing infinite recursion in scenes with multiple reflective surfaces while still allowing for realistic multi-bounce reflections.

## Future Enhancements

Potential additions:

- More primitive types (planes, boxes, triangles)
- Refraction and transparency
- Area lights and soft shadows
- Global illumination
- Textures and normal mapping
- Anti-aliasing
- Parallelized rendering
- Real-time camera movement
