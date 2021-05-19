/***************************************************************************
    File                 : CantorWorksheetDock.h
    Project              : LabPlot
    Description          : widget for CantorWorksheet properties
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

#include "ui_cantorworksheetdock.h"
#include "kdefrontend/dockwidgets/BaseDock.h"

#include <3rdparty/cantor/panelplugin.h>

#include <QPair>
#include <QWidget>

class CantorWorksheet;
class AbstractAspect;

class CantorWorksheetDock : public BaseDock {
	Q_OBJECT

public:
	explicit CantorWorksheetDock(QWidget *parent);
	void setCantorWorksheets(QList<CantorWorksheet*>);

private:
	Ui::CantorWorksheetDock ui;
	QList< CantorWorksheet* > m_cantorworksheetlist;
	CantorWorksheet* m_worksheet{nullptr};
	QList<int> index;

private slots:
	//SLOTs for changes triggered in WorksheetDock
	//"General"-tab
	void evaluateWorksheet();
	void restartBackend();
	void visibilityRequested();

signals:
	void info(const QString&);

};

#endif // CANTORWORKSHEETDOCK_H
