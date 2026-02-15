/*
	File			: AbstractDataSource.cpp
	Project			: LabPlot
	Description		: Abstract interface for data sources
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2009-2023 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2015 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "AbstractDataSource.h"
#include "backend/core/column/Column.h"
#include "backend/matrix/Matrix.h"
#include "backend/spreadsheet/Spreadsheet.h"

/*!
\class AbstractDataSource
\brief Interface for the data sources.

\ingroup datasources
*/

AbstractDataSource::AbstractDataSource(const QString& name, AspectType type)
	: AbstractPart(name, type) {
}

void AbstractDataSource::clear() {
	const auto& columns = children<Column>();
	for (auto* column : columns) {
		column->setUndoAware(false);
		column->setSuppressDataChangedSignal(true);
		column->clear();
		column->setUndoAware(true);
		column->setSuppressDataChangedSignal(false);
		column->setDataChanged();
	}
}
