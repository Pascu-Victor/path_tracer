#ifndef SCENE_WRAPPERS_H
#define SCENE_WRAPPERS_H

#include <glm/glm.hpp>
#include <unordered_map>
#include <vector>

// Forward declarations of GPU structures
struct GPUMaterial;
struct GPUSphere;
struct GPULight;
struct GPUVolumetricData;

// Material wrapper class with transparent access to packed parameters
class Material {
public:
  // Constructors
  constexpr Material() noexcept
      : data_{.color = glm::vec3(1.0f),
              .ambient = 0.1f,
              .diffuse = 0.5f,
              .specular = 0.5f,
              .shininess = 32.0f,
              .reflectivity = 0.0f,
              .transparency = 0.0f,
              .emissiveStrength = 0.0f,
              .emissive = glm::vec3(0.0f),
              .scatterColor = glm::vec3(0.0f),
              .absorptionCoeff = 0.0f} {}

  // Named constructors for common material types
  static constexpr Material Diffuse(const glm::vec3 &color,
                                    float diffuse = 0.7f,
                                    float ambient = 0.1f) noexcept {
    Material mat;
    mat.setColor(color);
    mat.setAmbient(ambient);
    mat.setDiffuse(diffuse);
    mat.setSpecular(0.1f);
    mat.setShininess(16.0f);
    return mat;
  }

  static constexpr Material Specular(const glm::vec3 &color, float specular,
                                     float shininess,
                                     float reflectivity = 0.0f) noexcept {
    Material mat;
    mat.setColor(color);
    mat.setSpecular(specular);
    mat.setShininess(shininess);
    mat.setReflectivity(reflectivity);
    return mat;
  }

  static constexpr Material Mirror(const glm::vec3 &color = glm::vec3(0.9f),
                                   float reflectivity = 0.7f) noexcept {
    Material mat;
    mat.setColor(color);
    mat.setDiffuse(0.2f);
    mat.setSpecular(0.8f);
    mat.setShininess(128.0f);
    mat.setReflectivity(reflectivity);
    return mat;
  }

  static constexpr Material Volumetric(const glm::vec3 &scatterColor,
                                       float absorptionCoeff) noexcept {
    Material mat;
    mat.setScatterColor(scatterColor);
    mat.setAbsorptionCoeff(absorptionCoeff);
    return mat;
  }

  static constexpr Material Emissive(const glm::vec3 &color,
                                     float strength = 1.0f) noexcept {
    Material mat;
    mat.setColor(color);
    mat.setAmbient(0.0f); // Emissive objects don't need ambient
    mat.setDiffuse(0.0f); // Emissive objects don't receive diffuse lighting
    mat.setEmissiveStrength(strength);
    return mat;
  }

  // Accessors
  constexpr glm::vec3 getColor() const noexcept { return data_.color; }
  constexpr float getAmbient() const noexcept { return data_.ambient; }
  constexpr float getDiffuse() const noexcept { return data_.diffuse; }
  constexpr float getSpecular() const noexcept { return data_.specular; }
  constexpr float getShininess() const noexcept { return data_.shininess; }
  constexpr float getReflectivity() const noexcept {
    return data_.reflectivity;
  }
  constexpr float getTransparency() const noexcept {
    return data_.transparency;
  }
  constexpr float getEmissiveStrength() const noexcept {
    return data_.emissiveStrength;
  }
  constexpr glm::vec3 getEmissive() const noexcept { return data_.emissive; }
  constexpr glm::vec3 getScatterColor() const noexcept {
    return data_.scatterColor;
  }
  constexpr float getAbsorptionCoeff() const noexcept {
    return data_.absorptionCoeff;
  }

  // Mutators
  constexpr void setColor(const glm::vec3 &color) noexcept {
    data_.color = color;
  }
  constexpr void setAmbient(float ambient) noexcept { data_.ambient = ambient; }
  constexpr void setDiffuse(float diffuse) noexcept { data_.diffuse = diffuse; }
  constexpr void setSpecular(float specular) noexcept {
    data_.specular = specular;
  }
  constexpr void setShininess(float shininess) noexcept {
    data_.shininess = shininess;
  }
  constexpr void setReflectivity(float reflectivity) noexcept {
    data_.reflectivity = reflectivity;
  }
  constexpr void setTransparency(float transparency) noexcept {
    data_.transparency = transparency;
  }
  constexpr void setEmissiveStrength(float strength) noexcept {
    data_.emissiveStrength = strength;
  }
  constexpr void setEmissive(const glm::vec3 &emissive) noexcept {
    data_.emissive = emissive;
  }
  constexpr void setScatterColor(const glm::vec3 &color) noexcept {
    data_.scatterColor = color;
  }
  constexpr void setAbsorptionCoeff(float coeff) noexcept {
    data_.absorptionCoeff = coeff;
  }

  // Convert to GPU format
  GPUMaterial toGPU() const noexcept;

private:
  // Union for transparent access to packed data
  union {
    struct {
      glm::vec3 color;
      float ambient;
      float diffuse;
      float specular;
      float shininess;
      float reflectivity;
      float transparency;
      float emissiveStrength;
      float padding1;
      float padding2;
      glm::vec3 emissive;
      float padding3;
      glm::vec3 scatterColor;
      float absorptionCoeff;
    } data_;
  };
};

// Sphere wrapper class
class Sphere {
public:
  Sphere() noexcept
      : center_(glm::vec3(0.0f)), radius_(1.0f), color_(glm::vec3(1.0f)),
        material_(nullptr), materialIndex_(-1) {}

  Sphere(const glm::vec3 &center, float radius, const glm::vec3 &color,
         const Material &material) noexcept
      : center_(center), radius_(radius), color_(color), material_(&material),
        materialIndex_(-1) {}

  // Accessors
  constexpr glm::vec3 getCenter() const noexcept { return center_; }
  constexpr float getRadius() const noexcept { return radius_; }
  constexpr glm::vec3 getColor() const noexcept { return color_; }
  constexpr int getMaterialIndex() const noexcept { return materialIndex_; }
  const Material *getMaterial() const noexcept { return material_; }

  // Mutators
  constexpr void setCenter(const glm::vec3 &center) noexcept {
    center_ = center;
  }
  constexpr void setRadius(float radius) noexcept { radius_ = radius; }
  constexpr void setColor(const glm::vec3 &color) noexcept { color_ = color; }
  void setMaterial(const Material &material) noexcept {
    material_ = &material;
    materialIndex_ = -1; // Reset index when material changes
  }

  // Internal method used by pre-render to set resolved material index
  void setMaterialIndex(int index) noexcept { materialIndex_ = index; }

  // Convert to GPU format
  GPUSphere toGPU() const noexcept;

private:
  glm::vec3 center_;
  float radius_;
  glm::vec3 color_;
  const Material *material_; // Reference to material instance
  int materialIndex_;        // Resolved index after pre-render
};

// Light wrapper class
class Light {
public:
  constexpr Light() noexcept
      : position_(glm::vec3(0.0f)), intensity_(1.0f), color_(glm::vec3(1.0f)) {}

  constexpr Light(const glm::vec3 &position, float intensity,
                  const glm::vec3 &color) noexcept
      : position_(position), intensity_(intensity), color_(color) {}

  // Accessors
  constexpr glm::vec3 getPosition() const noexcept { return position_; }
  constexpr float getIntensity() const noexcept { return intensity_; }
  constexpr glm::vec3 getColor() const noexcept { return color_; }

  // Mutators
  constexpr void setPosition(const glm::vec3 &position) noexcept {
    position_ = position;
  }
  constexpr void setIntensity(float intensity) noexcept {
    intensity_ = intensity;
  }
  constexpr void setColor(const glm::vec3 &color) noexcept { color_ = color; }

  // Convert to GPU format
  GPULight toGPU() const noexcept;

private:
  glm::vec3 position_;
  float intensity_;
  glm::vec3 color_;
};

// VolumetricData wrapper class
class VolumetricData {
public:
  VolumetricData() noexcept
      : position_(glm::vec3(0.0f)), scale_(1.0f), v0_(glm::vec3(0.0f)),
        v1_(glm::vec3(1.0f)), resolution_(glm::ivec3(1)), material_(nullptr),
        materialIndex_(-1) {}

  VolumetricData(const glm::vec3 &position, float scale, const glm::vec3 &v0,
                 const glm::vec3 &v1, const glm::ivec3 &resolution,
                 const Material &material) noexcept
      : position_(position), scale_(scale), v0_(v0), v1_(v1),
        resolution_(resolution), material_(&material), materialIndex_(-1) {}

  // Accessors
  constexpr glm::vec3 getPosition() const noexcept { return position_; }
  constexpr float getScale() const noexcept { return scale_; }
  constexpr glm::vec3 getV0() const noexcept { return v0_; }
  constexpr glm::vec3 getV1() const noexcept { return v1_; }
  constexpr glm::ivec3 getResolution() const noexcept { return resolution_; }
  constexpr int getMaterialIndex() const noexcept { return materialIndex_; }
  const Material *getMaterial() const noexcept { return material_; }

  // Mutators
  constexpr void setPosition(const glm::vec3 &position) noexcept {
    position_ = position;
  }
  constexpr void setScale(float scale) noexcept { scale_ = scale; }
  constexpr void setV0(const glm::vec3 &v0) noexcept { v0_ = v0; }
  constexpr void setV1(const glm::vec3 &v1) noexcept { v1_ = v1; }
  constexpr void setResolution(const glm::ivec3 &resolution) noexcept {
    resolution_ = resolution;
  }
  void setMaterial(const Material &material) noexcept {
    material_ = &material;
    materialIndex_ = -1; // Reset index when material changes
  }

  // Internal method used by pre-render to set resolved material index
  void setMaterialIndex(int index) noexcept { materialIndex_ = index; }

  // Convert to GPU format
  GPUVolumetricData toGPU() const noexcept;

private:
  glm::vec3 position_;
  float scale_;
  glm::vec3 v0_;
  glm::vec3 v1_;
  glm::ivec3 resolution_;
  const Material *material_; // Reference to material instance
  int materialIndex_;        // Resolved index after pre-render
};

// Scene management helper class for pre-render material mapping
class SceneManager {
public:
  // Pre-render method that builds material list and maps object materials to
  // indices Takes sphere and volumetric object lists, extracts unique
  // materials, and updates material indices in objects
  template <typename... ObjectContainers>
  static void prepareForRender(std::vector<Material> &outMaterials,
                               ObjectContainers &...objectContainers) {
    outMaterials.clear();
    std::unordered_map<const Material *, int> materialIndexMap;

    // Helper lambda to process a container of objects
    auto processContainer = [&](auto &container) {
      using ObjectType = typename std::decay_t<decltype(container)>::value_type;
      for (auto &obj : container) {
        const Material *mat = obj.getMaterial();
        if (mat == nullptr) {
          continue; // Skip objects without materials
        }

        // Check if this material is already in our map
        auto it = materialIndexMap.find(mat);
        if (it == materialIndexMap.end()) {
          // New material - add it to the list
          int newIndex = static_cast<int>(outMaterials.size());
          outMaterials.push_back(*mat);
          materialIndexMap[mat] = newIndex;
          obj.setMaterialIndex(newIndex);
        } else {
          // Material already exists - use existing index
          obj.setMaterialIndex(it->second);
        }
      }
    };

    // Process all provided containers
    (processContainer(objectContainers), ...);
  }
};

#endif // SCENE_WRAPPERS_H
