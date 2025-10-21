#ifndef OBJECT_H
#define OBJECT_H

#include "Ray.h"
#include "Material.h"
#include <memory>

// Hit record structure to store intersection information
struct HitRecord
{
    Vec3 point;         // Point of intersection
    Vec3 normal;        // Surface normal at intersection
    double t;           // Ray parameter at intersection
    Material *material; // Material of the object
    bool volumetricHit; // Whether the hit is inside a volumetric object
    double density;     // Density at the hit point (for volumetric materials)
    void *objectPtr;    // Pointer to the hit object for additional queries
};

class Object
{
public:
    std::shared_ptr<Material> material;

    Object();
    Object(std::shared_ptr<Material> material);
    virtual ~Object() = default;

    // Pure virtual function for ray intersection
    virtual bool hit(const Ray &ray, double tMin, double tMax, HitRecord &record) const = 0;
};

#endif // OBJECT_H
