/*
File			: AbstractDataSource.cpp
Project			: LabPlot
Description		: Abstract interface for data sources
--------------------------------------------------------------------
SPDX-FileCopyrightText: 2009-2017 Alexander Semke <alexander.semke@web.de>
SPDX-FileCopyrightText: 2015 Stefan Gerlach <stefan.gerlach@uni.kn>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "AbstractDataSource.h"
#include "backend/core/column/Column.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/matrix/Matrix.h"

/*!
\class AbstractDataSource
\brief Interface for the data sources.

\ingroup datasources
*/

AbstractDataSource::AbstractDataSource(const QString& name, AspectType type)
	: AbstractPart(name, type) {
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
