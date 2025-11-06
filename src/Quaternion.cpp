#include "Quaternion.h"
#include <cmath>
#include <stdexcept>

// Default constructor - identity quaternion
Quaternion::Quaternion() : w(1.0), x(0.0), y(0.0), z(0.0) {}

// Constructor from components
Quaternion::Quaternion(double w, double x, double y, double z)
    : w(w), x(x), y(y), z(z) {}

// Create from axis-angle representation
Quaternion Quaternion::fromAxisAngle(const Vec3 &axis, double angleRadians) {
  Vec3 normalizedAxis = axis.normalized();
  double halfAngle = angleRadians * 0.5;
  double sinHalfAngle = std::sin(halfAngle);
  double cosHalfAngle = std::cos(halfAngle);

  return Quaternion(cosHalfAngle, normalizedAxis.x * sinHalfAngle,
                    normalizedAxis.y * sinHalfAngle,
                    normalizedAxis.z * sinHalfAngle);
}

// Create from Euler angles (ZYX convention)
Quaternion Quaternion::fromEulerAngles(double roll, double pitch, double yaw) {
  double cy = std::cos(yaw * 0.5);
  double sy = std::sin(yaw * 0.5);
  double cp = std::cos(pitch * 0.5);
  double sp = std::sin(pitch * 0.5);
  double cr = std::cos(roll * 0.5);
  double sr = std::sin(roll * 0.5);

  Quaternion q;
  q.w = cr * cp * cy + sr * sp * sy;
  q.x = sr * cp * cy - cr * sp * sy;
  q.y = cr * sp * cy + sr * cp * sy;
  q.z = cr * cp * sy - sr * sp * cy;

  return q;
}

// Create from rotation matrix
Quaternion Quaternion::fromRotationMatrix(double m00, double m01, double m02,
                                          double m10, double m11, double m12,
                                          double m20, double m21, double m22) {
  double trace = m00 + m11 + m22;
  Quaternion q;

  if (trace > 0.0) {
    double s = 0.5 / std::sqrt(trace + 1.0);
    q.w = 0.25 / s;
    q.x = (m21 - m12) * s;
    q.y = (m02 - m20) * s;
    q.z = (m10 - m01) * s;
  } else if (m00 > m11 && m00 > m22) {
    double s = 2.0 * std::sqrt(1.0 + m00 - m11 - m22);
    q.w = (m21 - m12) / s;
    q.x = 0.25 * s;
    q.y = (m01 + m10) / s;
    q.z = (m02 + m20) / s;
  } else if (m11 > m22) {
    double s = 2.0 * std::sqrt(1.0 + m11 - m00 - m22);
    q.w = (m02 - m20) / s;
    q.x = (m01 + m10) / s;
    q.y = 0.25 * s;
    q.z = (m12 + m21) / s;
  } else {
    double s = 2.0 * std::sqrt(1.0 + m22 - m00 - m11);
    q.w = (m10 - m01) / s;
    q.x = (m02 + m20) / s;
    q.y = (m12 + m21) / s;
    q.z = 0.25 * s;
  }

  return q;
}

// Quaternion addition
Quaternion Quaternion::operator+(const Quaternion &q) const {
  return Quaternion(w + q.w, x + q.x, y + q.y, z + q.z);
}

// Quaternion subtraction
Quaternion Quaternion::operator-(const Quaternion &q) const {
  return Quaternion(w - q.w, x - q.x, y - q.y, z - q.z);
}

// Quaternion multiplication (Hamilton product)
Quaternion Quaternion::operator*(const Quaternion &q) const {
  return Quaternion(
      w * q.w - x * q.x - y * q.y - z * q.z,
      w * q.x + x * q.w + y * q.z - z * q.y,
      w * q.y - x * q.z + y * q.w + z * q.x,
      w * q.z + x * q.y - y * q.x + z * q.w);
}

// Scalar multiplication
Quaternion Quaternion::operator*(double scalar) const {
  return Quaternion(w * scalar, x * scalar, y * scalar, z * scalar);
}

// Scalar division
Quaternion Quaternion::operator/(double scalar) const {
  if (scalar == 0.0) {
    throw std::invalid_argument("Division by zero");
  }
  return Quaternion(w / scalar, x / scalar, y / scalar, z / scalar);
}

// Compound addition
Quaternion &Quaternion::operator+=(const Quaternion &q) {
  w += q.w;
  x += q.x;
  y += q.y;
  z += q.z;
  return *this;
}

// Compound multiplication
Quaternion &Quaternion::operator*=(const Quaternion &q) {
  *this = *this * q;
  return *this;
}

// Compound scalar multiplication
Quaternion &Quaternion::operator*=(double scalar) {
  w *= scalar;
  x *= scalar;
  y *= scalar;
  z *= scalar;
  return *this;
}

// Magnitude
double Quaternion::magnitude() const {
  return std::sqrt(w * w + x * x + y * y + z * z);
}

// Squared magnitude
double Quaternion::magnitudeSquared() const {
  return w * w + x * x + y * y + z * z;
}

// Normalize
Quaternion &Quaternion::normalize() {
  double mag = magnitude();
  if (mag > 0.0) {
    w /= mag;
    x /= mag;
    y /= mag;
    z /= mag;
  }
  return *this;
}

// Normalized copy
Quaternion Quaternion::normalized() const {
  Quaternion q = *this;
  q.normalize();
  return q;
}

// Conjugate
Quaternion Quaternion::conjugate() const {
  return Quaternion(w, -x, -y, -z);
}

// Inverse
Quaternion Quaternion::inverse() const {
  double magSq = magnitudeSquared();
  if (magSq == 0.0) {
    throw std::invalid_argument("Cannot invert a zero quaternion");
  }
  return conjugate() / magSq;
}

// Rotate a vector
Vec3 Quaternion::rotateVector(const Vec3 &v) const {
  // Convert vector to pure quaternion
  Quaternion p(0.0, v.x, v.y, v.z);

  // Perform rotation: q * p * q^-1
  Quaternion result = (*this) * p * inverse();

  return Vec3(result.x, result.y, result.z);
}

// Linear interpolation
Quaternion Quaternion::lerp(const Quaternion &q1, const Quaternion &q2,
                             double t) {
  if (t < 0.0)
    t = 0.0;
  if (t > 1.0)
    t = 1.0;

  double dot_product = dot(q1, q2);
  Quaternion q2_adjusted = q2;

  // Take the shortest path
  if (dot_product < 0.0) {
    q2_adjusted.w = -q2_adjusted.w;
    q2_adjusted.x = -q2_adjusted.x;
    q2_adjusted.y = -q2_adjusted.y;
    q2_adjusted.z = -q2_adjusted.z;
  }

  Quaternion result = q1 * (1.0 - t) + q2_adjusted * t;
  return result.normalized();
}

// Spherical linear interpolation (slerp)
Quaternion Quaternion::slerp(const Quaternion &q1, const Quaternion &q2,
                              double t) {
  if (t < 0.0)
    t = 0.0;
  if (t > 1.0)
    t = 1.0;

  double dot_product = dot(q1, q2);
  Quaternion q2_adjusted = q2;

  // Take the shortest path
  if (dot_product < 0.0) {
    q2_adjusted.w = -q2_adjusted.w;
    q2_adjusted.x = -q2_adjusted.x;
    q2_adjusted.y = -q2_adjusted.y;
    q2_adjusted.z = -q2_adjusted.z;
    dot_product = -dot_product;
  }

  // Clamp dot product to avoid numerical issues
  if (dot_product > 0.9995) {
    // Quaternions are very close, use linear interpolation
    return lerp(q1, q2_adjusted, t);
  }

  // Calculate the angle between quaternions
  double theta = std::acos(dot_product);
  double sin_theta = std::sin(theta);

  double w1 = std::sin((1.0 - t) * theta) / sin_theta;
  double w2 = std::sin(t * theta) / sin_theta;

  Quaternion result = q1 * w1 + q2_adjusted * w2;
  return result.normalized();
}

// Get axis
Vec3 Quaternion::getAxis() const {
  double sin_half_theta = std::sqrt(x * x + y * y + z * z);

  if (sin_half_theta < 1e-10) {
    // Angle is near zero, return arbitrary axis
    return Vec3(1.0, 0.0, 0.0);
  }

  return Vec3(x, y, z) / sin_half_theta;
}

// Get angle
double Quaternion::getAngle() const {
  double mag = magnitude();
  if (mag == 0.0)
    return 0.0;

  double w_normalized = w / mag;
  // Clamp to [-1, 1] to avoid numerical issues with acos
  w_normalized = w_normalized < -1.0 ? -1.0 : (w_normalized > 1.0 ? 1.0 : w_normalized);
  return 2.0 * std::acos(w_normalized);
}

// Convert to Euler angles (ZYX convention)
void Quaternion::toEulerAngles(double &roll, double &pitch, double &yaw) const {
  // Roll (rotation about X axis)
  double sinr_cosp = 2.0 * (w * x + y * z);
  double cosr_cosp = 1.0 - 2.0 * (x * x + y * y);
  roll = std::atan2(sinr_cosp, cosr_cosp);

  // Pitch (rotation about Y axis)
  double sinp = 2.0 * (w * y - z * x);
  if (std::abs(sinp) >= 1.0) {
    pitch = std::copysign(M_PI / 2.0, sinp); // Use 90 degrees
  } else {
    pitch = std::asin(sinp);
  }

  // Yaw (rotation about Z axis)
  double siny_cosp = 2.0 * (w * z + x * y);
  double cosy_cosp = 1.0 - 2.0 * (y * y + z * z);
  yaw = std::atan2(siny_cosp, cosy_cosp);
}

// Dot product
double Quaternion::dot(const Quaternion &q1, const Quaternion &q2) {
  return q1.w * q2.w + q1.x * q2.x + q1.y * q2.y + q1.z * q2.z;
}

// Helper function for scalar * quaternion
Quaternion operator*(double scalar, const Quaternion &q) {
  return q * scalar;
}
