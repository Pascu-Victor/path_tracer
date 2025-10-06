#include "Light.h"

Light::Light() : position(0, 0, 0), color(1.0, 1.0, 1.0), intensity(1.0) {}

Light::Light(const Vec3 &position, const Vec3 &color, double intensity)
    : position(position), color(color), intensity(intensity) {}
