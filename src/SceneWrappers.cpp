#include "SceneWrappers.h"
#include "VulkanRenderer.h"
#include <iostream>

GPUMaterial Material::toGPU(const std::unordered_map<std::string, int>
                                &shaderPathToIndex) const noexcept {
  GPUMaterial gpu{};
  gpu.colorAndAmbient = glm::vec4(data_.color, data_.ambient);
  gpu.diffuseSpecularShiny = glm::vec4(data_.diffuse, data_.specular,
                                       data_.shininess, data_.reflectivity);

  // Resolve shader path to index
  int shaderIndex = 0;
  if (!shaderPath_.empty()) {
    auto it = shaderPathToIndex.find(shaderPath_);
    if (it != shaderPathToIndex.end()) {
      shaderIndex = it->second;
    } else {
      for (const auto &[path, idx] : shaderPathToIndex) {
        std::cout << "  '" << path << "' -> " << idx << std::endl;
      }
    }
  }

  gpu.transparencyEmissive =
      glm::vec4(data_.transparency, data_.emissiveStrength,
                static_cast<float>(shaderIndex), 0.0f);
  gpu.emissive = glm::vec4(data_.emissive, 0.0f);
  gpu.scatterAndAbsorption =
      glm::vec4(data_.scatterColor, data_.absorptionCoeff);
  return gpu;
}

GPUSphere Sphere::toGPU() const noexcept {
  GPUSphere gpu{};
  gpu.center = center_;
  gpu.radius = radius_;
  gpu.color = color_;
  gpu.materialIndex = materialIndex_;
  return gpu;
}

GPULight Light::toGPU() const noexcept {
  GPULight gpu{};
  gpu.position = position_;
  gpu.intensity = intensity_;
  gpu.color = color_;
  gpu.padding = 0.0f;
  return gpu;
}

GPUVolumetricData VolumetricData::toGPU() const noexcept {
  GPUVolumetricData gpu{};
  gpu.position = position_;
  gpu.scale = scale_;
  gpu.v0 = v0_;
  gpu.v1 = v1_;
  gpu.resolution_x = resolution_.x;
  gpu.resolution_y = resolution_.y;
  gpu.resolution_z = resolution_.z;
  gpu.materialIndex = materialIndex_;
  gpu.padding[0] = 0;
  gpu.padding[1] = 0;
  return gpu;
}
