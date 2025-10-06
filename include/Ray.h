#ifndef RAY_H
#define RAY_H

#include "Vec3.h"

class Ray
{
public:
    Vec3 origin;
    Vec3 direction;

    Ray();
    Ray(const Vec3 &origin, const Vec3 &direction);

    // Get point along the ray at parameter t
    Vec3 at(double t) const;
};

#endif // RAY_H
