//LabPlot : Point.cc

#include "Point.h"

Point::Point(double a, double b)
{
	x = a;
	y = b;
	masked = false;
}

void Point::setPoint(double a, double b)
{
	x = a;
	y = b;
}
