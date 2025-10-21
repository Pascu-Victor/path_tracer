#ifndef VOLUMETRIC_H
#define VOLUMETRIC_H

#include "Object.h"
#include <string>

class Volumetric : public Object {
private:
  Vec3 _position;
  double _scale;
  uint8_t *_data;
  int _resolution[3] = {0};
  double _thickness[3] = {0};
  Vec3 _v0;
  Vec3 _v1;

public:
  Volumetric(std::string datFile, std::string rawFile, Vec3 position,
             double scale, std::shared_ptr<Material> material);
  bool hit(const Ray &ray, double tMin, double tMax,
           HitRecord &record) const override;

  // Public method to query density at a world position
  double getDensity(const Vec3 &point) const;

  // Get the exit point of a ray from the volume
  Vec3 getExitPoint(const Ray &ray, const Vec3 &entryPoint) const;

private:
  uint8_t Value(int x, int y, int z) const;
  Vec3 GetIndexes(Vec3 v) const;
  Vec3 GetNormal(Vec3 v) const;
};

#endif