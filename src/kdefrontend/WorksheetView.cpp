/***************************************************************************
    File                 : WorksheetView.cpp
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Stefan Gerlach, Alexander Semke
    Email (use @ for *)  : stefan.gerlach*uni-konstanz.de, alexander.semke*web.de
    Description          : provides a view for a worksheet object

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
 #include "Worksheet.h"
#include "WorksheetView.h"
#include "plots/Plot.h"
#include <KDebug>

WorksheetView::WorksheetView(QWidget* parent, Worksheet* w)
	:QWidget(parent){

// 	KConfigGroup conf(KSharedConfig::openConfig(),"Worksheet");
// 	// TODO : hardcoded :-(
// 	setMinimumSize(300,200);
// 	int w = conf.readEntry("Width",600);
// 	int h = conf.readEntry("Height",400);
// 	resize(w,h);
	worksheet=w;
	setMinimumSize(300,200);
	resize(600,400);

	QPixmap pm(20, 20);
	QPainter pmp(&pm);
	pmp.fillRect(0, 0, 10, 10, Qt::lightGray);
	pmp.fillRect(10, 10, 10, 10, Qt::lightGray);
	pmp.fillRect(0, 10, 10, 10, Qt::darkGray);
	pmp.fillRect(10, 0, 10, 10, Qt::darkGray);
	pmp.end();
	QPalette pal = palette();
	pal.setBrush(backgroundRole(), QBrush(pm));
	setAutoFillBackground(true);
	setPalette(pal);
}


WorksheetView::~WorksheetView(){

}

void WorksheetView::paintEvent(QPaintEvent *) {
	this->draw();
}

void WorksheetView::draw() {
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);
	int width=this->width();
	int height=this->height();

	Q_ASSERT(worksheet);
	QList<Plot*>* listPlots=worksheet->listPlots();
	for (int i=0; i<listPlots->size(); i++) {
 		listPlots->at(i)->draw(&painter, width, height);
		kDebug()<<"Plot "<<i<<" drawn."<<endl;
	}
}