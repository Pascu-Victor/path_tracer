#ifndef CAMERA_H
#define CAMERA_H

#include "Ray.h"
#include "Vec3.h"

class Camera
{
public:
    Vec3 origin;
    Vec3 lowerLeftCorner;
    Vec3 horizontal;
    Vec3 vertical;

    Camera();
    Camera(const Vec3 &lookFrom, const Vec3 &lookAt, const Vec3 &vup,
           double vfov, double aspectRatio);

    Ray getRay(double u, double v) const;
};

#endif // CAMERA_H
