#ifndef VEC3_H
#define VEC3_H

class Vec3 {
public:
  double x, y, z;

  Vec3();
  Vec3(double x, double y, double z);

  // Vector operations
  Vec3 operator+(const Vec3 &v) const;
  Vec3 operator-(const Vec3 &v) const;
  Vec3 operator*(double t) const;
  Vec3 operator/(double t) const;

  Vec3 &operator+=(const Vec3 &v);
  Vec3 &operator*=(double t);
  Vec3 &operator/=(double t);

  // Dot product
  double dot(const Vec3 &v) const;

  // Cross product
  Vec3 cross(const Vec3 &v) const;

  // Length operations
  double length() const;
  double lengthSquared() const;

  // Normalization
  Vec3 normalized() const;
  void normalize();
};

// Helper function for scalar * vector
Vec3 operator*(double t, const Vec3 &v);

#endif // VEC3_H
