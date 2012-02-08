/***************************************************************************
    File                 : WorksheetDock.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Stefan Gerlach
    Email (use @ for *)  : stefan.gerlach*uni-konstanz.de
    Description          : widget for worksheet properties
                           
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

#ifndef WORKSHEETDOCK_H
#define WORKSHEETDOCK_H

#include <QList>
#include "ui_worksheetdock.h"

class Worksheet;

class WorksheetDock: public QWidget{
	Q_OBJECT
	
public:
	WorksheetDock(QWidget *parent);
	void setWorksheets(QList<Worksheet*>);
	
private:
	Ui::WorksheetDock ui;
	QList<Worksheet*> m_worksheetList;
	bool m_initializing;

	void updatePaperSize();

private slots:
	void retranslateUi();
  
	//"General"-tab
	void nameChanged();
	void commentChanged();
	void sizeChanged(int);
	void sizeChanged();
	void orientationChanged(int);
	
	//"Background"-tab
  	void backgroundTypeChanged(int);
	void backgroundColorStyleChanged(int);
	void backgroundImageStyleChanged(int);
	void backgroundFirstColorChanged(const QColor&);
	void backgroundSecondColorChanged(const QColor&);
	void selectFile();
	void fileNameChanged();
	void opacityChanged(int);

	//"Layout"-tab
	void layoutTopMarginChanged(double);
	void layoutBottomMarginChanged(double);
	void layoutRightMarginChanged(double);
	void layoutLeftMarginChanged(double);
	void layoutHorizontalSpacingChanged(double);
	void layoutVerticalSpacingChanged(double);
	void layoutRowCountChanged(int);
	void layoutColumnCountChanged(int);

	void loadConfig(KConfig&);
	void saveConfig(KConfig&);
};

#endif // WORKSHEETDOCK_H
