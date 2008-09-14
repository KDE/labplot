#ifndef WORKSHEET_H
#define WORKSHEET_H

#include <QtGui>
#include <QtXml>

#include "plots/Plot.h"
#include "sheettype.h"

class MainWin;
class Set;

class Worksheet: public QWidget{
	Q_OBJECT

public:
	Worksheet(MainWin *m);
	~Worksheet();

	SheetType sheetType() const { return type; }
	int plotCount() const { return listPlots.count(); }
	void print(QString file=0);
	void addSet(Set set, Plot::PlotType ptype);
	Plot* activePlot();
	QDomElement save(QDomDocument doc);
	void open(QDomNode node);

private:
	MainWin *mw;
	SheetType type;			// needed for mw->active{Work,Spread}sheet()
	QList<Plot*> listPlots;		//!< list of plots
	int currentPlotIndex;

	void paintEvent(QPaintEvent *);
	void setTitle(QString title);
	void setupPrinter(QPrinter *pr, QString fn);
	void draw(QPainter *p, int w, int h);
	void addPlot(Plot::PlotType ptype);	//!< add plot of type ptype
};

#endif //WORKSHEET
