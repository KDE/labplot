#ifndef VECTOR3D_H
#define VECTOR3D_H

#include <array>

// QVector3D uses only floats not doubles
// Especially for datetimes this is critical
class Vector3D {
public:
	Vector3D(double x = 0, double y = 0, double z = 0) {
		v[0] = x;
		v[1] = y;
		v[2] = z;
	}
	double x() const {
		return v[0];
	}
	double y() const {
		return v[1];
	}
	double z() const {
		return v[2];
	}

	void setX(double x) {
		v[0] = x;
	}
	void setY(double y) {
		v[1] = y;
	}
	void setZ(double z) {
		v[2] = z;
	}

	inline const Vector3D operator-(const Vector3D& other) const {
		return Vector3D(v[0] - other.v[0], v[1] - other.v[1], v[2] - other.v[2]);
	}

private:
	std::array<double, 3> v;
};

#endif // VECTOR3D_H
