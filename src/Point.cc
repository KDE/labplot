//LabPlot : Point.cc

#include "Point.h"

Point::Point(double x, double y)
{
	m_x = x;
	m_y = y;
	m_masked = false;
}

void Point::setPoint(double x, double y)
{
	m_x = x;
	m_y = y;
}
