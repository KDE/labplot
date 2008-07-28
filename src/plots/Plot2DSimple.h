#ifndef PLOT2DSIMPLE_H
#define PLOT2DSIMPLE_H

#include "Plot2D.h"

//! simple 2D Plot	(line, error)
class Plot2DSimple:public Plot2D {
public:
	Plot2DSimple();
//	QStringList Info();
	void calculateXY(Point d,double *x, double *y, int w, int h);
	void drawFill(QPainter *p, int w, int h);
	void drawCurves(QPainter *p, const int w, const int h);

};

#endif // PLOT2DSIMPLE_H
