#include "Material.h"
#include <algorithm>

Material::Material()
    : color(1.0, 1.0, 1.0), ambient(0.1), diffuse(0.7), specular(0.2),
      shininess(32.0), reflectivity(0.0) {}

Material::Material(const Vec3 &color)
    : color(color), ambient(0.1), diffuse(0.7), specular(0.2), shininess(32.0),
      reflectivity(0.0) {}

Material::Material(double r, double g, double b)
    : color(r, g, b), ambient(0.1), diffuse(0.7), specular(0.2),
      shininess(32.0), reflectivity(0.0) {}

Material::Material(const Vec3 &color, double ambient, double diffuse,
                   double specular, double shininess, double reflectivity,
                   double transparency, const Vec3 &emissive, double emissiveStrength,
                   bool isVolumetric, double density, const Vec3 &scatterColor,
                   double absorptionCoeff)
    : color(color), ambient(ambient), diffuse(diffuse), specular(specular),
      shininess(shininess), reflectivity(reflectivity), transparency(transparency),
      emissive(emissive), emissiveStrength(emissiveStrength), isVolumetric(isVolumetric),
      density(density), scatterColor(scatterColor), absorptionCoeff(absorptionCoeff) {}

void Material::getColorBytes(unsigned char &r, unsigned char &g,
                             unsigned char &b) const
{
  // Clamp color values to [0, 1] and convert to [0, 255]
  r = static_cast<unsigned char>(std::clamp(color.x, 0.0, 1.0) * 255.99);
  g = static_cast<unsigned char>(std::clamp(color.y, 0.0, 1.0) * 255.99);
  b = static_cast<unsigned char>(std::clamp(color.z, 0.0, 1.0) * 255.99);
}
