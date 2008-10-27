/***************************************************************************
    File                 : Worksheet.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Stefan Gerlach
    Email (use @ for *)  : stefan.gerlach*uni-konstanz.de
    Description          : worksheet class
                           
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/
#ifndef WORKSHEET_H
#define WORKSHEET_H

#include <QtGui>
#include <QtXml>

#include "plots/Plot.h"
#include "elements/sheettype.h"

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
