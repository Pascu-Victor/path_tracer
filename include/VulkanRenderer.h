#ifndef VULKAN_RENDERER_H
#define VULKAN_RENDERER_H

#include "Ellipsoid.h"
#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan.h>

// Forward declarations
class VulkanDevice;

// GPU-friendly scene data structures
struct GPUSphere {
  glm::vec3 center;
  float radius;
  glm::vec3 color;
  int materialIndex;
};

// GPUEllipsoid is defined in Ellipsoid.h

struct GPUMaterial {
  glm::vec4 colorAndAmbient;      // color.xyz, ambient.w
  glm::vec4 diffuseSpecularShiny; // diffuse.x, specular.y, shininess.z,
                                  // reflectivity.w
  glm::vec4 transparencyEmissive; // transparency.x, emissiveStrength.y,
                                  // shaderFunctionIndex.z (as float), padding.w
  glm::vec4 emissive;             // emissive.xyz, padding.w
  glm::vec4 scatterAndAbsorption; // scatterColor.xyz, absorptionCoeff.w
};

struct GPULight {
  glm::vec3 position;
  float intensity;
  glm::vec3 color;
  float padding;
};

struct GPUVolumetricData {
  glm::vec3 position;
  float scale;
  glm::vec3 v0;
  int resolution_x;
  glm::vec3 v1;
  int resolution_y;
  int resolution_z;
  int materialIndex;
  int padding[2];
};

// Push constants for shader
struct PushConstants {
  glm::mat4 cameraMatrix;  // offset 0, size 64
  glm::vec3 cameraPos;     // offset 64, size 12
  float time;              // offset 76, size 4
  int numSpheres;          // offset 80, size 4
  int numEllipsoids;       // offset 84, size 4
  int numLights;           // offset 88, size 4
  int numVolumes;          // offset 92, size 4
  int maxDepth;            // offset 96, size 4
  int padding1;            // offset 100, size 4
  int padding2;            // offset 104, size 4
  int padding3;            // offset 108, size 4
  glm::vec3 bgColorTop;    // offset 112, size 12 (vec3 aligned to 16)
  float padding4;          // offset 124, size 4
  glm::vec3 bgColorBottom; // offset 128, size 12 (vec3 aligned to 16)
  float padding5;          // offset 140, size 4
};

class VulkanRenderer {
public:
  VulkanRenderer();
  ~VulkanRenderer();

  bool initialize(SDL_Window *window, int width, int height);
  void shutdown();

  void
  updateScene(const std::vector<GPUSphere> &spheres,
              const std::vector<GPUEllipsoid> &ellipsoids,
              const std::vector<GPUMaterial> &materials,
              const std::vector<GPULight> &lights,
              const std::vector<GPUVolumetricData> &volumes,
              const std::vector<uint8_t> &voxelData = std::vector<uint8_t>());

  void render(const PushConstants &pushConstants);
  void present();

  bool readbackOutputImage(std::vector<uint32_t> &imageData);
  bool saveFrameToPPM(const std::string &filename);

  VkImage getOutputImage() const { return vkOutputImage; }

  // Get shader path to index mapping (populated after shader loading)
  const std::unordered_map<std::string, int> &getShaderPathToIndexMap() const {
    return shaderPathToIndex_;
  }

private:
  bool createInstance();
  bool selectPhysicalDevice();
  bool createLogicalDevice();
  bool createCommandPool();
  bool createComputePipeline();
  bool createDescriptorSets();
  bool createBuffers();
  bool createSwapChain(SDL_Window *window);

  void recordComputeCommands(const PushConstants &pushConstants);
  void submitAndPresent();
  bool blitToSwapchain();

  // Vulkan core
  VkInstance vkInstance = VK_NULL_HANDLE;
  VkPhysicalDevice vkPhysicalDevice = VK_NULL_HANDLE;
  VkDevice vkDevice = VK_NULL_HANDLE;
  VkQueue vkComputeQueue = VK_NULL_HANDLE;
  VkCommandPool vkCommandPool = VK_NULL_HANDLE;

  // Multiple command buffers for frames in flight
  static constexpr int MAX_FRAMES_IN_FLIGHT = 2;
  VkCommandBuffer vkCommandBuffers[MAX_FRAMES_IN_FLIGHT];
  VkFence vkInFlightFences[MAX_FRAMES_IN_FLIGHT];
  uint32_t currentFrame = 0;

  // Compute pipeline
  VkPipelineLayout vkPipelineLayout = VK_NULL_HANDLE;
  VkPipeline vkComputePipeline = VK_NULL_HANDLE;
  VkDescriptorSetLayout vkDescriptorSetLayout = VK_NULL_HANDLE;
  VkDescriptorPool vkDescriptorPool = VK_NULL_HANDLE;
  VkDescriptorSet vkDescriptorSet = VK_NULL_HANDLE;

  // Output image
  VkImage vkOutputImage = VK_NULL_HANDLE;
  VkDeviceMemory vkOutputImageMemory = VK_NULL_HANDLE;
  VkImageView vkOutputImageView = VK_NULL_HANDLE;

  // Storage buffers
  VkBuffer vkSphereBuffer = VK_NULL_HANDLE;
  VkDeviceMemory vkSphereBufferMemory = VK_NULL_HANDLE;

  VkBuffer vkEllipsoidBuffer = VK_NULL_HANDLE;
  VkDeviceMemory vkEllipsoidBufferMemory = VK_NULL_HANDLE;

  VkBuffer vkMaterialBuffer = VK_NULL_HANDLE;
  VkDeviceMemory vkMaterialBufferMemory = VK_NULL_HANDLE;

  VkBuffer vkLightBuffer = VK_NULL_HANDLE;
  VkDeviceMemory vkLightBufferMemory = VK_NULL_HANDLE;

  VkBuffer vkVolumeBuffer = VK_NULL_HANDLE;
  VkDeviceMemory vkVolumeBufferMemory = VK_NULL_HANDLE;

  VkBuffer vkVoxelDataBuffer = VK_NULL_HANDLE;
  VkDeviceMemory vkVoxelDataBufferMemory = VK_NULL_HANDLE;

  int windowWidth = 800;
  int windowHeight = 600;

  // Swapchain for direct presentation
  VkSurfaceKHR vkSurface = VK_NULL_HANDLE;
  VkSwapchainKHR vkSwapchain = VK_NULL_HANDLE;
  std::vector<VkImage> vkSwapchainImages;
  std::vector<VkImageView> vkSwapchainImageViews;
  VkFormat vkSwapchainImageFormat;
  VkExtent2D vkSwapchainExtent;
  std::vector<VkSemaphore> vkImageAvailableSemaphores;
  std::vector<VkSemaphore> vkRenderFinishedSemaphores;
  uint32_t currentImageIndex = 0;

  // Readback buffer for GPU to CPU transfer (only for PPM export)
  VkBuffer vkReadbackBuffer = VK_NULL_HANDLE;
  VkDeviceMemory vkReadbackBufferMemory = VK_NULL_HANDLE;

  VkPhysicalDeviceProperties deviceProperties;
  VkPhysicalDeviceMemoryProperties memoryProperties;

  // Shader path to index mapping (populated during shader loading)
  std::unordered_map<std::string, int> shaderPathToIndex_;
};

#endif // VULKAN_RENDERER_H
