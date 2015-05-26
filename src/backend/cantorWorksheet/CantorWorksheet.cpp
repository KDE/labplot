/***************************************************************************
    File                 : CantorWorksheet.cpp
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

#include "CantorWorksheet.h"
#include "commonfrontend/cantorWorksheet/CantorWorksheetView.h"

#include <QDebug>
#include <QPushButton>

CantorWorksheet::CantorWorksheet(AbstractScriptingEngine *engine, const QString &name)
		: AbstractPart(name), scripted(engine){
    addChild(new QPushButton("Hey"));
}

QWidget* CantorWorksheet::view() const {
    if (!m_view) {
	m_view = new CantorWorksheetView(const_cast<CantorWorksheet *>(this));
    }
    return m_view;
}
//! Return a new context menu.
/**
 * The caller takes ownership of the menu.
 */
QMenu *CantorWorksheet::createContextMenu() {
    QMenu *menu = AbstractPart::createContextMenu();
    Q_ASSERT(menu);
    emit requestProjectContextMenu(menu);
    return menu;
}
