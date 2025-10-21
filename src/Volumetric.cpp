#include <Volumetric.h>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>

Volumetric::Volumetric(std::string datFile, std::string rawFile, Vec3 position,
                       double scale, std::shared_ptr<Material> material)
    : Object(material) {
  _position = position;
  _scale = scale;
  std::ifstream file(datFile);
  std::string line;
  std::cout << "Loading volumetric data from " << datFile
            << " found: " << file.is_open() << " and " << rawFile << std::endl;
  while (std::getline(file, line)) {
    // Parse key: value format, handling tabs and spaces
    size_t colonPos = line.find(':');
    if (colonPos == std::string::npos)
      continue;

    std::string key = line.substr(0, colonPos);
    // Trim key
    key.erase(key.find_last_not_of(" \t\r\n") + 1);

    std::string valueStr = line.substr(colonPos + 1);
    // Trim leading whitespace from valueStr
    valueStr.erase(0, valueStr.find_first_not_of(" \t\r\n"));

    if (key == "Resolution") {
      std::stringstream ss(valueStr);
      ss >> _resolution[0] >> _resolution[1] >> _resolution[2];
      std::cout << "Resolution: " << _resolution[0] << " " << _resolution[1]
                << " " << _resolution[2] << std::endl;
    } else if (key == "SliceThickness") {
      std::stringstream ss(valueStr);
      ss >> _thickness[0] >> _thickness[1] >> _thickness[2];
      std::cout << "SliceThickness: " << _thickness[0] << " " << _thickness[1]
                << " " << _thickness[2] << std::endl;
    }
  }
  file.close();

  _v0 = position;
  _v1 = position + Vec3(_resolution[0] * _thickness[0] * scale,
                        _resolution[1] * _thickness[1] * scale,
                        _resolution[2] * _thickness[2] * scale);

  std::cout << "Volume bounds: (" << _v0.x << ", " << _v0.y << ", " << _v0.z
            << ") to (" << _v1.x << ", " << _v1.y << ", " << _v1.z << ")"
            << std::endl;

  long long len = (long long)_resolution[0] * _resolution[1] * _resolution[2];
  _data = new uint8_t[len];

  // Load volumetric data from raw file
  std::ifstream rawFile_stream(rawFile, std::ios::binary);
  if (rawFile_stream.is_open()) {
    rawFile_stream.read(reinterpret_cast<char *>(_data), len);
    long long bytesRead = rawFile_stream.gcount();
    rawFile_stream.close();
    std::cout << "Loaded " << bytesRead << " bytes of volumetric data from "
              << rawFile << " (expected " << len << " bytes)" << std::endl;
    if (bytesRead != len) {
      std::cerr << "Warning: Read " << bytesRead << " bytes, expected " << len
                << std::endl;
    }
  } else {
    std::cerr << "Failed to load volumetric data from " << rawFile << std::endl;
  }
}

bool Volumetric::hit(const Ray &ray, double tMin, double tMax,
                     HitRecord &record) const {
  double t0 = tMin;
  double t1 = tMax;
  const double EPSILON = 1e-6;

  // Check X axis
  if (std::abs(ray.direction.x) > 1e-6) {
    double invDx = 1.0 / ray.direction.x;
    double tNearX = (_v0.x - ray.origin.x) * invDx;
    double tFarX = (_v1.x - ray.origin.x) * invDx;
    if (invDx < 0.0) {
      std::swap(tNearX, tFarX);
    }
    t0 = tNearX > t0 ? tNearX : t0;
    t1 = tFarX < t1 ? tFarX : t1;
  } else {
    // Ray is parallel to YZ plane
    if (ray.origin.x < _v0.x - EPSILON || ray.origin.x > _v1.x + EPSILON) {
      return false;
    }
  }
  if (t1 <= t0 + EPSILON) {
    return false;
  }

  // Check Y axis
  if (std::abs(ray.direction.y) > 1e-6) {
    double invDy = 1.0 / ray.direction.y;
    double tNearY = (_v0.y - ray.origin.y) * invDy;
    double tFarY = (_v1.y - ray.origin.y) * invDy;
    if (invDy < 0.0) {
      std::swap(tNearY, tFarY);
    }
    t0 = tNearY > t0 ? tNearY : t0;
    t1 = tFarY < t1 ? tFarY : t1;
  } else {
    // Ray is parallel to XZ plane
    if (ray.origin.y < _v0.y - EPSILON || ray.origin.y > _v1.y + EPSILON) {
      return false;
    }
  }
  if (t1 <= t0 + EPSILON) {
    return false;
  }

  // Check Z axis
  if (std::abs(ray.direction.z) > 1e-6) {
    double invDz = 1.0 / ray.direction.z;
    double tNearZ = (_v0.z - ray.origin.z) * invDz;
    double tFarZ = (_v1.z - ray.origin.z) * invDz;
    if (invDz < 0.0) {
      std::swap(tNearZ, tFarZ);
    }
    t0 = tNearZ > t0 ? tNearZ : t0;
    t1 = tFarZ < t1 ? tFarZ : t1;
  } else {
    // Ray is parallel to XY plane
    if (ray.origin.z < _v0.z - EPSILON || ray.origin.z > _v1.z + EPSILON) {
      return false;
    }
  }
  if (t1 <= t0 + EPSILON) {
    return false;
  }

  if (t0 < tMin || t0 > tMax) {
    return false;
  }

  record.t = t0;
  record.point = ray.at(t0);
  record.normal = GetNormal(record.point);
  record.material = material.get();
  record.volumetricHit = true;
  record.objectPtr = (void *)this; // Store pointer to this volumetric object

  // Return true for any intersection with the bounding box
  // Density will be queried during ray marching in traceRay()
  return true;
}

uint8_t Volumetric::Value(int x, int y, int z) const {
  if (x < 0 || y < 0 || z < 0 || x >= _resolution[0] || y >= _resolution[1] ||
      z >= _resolution[2]) {
    return 0;
  }

  return _data[z * _resolution[1] * _resolution[0] + y * _resolution[0] + x];
}

Vec3 Volumetric::GetIndexes(Vec3 v) const {
  Vec3 localPos = v - _position;
  double voxelSizeX = _thickness[0] * _scale;
  double voxelSizeY = _thickness[1] * _scale;
  double voxelSizeZ = _thickness[2] * _scale;

  return Vec3((int)std::floor(localPos.x / voxelSizeX),
              (int)std::floor(localPos.y / voxelSizeY),
              (int)std::floor(localPos.z / voxelSizeZ));
}

Vec3 Volumetric::GetNormal(Vec3 v) const {
  Vec3 idx = GetIndexes(v);
  double x0 = Value(idx.x - 1, idx.y, idx.z);
  double x1 = Value(idx.x + 1, idx.y, idx.z);
  double y0 = Value(idx.x, idx.y - 1, idx.z);
  double y1 = Value(idx.x, idx.y + 1, idx.z);
  double z0 = Value(idx.x, idx.y, idx.z - 1);
  double z1 = Value(idx.x, idx.y, idx.z + 1);

  return Vec3(x1 - x0, y1 - y0, z1 - z0).normalized();
}

double Volumetric::getDensity(const Vec3 &point) const {
  Vec3 idx = GetIndexes(point);
  uint8_t value = Value((int)std::floor(idx.x), (int)std::floor(idx.y),
                        (int)std::floor(idx.z));
  return value / 255.0; // Normalize to [0, 1] from uint8_t range [0, 255]
}

Vec3 Volumetric::getExitPoint(const Ray &ray, const Vec3 &entryPoint) const {
  // Compute where the ray exits the bounding box starting from entry point
  double t0 = 0.0;
  double t1 = 1e10;

  // Check X axis
  if (std::abs(ray.direction.x) > 1e-6) {
    double invDx = 1.0 / ray.direction.x;
    double tNearX = (_v0.x - entryPoint.x) * invDx;
    double tFarX = (_v1.x - entryPoint.x) * invDx;
    if (invDx < 0.0) {
      std::swap(tNearX, tFarX);
    }
    t0 = std::max(t0, tNearX);
    t1 = std::min(t1, tFarX);
  }

  // Check Y axis
  if (std::abs(ray.direction.y) > 1e-6) {
    double invDy = 1.0 / ray.direction.y;
    double tNearY = (_v0.y - entryPoint.y) * invDy;
    double tFarY = (_v1.y - entryPoint.y) * invDy;
    if (invDy < 0.0) {
      std::swap(tNearY, tFarY);
    }
    t0 = std::max(t0, tNearY);
    t1 = std::min(t1, tFarY);
  }

  // Check Z axis
  if (std::abs(ray.direction.z) > 1e-6) {
    double invDz = 1.0 / ray.direction.z;
    double tNearZ = (_v0.z - entryPoint.z) * invDz;
    double tFarZ = (_v1.z - entryPoint.z) * invDz;
    if (invDz < 0.0) {
      std::swap(tNearZ, tFarZ);
    }
    t0 = std::max(t0, tNearZ);
    t1 = std::min(t1, tFarZ);
  }

  // Return the exit point (at t1, the far intersection)
  return entryPoint + ray.direction * t1;
}