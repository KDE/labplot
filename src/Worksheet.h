#ifndef WORKSHEET_H
#define WORKSHEET_H

#include <QWidget>
#include <QPainter>
#include <QList>

#include "Plot.h"
#include "Set.h"
#include "sheettype.h"
#include "plottype.h"

class MainWin;

class Worksheet: public QWidget
{
	Q_OBJECT
public:
	Worksheet(MainWin *mw);
	SheetType sheetType() { return type; }
	int plotCount() { return plot.count(); }
	void print(QString file=0);
	void addSet(Set *set, PlotType ptype);
	Plot *getActivePlot() { return plot[api]; }
private:
	MainWin *mw;
	SheetType type;			// needed for mw->active{Work,Spread}sheet()
	QList<Plot *> plot;		//!< list of plots
	int api;			//!< active plot index
	void paintEvent(QPaintEvent *);
	void setupPrinter(QPrinter *pr, QString fn);
	void draw(QPainter *p, int w, int h);
	void addPlot(PlotType ptype);	//!< add plot of type ptype
};

#endif //WORKSHEET
