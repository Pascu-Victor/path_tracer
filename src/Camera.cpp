#include "Camera.h"
#include <cmath>

Camera::Camera()
    : origin(0, 0, 0),
      lowerLeftCorner(-2, -1, -1),
      horizontal(4, 0, 0),
      vertical(0, 2, 0) {}

Camera::Camera(const Vec3 &lookFrom, const Vec3 &lookAt, const Vec3 &vup,
               double vfov, double aspectRatio)
{

    // Convert vertical field of view from degrees to radians
    double theta = vfov * M_PI / 180.0;
    double h = std::tan(theta / 2.0);
    double viewportHeight = 2.0 * h;
    double viewportWidth = aspectRatio * viewportHeight;

    // Calculate camera coordinate frame
    Vec3 w = (lookFrom - lookAt).normalized(); // Camera backward direction
    Vec3 u = vup.cross(w).normalized();        // Camera right direction
    Vec3 v = w.cross(u);                       // Camera up direction

    origin = lookFrom;
    horizontal = viewportWidth * u;
    vertical = viewportHeight * v;
    lowerLeftCorner = origin - horizontal / 2.0 - vertical / 2.0 - w;
}

Ray Camera::getRay(double u, double v) const
{
    Vec3 direction = lowerLeftCorner + u * horizontal + v * vertical - origin;
    return Ray(origin, direction.normalized());
}
