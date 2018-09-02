/***************************************************************************
File		: AbstractDataSource.h
Project		: LabPlot
Description 	: Interface for data sources
--------------------------------------------------------------------
Copyright	: (C) 2009-2017 Alexander Semke (alexander.semke@web.de)
Copyright	: (C) 2015 Stefan Gerlach (stefan.gerlach@uni.kn)
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
#ifndef ABSTRACTDATASOURCE_H
#define ABSTRACTDATASOURCE_H

#include "backend/core/AbstractPart.h"
#include "backend/datasources/filters/AbstractFileFilter.h"
#include "backend/core/AbstractColumn.h"
#include <QVector>

class QStringList;

class AbstractDataSource : public AbstractPart {
	Q_OBJECT

public:
	explicit AbstractDataSource(const QString& name);
	~AbstractDataSource() override = default;

	void clear();
	virtual int prepareImport(QVector<void*>& dataContainer, AbstractFileFilter::ImportMode, int actualRows, int actualCols,
			QStringList colNameList = QStringList(), QVector<AbstractColumn::ColumnMode> = QVector<AbstractColumn::ColumnMode>()) = 0;
	virtual void finalizeImport(int columnOffset = 0, int startColumn = 0, int endColumn = 0, int numRows = 0,
			const QString& dateTimeFormat = QString(), AbstractFileFilter::ImportMode importMode = AbstractFileFilter::Replace) = 0;
};

#endif // ABSTRACTDATASOURCE_H
