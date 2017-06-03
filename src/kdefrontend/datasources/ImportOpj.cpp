/***************************************************************************
    File                 : ImportOpj.cpp
    Project              : LabPlot
    Description          : Import Origin project
    --------------------------------------------------------------------
    Copyright            : (C) 2017 Stefan Gerlach (stefan.gerlach@uni.kn)

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

#include "ImportOpj.h"
#include "kdefrontend/MainWin.h"
#include "backend/lib/macros.h"
#include "backend/spreadsheet/Spreadsheet.h"

#include <liborigin/OriginFile.h>

#include <QProgressBar>
#include <QStatusBar>

/*!
    \class ImportOpj
    \brief Importing an Origin project.

	\ingroup kdefrontend
 */
ImportOpj::ImportOpj(MainWin* parent, const QString& filename) : mw(parent) {
	DEBUG("Opj import started ...");

	OriginFile opj((const char *)filename.toLocal8Bit());
	int status = opj.parse();
	DEBUG("Parsing done. Starting conversion ...");

	importTables(opj);
        //TODO  
//      importGraphs(opj);
//      importNotes(opj);
//      if(filename.endsWith(".opj", Qt::CaseInsensitive))
//              createProjectTree(opj);

}

ImportOpj::~ImportOpj() {
}

bool ImportOpj::importTables(const OriginFile &opj) {
        for(unsigned int s = 0; s < opj.spreadCount(); ++s) {
                Origin::SpreadSheet spread = opj.spread(s);
                int columnCount = spread.columns.size();
                if(!columnCount) //remove tables without cols
                        continue;
                importSpreadsheet(opj, spread);
        }

	return true;
}

bool ImportOpj::importSpreadsheet(const OriginFile &opj, const Origin::SpreadSheet &spread) {
	int cols = spread.columns.size();
	int maxrows = spread.maxRows;
	if(!cols) // do not create spreadsheets without columns
		return false;

	Spreadsheet* spreadsheet = new Spreadsheet(0, spread.name.c_str());
	spreadsheet->setRowCount(maxrows);
	spreadsheet->setColumnCount(cols);

	//TODO

//	if (spread.hidden || spread.loose)
//		mw->hideWindow(table);
	mw->addAspectToProject(spreadsheet);

	return true;
}

