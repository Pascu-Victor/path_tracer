#ifndef SPHERE_H
#define SPHERE_H

#include "Object.h"

class Sphere : public Object
{
public:
    Vec3 center;
    double radius;

    Sphere();
    Sphere(const Vec3 &center, double radius, std::shared_ptr<Material> material);

    bool hit(const Ray &ray, double tMin, double tMax, HitRecord &record) const override;
};

#endif // SPHERE_H
