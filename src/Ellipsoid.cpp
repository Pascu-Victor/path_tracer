#include "Ellipsoid.h"

GPUEllipsoid Ellipsoid::toGPU() const noexcept {
  GPUEllipsoid gpu{};
  gpu.center = center_;
  gpu.radii = radii_;
  gpu.color = color_;
  gpu.materialIndex = materialIndex_;
  gpu.rotation = rotation_;  // Include quaternion rotation
  gpu.padding1 = 0.0f;
  gpu.padding2 = 0.0f;
  return gpu;
}
