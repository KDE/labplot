/*
    File		: AbstractDataSource.h
    Project		: LabPlot
    Description 	: Interface for data sources
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2009-2017 Alexander Semke <alexander.semke@web.de>
    SPDX-FileCopyrightText: 2015 Stefan Gerlach <stefan.gerlach@uni.kn>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
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
	AbstractDataSource(const QString& name, AspectType type);
	~AbstractDataSource() override = default;

	void clear();
	virtual int prepareImport(std::vector<void*>& dataContainer, AbstractFileFilter::ImportMode, int actualRows, int actualCols,
			QStringList colNameList = QStringList(), QVector<AbstractColumn::ColumnMode> = QVector<AbstractColumn::ColumnMode>()) = 0;
	virtual void finalizeImport(size_t columnOffset = 0, size_t startColumn = 0, size_t endColumn = 0,
			const QString& dateTimeFormat = QString(), AbstractFileFilter::ImportMode importMode = AbstractFileFilter::ImportMode::Replace) = 0;
};

#endif // ABSTRACTDATASOURCE_H
