//LabPlot : Plot.h

#ifndef PLOT_H
#define PLOT_H

#include <QBrush>
#include "Point.h"
#include "LRange.h"
#include "Label.h"
#include "plottype.h"

class Set;
class Axis;
class Style;
class Symbol;

class Plot {
public:
	Plot();
	virtual ~Plot() {}
	PlotType Type() { return type; }
	Label *Title() { return title; }
	void addSet(Set *s) { set.append(s); }
	virtual void setRange(LRange *,int i) = 0;
	LRange* Range(int i) { return &range[i];}
	virtual void setActRange(LRange *,int i) = 0;
	LRange* ActRange(int i) { return &actrange[i];}
	virtual void setRanges(LRange *) = 0;
	LRange* Ranges() { return range;}
	virtual void setActRanges(LRange *) = 0;
	LRange* ActRanges() { return actrange;}
	virtual void draw(QPainter *p, int w, int h) = 0;
	QString TicLabel(int atlf, int prec, QString dtf, double value);
	void resetRanges();
	void drawStyle(QPainter *p, Style *style, Symbol *symbol, QVector<QPoint> pa, int xmin, int xmax, int ymin, int ymax);
protected:
	PlotType type;
	QList<Set *> set;
	QList<Axis *> axis;
	Point p1, p2;				// plotting area	(0..1)
	Label *title;
	LRange range[3];			// set ranges	(x,y,z)
	LRange actrange[3];			// actual plotting range (x,y,z)
	QBrush background, areabackground;
	Point position, size;
	bool transparent;
};

#endif // PLOT_H
