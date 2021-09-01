/*
File                 : BinaryFilterPrivate.h
Project              : LabPlot
Description          : Private implementation class for BinaryFilter.
--------------------------------------------------------------------
SPDX-FileCopyrightText: 2015-2017 Stefan Gerlach <stefan.gerlach@uni.kn>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef BINARYFILTERPRIVATE_H
#define BINARYFILTERPRIVATE_H

#include <QVector>

class AbstractDataSource;
class AbstractColumn;

class BinaryFilterPrivate {

public:
	explicit BinaryFilterPrivate(BinaryFilter*);

	int prepareStreamToRead(QDataStream&);
	void readDataFromDevice(QIODevice& device, AbstractDataSource* = nullptr,
			AbstractFileFilter::ImportMode = AbstractFileFilter::ImportMode::Replace, int lines = -1);
	void readDataFromFile(const QString& fileName, AbstractDataSource* = nullptr,
			AbstractFileFilter::ImportMode = AbstractFileFilter::ImportMode::Replace);
	void write(const QString& fileName, AbstractDataSource*);
	QVector<QStringList> preview(const QString& fileName, int lines);

	const BinaryFilter* q;

	size_t vectors{2};
	BinaryFilter::DataType dataType{BinaryFilter::DataType::INT8};
	QDataStream::ByteOrder byteOrder{QDataStream::LittleEndian};
	QVector<AbstractColumn::ColumnMode> columnModes;

	int startRow{1};		// start row (value*vectors) to read (can be -1)
	int endRow{-1};			// end row to (value*vectors) read (can be -1)
	size_t numRows{0};		// number of rows
	size_t skipStartBytes{0};	// bytes to skip at start
	size_t skipBytes{0};		// bytes to skip after each value
	bool createIndexEnabled{false};	// if create index column

	bool autoModeEnabled{true};

private:
	int m_actualRows{0};
	int m_actualCols{0};
};

#endif
