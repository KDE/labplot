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

#include "backend/core/AbstractColumn.h"
#include "backend/core/AbstractPart.h"
#include "backend/datasources/filters/AbstractFileFilter.h"

#ifdef SDK
#include "labplot_export.h"
class LABPLOT_EXPORT AbstractDataSource : public AbstractPart {
#else
class AbstractDataSource : public AbstractPart {
#endif
	Q_OBJECT

public:
	AbstractDataSource(const QString& name, AspectType type);
	~AbstractDataSource() override = default;

	void clear();
	virtual int prepareImport(std::vector<void*>& dataContainer,
							  AbstractFileFilter::ImportMode,
							  int actualRows,
							  int actualCols,
							  const QStringList& colNameList,
							  const QVector<AbstractColumn::ColumnMode>&,
							  bool& ok,
							  bool initializeDataContainer = true) = 0;
	virtual void finalizeImport(size_t columnOffset = 0,
								size_t startColumn = 0,
								size_t endColumn = 0,
								const QString& dateTimeFormat = QString(),
								AbstractFileFilter::ImportMode importMode = AbstractFileFilter::ImportMode::Replace) = 0;
};

#endif // ABSTRACTDATASOURCE_H
