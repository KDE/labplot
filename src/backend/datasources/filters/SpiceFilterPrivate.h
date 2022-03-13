/*
    File                 : SpiceFilterPrivate.h
    Project              : LabPlot
    Description          : Private of Spice Filter
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2022 Martin Marmsoler <martin.marmsoler@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SPICEFILTERPRIVATE_H
#define SPICEFILTERPRIVATE_H

#include <QStringList>
#include "AbstractFileFilter.h"

class AbstractDataSource;
class SpiceFileReader;
class SpiceFilter;

class SpiceFilterPrivate {

public:
	explicit SpiceFilterPrivate(SpiceFilter *);

	QVector<QStringList> preview(const QString& fileName, int lines);
	void readDataFromFile(const QString& fileName, AbstractDataSource* = nullptr,
			AbstractFileFilter::ImportMode = AbstractFileFilter::ImportMode::Replace);
	void write(const QString& fileName, AbstractDataSource*);
	void generateVectorNamesColumnModes(const SpiceFileReader& reader);

	const SpiceFilter* q;

	QStringList vectorNames;
	QVector<AbstractColumn::ColumnMode> columnModes;
	int startRow{1};
	int endRow{-1};

private:
	std::vector<void*> m_dataContainer; // pointers to the actual data containers
};

#endif // SPICEFILTERPRIVATE_H
