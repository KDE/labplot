//LabPlot : Point.h

#ifndef POINT_H
#define POINT_H

class Point
{
public:
	Point(double a=0, double b=0);
	void setPoint(double, double);
	double X() { return x; }
	void setX(double s) { x=s; }
	double Y() { return y; }
	void setY(double s) { y=s; }
	void setMasked(bool m=true) { masked = m; }
	bool Masked() { return masked; }
protected:
	double x,y;		
	bool masked;
};

#endif //POINT_H
