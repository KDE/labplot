/*
    File                 : MatioFilterPrivate.h
    Project              : LabPlot
    Description          : Private implementation class for MatioFilter.
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2021 Stefan Gerlach <stefan.gerlach@uni.kn>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef MATIOFILTERPRIVATE_H
#define MATIOFILTERPRIVATE_H

#ifdef HAVE_MATIO
#include <matio.h>
#endif

class AbstractDataSource;

class MatioFilterPrivate {

public:
	explicit MatioFilterPrivate(MatioFilter*);

	void parse(const QString & fileName);
	void readDataFromFile(const QString& fileName, AbstractDataSource* = nullptr,
			AbstractFileFilter::ImportMode = AbstractFileFilter::ImportMode::Replace);
	QVector<QStringList> readCurrentVar(const QString& fileName, AbstractDataSource* = nullptr,
			AbstractFileFilter::ImportMode = AbstractFileFilter::ImportMode::Replace, size_t lines = 0);
	void write(const QString& fileName, AbstractDataSource*);

//helper functions
#ifdef HAVE_MATIO
	static QString className(matio_classes classType);
	static QString typeName(matio_types dataType);
	static AbstractColumn::ColumnMode classMode(matio_classes classType);
	static AbstractColumn::ColumnMode typeMode(matio_types dataType);
#endif

	const MatioFilter* q;

	size_t varCount;
	QString currentVarName;
	QStringList selectedVarNames;
	QVector<QStringList> varsInfo;
	int startRow{1};
	int endRow{-1};
	int startColumn{1};
	int endColumn{-1};

private:
#ifdef HAVE_MATIO
	mat_t* matfp{nullptr};
#endif
};

#endif
