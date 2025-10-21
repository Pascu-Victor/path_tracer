#pragma once

#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <memory>
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

struct GPUMaterial {
  glm::vec3 color;
  float ambient;
  float diffuse;
  float specular;
  float shininess;
  float reflectivity;
  float transparency;
  int isVolumetric;
  glm::vec3 emissive;
  float emissiveStrength;
  glm::vec3 scatterColor;
  float absorptionCoeff;
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
  int padding[3];
};

// Push constants for shader
struct PushConstants {
  glm::mat4 cameraMatrix;
  glm::vec3 cameraPos;
  float time;
  int numSpheres;
  int numLights;
  int numVolumes;
  int maxDepth;
};

class VulkanRenderer {
public:
  VulkanRenderer();
  ~VulkanRenderer();

  bool initialize(int width, int height);
  void shutdown();

  void updateScene(const std::vector<GPUSphere> &spheres,
                   const std::vector<GPUMaterial> &materials,
                   const std::vector<GPULight> &lights,
                   const std::vector<GPUVolumetricData> &volumes);

  void render(const PushConstants &pushConstants);

  // Get the output data as RGB24
  std::vector<unsigned char> getFrameData();

  VkImage getOutputImage() const { return vkOutputImage; }

private:
  bool createInstance();
  bool selectPhysicalDevice();
  bool createLogicalDevice();
  bool createCommandPool();
  bool createComputePipeline();
  bool createDescriptorSets();
  bool createBuffers();
  bool createOutputImage();

  void recordComputeCommands(const PushConstants &pushConstants);
  void readbackOutputImage(std::vector<unsigned char> &outPixels);

  // Vulkan core
  VkInstance vkInstance = VK_NULL_HANDLE;
  VkPhysicalDevice vkPhysicalDevice = VK_NULL_HANDLE;
  VkDevice vkDevice = VK_NULL_HANDLE;
  VkQueue vkComputeQueue = VK_NULL_HANDLE;
  VkQueue vkTransferQueue = VK_NULL_HANDLE;
  VkCommandPool vkCommandPool = VK_NULL_HANDLE;
  VkCommandBuffer vkCommandBuffer = VK_NULL_HANDLE;

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

  // Readback buffer
  VkBuffer vkReadbackBuffer = VK_NULL_HANDLE;
  VkDeviceMemory vkReadbackBufferMemory = VK_NULL_HANDLE;

  // Storage buffers
  VkBuffer vkSphereBuffer = VK_NULL_HANDLE;
  VkDeviceMemory vkSphereBufferMemory = VK_NULL_HANDLE;

  VkBuffer vkMaterialBuffer = VK_NULL_HANDLE;
  VkDeviceMemory vkMaterialBufferMemory = VK_NULL_HANDLE;

  VkBuffer vkLightBuffer = VK_NULL_HANDLE;
  VkDeviceMemory vkLightBufferMemory = VK_NULL_HANDLE;

  VkBuffer vkVolumeBuffer = VK_NULL_HANDLE;
  VkDeviceMemory vkVolumeBufferMemory = VK_NULL_HANDLE;

  int windowWidth = 800;
  int windowHeight = 600;

  VkPhysicalDeviceProperties deviceProperties;
  VkPhysicalDeviceMemoryProperties memoryProperties;
};