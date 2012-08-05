/***************************************************************************
    File                 : PlotAreaDock.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2011 Alexander Semke
    Email (use @ for *)  : alexander.semke*web.de
    Description          : widget for plot area properties
                           
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

#ifndef PLOTAREADOCK_H
#define PLOTAREADOCK_H

#include <QList>
#include "ui_plotareadock.h"
class PlotArea;

class PlotAreaDock: public QWidget{
	Q_OBJECT
	
public:
	PlotAreaDock(QWidget *parent);
	void setPlotAreas(QList<PlotArea*>);
	
private:
	Ui::PlotAreaDock ui;
	QList<PlotArea*> m_plotAreaList;
	bool m_initializing;

private slots:
	void init();
	void retranslateUi();
  
	//"General"-tab
	void nameChanged();
	void commentChanged();
	void opacityChanged(int);
	
	//"Background"-tab
  	void backgroundTypeChanged(int);
	void backgroundColorStyleChanged(int);
	void backgroundImageStyleChanged(int);
	void backgroundFirstColorChanged(const QColor&);
	void backgroundSecondColorChanged(const QColor&);
	void selectFile();
	void fileNameChanged();
	
	//"Border"-tab
  	void borderStyleChanged(int);
	void borderColorChanged(const QColor&);
	void borderWidthChanged(int);
};

#endif
