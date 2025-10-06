#ifndef MATERIAL_H
#define MATERIAL_H

#include "Vec3.h"

class Material {
public:
  Vec3 color;          // Base color (albedo) (values 0-1)
  double ambient;      // Ambient reflection coefficient (0-1)
  double diffuse;      // Diffuse reflection coefficient (0-1)
  double specular;     // Specular reflection coefficient (0-1)
  double shininess;    // Specular shininess exponent
  double reflectivity; // Reflectivity for bounce lighting (0-1)

  Material();
  Material(const Vec3 &color);
  Material(double r, double g, double b);
  Material(const Vec3 &color, double ambient, double diffuse, double specular,
           double shininess, double reflectivity);

  // Get color as RGB bytes (0-255)
  void getColorBytes(unsigned char &r, unsigned char &g,
                     unsigned char &b) const;
};

#endif // MATERIAL_H
