/*
File                 : NgspiceRawNgspiceRawAsciiFilterPrivate.h
Project              : LabPlot
Description          : Ngspice RAW ASCII filter
--------------------------------------------------------------------
SPDX-FileCopyrightText: 2018 Alexander Semke (alexander.semke@web.de)
*/

/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#ifndef NGSPICERAWASCIIFILTERPRIVATE_H
#define NGSPICERAWASCIIFILTERPRIVATE_H

class AbstractDataSource;

class NgspiceRawAsciiFilterPrivate {

public:
	explicit NgspiceRawAsciiFilterPrivate(NgspiceRawAsciiFilter*);

	QVector<QStringList> preview(const QString& fileName, int lines);
	void readDataFromFile(const QString& fileName, AbstractDataSource* = nullptr,
			AbstractFileFilter::ImportMode = AbstractFileFilter::ImportMode::Replace);
	void write(const QString& fileName, AbstractDataSource*);

	const NgspiceRawAsciiFilter* q;

	QStringList vectorNames;
	QVector<AbstractColumn::ColumnMode> columnModes;
	int startRow{1};
	int endRow{-1};

private:
	std::vector<void*> m_dataContainer; // pointers to the actual data containers
};

#endif
