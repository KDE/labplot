#ifndef WORKSHEET_H
#define WORKSHEET_H

#include <QWidget>
#include <QPainter>
#include <QList>
#include <QDomElement>

#include "Plot.h"
#include "Set.h"
#include "sheettype.h"
#include "plottype.h"

class MainWin;

class Worksheet: public QWidget
{
	Q_OBJECT
public:
	Worksheet(MainWin *m);
	~Worksheet();
	SheetType sheetType() const { return type; }
	int plotCount() const { return plot.count(); }
	void print(QString file=0);
	void addSet(Set *set, PlotType ptype);
	Plot *getActivePlot() const { return plot[api]; }
	QDomElement save(QDomDocument doc);
	void open(QDomNode node);
private:
	MainWin *mw;
	SheetType type;			// needed for mw->active{Work,Spread}sheet()
	QList<Plot *> plot;		//!< list of plots
	int api;			//!< active plot index
	void paintEvent(QPaintEvent *);
	void setTitle(QString title);
	void setupPrinter(QPrinter *pr, QString fn);
	void draw(QPainter *p, int w, int h);
	void addPlot(PlotType ptype);	//!< add plot of type ptype
};

#endif //WORKSHEET
