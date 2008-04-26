#include "Point.h"

Point::Point(double x, double y){
	m_x = x;
	m_y = y;
	m_masked = false;
}

void Point::setPoint(const double x, const double y){
	m_x = x;
	m_y = y;
}
