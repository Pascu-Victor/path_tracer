#ifndef MATERIAL_H
#define MATERIAL_H

#include "Vec3.h"

class Material
{
public:
  Vec3 color;          // Base color (albedo) (values 0-1)
  double ambient;      // Ambient reflection coefficient (0-1)
  double diffuse;      // Diffuse reflection coefficient (0-1)
  double specular;     // Specular reflection coefficient (0-1)
  double shininess;    // Specular shininess exponent
  double reflectivity; // Reflectivity for bounce lighting (0-1)
  double transparency; // Transparency for refraction (0-1)

  // Color shading properties
  Vec3 emissive;           // Emissive color for self-illumination (0-1)
  double emissiveStrength; // Emissive intensity multiplier

  // Volumetric properties
  bool isVolumetric;      // Whether material has volumetric properties
  double density;         // Volume density for scattering (0-1)
  Vec3 scatterColor;      // Color of scattered light in volume
  double absorptionCoeff; // Light absorption coefficient

  Material();
  Material(const Vec3 &color);
  Material(double r, double g, double b);
  Material(const Vec3 &color, double ambient, double diffuse, double specular,
           double shininess, double reflectivity, double transparency = 0.0,
           const Vec3 &emissive = Vec3(0, 0, 0), double emissiveStrength = 0.0,
           bool isVolumetric = false, double density = 0.0,
           const Vec3 &scatterColor = Vec3(1, 1, 1),
           double absorptionCoeff = 0.0);

  // Get color as RGB bytes (0-255)
  void getColorBytes(unsigned char &r, unsigned char &g,
                     unsigned char &b) const;
};

#endif // MATERIAL_H
