/*
	File                 : AbstractFileFilter.h
	Project              : LabPlot
	Description          : file I/O-filter related interface
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2009-2024 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2017-2024 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ABSTRACTFILEFILTER_H
#define ABSTRACTFILEFILTER_H

#include "backend/core/AbstractColumn.h"
#include <QLocale>
#include <QObject>
#include <memory> // unique_ptr

class AbstractDataSource;
class XmlStreamReader;
class QXmlStreamWriter;
class KConfig;

class AbstractFileFilter : public QObject {
	Q_OBJECT
	Q_ENUMS(FileType)
	Q_ENUMS(ImportMode)

public:
	enum class FileType { Ascii, Binary, XLSX, Ods, Image, HDF5, NETCDF, FITS, JSON, ROOT, Spice, READSTAT, MATIO, VECTOR_BLF, MCAP };
	enum class ImportMode { Append, Prepend, Replace };

	explicit AbstractFileFilter(FileType type)
		: m_type(type) {
	}
	~AbstractFileFilter() override = default;

	static bool isNan(const QString&);
	static AbstractColumn::ColumnMode columnMode(const QString& valueString, QString& dateTimeFormat, QLocale::Language);
	static AbstractColumn::ColumnMode columnMode(const QString& valueString, QString& dateTimeFormat, const QLocale& = QLocale());
	static QString dateTimeFormat(const QString& valueString);
	static QStringList numberFormats();
	static FileType fileType(const QString&);
	// static QStringList fileTypes();
	static QString convertFromNumberToColumn(int n);

	virtual void readDataFromFile(const QString& fileName, AbstractDataSource* = nullptr, ImportMode = ImportMode::Replace) = 0;
	virtual void write(const QString& fileName, AbstractDataSource*) = 0;

	QString lastError() const;
	void setLastError(const QString&);

	QStringList lastWarnings() const;
	void addWarning(const QString&);
	void clearLastWarnings();

	// save/load in the project XML
	virtual void save(QXmlStreamWriter*) const = 0;
	virtual bool load(XmlStreamReader*) = 0;

	FileType type() const {
		return m_type;
	}

Q_SIGNALS:
	void completed(int) const; //!< int ranging from 0 to 100 notifies about the status of a read/write process

protected:
	const FileType m_type;
	QString m_lastError;
	QStringList m_lastWarnings;
};

#endif
