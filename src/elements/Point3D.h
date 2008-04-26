#ifndef POINT3D_H
#define POINT3D_H

#include "Point.h"

class Point3D : public Point{
public:
	Point3D( double a=0, double b=0, double c=0);
	void setPoint(const double, const double, const double);
	ACCESS(double, z, Z);

protected:
	double m_z;
};

#endif //POINT3D_H
