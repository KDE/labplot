/***************************************************************************
    File                 : WorksheetDock.cpp
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2010 by Alexander Semke
    Email (use @ for *)  : alexander.semke*web.de
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

#include "WorksheetDock.h"
#include "worksheet/Worksheet.h"

/*!
  \class GuiObserver
  \brief  Provides a widget for editing the properties of the worksheets currently selected in the project explorer.

  \ingroup kdefrontend
*/

WorksheetDock::WorksheetDock(QWidget *parent): QWidget(parent){
	ui.setupUi(this);
}

void WorksheetDock::setWorksheets(QList<Worksheet*> list){

}
