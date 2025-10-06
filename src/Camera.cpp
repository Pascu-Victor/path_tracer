#include "Camera.h"
#include "Vec3.h"
#include <cmath>

Camera::Camera()
    : origin(0, 0, 0), lowerLeftCorner(-2, -1, -1), horizontal(4, 0, 0),
      vertical(0, 2, 0) {}

void Camera::computeCoords() {

  // Calculate camera coordinate frame
  Vec3 w = (this->m_lookFrom - this->m_lookAt)
               .normalized();                 // Camera backward direction
  Vec3 u = this->m_vup.cross(w).normalized(); // Camera right direction
  Vec3 v = w.cross(u);                        // Camera up direction

  origin = this->m_lookFrom;
  horizontal = this->m_viewportWidth * u;
  vertical = this->m_viewportHeight * v;
  lowerLeftCorner = origin - horizontal / 2.0 - vertical / 2.0 - w;
}

Camera::Camera(const Vec3 &lookFrom, const Vec3 &lookAt, const Vec3 &vup,
               double vfov, double aspectRatio) {

  // Convert vertical field of view from degrees to radians
  double theta = vfov * M_PI / 180.0;
  double h = std::tan(theta / 2.0);
  this->m_viewportHeight = 2.0 * h;
  this->m_viewportWidth = aspectRatio * this->m_viewportHeight;
  this->m_lookFrom = lookFrom;
  this->m_lookAt = lookAt;
  this->m_vup = vup;

  computeCoords();
}

Ray Camera::getRay(double u, double v) const {
  Vec3 direction = lowerLeftCorner + u * horizontal + v * vertical - origin;
  return Ray(origin, direction.normalized());
}

void Camera::setLookFrom(const Vec3 &lookFrom) {
  this->m_lookFrom = lookFrom;
  computeCoords();
}

void Camera::setLookAt(const Vec3 &lookAt) {
  this->m_lookAt = lookAt;
  computeCoords();
}

void Camera::setVup(const Vec3 &vup) {
  this->m_vup = vup;
  computeCoords();
}