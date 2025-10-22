#ifndef ELLIPSOID_H
#define ELLIPSOID_H

#include <glm/glm.hpp>

// Forward declarations
class Material;

// GPU structure for ellipsoid (defined here to avoid conflicts)
struct GPUEllipsoid {
  glm::vec3 center;
  float padding1;
  glm::vec3 radii;
  int materialIndex;
  glm::vec3 color;
  float padding2;
};

// Ellipsoid wrapper class
class Ellipsoid {
public:
  Ellipsoid() noexcept
      : center_(glm::vec3(0.0f)), radii_(glm::vec3(1.0f)),
        color_(glm::vec3(1.0f)), material_(nullptr), materialIndex_(-1) {}

  Ellipsoid(const glm::vec3 &center, const glm::vec3 &radii,
            const glm::vec3 &color, const Material &material) noexcept
      : center_(center), radii_(radii), color_(color), material_(&material),
        materialIndex_(-1) {}

  // Accessors
  constexpr glm::vec3 getCenter() const noexcept { return center_; }
  constexpr glm::vec3 getRadii() const noexcept { return radii_; }
  constexpr glm::vec3 getColor() const noexcept { return color_; }
  constexpr int getMaterialIndex() const noexcept { return materialIndex_; }
  const Material *getMaterial() const noexcept { return material_; }

  // Mutators
  constexpr void setCenter(const glm::vec3 &center) noexcept {
    center_ = center;
  }
  constexpr void setRadii(const glm::vec3 &radii) noexcept { radii_ = radii; }
  constexpr void setColor(const glm::vec3 &color) noexcept { color_ = color; }
  void setMaterial(const Material &material) noexcept {
    material_ = &material;
    materialIndex_ = -1; // Reset index when material changes
  }

  // Internal method used by pre-render to set resolved material index
  void setMaterialIndex(int index) noexcept { materialIndex_ = index; }

  // Convert to GPU format
  GPUEllipsoid toGPU() const noexcept;

private:
  glm::vec3 center_;
  glm::vec3 radii_; // Radii in x, y, z directions
  glm::vec3 color_;
  const Material *material_; // Reference to material instance
  int materialIndex_;        // Resolved index after pre-render
};

#endif // ELLIPSOID_H
