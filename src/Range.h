// LabPlot : Range.h

#ifndef RANGE_H
#define RANGE_H

#include "definitions.h"

class Range
{
public:
	Range(double min=0, double max=0);
	void setRange(double min=0, double max=1) {m_min=min; m_max=max; }
	double Diff() const { return m_max - m_min; }
	ACCESS(double, min, Min);
	ACCESS(double, max, Max);
	
private:
	double m_min, m_max;
};

#endif //RANGE_H
