// LabPlot : Range.h

#ifndef RANGE_H
#define RANGE_H

#include "defs.h"

class Range
{
public:
	Range(double min=0, double max=0);
	void setRange(double rmin=0, double rmax=1) {min=rmin; max=rmax; }
	double Diff() const { return max-min; }
	ACCESS(double, min, Min);
	ACCESS(double, max, Max);
	
private:
	double min, max;
};

#endif //RANGE_H
