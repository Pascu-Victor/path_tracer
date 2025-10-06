#include "Material.h"
#include <algorithm>

Material::Material() : color(1.0, 1.0, 1.0) {}

Material::Material(const Vec3 &color) : color(color) {}

Material::Material(double r, double g, double b) : color(r, g, b) {}

void Material::getColorBytes(unsigned char &r, unsigned char &g, unsigned char &b) const
{
    // Clamp color values to [0, 1] and convert to [0, 255]
    r = static_cast<unsigned char>(std::clamp(color.x, 0.0, 1.0) * 255.99);
    g = static_cast<unsigned char>(std::clamp(color.y, 0.0, 1.0) * 255.99);
    b = static_cast<unsigned char>(std::clamp(color.z, 0.0, 1.0) * 255.99);
}
