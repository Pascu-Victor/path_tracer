#ifndef QUATERNION_H
#define QUATERNION_H

#include "Vec3.h"

/**
 * @class Quaternion
 * @brief Represents a quaternion for rotation operations
 *
 * A quaternion is represented as q = w + xi + yj + zk, where w is the scalar
 * part and (x, y, z) is the vector part. This class provides operations for
 * creating, manipulating, and applying quaternion rotations to 3D vectors.
 */
class Quaternion {
public:
  double w, x, y, z; // w is the scalar part, (x,y,z) is the vector part

  /**
   * @brief Default constructor - creates identity quaternion (1, 0, 0, 0)
   */
  Quaternion();

  /**
   * @brief Constructor from components
   * @param w Scalar part
   * @param x X component of vector part
   * @param y Y component of vector part
   * @param z Z component of vector part
   */
  Quaternion(double w, double x, double y, double z);

  /**
   * @brief Create a quaternion from axis-angle representation
   * @param axis Rotation axis (should be normalized)
   * @param angleRadians Rotation angle in radians
   * @return Quaternion representing the rotation
   */
  static Quaternion fromAxisAngle(const Vec3 &axis, double angleRadians);

  /**
   * @brief Create a quaternion from Euler angles (ZYX convention)
   * @param roll Rotation around X axis in radians
   * @param pitch Rotation around Y axis in radians
   * @param yaw Rotation around Z axis in radians
   * @return Quaternion representing the combined rotation
   */
  static Quaternion fromEulerAngles(double roll, double pitch, double yaw);

  /**
   * @brief Create a quaternion from a rotation matrix
   * @param m00 Matrix element [0][0]
   * @param m01 Matrix element [0][1]
   * @param m02 Matrix element [0][2]
   * @param m10 Matrix element [1][0]
   * @param m11 Matrix element [1][1]
   * @param m12 Matrix element [1][2]
   * @param m20 Matrix element [2][0]
   * @param m21 Matrix element [2][1]
   * @param m22 Matrix element [2][2]
   * @return Quaternion representing the rotation
   */
  static Quaternion fromRotationMatrix(double m00, double m01, double m02,
                                       double m10, double m11, double m12,
                                       double m20, double m21, double m22);

  // Quaternion operations
  /**
   * @brief Quaternion addition
   */
  Quaternion operator+(const Quaternion &q) const;

  /**
   * @brief Quaternion subtraction
   */
  Quaternion operator-(const Quaternion &q) const;

  /**
   * @brief Quaternion multiplication (Hamilton product)
   */
  Quaternion operator*(const Quaternion &q) const;

  /**
   * @brief Scalar multiplication
   */
  Quaternion operator*(double scalar) const;

  /**
   * @brief Scalar division
   */
  Quaternion operator/(double scalar) const;

  /**
   * @brief Compound addition
   */
  Quaternion &operator+=(const Quaternion &q);

  /**
   * @brief Compound multiplication
   */
  Quaternion &operator*=(const Quaternion &q);

  /**
   * @brief Compound scalar multiplication
   */
  Quaternion &operator*=(double scalar);

  /**
   * @brief Compute the magnitude (norm) of the quaternion
   */
  double magnitude() const;

  /**
   * @brief Compute the squared magnitude
   */
  double magnitudeSquared() const;

  /**
   * @brief Normalize the quaternion to unit length
   * @return Reference to this quaternion after normalization
   */
  Quaternion &normalize();

  /**
   * @brief Get a normalized copy of this quaternion
   */
  Quaternion normalized() const;

  /**
   * @brief Compute the conjugate of the quaternion (w, -x, -y, -z)
   */
  Quaternion conjugate() const;

  /**
   * @brief Compute the inverse of the quaternion
   */
  Quaternion inverse() const;

  /**
   * @brief Rotate a 3D vector by this quaternion
   * @param v Vector to rotate
   * @return Rotated vector
   */
  Vec3 rotateVector(const Vec3 &v) const;

  /**
   * @brief Linear interpolation between two quaternions
   * @param q1 Start quaternion
   * @param q2 End quaternion
   * @param t Interpolation parameter (0 to 1)
   * @return Interpolated quaternion
   */
  static Quaternion lerp(const Quaternion &q1, const Quaternion &q2, double t);

  /**
   * @brief Spherical linear interpolation (slerp) between two quaternions
   * @param q1 Start quaternion
   * @param q2 End quaternion
   * @param t Interpolation parameter (0 to 1)
   * @return Interpolated quaternion
   */
  static Quaternion slerp(const Quaternion &q1, const Quaternion &q2, double t);

  /**
   * @brief Get the rotation axis from this quaternion
   * @return Normalized rotation axis
   */
  Vec3 getAxis() const;

  /**
   * @brief Get the rotation angle from this quaternion
   * @return Rotation angle in radians (0 to 2π)
   */
  double getAngle() const;

  /**
   * @brief Convert quaternion to Euler angles (ZYX convention)
   * @param roll Output: rotation around X axis in radians
   * @param pitch Output: rotation around Y axis in radians
   * @param yaw Output: rotation around Z axis in radians
   */
  void toEulerAngles(double &roll, double &pitch, double &yaw) const;

  /**
   * @brief Dot product of two quaternions
   */
  static double dot(const Quaternion &q1, const Quaternion &q2);
};

// Helper function for scalar * quaternion
Quaternion operator*(double scalar, const Quaternion &q);

#endif // QUATERNION_H
