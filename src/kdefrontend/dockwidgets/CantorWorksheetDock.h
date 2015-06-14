/***************************************************************************
    File                 : CantorWorksheetDock.h
    Project              : LabPlot
    Description          : Aspect providing a Cantor Worksheets for Multiple backends
    --------------------------------------------------------------------
    Copyright            : (C) 2015 Garvit Khatri (garvitdelhi@gmail.com)

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

#ifndef CANTORWORKSHEETDOCK_H
#define CANTORWORKSHEETDOCK_H

#include <QtWidgets/QWidget>
#include <QPair>
#include <cantor/panelpluginhandler.h>
#include <cantor/panelplugin.h>
#include "ui_cantorworksheetdock.h"

class CantorWorksheet;
class AbstractAspect;

class CantorWorksheetDock : public QWidget {
    Q_OBJECT
    
    public:
	explicit CantorWorksheetDock(QWidget *parent);
	void setCantorWorksheets(QList<CantorWorksheet*>);
	
    private:
	Ui::CantorWorksheetDock ui;
	CantorWorksheet* m_cantorworksheet;
	QList< CantorWorksheet* > m_cantorworksheetlist;
	QList<QPair<QString, QWidget*> > panelsWidgets;
	QList<int> index;
	QWidget* w = NULL;
	
    signals:
	void info(const QString&);
	
};

#endif // CANTORWORKSHEETDOCK_H
