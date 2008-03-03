// LabPlot : LRange.h

#ifndef LRANGE_H
#define LRANGE_H

class LRange
{
public:
	LRange(double min=0, double max=0);
	double Diff() { return max-min; }
	double rMin() { return min; }
	double rMax() { return max; }
	void setMin(double m) { min=m; }
	void setMax(double m) { max=m; }
	void setRange(double rmin=0, double rmax=1) {min=rmin; max=rmax; }
	
private:
	double min,max;
};

#endif //LRANGE_H
