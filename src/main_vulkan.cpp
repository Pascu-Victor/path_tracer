#include "Camera.h"
#include "Vec3.h"
#include "VulkanRenderer.h"

#include <SDL3/SDL.h>
#include <cmath>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>

// Window dimensions
const int WINDOW_WIDTH = 1920;
const int WINDOW_HEIGHT = 1080;

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
    // Expected format like: "ObjectModel:	I" or "Resolution:	400 296
    // 352"
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

  // Initialize SDL renderer and texture for display
  if (window && !vulkanRenderer.initializeSDLRenderer(window)) {
    std::cerr << "SDL renderer initialization failed" << std::endl;
  }

  // Setup camera
  double aspectRatio = static_cast<double>(WINDOW_WIDTH) / WINDOW_HEIGHT;
  const Vec3 cameraOrigin = Vec3(0, 1.5, 6);

  Camera camera(cameraOrigin,    // Camera position
                Vec3(2, 1.5, 0), // Look at center of volume
                Vec3(0, 1, 0),   // Up vector
                60.0,            // Vertical field of view
                aspectRatio);

  // Create scene with objects
  std::vector<GPUSphere> gpuSpheres;
  std::vector<GPUMaterial> gpuMaterials;
  std::vector<GPULight> gpuLights;
  std::vector<GPUVolumetricData> gpuVolumes;

  // Material definitions (converted to GPU format)
  GPUMaterial redMat{};
  redMat.colorAndAmbient = glm::vec4(0.8f, 0.2f, 0.2f, 0.1f);
  redMat.diffuseSpecularShiny = glm::vec4(0.7f, 0.3f, 32.0f, 0.0f);
  redMat.transIsoEmissive = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
  redMat.emissive = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
  redMat.scatterAndAbsorption = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
  gpuMaterials.push_back(redMat);

  GPUMaterial yellowMat{};
  yellowMat.colorAndAmbient = glm::vec4(0.8f, 0.8f, 0.2f, 0.1f);
  yellowMat.diffuseSpecularShiny = glm::vec4(0.6f, 0.1f, 16.0f, 0.0f);
  yellowMat.transIsoEmissive = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
  yellowMat.emissive = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
  yellowMat.scatterAndAbsorption = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
  gpuMaterials.push_back(yellowMat);

  GPUMaterial greenMat{};
  greenMat.colorAndAmbient = glm::vec4(0.2f, 0.8f, 0.2f, 0.1f);
  greenMat.diffuseSpecularShiny = glm::vec4(0.7f, 0.3f, 32.0f, 0.0f);
  greenMat.transIsoEmissive = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
  greenMat.emissive = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
  greenMat.scatterAndAbsorption = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
  gpuMaterials.push_back(greenMat);

  GPUMaterial blueMat{};
  blueMat.colorAndAmbient = glm::vec4(0.2f, 0.2f, 0.8f, 0.1f);
  blueMat.diffuseSpecularShiny = glm::vec4(0.7f, 0.3f, 32.0f, 0.0f);
  blueMat.transIsoEmissive = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
  blueMat.emissive = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
  blueMat.scatterAndAbsorption = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
  gpuMaterials.push_back(blueMat);

  GPUMaterial mirrorMat{};
  mirrorMat.colorAndAmbient = glm::vec4(0.9f, 0.9f, 0.9f, 0.1f);
  mirrorMat.diffuseSpecularShiny = glm::vec4(0.2f, 0.8f, 128.0f, 0.7f);
  mirrorMat.transIsoEmissive = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
  mirrorMat.emissive = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
  mirrorMat.scatterAndAbsorption = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
  gpuMaterials.push_back(mirrorMat);

  // Volumetric material for walnut
  GPUMaterial volumetricMat{};
  volumetricMat.colorAndAmbient = glm::vec4(0.0f, 0.0f, 0.0f, 0.1f);
  volumetricMat.diffuseSpecularShiny = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
  volumetricMat.transIsoEmissive =
      glm::vec4(0.0f, 1.0f, 0.0f, 0.0f); // isVolumetric = 1
  volumetricMat.emissive = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
  volumetricMat.scatterAndAbsorption =
      glm::vec4(0.8f, 0.6f, 0.4f, 2.0f); // scatterColor, absorptionCoeff
  gpuMaterials.push_back(volumetricMat);

  // Sphere definitions
  GPUSphere redSphere{};
  redSphere.center = glm::vec3(0.0f, 0.0f, -1.0f);
  redSphere.radius = 0.5f;
  redSphere.color = glm::vec3(0.8f, 0.2f, 0.2f);
  redSphere.materialIndex = 0;
  gpuSpheres.push_back(redSphere);

  GPUSphere greenSphere{};
  greenSphere.center = glm::vec3(-1.0f, 0.0f, -1.0f);
  greenSphere.radius = 0.3f;
  greenSphere.color = glm::vec3(0.2f, 0.8f, 0.2f);
  greenSphere.materialIndex = 2;
  gpuSpheres.push_back(greenSphere);

  GPUSphere blueSphere{};
  blueSphere.center = glm::vec3(1.0f, 0.0f, -1.0f);
  blueSphere.radius = 0.3f;
  blueSphere.color = glm::vec3(0.2f, 0.2f, 0.8f);
  blueSphere.materialIndex = 3;
  gpuSpheres.push_back(blueSphere);

  GPUSphere groundSphere{};
  groundSphere.center = glm::vec3(0.0f, -100.5f, -1.0f);
  groundSphere.radius = 100.0f;
  groundSphere.color = glm::vec3(0.8f, 0.8f, 0.2f);
  groundSphere.materialIndex = 1;
  gpuSpheres.push_back(groundSphere);

  GPUSphere mirrorSphere{};
  mirrorSphere.center = glm::vec3(0.5f, 0.3f, -0.5f);
  mirrorSphere.radius = 0.2f;
  mirrorSphere.color = glm::vec3(0.9f, 0.9f, 0.9f);
  mirrorSphere.materialIndex = 4;
  gpuSpheres.push_back(mirrorSphere);

  GPUSphere yellowSphere{};
  yellowSphere.center = glm::vec3(2.0f, 0.5f, 1.5f);
  yellowSphere.radius = 1.0f;
  yellowSphere.color = glm::vec3(0.8f, 0.8f, 0.2f);
  yellowSphere.materialIndex = 1;
  gpuSpheres.push_back(yellowSphere);

  // Light definitions
  GPULight light1{};
  light1.position = glm::vec3(2.0f, 2.0f, 1.0f);
  light1.intensity = 1.0f;
  light1.color = glm::vec3(1.0f, 0.9f, 0.8f);
  gpuLights.push_back(light1);

  GPULight light2{};
  light2.position = glm::vec3(-2.0f, 1.0f, 0.0f);
  light2.intensity = 1.0f;
  light2.color = glm::vec3(0.3f, 0.5f, 1.0f);
  gpuLights.push_back(light2);

  GPULight light3{};
  light3.position = glm::vec3(0.0f, -0.2f, 0.5f);
  light3.intensity = 1.0f;
  light3.color = glm::vec3(1.0f, 0.4f, 0.4f);
  gpuLights.push_back(light3);

  // Load volumetric data (walnut)
  std::vector<uint8_t> volumeData;
  glm::ivec3 volumeResolution;
  std::string datPath = "volume/walnut.dat";
  std::string rawPath = "volume/walnut.raw";

  if (loadVolumetricData(datPath, rawPath, volumeData, volumeResolution)) {
    // Create GPU volumetric data structure
    GPUVolumetricData volData{};
    volData.position =
        glm::vec3(1.5f, 1.0f, -0.5f); // Position where camera is looking
    volData.scale = 0.001f;           // Scale factor
    volData.v0 =
        glm::vec3(0.0f, 0.0f, 0.0f); // Volume min corner (relative to position)
    volData.v1 = glm::vec3(
        2.0f, 2.0f, 2.0f); // Volume max corner (2-unit cube for visibility)
    volData.resolution_x = volumeResolution.x;
    volData.resolution_y = volumeResolution.y;
    volData.resolution_z = volumeResolution.z;
    volData.materialIndex = 5; // Index of volumetric material
    gpuVolumes.push_back(volData);
    std::cout << "Volumetric data loaded and added to scene" << std::endl;
  } else {
    std::cerr << "Warning: Failed to load volumetric data, continuing without "
                 "volume"
              << std::endl;
  }

  // Update scene in GPU
  vulkanRenderer.updateScene(gpuSpheres, gpuMaterials, gpuLights, gpuVolumes,
                             volumeData);

  std::cout << "Scene data uploaded to GPU" << std::endl;

  // Main loop - render a few frames
  bool running = true;
  SDL_Event event;

  double theta = 0.0;
  const double orbitRadius = 3.0;
  float time = 0.0f;
  int frameCount = 0;

  // Delta time calculation
  uint64_t lastFrameTime = SDL_GetTicksNS();
  const float targetDeltaTime = 16.67f; // Target ~60 FPS (16.67ms per frame)

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

    // Orbit camera around the volume center at (2, 1.5, 0)
    double x = 2.0 + orbitRadius * std::cos(theta);
    double z = 6.0 + orbitRadius * std::sin(theta);
    Vec3 newCameraPos(x, 1.5, z);

    camera.setLookFrom(newCameraPos);
    theta += 1.0 / 180.0;

    // Prepare push constants for GPU
    PushConstants pushConst{};

    // Build camera matrix from camera position and look direction
    glm::vec3 eye(camera.origin.x, camera.origin.y, camera.origin.z);
    glm::vec3 center(2.0f, 1.5f, 0.0f);
    glm::vec3 up(0.0f, 1.0f, 0.0f);

    glm::mat4 view = glm::lookAt(eye, center, up);
    glm::mat4 proj = glm::perspective(
        glm::radians(60.0f), static_cast<float>(aspectRatio), 0.1f, 100.0f);
    pushConst.cameraMatrix = proj * view;
    pushConst.cameraPos = eye;
    pushConst.time = time;
    pushConst.numSpheres = static_cast<int>(gpuSpheres.size());
    pushConst.numLights = static_cast<int>(gpuLights.size());
    pushConst.numVolumes = static_cast<int>(gpuVolumes.size());
    pushConst.maxDepth = MAX_DEPTH;
    // Background gradient colors
    pushConst.bgColorBottom = glm::vec3(.0f, .0f, .0f); // White
    pushConst.bgColorTop = glm::vec3(0.0f, 0.0f, .0f);  // Sky blue

    vulkanRenderer.render(pushConst);

    // Update SDL display with GPU-rendered content
    // Only update display every 3 frames to reduce GPU-CPU readback overhead
    if (window && frameCount % 3 == 0) {
      vulkanRenderer.updateSDLDisplay();
    }

    vulkanRenderer.present();

    frameCount++;
    time += deltaTime;
    // Let GPU run at full speed - no artificial delay
  }

  // Cleanup
  vulkanRenderer.shutdown();
  if (window)
    SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
