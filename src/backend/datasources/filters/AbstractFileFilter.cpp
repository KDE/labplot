/*
	File                 : AbstractFileFilter.h
	Project              : LabPlot
	Description          : file I/O-filter related interface
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2009-2017 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2017-2024 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "backend/datasources/filters/AbstractFileFilter.h"
#include "backend/datasources/filters/SpiceFilter.h"
#include "backend/datasources/filters/VectorBLFFilter.h"
#include "backend/lib/hostprocess.h"
#include "backend/lib/macros.h"

#include <KLocalizedString>

#include <QDateTime>
#include <QImageReader>
#include <QLocale>
#include <QProcess>
#include <QStandardPaths>

bool AbstractFileFilter::isNan(const QString& s) {
	const static QStringList nanStrings{QStringLiteral("NA"),
										QStringLiteral("NAN"),
										QStringLiteral("N/A"),
										QStringLiteral("-NA"),
										QStringLiteral("-NAN"),
										QStringLiteral("NULL")};
	if (nanStrings.contains(s, Qt::CaseInsensitive))
		return true;

	return false;
}

AbstractColumn::ColumnMode AbstractFileFilter::columnMode(const QString& valueString, QString& dateTimeFormat, QLocale::Language lang) {
	return columnMode(valueString, dateTimeFormat, QLocale(lang));
}

/*!
 * return the column mode for the given value string and settings \c dateTimeFormat and \c locale.
 * in case \c dateTimeFormat is empty, all possible datetime formats are tried out to determine the valid datetime object.
 */
AbstractColumn::ColumnMode
AbstractFileFilter::columnMode(const QString& valueString, QString& dateTimeFormat, const QLocale& locale, bool intAsDouble, int baseYear) {
	// TODO: use BigInt as default integer?
	auto mode = AbstractColumn::ColumnMode::Integer;
	if (valueString.size() == 0) // empty string treated as integer (meaning the non-empty strings will determine the data type)
		return mode;

	if (isNan(valueString))
		return AbstractColumn::ColumnMode::Double;

	// check if integer first
	bool ok;
	int intValue = locale.toInt(valueString, &ok);
	DEBUG(Q_FUNC_INFO << ", " << STDSTRING(valueString) << " : toInt " << intValue << " ?: " << ok);
	Q_UNUSED(intValue)
	if (!ok) {
		// if not a int, check datetime. if that fails: check double and big int, else it's a string
		QDateTime valueDateTime;
		if (dateTimeFormat.isEmpty()) {
			for (const auto& format : AbstractColumn::dateTimeFormats()) {
				valueDateTime = QDateTime::fromString(valueString, format, baseYear);
				if (valueDateTime.isValid()) {
					DEBUG(Q_FUNC_INFO << ", " << STDSTRING(valueString) << " : valid DateTime format - " << STDSTRING(format));
					dateTimeFormat = format;
					break;
				}
			}
		} else
			valueDateTime = QDateTime::fromString(valueString, dateTimeFormat);

		if (valueDateTime.isValid()) {
			mode = AbstractColumn::ColumnMode::DateTime;
		} else {
			DEBUG(Q_FUNC_INFO << ", DATETIME invalid! String: " << STDSTRING(valueString) << " DateTime format: " << STDSTRING(dateTimeFormat))

			// check if big integer
			qint64 bigIntValue = locale.toLongLong(valueString, &ok);
			DEBUG(Q_FUNC_INFO << ", " << STDSTRING(valueString) << " : toBigInt " << bigIntValue << " ?: " << ok);
			Q_UNUSED(bigIntValue)
			if (ok)
				return AbstractColumn::ColumnMode::BigInt;

			// check if double
			double value = locale.toDouble(valueString, &ok);
			DEBUG(Q_FUNC_INFO << ", " << STDSTRING(valueString) << " : toDouble " << value << " ?: " << ok);
			Q_UNUSED(value)

			mode = ok ? AbstractColumn::ColumnMode::Double : AbstractColumn::ColumnMode::Text;
		}
	} else if (intAsDouble) {
		mode = AbstractColumn::ColumnMode::Double;
	}

	return mode;
}

QString AbstractFileFilter::dateTimeFormat(const QString& valueString) {
	QDateTime valueDateTime;
	for (const auto& format : AbstractColumn::dateTimeFormats()) {
		valueDateTime = QDateTime::fromString(valueString, format);
		if (valueDateTime.isValid())
			return format;
	}
	return QLatin1String("yyyy-MM-dd hh:mm:ss.zzz");
}

/*
returns the list of all supported locales for numeric data
*/
QStringList AbstractFileFilter::numberFormats() {
	QStringList formats;
	for (int l = 0; l < ENUM_COUNT(QLocale, Language); ++l)
		formats << QLocale::languageToString((QLocale::Language)l);

	return formats;
}

/*!
 * Returns the last error that occured during the last parse step.
 */
QString AbstractFileFilter::lastError() const {
	return m_lastError;
}

void AbstractFileFilter::setLastError(const QString& error) {
	m_lastError = error;
}

void AbstractFileFilter::clearLastError() {
	m_lastError.clear();
}

/*!
 * Returns the list of warnings that occured during the last parse step.
 */
QStringList AbstractFileFilter::lastWarnings() const {
	return m_lastWarnings;
}

void AbstractFileFilter::addWarning(const QString& warning) {
	m_lastWarnings << warning;
}

void AbstractFileFilter::clearLastWarnings() {
	m_lastWarnings.clear();
}

/*!
 * which file types are exclusive (filter can only open files of this type)
 * used to deactivate selection if of other type
 */
bool AbstractFileFilter::exclusiveFileType(const AbstractFileFilter::FileType type) {
	switch (type) {
	case FileType::Ods:
	case FileType::HDF5:
	case FileType::NETCDF:
	case FileType::XLSX:
	case FileType::FITS:
	case FileType::ROOT:
	case FileType::Spice:
	case FileType::MATIO:
	case FileType::VECTOR_BLF:
	case FileType::MCAP:
		return true;
	case FileType::Ascii:
	case FileType::Binary:
	case FileType::Image:
	case FileType::JSON:
	case FileType::READSTAT:
		break;
	}

	return false;
}

AbstractFileFilter::FileType AbstractFileFilter::fileType(const QString& fileName) {
	DEBUG(Q_FUNC_INFO)
	QString fileInfo;
#ifndef HAVE_WINDOWS
	// check, if we can guess the file type by content
	const QString fileFullPath = safeExecutableName(QStringLiteral("file"));
	if (!fileFullPath.isEmpty()) {
		QProcess proc;
		startHostProcess(proc, fileFullPath, QStringList() << QStringLiteral("-b") << QStringLiteral("-z") << fileName);
		if (!proc.waitForFinished(1000)) {
			proc.kill();
			DEBUG("ERROR: reading file type of file" << STDSTRING(fileName));
			return FileType::Binary;
		}
		fileInfo = QLatin1String(proc.readLine());
	}
#endif

	FileType fileType;
	QByteArray imageFormat = QImageReader::imageFormat(fileName);
	if (fileInfo.contains(QLatin1String("JSON"))
		|| fileName.endsWith(QLatin1String("json"), Qt::CaseInsensitive)
		// json file can be compressed. add all formats supported by KFilterDev, \sa KCompressionDevice::CompressionType
		|| fileName.endsWith(QLatin1String("json.gz"), Qt::CaseInsensitive) || fileName.endsWith(QLatin1String("json.bz2"), Qt::CaseInsensitive)
		|| fileName.endsWith(QLatin1String("json.lzma"), Qt::CaseInsensitive) || fileName.endsWith(QLatin1String("json.xz"), Qt::CaseInsensitive)
		|| fileName.endsWith(QLatin1String("har"), Qt::CaseInsensitive)) {
		//*.json files can be recognized as ASCII. so, do the check for the json-extension as first.
		fileType = FileType::JSON;
	} else if (SpiceFilter::isSpiceFile(fileName))
		fileType = FileType::Spice;
#ifdef HAVE_QXLSX // before ASCII, because XLSX is XML and XML is ASCII
	else if (fileInfo.contains(QLatin1String("Microsoft Excel")) || fileName.endsWith(QLatin1String("xlsx"), Qt::CaseInsensitive))
		fileType = FileType::XLSX;
#endif
#ifdef HAVE_ORCUS // before ASCII, because ODS is XML and XML is ASCII
	else if (fileInfo.contains(QLatin1String("OpenDocument Spreadsheet")) || fileName.endsWith(QLatin1String("ods"), Qt::CaseInsensitive))
		fileType = FileType::Ods;
#endif
	else if (fileInfo.contains(QLatin1String("ASCII")) || fileName.endsWith(QLatin1String("txt"), Qt::CaseInsensitive)
			 || fileName.endsWith(QLatin1String("csv"), Qt::CaseInsensitive) || fileName.endsWith(QLatin1String("dat"), Qt::CaseInsensitive)
			 || fileInfo.contains(QLatin1String("compressed data")) /* for gzipped ascii data */) {
		if (fileName.endsWith(QLatin1String(".sas7bdat"), Qt::CaseInsensitive))
			fileType = FileType::READSTAT;
		else // probably ascii data
			fileType = FileType::Ascii;
	}
#ifdef HAVE_MATIO // before HDF5 to prefer this filter for MAT 7.4 files
	else if (fileInfo.contains(QLatin1String("Matlab")) || fileName.endsWith(QLatin1String("mat"), Qt::CaseInsensitive))
		fileType = FileType::MATIO;
#endif
#ifdef HAVE_HDF5 // before NETCDF to treat NetCDF 4 files with .nc ending as HDF5 when fileInfo detects it (HDF4 not supported)
	else if (fileInfo.contains(QLatin1String("Hierarchical Data Format (version 5)")) || fileName.endsWith(QLatin1String("h5"), Qt::CaseInsensitive)
			 || (fileName.endsWith(QLatin1String("hdf"), Qt::CaseInsensitive) && !fileInfo.contains(QLatin1String("(version 4)")))
			 || fileName.endsWith(QLatin1String("hdf5"), Qt::CaseInsensitive) || fileName.endsWith(QLatin1String("nc4"), Qt::CaseInsensitive))
		fileType = FileType::HDF5;
#endif
#ifdef HAVE_NETCDF
	else if (fileInfo.contains(QLatin1String("NetCDF Data Format")) || fileName.endsWith(QLatin1String("nc"), Qt::CaseInsensitive)
			 || fileName.endsWith(QLatin1String("netcdf"), Qt::CaseInsensitive) || fileName.endsWith(QLatin1String("cdf"), Qt::CaseInsensitive))
		fileType = FileType::NETCDF;
#endif
#ifdef HAVE_VECTOR_BLF
	else if (fileName.endsWith(QLatin1String("blf")) && VectorBLFFilter::isValid(fileName))
		fileType = FileType::VECTOR_BLF;
#endif
#ifdef HAVE_FITS
	else if (fileInfo.contains(QLatin1String("FITS image data")) || fileName.endsWith(QLatin1String("fits"), Qt::CaseInsensitive)
			 || fileName.endsWith(QLatin1String("fit"), Qt::CaseInsensitive) || fileName.endsWith(QLatin1String("fts"), Qt::CaseInsensitive))
		fileType = FileType::FITS;
#endif
#ifdef HAVE_ZIP
	else if (fileInfo.contains(QLatin1String("ROOT")) // can be "ROOT Data Format" or "ROOT file Version ??? (Compression: 1)"
			 || fileName.endsWith(QLatin1String("root"), Qt::CaseInsensitive)) // TODO find out file description
		fileType = FileType::ROOT;
#endif
#ifdef HAVE_READSTAT // sas7bdat -> ASCII
	else if (fileInfo.startsWith(QLatin1String("SAS")) || fileInfo.startsWith(QLatin1String("SPSS"))
			 || fileName.endsWith(QLatin1String(".dta"), Qt::CaseInsensitive) || fileName.endsWith(QLatin1String(".sav"), Qt::CaseInsensitive)
			 || fileName.endsWith(QLatin1String(".zsav"), Qt::CaseInsensitive) || fileName.endsWith(QLatin1String(".por"), Qt::CaseInsensitive)
			 || fileName.endsWith(QLatin1String(".sas7bcat"), Qt::CaseInsensitive) || fileName.endsWith(QLatin1String(".xpt"), Qt::CaseInsensitive)
			 || fileName.endsWith(QLatin1String(".xpt5"), Qt::CaseInsensitive) || fileName.endsWith(QLatin1String(".xpt8"), Qt::CaseInsensitive))
		fileType = FileType::READSTAT;
#endif
	else if (fileInfo.contains(QLatin1String("image")) || fileInfo.contains(QLatin1String("bitmap")) || !imageFormat.isEmpty())
		fileType = FileType::Image;
#ifdef HAVE_MCAP
	else if (fileInfo.contains(QLatin1String("mcap")) || fileName.endsWith(QLatin1String(".mcap")))
		fileType = FileType::MCAP;
#endif
	else
		fileType = FileType::Binary;

	return fileType;
}

/*!
  returns the list of all supported data file formats
*/
/*QStringList AbstractFileFilter::fileTypes() {
	// TODO: #ifdef HAVE_QXLSX?
	return (QStringList() << i18n("ASCII Data") << i18n("Binary Data") << i18n("Image") << i18n("Excel") << i18n("Hierarchical Data Format 5 (HDF5)")
						  << i18n("Network Common Data Format (NetCDF)") << i18n("Flexible Image Transport System Data Format (FITS)") << i18n("JSON Data")
						  << i18n("ROOT (CERN) Histograms") << i18n("Spice") << i18n("SAS, Stata or SPSS"));
}*/

QString AbstractFileFilter::convertFromNumberToColumn(int n) {
	// main code from https://www.geeksforgeeks.org/find-excel-column-name-given-number/
	// Function to print column name for a given column number

	char str[1000]; // To store result (column name)
	int i = 0; // To store current index in str which is result

	while (n > 0) {
		// Find remainder
		int rem = n % 26;

		// If remainder is 0, then a 'Z' must be there in output
		if (rem == 0) {
			str[i++] = 'Z';
			n = (n / 26) - 1;
		} else // If remainder is non-zero
		{
			str[i++] = (rem - 1) + 'A';
			n = n / 26;
		}
	}
	str[i] = '\0';

	// Reverse the string and print result
	std::reverse(str, str + strlen(str));

	return QLatin1String(str);
}

int AbstractFileFilter::previewPrecision() const {
	return m_previewPrecision;
}

void AbstractFileFilter::setPreviewPrecision(int precision) {
	m_previewPrecision = precision;
}
