/***************************************************************************
File			: AbstractDataSource.cpp
Project			: LabPlot
Description		: Abstract interface for data sources
--------------------------------------------------------------------
Copyright		: (C) 2009-2017 Alexander Semke (alexander.semke@web.de)
Copyright		: (C) 2015 Stefan Gerlach (stefan.gerlach@uni.kn)
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
#include "AbstractDataSource.h"
#include "backend/core/column/Column.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/matrix/Matrix.h"

/*!
\class AbstractDataSource
\brief Interface for the data sources.

\ingroup datasources
*/

AbstractDataSource::AbstractDataSource(const QString& name) : AbstractPart(name) {
}

void AbstractDataSource::clear() {
	int columns = childCount<Column>();
	for (int i = 0; i < columns; ++i) {
		child<Column>(i)->setUndoAware(false);
		child<Column>(i)->setSuppressDataChangedSignal(true);
		child<Column>(i)->clear();
		child<Column>(i)->setUndoAware(true);
		child<Column>(i)->setSuppressDataChangedSignal(false);
		child<Column>(i)->setChanged();
	}
}
