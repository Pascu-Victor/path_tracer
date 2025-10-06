#ifndef MATERIAL_H
#define MATERIAL_H

#include "Vec3.h"

class Material
{
public:
    Vec3 color; // RGB color (values 0-1)

    Material();
    Material(const Vec3 &color);
    Material(double r, double g, double b);

    // Get color as RGB bytes (0-255)
    void getColorBytes(unsigned char &r, unsigned char &g, unsigned char &b) const;
};

#endif // MATERIAL_H
