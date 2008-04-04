//LabPlot : Point.h

#ifndef POINT_H
#define POINT_H

#include "definitions.h"

class Point
{
public:
	Point(double a=0, double b=0);
	void setPoint(double, double);
	ACCESS(double,x,X);
	ACCESS(double,y,Y);
	ACCESSFLAG(m_masked,Masked);
protected:
	double m_x,m_y;
	bool m_masked;
};

#endif //POINT_H
