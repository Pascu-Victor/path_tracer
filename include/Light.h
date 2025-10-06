#ifndef LIGHT_H
#define LIGHT_H

#include "Vec3.h"

class Light {
public:
  Vec3 position;
  Vec3 color; // RGB color of the light (0-1 range)
  double intensity;

  Light();
  Light(const Vec3 &position, const Vec3 &color, double intensity);
};

#endif // LIGHT_H
