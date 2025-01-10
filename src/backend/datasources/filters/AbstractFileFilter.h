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

public:
	enum class FileType { Ascii, Binary, XLSX, Ods, Image, HDF5, NETCDF, FITS, JSON, ROOT, Spice, READSTAT, MATIO, VECTOR_BLF, MCAP };
	Q_ENUM(FileType)
	enum class ImportMode { Append, Prepend, Replace };
	Q_ENUM(ImportMode)

	explicit AbstractFileFilter(FileType type)
		: m_type(type) {
	}
	~AbstractFileFilter() override = default;

	static bool isNan(const QString&);
	static AbstractColumn::ColumnMode columnMode(const QString& valueString, QString& dateTimeFormat, QLocale::Language);
	static AbstractColumn::ColumnMode columnMode(const QString& valueString,
												 QString& dateTimeFormat,
												 const QLocale& = QLocale(),
												 bool intAsDouble = false,
												 int baseYear = QLocale::DefaultTwoDigitBaseYear);
	static QString dateTimeFormat(const QString& valueString);
	static QStringList numberFormats();
	static bool exclusiveFileType(FileType);
	static FileType fileType(const QString&);
	// static QStringList fileTypes();
	static QString convertFromNumberToColumn(int n);
	int previewPrecision() const;
	void setPreviewPrecision(int);

	virtual void readDataFromFile(const QString& fileName, AbstractDataSource* = nullptr, ImportMode = ImportMode::Replace) = 0;
	/*!
	 * Writing the content of the datasource to the file with filename \p fileName
	 * \brief write
	 * \param fileName
	 */
	virtual void write(const QString& fileName, AbstractDataSource*) = 0;

	QString lastError() const;
	void setLastError(const QString&);
	void clearLastError();

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
	int m_previewPrecision{6};
};

#endif
