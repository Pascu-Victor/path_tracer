#ifndef VULKAN_RENDERER_H
#define VULKAN_RENDERER_H

#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <string>
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
  glm::vec4 colorAndAmbient;      // color.xyz, ambient.w
  glm::vec4 diffuseSpecularShiny; // diffuse.x, specular.y, shininess.z,
                                  // reflectivity.w
  glm::vec4 transIsoEmissive;     // transparency.x, isVolumetric.y,
                                  // emissiveStrength.z, padding.w
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
  glm::mat4 cameraMatrix;
  glm::vec3 cameraPos;
  float time;
  int numSpheres;
  int numLights;
  int numVolumes;
  int maxDepth;
  glm::vec3 bgColorTop;
  float padding1;
  glm::vec3 bgColorBottom;
  float padding2;
};

class VulkanRenderer {
public:
  VulkanRenderer();
  ~VulkanRenderer();

  bool initialize(SDL_Window *window, int width, int height);
  void shutdown();

  void
  updateScene(const std::vector<GPUSphere> &spheres,
              const std::vector<GPUMaterial> &materials,
              const std::vector<GPULight> &lights,
              const std::vector<GPUVolumetricData> &volumes,
              const std::vector<uint8_t> &voxelData = std::vector<uint8_t>());

  void render(const PushConstants &pushConstants);
  void present();

  bool readbackOutputImage(std::vector<uint32_t> &imageData);
  bool initializeSDLRenderer(SDL_Window *window);
  bool updateSDLDisplay();
  bool saveFrameToPPM(const std::string &filename);

  VkImage getOutputImage() const { return vkOutputImage; }

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

  // Vulkan core
  VkInstance vkInstance = VK_NULL_HANDLE;
  VkPhysicalDevice vkPhysicalDevice = VK_NULL_HANDLE;
  VkDevice vkDevice = VK_NULL_HANDLE;
  VkQueue vkComputeQueue = VK_NULL_HANDLE;
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

  // Storage buffers
  VkBuffer vkSphereBuffer = VK_NULL_HANDLE;
  VkDeviceMemory vkSphereBufferMemory = VK_NULL_HANDLE;

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

  // SDL for presentation
  SDL_Renderer *sdlRenderer = nullptr;
  SDL_Texture *sdlTexture = nullptr;

  // Readback buffer for GPU to CPU transfer
  VkBuffer vkReadbackBuffer = VK_NULL_HANDLE;
  VkDeviceMemory vkReadbackBufferMemory = VK_NULL_HANDLE;

  VkPhysicalDeviceProperties deviceProperties;
  VkPhysicalDeviceMemoryProperties memoryProperties;
};

#endif // VULKAN_RENDERER_H
