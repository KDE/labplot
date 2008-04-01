//LabPlot : Point.h

#ifndef POINT_H
#define POINT_H

#include "defs.h"

class Point
{
public:
	Point(double a=0, double b=0);
	void setPoint(double, double);
	ACCESS(double,x, X);
	ACCESS(double,y,Y);
	ACCESSFLAG(masked,Masked);
protected:
	double x,y;
	bool masked;
};

#endif //POINT_H
