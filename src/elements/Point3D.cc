#include "Point3D.h"

Point3D::Point3D(double a, double b, double c):Point(a,b){
	m_z = c;
}

void Point3D::setPoint(const double a, const double b, const double c){
	m_x = a;
	m_y = b;
	m_z = c;
}
