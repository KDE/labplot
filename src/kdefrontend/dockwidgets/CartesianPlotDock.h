/***************************************************************************
    File                 : CartesianPlotDock.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2011 Alexander Semke
    Email (use @ for *)  : alexander.semke*web.de
    Description          : widget for cartesian plot properties
                           
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

#ifndef CARTESIANPLOTDOCK_H
#define CARTESIANPLOTDOCK_H

#include <QList>
#include "ui_cartesianplotdock.h"

class CartesianPlot;

class CartesianPlotDock: public QWidget{
	Q_OBJECT
	
public:
	CartesianPlotDock(QWidget *parent);
	void setPlots(QList<CartesianPlot*>);
	
private:
	Ui::CartesianPlotDock ui;
	QList<CartesianPlot*> m_plotList;
	bool m_initializing;
	
	void load(const KConfig&);
	void save(const KConfig&);

private slots:
	void init();
	void retranslateUi();
  
	//"General"-tab
	void nameChanged();
	void commentChanged();
	void visibilityChanged(int);

	//"Coordinate system"-tab
	void toggleXBreak(int);
	void toggleYBreak(int);
	
	//"Plot area"-tab
  	void backgroundTypeChanged(int);
	void backgroundColorStyleChanged(int);
	void backgroundImageStyleChanged(int);
	void backgroundFirstColorChanged(const QColor&);
	void backgroundSecondColorChanged(const QColor&);
	void selectFile();
	void fileNameChanged();
	void backgroundOpacityChanged(int);
  	void borderStyleChanged(int);
	void borderColorChanged(const QColor&);
	void borderWidthChanged(double value);
	void borderOpacityChanged(int);

	void loadSettings();
	void saveSettings();
	void saveDefaults();
};

#endif
