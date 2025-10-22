#include "Camera.h"
#include "Ellipsoid.h"
#include "SceneWrappers.h"
#include "Vec3.h"
#include "VulkanRenderer.h"

#include <SDL3/SDL.h>
#include <algorithm>
#include <cmath>
#include <execution>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>

// Window dimensions
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

// Maximum recursion depth for bounce lighting
const int MAX_DEPTH = 5;

// Helper function to load volumetric data from .dat and .raw files
bool loadVolumetricData(const std::string &datFile, const std::string &rawFile,
                        std::vector<uint8_t> &volumeData,
                        glm::ivec3 &resolution) {
  // Read .dat file to get metadata
  std::ifstream datStream(datFile);
  if (!datStream.is_open()) {
    std::cerr << "Failed to open .dat file: " << datFile << std::endl;
    return false;
  }

  std::string line;
  resolution = glm::ivec3(1, 1, 1);

  while (std::getline(datStream, line)) {
    // Parse resolution from .dat file
    // Expected format: "ObjectModel:	I" or "Resolution:	400 296 352"
    if (line.find("Resolution") != std::string::npos) {
      int x, y, z;
      if (sscanf(line.c_str(), "Resolution: %d %d %d", &x, &y, &z) == 3 ||
          sscanf(line.c_str(), "Resolution:\t%d %d %d", &x, &y, &z) == 3) {
        resolution = glm::ivec3(x, y, z);
        std::cout << "Loaded resolution: " << x << " x " << y << " x " << z
                  << std::endl;
      }
    }
  }
  datStream.close();

  // Read raw binary data
  std::ifstream rawStream(rawFile, std::ios::binary);
  if (!rawStream.is_open()) {
    std::cerr << "Failed to open .raw file: " << rawFile << std::endl;
    return false;
  }

  size_t expectedSize =
      static_cast<size_t>(resolution.x * resolution.y * resolution.z);
  volumeData.resize(expectedSize);

  rawStream.read(reinterpret_cast<char *>(volumeData.data()), expectedSize);
  size_t bytesRead = rawStream.gcount();
  rawStream.close();

  if (bytesRead != expectedSize) {
    std::cerr << "Warning: Expected " << expectedSize << " bytes, got "
              << bytesRead << " bytes" << std::endl;
  }

  std::cout << "Loaded volumetric data: " << bytesRead << " bytes" << std::endl;
  return true;
}

int main(int argc, char *argv[]) {
  // Initialize SDL
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    std::cerr << "SDL initialization failed: " << SDL_GetError() << std::endl;
    return 1;
  }

  std::cout << "SDL initialized successfully" << std::endl;

  // Create window with Vulkan support
  SDL_Window *window = SDL_CreateWindow("Path Tracer (Vulkan)", WINDOW_WIDTH,
                                        WINDOW_HEIGHT, SDL_WINDOW_VULKAN);

  if (!window) {
    std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
    std::cerr << "Continuing in headless mode..." << std::endl;
    // Continue anyway - we can render without displaying
  } else {
    std::cout << "Window created successfully" << std::endl;
  }

  // Initialize Vulkan renderer
  VulkanRenderer vulkanRenderer;
  if (!vulkanRenderer.initialize(window, WINDOW_WIDTH, WINDOW_HEIGHT)) {
    std::cerr << "Vulkan renderer initialization failed" << std::endl;
    if (window)
      SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  }

  // Initialize SDL renderer and texture for display (no longer needed with
  // direct Vulkan rendering) SDL now acts as a window provider only - Vulkan
  // handles all rendering

  // Setup camera
  double aspectRatio = static_cast<double>(WINDOW_WIDTH) / WINDOW_HEIGHT;
  const Vec3 cameraOrigin = Vec3(0, 1.5, 6);

  Camera camera(cameraOrigin,    // Camera position
                Vec3(2, 1.5, 0), // Look at center of volume
                Vec3(0, 1, 0),   // Up vector
                60.0,            // Vertical field of view
                aspectRatio);

  // Create scene with objects using wrapper classes
  std::vector<Sphere> spheres;
  std::vector<Ellipsoid> ellipsoids;
  std::vector<Light> lights;
  std::vector<VolumetricData> volumes;

  // Material definitions using clean wrapper API
  // Red diffuse material
  Material redMat = Material::Diffuse(glm::vec3(0.8f, 0.2f, 0.2f), 0.7f, 0.1f);
  redMat.setSpecular(0.3f);
  redMat.setShininess(32.0f);

  // Yellow diffuse material with low specularity
  Material yellowMat =
      Material::Diffuse(glm::vec3(0.8f, 0.8f, 0.2f), 0.6f, 0.1f);
  yellowMat.setSpecular(0.1f);
  yellowMat.setShininess(16.0f);

  // Green emissive material
  Material greenMat = Material::Emissive(glm::vec3(0.2f, 0.8f, 0.2f), 2.0f);

  // Blue semi-glossy material
  Material blueMat = Material::Diffuse(glm::vec3(0.2f, 0.2f, 0.8f), 0.7f, 0.1f);
  blueMat.setSpecular(0.5f);
  blueMat.setShininess(64.0f);

  // Mirror material
  Material mirrorMat = Material::Mirror(glm::vec3(0.9f, 0.9f, 0.9f), 0.9f);

  // Volumetric material for walnut
  Material volumetricMat =
      Material::Volumetric(glm::vec3(0.8f, 0.6f, 0.4f), 8.0f);

  // Sphere definitions using wrapper class with Material references
  spheres.push_back(Sphere(glm::vec3(0.0f, 0.0f, -1.0f), 0.5f,
                           glm::vec3(0.8f, 0.2f, 0.2f), redMat)); // Red sphere

  spheres.push_back(Sphere(glm::vec3(-1.0f, 0.0f, -1.0f), 0.3f,
                           glm::vec3(0.2f, 0.8f, 0.2f),
                           greenMat)); // Green sphere

  spheres.push_back(Sphere(glm::vec3(1.0f, 0.0f, -1.0f), 0.3f,
                           glm::vec3(0.2f, 0.2f, 0.8f),
                           blueMat)); // Blue sphere

  spheres.push_back(Sphere(glm::vec3(0.0f, -100.5f, -1.0f), 100.0f,
                           glm::vec3(0.8f, 0.8f, 0.2f),
                           yellowMat)); // Ground sphere

  spheres.push_back(Sphere(glm::vec3(0.5f, 0.3f, -0.5f), 0.2f,
                           glm::vec3(0.9f, 0.9f, 0.9f),
                           mirrorMat)); // Mirror sphere

  spheres.push_back(Sphere(glm::vec3(2.0f, 0.5f, 1.5f), 1.0f,
                           glm::vec3(0.8f, 0.8f, 0.2f),
                           yellowMat)); // Yellow sphere

  // Ellipsoid definitions using wrapper class with Material references
  ellipsoids.push_back(Ellipsoid(glm::vec3(-2.0f, 0.8f, -1.0f), // Center
                                 glm::vec3(0.5f, 0.8f, 0.3f), // Radii (x, y, z)
                                 glm::vec3(0.8f, 0.4f, 0.8f), // Purple color
                                 mirrorMat));                 // Mirror material

  ellipsoids.push_back(Ellipsoid(glm::vec3(0.0f, 1.2f, -2.0f), // Center
                                 glm::vec3(0.6f, 0.4f, 0.4f), // Radii (x, y, z)
                                 glm::vec3(0.4f, 0.8f, 0.8f), // Cyan color
                                 blueMat));                   // Blue material

  // Light definitions using wrapper class
  lights.push_back(Light(glm::vec3(2.0f, 2.0f, 1.0f), 1.0f,
                         glm::vec3(1.0f, 0.9f, 0.8f))); // Warm light

  lights.push_back(Light(glm::vec3(-2.0f, 1.0f, 0.0f), 1.0f,
                         glm::vec3(0.3f, 0.5f, 1.0f))); // Cool light

  lights.push_back(Light(glm::vec3(0.0f, -0.2f, 0.5f), 1.0f,
                         glm::vec3(1.0f, 0.4f, 0.4f))); // Red accent light

  // Load volumetric data (walnut)
  std::vector<uint8_t> volumeData;
  glm::ivec3 volumeResolution;
  std::string datPath = "volume/walnut.dat";
  std::string rawPath = "volume/walnut.raw";

  if (loadVolumetricData(datPath, rawPath, volumeData, volumeResolution)) {
    VolumetricData volData(
        glm::vec3(1.5f, 1.0f, -0.5f), // Position where camera is looking
        0.001f,                       // Scale factor
        glm::vec3(0.0f, 0.0f, 0.0f), // Volume min corner (relative to position)
        glm::vec3(2.0f, 2.0f, 2.0f), // Volume max corner (2-unit cube)
        volumeResolution,            // Resolution from file
        volumetricMat);              // Material reference
    volumes.push_back(volData);
    std::cout << "Volumetric data loaded and added to scene" << std::endl;
  } else {
    std::cerr << "Warning: Failed to load volumetric data, continuing without "
                 "volume"
              << std::endl;
  }

  // Pre-render: Build material list and map object materials to indices
  std::vector<Material> materials;
  SceneManager::prepareForRender(materials, spheres, ellipsoids, volumes);

  std::cout << "Pre-render complete: " << materials.size()
            << " unique materials collected" << std::endl;
  std::cout << "Sphere material indices: ";
  for (const auto &sphere : spheres) {
    std::cout << sphere.getMaterialIndex() << " ";
  }
  std::cout << std::endl;
  std::cout << "Ellipsoid material indices: ";
  for (const auto &ellipsoid : ellipsoids) {
    std::cout << ellipsoid.getMaterialIndex() << " ";
  }
  std::cout << std::endl;
  if (!volumes.empty()) {
    std::cout << "Volume material indices: ";
    for (const auto &volume : volumes) {
      std::cout << volume.getMaterialIndex() << " ";
    }
    std::cout << std::endl;
  }

  // Convert wrapper objects to GPU format
  std::vector<GPUSphere> gpuSpheres(spheres.size());
  std::vector<GPUEllipsoid> gpuEllipsoids(ellipsoids.size());
  std::vector<GPUMaterial> gpuMaterials(materials.size());
  std::vector<GPULight> gpuLights(lights.size());
  std::vector<GPUVolumetricData> gpuVolumes(volumes.size());

  // Parallel transform for spheres
  std::transform(std::execution::par_unseq, spheres.begin(), spheres.end(),
                 gpuSpheres.begin(),
                 [](const Sphere &sphere) { return sphere.toGPU(); });

  // Parallel transform for ellipsoids
  std::transform(std::execution::par_unseq, ellipsoids.begin(),
                 ellipsoids.end(), gpuEllipsoids.begin(),
                 [](const Ellipsoid &ellipsoid) { return ellipsoid.toGPU(); });

  // Parallel transform for materials
  std::transform(std::execution::par_unseq, materials.begin(), materials.end(),
                 gpuMaterials.begin(),
                 [](const Material &material) { return material.toGPU(); });

  // Parallel transform for lights
  std::transform(std::execution::par_unseq, lights.begin(), lights.end(),
                 gpuLights.begin(),
                 [](const Light &light) { return light.toGPU(); });

  // Parallel transform for volumes
  std::transform(std::execution::par_unseq, volumes.begin(), volumes.end(),
                 gpuVolumes.begin(),
                 [](const VolumetricData &volume) { return volume.toGPU(); });

  // Update scene in GPU
  vulkanRenderer.updateScene(gpuSpheres, gpuEllipsoids, gpuMaterials, gpuLights,
                             gpuVolumes, volumeData);

  std::cout << "Scene data uploaded to GPU" << std::endl;

  // Prepare static push constants
  PushConstants pushConst{};
  pushConst.numSpheres = static_cast<int>(gpuSpheres.size());
  pushConst.numEllipsoids = static_cast<int>(gpuEllipsoids.size());
  pushConst.numLights = static_cast<int>(gpuLights.size());
  pushConst.numVolumes = static_cast<int>(gpuVolumes.size());
  pushConst.maxDepth = MAX_DEPTH;
  pushConst.bgColorBottom = glm::vec3(1.0f, 1.0f, 1.0f); // White
  pushConst.bgColorTop = glm::vec3(0.4f, 0.45f, 1.0f);   // Sky blue

  bool running = true;
  SDL_Event event;

  double theta = 0.0;
  const double orbitRadius = 3.0;
  float time = 0.0f;
  int frameCount = 0;

  // Delta time calculation
  uint64_t lastFrameTime = SDL_GetTicksNS();
  const float targetFrameTime = 1000.0f / 120.0f; // 8.33ms per frame

  while (running) {
    // Calculate delta time
    uint64_t currentTime = SDL_GetTicksNS();
    float deltaTime = static_cast<float>(currentTime - lastFrameTime) /
                      1000000.0f; // Convert to ms
    lastFrameTime = currentTime;

    // Poll events with non-blocking
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT) {
        running = false;
      } else if (event.type == SDL_EVENT_KEY_DOWN) {
        if (event.key.key == SDLK_ESCAPE) {
          running = false;
        }
      }
    }

    // Orbit camera around the volume center
    double x = 2.0 + orbitRadius * std::cos(theta);
    double z = 6.0 + orbitRadius * std::sin(theta);
    Vec3 newCameraPos(x, 1.5, z);

    camera.setLookFrom(newCameraPos);
    theta += 1.0 / 180.0;

    // Update only dynamic push constants
    glm::vec3 eye(camera.origin.x, camera.origin.y, camera.origin.z);
    glm::vec3 center(2.0f, 1.5f, 0.0f);
    glm::vec3 up(0.0f, 1.0f, 0.0f);

    glm::mat4 view = glm::lookAt(eye, center, up);
    glm::mat4 proj = glm::perspective(
        glm::radians(60.0f), static_cast<float>(aspectRatio), 0.1f, 100.0f);
    pushConst.cameraMatrix = proj * view;
    pushConst.cameraPos = eye;
    pushConst.time = time;

    vulkanRenderer.render(pushConst);

    vulkanRenderer.present();

    frameCount++;
    time += deltaTime;

    // Cap frame rate
    if (deltaTime < targetFrameTime) {
      uint32_t delayMs = static_cast<uint32_t>(targetFrameTime - deltaTime);
      SDL_Delay(delayMs);
    }
  }

  // Cleanup
  vulkanRenderer.shutdown();
  if (window)
    SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
