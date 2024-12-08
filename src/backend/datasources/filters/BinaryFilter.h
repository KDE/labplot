/*
	File                 : BinaryFilter.h
	Project              : LabPlot
	Description          : Binary I/O-filter
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2015-2018 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef BINARYFILTER_H
#define BINARYFILTER_H

#include "backend/datasources/filters/AbstractFileFilter.h"
#include <QDataStream>
#include <limits>

class BinaryFilterPrivate;
class QIODevice;

class BinaryFilter : public AbstractFileFilter {
	Q_OBJECT

public:
	// TODO; use ColumnMode when it supports all these types
	enum class DataType { INT8, INT16, INT32, INT64, UINT8, UINT16, UINT32, UINT64, REAL32, REAL64 };
	Q_ENUM(DataType)

	BinaryFilter();
	~BinaryFilter() override;

	static QStringList dataTypes();
	static int dataSize(BinaryFilter::DataType);
	static size_t rowNumber(const QString& fileName, size_t vectors, BinaryFilter::DataType, size_t maxRows = std::numeric_limits<std::size_t>::max());
	static QString fileInfoString(const QString&);

	// read data from any device
	void readDataFromDevice(QIODevice&, AbstractDataSource* = nullptr, ImportMode = ImportMode::Replace, int lines = -1);
	void readDataFromFile(const QString& fileName, AbstractDataSource*, ImportMode = ImportMode::Replace) override;
	void write(const QString& fileName, AbstractDataSource*) override;
	QVector<QStringList> preview(const QString& fileName, int lines);

	void setVectors(const size_t);
	size_t vectors() const;

	void setDataType(const BinaryFilter::DataType);
	BinaryFilter::DataType dataType() const;
	void setByteOrder(const QDataStream::ByteOrder);
	QDataStream::ByteOrder byteOrder() const;
	void setSkipStartBytes(const size_t);
	size_t skipStartBytes() const;
	void setStartRow(const int);
	int startRow() const;
	void setEndRow(const int);
	int endRow() const;
	void setSkipBytes(const size_t);
	size_t skipBytes() const;
	void setCreateIndexEnabled(const bool);

	void setAutoModeEnabled(const bool);
	bool isAutoModeEnabled() const;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*) override;

private:
	std::unique_ptr<BinaryFilterPrivate> const d;
	friend class BinaryFilterPrivate;
};

#endif
