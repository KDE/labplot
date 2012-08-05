/***************************************************************************
    File                 : WorksheetModel.cpp
    Project              : LabPlot/SciDAVis
    Description          : Model for the access to a Worksheet.
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs*gmx.net)
                           (replace * with @ in the email addresses) 
                           
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

#include "worksheet/WorksheetModel.h"
#include "worksheet/Worksheet.h"
//#include "worksheet/WorksheetGraphicsScene.h"

/**
 * \class WorksheetModel
 * \brief Model for the access to Worksheet.
 *
 * This class is just a very thin wrapper around Worksheet to honor the
 * 5 layer paradigm.
 */

WorksheetModel::WorksheetModel(Worksheet * worksheet)
	: m_worksheet(worksheet) {
}

WorksheetModel::~WorksheetModel() {
}

QGraphicsScene *WorksheetModel::scene() const {
	return m_worksheet->scene();
}

