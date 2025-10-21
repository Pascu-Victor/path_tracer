#include "VulkanRenderer.h"
#include "ShaderCompiler.h"
#include <algorithm>
#include <array>
#include <cstring>
#include <fstream>
#include <iostream>

// Helper function for memory type lookup
uint32_t findMemoryType(VkPhysicalDeviceMemoryProperties props,
                        uint32_t typeFilter, VkMemoryPropertyFlags flags) {
  for (uint32_t i = 0; i < props.memoryTypeCount; i++) {
    if ((typeFilter & (1 << i)) &&
        (props.memoryTypes[i].propertyFlags & flags) == flags) {
      return i;
    }
  }
  return 0;
}

VulkanRenderer::VulkanRenderer() = default;

VulkanRenderer::~VulkanRenderer() { shutdown(); }

bool VulkanRenderer::initialize(SDL_Window *window, int width, int height) {
  windowWidth = width;
  windowHeight = height;

  if (!createInstance()) {
    std::cerr << "Failed to create Vulkan instance" << std::endl;
    return false;
  }

  if (!selectPhysicalDevice()) {
    std::cerr << "Failed to select physical device" << std::endl;
    return false;
  }

  if (!createLogicalDevice()) {
    std::cerr << "Failed to create logical device" << std::endl;
    return false;
  }

  if (!createCommandPool()) {
    std::cerr << "Failed to create command pool" << std::endl;
    return false;
  }

  if (!createBuffers()) {
    std::cerr << "Failed to create buffers" << std::endl;
    return false;
  }

  if (!createDescriptorSets()) {
    std::cerr << "Failed to create descriptor sets" << std::endl;
    return false;
  }

  if (!createComputePipeline()) {
    std::cerr << "Failed to create compute pipeline" << std::endl;
    return false;
  }

  if (!createSwapChain(window)) {
    std::cerr << "Failed to create swap chain" << std::endl;
    return false;
  }

  return true;
}

bool VulkanRenderer::createInstance() {
  VkApplicationInfo appInfo{};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = "PathTracer";
  appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.pEngineName = "Custom";
  appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.apiVersion = VK_API_VERSION_1_3;

  // Extensions required
  std::vector<const char *> extensions;
  extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
  // SDL will use XCB or Wayland depending on the system
  extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

  VkInstanceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pApplicationInfo = &appInfo;
  createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
  createInfo.ppEnabledExtensionNames = extensions.data();

  if (vkCreateInstance(&createInfo, nullptr, &vkInstance) != VK_SUCCESS) {
    return false;
  }

  std::cout << "Vulkan instance created successfully" << std::endl;
  return true;
}

bool VulkanRenderer::selectPhysicalDevice() {
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(vkInstance, &deviceCount, nullptr);

  if (deviceCount == 0) {
    return false;
  }

  std::vector<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(vkInstance, &deviceCount, devices.data());

  // Select first device with compute support
  for (const auto &device : devices) {
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
                                             nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
                                             queueFamilies.data());

    // Find compute queue family
    for (uint32_t i = 0; i < queueFamilyCount; i++) {
      if (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
        vkPhysicalDevice = device;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        vkGetPhysicalDeviceMemoryProperties(device, &memoryProperties);

        std::cout << "Selected device: " << deviceProperties.deviceName
                  << std::endl;
        return true;
      }
    }
  }

  return false;
}

bool VulkanRenderer::createLogicalDevice() {
  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyCount,
                                           nullptr);

  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyCount,
                                           queueFamilies.data());

  // Find compute queue family
  uint32_t computeQueueFamily = 0;
  for (uint32_t i = 0; i < queueFamilyCount; i++) {
    if (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
      computeQueueFamily = i;
      break;
    }
  }

  float queuePriority = 1.0f;
  VkDeviceQueueCreateInfo queueCreateInfo{};
  queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queueCreateInfo.queueFamilyIndex = computeQueueFamily;
  queueCreateInfo.queueCount = 1;
  queueCreateInfo.pQueuePriorities = &queuePriority;

  // Device extensions
  std::vector<const char *> deviceExtensions;
  deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

  VkPhysicalDeviceFeatures deviceFeatures{};

  VkDeviceCreateInfo deviceCreateInfo{};
  deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  deviceCreateInfo.queueCreateInfoCount = 1;
  deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
  deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
  deviceCreateInfo.enabledExtensionCount =
      static_cast<uint32_t>(deviceExtensions.size());
  deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

  if (vkCreateDevice(vkPhysicalDevice, &deviceCreateInfo, nullptr, &vkDevice) !=
      VK_SUCCESS) {
    return false;
  }

  vkGetDeviceQueue(vkDevice, computeQueueFamily, 0, &vkComputeQueue);

  std::cout << "Logical device created successfully" << std::endl;
  return true;
}

bool VulkanRenderer::createCommandPool() {
  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyCount,
                                           nullptr);

  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyCount,
                                           queueFamilies.data());

  uint32_t computeQueueFamily = 0;
  for (uint32_t i = 0; i < queueFamilyCount; i++) {
    if (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
      computeQueueFamily = i;
      break;
    }
  }

  VkCommandPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  poolInfo.queueFamilyIndex = computeQueueFamily;
  poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

  if (vkCreateCommandPool(vkDevice, &poolInfo, nullptr, &vkCommandPool) !=
      VK_SUCCESS) {
    return false;
  }

  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.commandPool = vkCommandPool;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandBufferCount = 1;

  if (vkAllocateCommandBuffers(vkDevice, &allocInfo, &vkCommandBuffer) !=
      VK_SUCCESS) {
    return false;
  }

  std::cout << "Command pool created successfully" << std::endl;
  return true;
}

bool VulkanRenderer::createBuffers() {
  // For now, create minimal buffers - will be resized during updateScene
  // Sphere buffer needs 2 vec4s per sphere (center+radius, then
  // materialIndex+padding)
  size_t sphereBufferSize =
      sizeof(glm::vec4) * 200; // 2 vec4s per sphere * 100 spheres
  size_t materialBufferSize = sizeof(GPUMaterial) * 100;
  size_t lightBufferSize = sizeof(GPULight) * 32;
  size_t volumeBufferSize = sizeof(GPUVolumetricData) * 32;

  // Sphere buffer
  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = sphereBufferSize;
  bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateBuffer(vkDevice, &bufferInfo, nullptr, &vkSphereBuffer) !=
      VK_SUCCESS) {
    return false;
  }

  VkMemoryRequirements memReqs;
  vkGetBufferMemoryRequirements(vkDevice, vkSphereBuffer, &memReqs);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memReqs.size;
  allocInfo.memoryTypeIndex =
      findMemoryType(memoryProperties, memReqs.memoryTypeBits,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

  if (vkAllocateMemory(vkDevice, &allocInfo, nullptr, &vkSphereBufferMemory) !=
      VK_SUCCESS) {
    return false;
  }

  vkBindBufferMemory(vkDevice, vkSphereBuffer, vkSphereBufferMemory, 0);

  // Create material, light, and volume buffers similarly
  bufferInfo.size = materialBufferSize;
  if (vkCreateBuffer(vkDevice, &bufferInfo, nullptr, &vkMaterialBuffer) !=
      VK_SUCCESS) {
    return false;
  }
  vkGetBufferMemoryRequirements(vkDevice, vkMaterialBuffer, &memReqs);
  allocInfo.allocationSize = memReqs.size;
  allocInfo.memoryTypeIndex =
      findMemoryType(memoryProperties, memReqs.memoryTypeBits,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
  if (vkAllocateMemory(vkDevice, &allocInfo, nullptr,
                       &vkMaterialBufferMemory) != VK_SUCCESS) {
    return false;
  }
  vkBindBufferMemory(vkDevice, vkMaterialBuffer, vkMaterialBufferMemory, 0);

  bufferInfo.size = lightBufferSize;
  if (vkCreateBuffer(vkDevice, &bufferInfo, nullptr, &vkLightBuffer) !=
      VK_SUCCESS) {
    return false;
  }
  vkGetBufferMemoryRequirements(vkDevice, vkLightBuffer, &memReqs);
  allocInfo.allocationSize = memReqs.size;
  allocInfo.memoryTypeIndex =
      findMemoryType(memoryProperties, memReqs.memoryTypeBits,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
  if (vkAllocateMemory(vkDevice, &allocInfo, nullptr, &vkLightBufferMemory) !=
      VK_SUCCESS) {
    return false;
  }
  vkBindBufferMemory(vkDevice, vkLightBuffer, vkLightBufferMemory, 0);

  bufferInfo.size = volumeBufferSize;
  if (vkCreateBuffer(vkDevice, &bufferInfo, nullptr, &vkVolumeBuffer) !=
      VK_SUCCESS) {
    return false;
  }
  vkGetBufferMemoryRequirements(vkDevice, vkVolumeBuffer, &memReqs);
  allocInfo.allocationSize = memReqs.size;
  allocInfo.memoryTypeIndex =
      findMemoryType(memoryProperties, memReqs.memoryTypeBits,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
  if (vkAllocateMemory(vkDevice, &allocInfo, nullptr, &vkVolumeBufferMemory) !=
      VK_SUCCESS) {
    return false;
  }
  vkBindBufferMemory(vkDevice, vkVolumeBuffer, vkVolumeBufferMemory, 0);

  // Voxel data buffer (for actual volumetric CT data)
  size_t voxelDataBufferSize = 50 * 1024 * 1024; // 50MB for voxel data
  bufferInfo.size = voxelDataBufferSize;
  if (vkCreateBuffer(vkDevice, &bufferInfo, nullptr, &vkVoxelDataBuffer) !=
      VK_SUCCESS) {
    return false;
  }
  vkGetBufferMemoryRequirements(vkDevice, vkVoxelDataBuffer, &memReqs);
  allocInfo.allocationSize = memReqs.size;
  allocInfo.memoryTypeIndex =
      findMemoryType(memoryProperties, memReqs.memoryTypeBits,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
  if (vkAllocateMemory(vkDevice, &allocInfo, nullptr,
                       &vkVoxelDataBufferMemory) != VK_SUCCESS) {
    return false;
  }
  vkBindBufferMemory(vkDevice, vkVoxelDataBuffer, vkVoxelDataBufferMemory, 0);

  std::cout << "Storage buffers created successfully" << std::endl;
  return true;
}

bool VulkanRenderer::createDescriptorSets() {
  // Create descriptor set layout
  std::array<VkDescriptorSetLayoutBinding, 6> bindings{};

  // Output image binding
  bindings[0].binding = 0;
  bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
  bindings[0].descriptorCount = 1;
  bindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

  // Sphere buffer binding
  bindings[1].binding = 1;
  bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  bindings[1].descriptorCount = 1;
  bindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

  // Material buffer binding
  bindings[2].binding = 2;
  bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  bindings[2].descriptorCount = 1;
  bindings[2].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

  // Light buffer binding
  bindings[3].binding = 3;
  bindings[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  bindings[3].descriptorCount = 1;
  bindings[3].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

  // Volume buffer binding
  bindings[4].binding = 4;
  bindings[4].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  bindings[4].descriptorCount = 1;
  bindings[4].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

  // Voxel data buffer binding
  bindings[5].binding = 5;
  bindings[5].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  bindings[5].descriptorCount = 1;
  bindings[5].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

  VkDescriptorSetLayoutCreateInfo layoutInfo{};
  layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
  layoutInfo.pBindings = bindings.data();

  if (vkCreateDescriptorSetLayout(vkDevice, &layoutInfo, nullptr,
                                  &vkDescriptorSetLayout) != VK_SUCCESS) {
    return false;
  }

  // Create descriptor pool
  std::array<VkDescriptorPoolSize, 2> poolSizes{};
  poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
  poolSizes[0].descriptorCount = 1;
  poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  poolSizes[1].descriptorCount =
      5; // sphere, material, light, volume, voxelData

  VkDescriptorPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
  poolInfo.pPoolSizes = poolSizes.data();
  poolInfo.maxSets = 1;

  if (vkCreateDescriptorPool(vkDevice, &poolInfo, nullptr, &vkDescriptorPool) !=
      VK_SUCCESS) {
    return false;
  }

  // Allocate descriptor set
  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = vkDescriptorPool;
  allocInfo.descriptorSetCount = 1;
  allocInfo.pSetLayouts = &vkDescriptorSetLayout;

  if (vkAllocateDescriptorSets(vkDevice, &allocInfo, &vkDescriptorSet) !=
      VK_SUCCESS) {
    return false;
  }

  std::cout << "Descriptor sets created successfully" << std::endl;
  return true;
}

bool VulkanRenderer::createComputePipeline() {
  // Compile shader
  std::vector<uint32_t> shaderCode;
  try {
    shaderCode = ShaderCompiler::compileShader("shaders/raytrace.comp",
                                               VK_SHADER_STAGE_COMPUTE_BIT);
  } catch (const std::exception &e) {
    std::cerr << "Shader compilation failed: " << e.what() << std::endl;
    return false;
  }

  VkShaderModuleCreateInfo moduleInfo{};
  moduleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  moduleInfo.codeSize = shaderCode.size() * sizeof(uint32_t);
  moduleInfo.pCode = shaderCode.data();

  VkShaderModule shaderModule;
  if (vkCreateShaderModule(vkDevice, &moduleInfo, nullptr, &shaderModule) !=
      VK_SUCCESS) {
    return false;
  }

  VkPipelineShaderStageCreateInfo stageInfo{};
  stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  stageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
  stageInfo.module = shaderModule;
  stageInfo.pName = "main";

  // Push constant range
  VkPushConstantRange pushConstantRange{};
  pushConstantRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
  pushConstantRange.size = sizeof(PushConstants);
  pushConstantRange.offset = 0;

  VkPipelineLayoutCreateInfo layoutInfo{};
  layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  layoutInfo.setLayoutCount = 1;
  layoutInfo.pSetLayouts = &vkDescriptorSetLayout;
  layoutInfo.pushConstantRangeCount = 1;
  layoutInfo.pPushConstantRanges = &pushConstantRange;

  if (vkCreatePipelineLayout(vkDevice, &layoutInfo, nullptr,
                             &vkPipelineLayout) != VK_SUCCESS) {
    return false;
  }

  VkComputePipelineCreateInfo pipelineInfo{};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
  pipelineInfo.layout = vkPipelineLayout;
  pipelineInfo.stage = stageInfo;

  if (vkCreateComputePipelines(vkDevice, VK_NULL_HANDLE, 1, &pipelineInfo,
                               nullptr, &vkComputePipeline) != VK_SUCCESS) {
    return false;
  }

  vkDestroyShaderModule(vkDevice, shaderModule, nullptr);

  std::cout << "Compute pipeline created successfully" << std::endl;
  return true;
}

bool VulkanRenderer::createSwapChain(SDL_Window *window) {
  // For now, just create the output image
  // Full swap chain implementation would follow

  VkImageCreateInfo imageInfo{};
  imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.imageType = VK_IMAGE_TYPE_2D;
  imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
  imageInfo.extent = {static_cast<uint32_t>(windowWidth),
                      static_cast<uint32_t>(windowHeight), 1};
  imageInfo.mipLevels = 1;
  imageInfo.arrayLayers = 1;
  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
  imageInfo.usage =
      VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

  if (vkCreateImage(vkDevice, &imageInfo, nullptr, &vkOutputImage) !=
      VK_SUCCESS) {
    return false;
  }

  VkMemoryRequirements memReqs;
  vkGetImageMemoryRequirements(vkDevice, vkOutputImage, &memReqs);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memReqs.size;
  allocInfo.memoryTypeIndex =
      findMemoryType(memoryProperties, memReqs.memoryTypeBits,
                     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  if (vkAllocateMemory(vkDevice, &allocInfo, nullptr, &vkOutputImageMemory) !=
      VK_SUCCESS) {
    return false;
  }

  if (vkBindImageMemory(vkDevice, vkOutputImage, vkOutputImageMemory, 0) !=
      VK_SUCCESS) {
    return false;
  }

  VkImageViewCreateInfo viewInfo{};
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image = vkOutputImage;
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
  viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  viewInfo.subresourceRange.baseMipLevel = 0;
  viewInfo.subresourceRange.levelCount = 1;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount = 1;

  if (vkCreateImageView(vkDevice, &viewInfo, nullptr, &vkOutputImageView) !=
      VK_SUCCESS) {
    return false;
  }

  std::cout << "Output image created successfully" << std::endl;
  return true;
}

void VulkanRenderer::updateScene(const std::vector<GPUSphere> &spheres,
                                 const std::vector<GPUMaterial> &materials,
                                 const std::vector<GPULight> &lights,
                                 const std::vector<GPUVolumetricData> &volumes,
                                 const std::vector<uint8_t> &voxelData) {
  // Convert sphere data to include materialIndex
  if (!spheres.empty()) {
    // Create a packed structure matching GLSL layout
    struct PackedSphere {
      glm::vec3 center;
      float radius;
      int32_t materialIndex;
      int32_t padding[3];
    };
    static_assert(sizeof(PackedSphere) == 32,
                  "PackedSphere size must be 32 bytes");

    std::vector<PackedSphere> sphereData;
    for (const auto &sphere : spheres) {
      PackedSphere packed{};
      packed.center = sphere.center;
      packed.radius = sphere.radius;
      packed.materialIndex = sphere.materialIndex;
      sphereData.push_back(packed);
    }

    void *data;
    vkMapMemory(vkDevice, vkSphereBufferMemory, 0,
                sphereData.size() * sizeof(PackedSphere), 0, &data);
    std::memcpy(data, sphereData.data(),
                sphereData.size() * sizeof(PackedSphere));
    vkUnmapMemory(vkDevice, vkSphereBufferMemory);
  }

  // Convert light data to vec4 format (position.xyz, intensity.w)
  if (!lights.empty()) {
    std::vector<glm::vec4> lightData;
    for (const auto &light : lights) {
      lightData.push_back(glm::vec4(light.position, light.intensity));
    }

    void *data;
    vkMapMemory(vkDevice, vkLightBufferMemory, 0,
                lightData.size() * sizeof(glm::vec4), 0, &data);
    std::memcpy(data, lightData.data(), lightData.size() * sizeof(glm::vec4));
    vkUnmapMemory(vkDevice, vkLightBufferMemory);
  }

  // Copy material data
  if (!materials.empty()) {
    void *data;
    vkMapMemory(vkDevice, vkMaterialBufferMemory, 0,
                materials.size() * sizeof(GPUMaterial), 0, &data);
    std::memcpy(data, materials.data(), materials.size() * sizeof(GPUMaterial));
    vkUnmapMemory(vkDevice, vkMaterialBufferMemory);
  }

  // Copy volume data
  if (!volumes.empty()) {
    void *data;
    vkMapMemory(vkDevice, vkVolumeBufferMemory, 0,
                volumes.size() * sizeof(GPUVolumetricData), 0, &data);
    std::memcpy(data, volumes.data(),
                volumes.size() * sizeof(GPUVolumetricData));
    vkUnmapMemory(vkDevice, vkVolumeBufferMemory);
  }

  // Copy voxel data
  if (!voxelData.empty()) {
    void *data;
    vkMapMemory(vkDevice, vkVoxelDataBufferMemory, 0, voxelData.size(), 0,
                &data);
    std::memcpy(data, voxelData.data(), voxelData.size());
    vkUnmapMemory(vkDevice, vkVoxelDataBufferMemory);
    std::cout << "Uploaded " << voxelData.size()
              << " bytes of voxel data to GPU" << std::endl;
  }

  // Update descriptor set with new buffers
  std::array<VkWriteDescriptorSet, 6> descriptorWrites{};

  VkDescriptorImageInfo imageInfo{};
  imageInfo.imageView = vkOutputImageView;
  imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

  descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptorWrites[0].dstSet = vkDescriptorSet;
  descriptorWrites[0].dstBinding = 0;
  descriptorWrites[0].descriptorCount = 1;
  descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
  descriptorWrites[0].pImageInfo = &imageInfo;

  VkDescriptorBufferInfo sphereBufferInfo{};
  sphereBufferInfo.buffer = vkSphereBuffer;
  sphereBufferInfo.offset = 0;
  sphereBufferInfo.range = VK_WHOLE_SIZE;

  descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptorWrites[1].dstSet = vkDescriptorSet;
  descriptorWrites[1].dstBinding = 1;
  descriptorWrites[1].descriptorCount = 1;
  descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  descriptorWrites[1].pBufferInfo = &sphereBufferInfo;

  VkDescriptorBufferInfo materialBufferInfo{};
  materialBufferInfo.buffer = vkMaterialBuffer;
  materialBufferInfo.offset = 0;
  materialBufferInfo.range = VK_WHOLE_SIZE;

  descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptorWrites[2].dstSet = vkDescriptorSet;
  descriptorWrites[2].dstBinding = 2;
  descriptorWrites[2].descriptorCount = 1;
  descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  descriptorWrites[2].pBufferInfo = &materialBufferInfo;

  VkDescriptorBufferInfo lightBufferInfo{};
  lightBufferInfo.buffer = vkLightBuffer;
  lightBufferInfo.offset = 0;
  lightBufferInfo.range = VK_WHOLE_SIZE;

  descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptorWrites[3].dstSet = vkDescriptorSet;
  descriptorWrites[3].dstBinding = 3;
  descriptorWrites[3].descriptorCount = 1;
  descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  descriptorWrites[3].pBufferInfo = &lightBufferInfo;

  VkDescriptorBufferInfo volumeBufferInfo{};
  volumeBufferInfo.buffer = vkVolumeBuffer;
  volumeBufferInfo.offset = 0;
  volumeBufferInfo.range = VK_WHOLE_SIZE;

  descriptorWrites[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptorWrites[4].dstSet = vkDescriptorSet;
  descriptorWrites[4].dstBinding = 4;
  descriptorWrites[4].descriptorCount = 1;
  descriptorWrites[4].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  descriptorWrites[4].pBufferInfo = &volumeBufferInfo;

  VkDescriptorBufferInfo voxelDataBufferInfo{};
  voxelDataBufferInfo.buffer = vkVoxelDataBuffer;
  voxelDataBufferInfo.offset = 0;
  voxelDataBufferInfo.range = VK_WHOLE_SIZE;

  descriptorWrites[5].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptorWrites[5].dstSet = vkDescriptorSet;
  descriptorWrites[5].dstBinding = 5;
  descriptorWrites[5].descriptorCount = 1;
  descriptorWrites[5].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  descriptorWrites[5].pBufferInfo = &voxelDataBufferInfo;

  vkUpdateDescriptorSets(vkDevice,
                         static_cast<uint32_t>(descriptorWrites.size()),
                         descriptorWrites.data(), 0, nullptr);
}

void VulkanRenderer::render(const PushConstants &pushConstants) {
  recordComputeCommands(pushConstants);

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &vkCommandBuffer;

  // Don't wait idle - let GPU work asynchronously
  vkQueueSubmit(vkComputeQueue, 1, &submitInfo, VK_NULL_HANDLE);
  // Removed vkQueueWaitIdle - this was blocking CPU waiting for GPU!
}

void VulkanRenderer::recordComputeCommands(const PushConstants &pushConstants) {
  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

  vkBeginCommandBuffer(vkCommandBuffer, &beginInfo);

  // Transition image layout to GENERAL for compute access
  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.image = vkOutputImage;
  barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
  barrier.srcAccessMask = 0;
  barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.layerCount = 1;

  vkCmdPipelineBarrier(vkCommandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                       VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr, 0,
                       nullptr, 1, &barrier);

  vkCmdBindPipeline(vkCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                    vkComputePipeline);
  vkCmdBindDescriptorSets(vkCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                          vkPipelineLayout, 0, 1, &vkDescriptorSet, 0, nullptr);

  // Push constants to shader
  vkCmdPushConstants(vkCommandBuffer, vkPipelineLayout,
                     VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(PushConstants),
                     &pushConstants);

  // Dispatch compute shader
  uint32_t groupCountX = (windowWidth + 15) / 16;
  uint32_t groupCountY = (windowHeight + 15) / 16;
  vkCmdDispatch(vkCommandBuffer, groupCountX, groupCountY, 1);

  vkEndCommandBuffer(vkCommandBuffer);
}

void VulkanRenderer::present() {
  // Placeholder - full swap chain presentation would go here
}

bool VulkanRenderer::readbackOutputImage(std::vector<uint32_t> &imageData) {
  // Create readback buffer if not already created
  if (vkReadbackBuffer == VK_NULL_HANDLE) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = windowWidth * windowHeight * sizeof(uint32_t);
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(vkDevice, &bufferInfo, nullptr, &vkReadbackBuffer) !=
        VK_SUCCESS) {
      return false;
    }

    VkMemoryRequirements memReqs;
    vkGetBufferMemoryRequirements(vkDevice, vkReadbackBuffer, &memReqs);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReqs.size;
    allocInfo.memoryTypeIndex =
        findMemoryType(memoryProperties, memReqs.memoryTypeBits,
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                           VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    if (vkAllocateMemory(vkDevice, &allocInfo, nullptr,
                         &vkReadbackBufferMemory) != VK_SUCCESS) {
      return false;
    }

    if (vkBindBufferMemory(vkDevice, vkReadbackBuffer, vkReadbackBufferMemory,
                           0) != VK_SUCCESS) {
      return false;
    }
  }

  // Create command buffer for copy
  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.commandPool = vkCommandPool;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandBufferCount = 1;

  VkCommandBuffer copyCmd;
  if (vkAllocateCommandBuffers(vkDevice, &allocInfo, &copyCmd) != VK_SUCCESS) {
    return false;
  }

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(copyCmd, &beginInfo);

  // Transition image for reading
  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.image = vkOutputImage;
  barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
  barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
  barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
  barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.layerCount = 1;

  vkCmdPipelineBarrier(copyCmd, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                       VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0,
                       nullptr, 1, &barrier);

  // Copy image to buffer
  VkBufferImageCopy region{};
  region.bufferOffset = 0;
  region.bufferRowLength = 0;
  region.bufferImageHeight = 0;
  region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.mipLevel = 0;
  region.imageSubresource.baseArrayLayer = 0;
  region.imageSubresource.layerCount = 1;
  region.imageOffset = {0, 0, 0};
  region.imageExtent = {static_cast<uint32_t>(windowWidth),
                        static_cast<uint32_t>(windowHeight), 1};

  vkCmdCopyImageToBuffer(copyCmd, vkOutputImage,
                         VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, vkReadbackBuffer,
                         1, &region);

  // Transition image back
  barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
  barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
  barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
  barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;

  vkCmdPipelineBarrier(copyCmd, VK_PIPELINE_STAGE_TRANSFER_BIT,
                       VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr, 0,
                       nullptr, 1, &barrier);

  vkEndCommandBuffer(copyCmd);

  // Submit and wait
  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &copyCmd;

  vkQueueSubmit(vkComputeQueue, 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(vkComputeQueue);

  // Map and read data
  void *mappedData;
  vkMapMemory(vkDevice, vkReadbackBufferMemory, 0,
              windowWidth * windowHeight * sizeof(uint32_t), 0, &mappedData);

  imageData.resize(windowWidth * windowHeight);

  // Direct fast copy - GPU outputs RGBA which SDL can use directly
  std::memcpy(imageData.data(), mappedData,
              windowWidth * windowHeight * sizeof(uint32_t));

  vkUnmapMemory(vkDevice, vkReadbackBufferMemory);

  vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &copyCmd);

  return true;
}

bool VulkanRenderer::initializeSDLRenderer(SDL_Window *window) {
  // Create SDL renderer
  sdlRenderer = SDL_CreateRenderer(window, nullptr);
  if (!sdlRenderer) {
    std::cerr << "Failed to create SDL renderer: " << SDL_GetError()
              << std::endl;
    return false;
  }

  std::cout << "SDL renderer initialized successfully" << std::endl;
  return true;
}

bool VulkanRenderer::updateSDLDisplay() {
  // Skip if SDL not initialized
  if (!sdlRenderer) {
    return false;
  }

  // Read output image from GPU
  std::vector<uint32_t> imageData;
  if (!readbackOutputImage(imageData)) {
    std::cerr << "Failed to readback output image" << std::endl;
    return false;
  }

  if (imageData.empty()) {
    std::cerr << "No image data to display" << std::endl;
    return false;
  }

  // Create a surface with proper pixel format
  SDL_Surface *surface =
      SDL_CreateSurface(windowWidth, windowHeight, SDL_PIXELFORMAT_ARGB8888);
  if (!surface) {
    std::cerr << "Failed to create SDL surface: " << SDL_GetError()
              << std::endl;
    return false;
  }

  // Copy the pixel data into the surface
  std::memcpy(surface->pixels, imageData.data(),
              imageData.size() * sizeof(uint32_t));

  // Destroy old texture if it exists
  if (sdlTexture) {
    SDL_DestroyTexture(sdlTexture);
  }

  // Create texture from surface
  sdlTexture = SDL_CreateTextureFromSurface(sdlRenderer, surface);
  SDL_DestroySurface(surface);

  if (!sdlTexture) {
    std::cerr << "Failed to create texture from surface: " << SDL_GetError()
              << std::endl;
    return false;
  }

  // Render to screen
  SDL_RenderClear(sdlRenderer);
  SDL_RenderTexture(sdlRenderer, sdlTexture, nullptr, nullptr);
  SDL_RenderPresent(sdlRenderer);

  return true;
}

bool VulkanRenderer::saveFrameToPPM(const std::string &filename) {
  // Read output image from GPU
  std::vector<uint32_t> imageData;
  if (!readbackOutputImage(imageData)) {
    std::cerr << "Failed to readback output image for PPM" << std::endl;
    return false;
  }

  // Open file for writing
  std::ofstream file(filename, std::ios::binary);
  if (!file.is_open()) {
    std::cerr << "Failed to open file: " << filename << std::endl;
    return false;
  }

  // Write PPM header
  file << "P6\n";
  file << windowWidth << " " << windowHeight << "\n";
  file << "255\n";

  // Write pixel data (convert ARGB to RGB)
  for (int i = 0; i < windowWidth * windowHeight; i++) {
    uint32_t pixel = imageData[i];
    uint8_t r = (pixel >> 16) & 0xFF;
    uint8_t g = (pixel >> 8) & 0xFF;
    uint8_t b = (pixel >> 0) & 0xFF;
    file.write(reinterpret_cast<char *>(&r), 1);
    file.write(reinterpret_cast<char *>(&g), 1);
    file.write(reinterpret_cast<char *>(&b), 1);
  }

  file.close();
  std::cout << "Frame saved to: " << filename << std::endl;
  return true;
}

void VulkanRenderer::shutdown() {
  // Clean up SDL resources
  if (sdlTexture) {
    SDL_DestroyTexture(sdlTexture);
    sdlTexture = nullptr;
  }
  if (sdlRenderer) {
    SDL_DestroyRenderer(sdlRenderer);
    sdlRenderer = nullptr;
  }

  if (vkDevice != VK_NULL_HANDLE) {
    vkDeviceWaitIdle(vkDevice);

    // Only destroy resources that were successfully created
    if (vkOutputImageView != VK_NULL_HANDLE) {
      vkDestroyImageView(vkDevice, vkOutputImageView, nullptr);
      vkOutputImageView = VK_NULL_HANDLE;
    }
    if (vkOutputImage != VK_NULL_HANDLE) {
      vkDestroyImage(vkDevice, vkOutputImage, nullptr);
      vkOutputImage = VK_NULL_HANDLE;
    }
    if (vkOutputImageMemory != VK_NULL_HANDLE) {
      vkFreeMemory(vkDevice, vkOutputImageMemory, nullptr);
      vkOutputImageMemory = VK_NULL_HANDLE;
    }

    if (vkSphereBuffer != VK_NULL_HANDLE) {
      vkDestroyBuffer(vkDevice, vkSphereBuffer, nullptr);
      vkSphereBuffer = VK_NULL_HANDLE;
    }
    if (vkSphereBufferMemory != VK_NULL_HANDLE) {
      vkFreeMemory(vkDevice, vkSphereBufferMemory, nullptr);
      vkSphereBufferMemory = VK_NULL_HANDLE;
    }

    if (vkMaterialBuffer != VK_NULL_HANDLE) {
      vkDestroyBuffer(vkDevice, vkMaterialBuffer, nullptr);
      vkMaterialBuffer = VK_NULL_HANDLE;
    }
    if (vkMaterialBufferMemory != VK_NULL_HANDLE) {
      vkFreeMemory(vkDevice, vkMaterialBufferMemory, nullptr);
      vkMaterialBufferMemory = VK_NULL_HANDLE;
    }

    if (vkLightBuffer != VK_NULL_HANDLE) {
      vkDestroyBuffer(vkDevice, vkLightBuffer, nullptr);
      vkLightBuffer = VK_NULL_HANDLE;
    }
    if (vkLightBufferMemory != VK_NULL_HANDLE) {
      vkFreeMemory(vkDevice, vkLightBufferMemory, nullptr);
      vkLightBufferMemory = VK_NULL_HANDLE;
    }

    if (vkVolumeBuffer != VK_NULL_HANDLE) {
      vkDestroyBuffer(vkDevice, vkVolumeBuffer, nullptr);
      vkVolumeBuffer = VK_NULL_HANDLE;
    }
    if (vkVolumeBufferMemory != VK_NULL_HANDLE) {
      vkFreeMemory(vkDevice, vkVolumeBufferMemory, nullptr);
      vkVolumeBufferMemory = VK_NULL_HANDLE;
    }

    if (vkComputePipeline != VK_NULL_HANDLE) {
      vkDestroyPipeline(vkDevice, vkComputePipeline, nullptr);
      vkComputePipeline = VK_NULL_HANDLE;
    }
    if (vkPipelineLayout != VK_NULL_HANDLE) {
      vkDestroyPipelineLayout(vkDevice, vkPipelineLayout, nullptr);
      vkPipelineLayout = VK_NULL_HANDLE;
    }
    if (vkDescriptorPool != VK_NULL_HANDLE) {
      vkDestroyDescriptorPool(vkDevice, vkDescriptorPool, nullptr);
      vkDescriptorPool = VK_NULL_HANDLE;
    }
    if (vkDescriptorSetLayout != VK_NULL_HANDLE) {
      vkDestroyDescriptorSetLayout(vkDevice, vkDescriptorSetLayout, nullptr);
      vkDescriptorSetLayout = VK_NULL_HANDLE;
    }

    if (vkCommandPool != VK_NULL_HANDLE) {
      vkDestroyCommandPool(vkDevice, vkCommandPool, nullptr);
      vkCommandPool = VK_NULL_HANDLE;
    }

    if (vkReadbackBuffer != VK_NULL_HANDLE) {
      vkDestroyBuffer(vkDevice, vkReadbackBuffer, nullptr);
      vkReadbackBuffer = VK_NULL_HANDLE;
    }
    if (vkReadbackBufferMemory != VK_NULL_HANDLE) {
      vkFreeMemory(vkDevice, vkReadbackBufferMemory, nullptr);
      vkReadbackBufferMemory = VK_NULL_HANDLE;
    }

    vkDestroyDevice(vkDevice, nullptr);
    vkDevice = VK_NULL_HANDLE;
  }

  if (vkInstance != VK_NULL_HANDLE) {
    vkDestroyInstance(vkInstance, nullptr);
    vkInstance = VK_NULL_HANDLE;
  }
}
