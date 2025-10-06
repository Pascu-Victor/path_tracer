#include "Sphere.h"
#include <cmath>

Sphere::Sphere() : Object(), center(), radius(1.0) {}

Sphere::Sphere(const Vec3 &center, double radius, std::shared_ptr<Material> material)
    : Object(material), center(center), radius(radius) {}

bool Sphere::hit(const Ray &ray, double tMin, double tMax, HitRecord &record) const
{
    // Ray-sphere intersection using quadratic formula
    // (origin + t * direction - center)^2 = radius^2

    Vec3 oc = ray.origin - center;
    double a = ray.direction.dot(ray.direction);
    double half_b = oc.dot(ray.direction);
    double c = oc.dot(oc) - radius * radius;

    double discriminant = half_b * half_b - a * c;

    if (discriminant < 0)
    {
        return false; // No intersection
    }

    double sqrtd = std::sqrt(discriminant);

    // Find the nearest root that lies in the acceptable range
    double root = (-half_b - sqrtd) / a;
    if (root < tMin || root > tMax)
    {
        root = (-half_b + sqrtd) / a;
        if (root < tMin || root > tMax)
        {
            return false;
        }
    }

    // Fill hit record
    record.t = root;
    record.point = ray.at(root);
    record.normal = (record.point - center) / radius;
    record.material = material.get();

    return true;
}
