/***************************************************************************
    File                 : CantorWorksheetView.h
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

#ifndef CANTORWORKSHEETVIEW_H
#define CANTORWORKSHEETVIEW_H

#include <QWidget>
#include <QAction>
#include <QMenu>
#include <QTableView>
#include "backend/cantorWorksheet/CantorWorksheet.h"
#include <cantor/session.h>
#include <KParts/ReadWritePart>

class CantorWorksheetView : public QWidget {
    Q_OBJECT
    
    public:
	CantorWorksheetView(CantorWorksheet* cantorWorksheet);
	
	~CantorWorksheetView();
	
    public slots:
	void createContextMenu(QMenu*) const;

    private:
	CantorWorksheet* m_worksheet;  
	QAction* m_restartBackendAction;
	
	void initActions();
};

#endif // CANTORWORKSHEETVIEW_H
