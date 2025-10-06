#ifndef CAMERA_H
#define CAMERA_H

#include "Ray.h"
#include "Vec3.h"

class Camera {
  double m_viewportHeight;
  double m_viewportWidth;
  Vec3 m_lookFrom;
  Vec3 m_lookAt;
  Vec3 m_vup;

  void computeCoords();

public:
  Vec3 origin;
  Vec3 lowerLeftCorner;
  Vec3 horizontal;
  Vec3 vertical;

  Camera();
  Camera(const Vec3 &lookFrom, const Vec3 &lookAt, const Vec3 &vup, double vfov,
         double aspectRatio);

  Ray getRay(double u, double v) const;

  void setLookFrom(const Vec3 &lookFrom);

  void setLookAt(const Vec3 &lookAt);

  void setVup(const Vec3 &vup);
};

#endif // CAMERA_H
