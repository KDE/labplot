/*
File                 : NgspiceRawNgspiceRawBinaryFilterPrivate.h
Project              : LabPlot
Description          : Ngspice RAW Binary filter
--------------------------------------------------------------------
SPDX-FileCopyrightText: 2018 Alexander Semke (alexander.semke@web.de)

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef NGSPICERAWBINARYFILTERPRIVATE_H
#define NGSPICERAWBINARYFILTERPRIVATE_H

class AbstractDataSource;

class NgspiceRawBinaryFilterPrivate {

public:
	explicit NgspiceRawBinaryFilterPrivate(NgspiceRawBinaryFilter*);

	void readDataFromFile(const QString& fileName, AbstractDataSource* = nullptr,
			AbstractFileFilter::ImportMode = AbstractFileFilter::ImportMode::Replace);
	void write(const QString& fileName, AbstractDataSource*);
	QVector<QStringList> preview(const QString& fileName, int lines);

	const NgspiceRawBinaryFilter* q;

	QStringList vectorNames;
	QVector<AbstractColumn::ColumnMode> columnModes;
	int startRow{1};
	int endRow{-1};

private:
	const static int BYTE_SIZE = 8;

	std::vector<void*> m_dataContainer; // pointers to the actual data containers
};

#endif
