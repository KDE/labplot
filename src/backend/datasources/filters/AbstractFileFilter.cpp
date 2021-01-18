/***************************************************************************
File                 : AbstractFileFilter.h
Project              : LabPlot
Description          : file I/O-filter related interface
--------------------------------------------------------------------
Copyright            : (C) 2009-2017 Alexander Semke (alexander.semke@web.de)
Copyright            : (C) 2017 Stefan Gerlach (stefan.gerlach@uni.kn)
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/

#include "backend/datasources/filters/AbstractFileFilter.h"
#include "backend/datasources/filters/NgspiceRawAsciiFilter.h"
#include "backend/datasources/filters/NgspiceRawBinaryFilter.h"
#include "backend/lib/macros.h"

#include <QDateTime>
#include <QImageReader>
#include <QProcess>
#include <QLocale>
#include <KLocalizedString>

bool AbstractFileFilter::isNan(const QString& s) {
	const static QStringList nanStrings{"NA", "NAN", "N/A", "-NA", "-NAN", "NULL"};
	if (nanStrings.contains(s, Qt::CaseInsensitive))
		return true;

	return false;
}

AbstractColumn::ColumnMode AbstractFileFilter::columnMode(const QString& valueString, const QString& dateTimeFormat, QLocale::Language lang) {
	return columnMode(valueString, dateTimeFormat, QLocale(lang));
}

/*!
 * return the column mode for the given value string and settings \c dateTimeFormat and \c locale.
 * in case \c dateTimeFormat is empty, all possible datetime formats are tried out to determine the valid datetime object.
 */
AbstractColumn::ColumnMode AbstractFileFilter::columnMode(const QString& valueString, const QString& dateTimeFormat, const QLocale& locale) {
	//TODO: use BigInt as default integer?
	if (valueString.size() == 0)	// empty string treated as integer (meaning the non-empty strings will determine the data type)
		return AbstractColumn::ColumnMode::Integer;

	if (isNan(valueString))
		return AbstractColumn::ColumnMode::Numeric;

	// check if integer first
	bool ok;
	int intValue = locale.toInt(valueString, &ok);
	DEBUG(Q_FUNC_INFO << ", " << STDSTRING(valueString) << " : toInt " << intValue << " ?: " << ok);
	Q_UNUSED(intValue)
	if (ok || isNan(valueString))
		return AbstractColumn::ColumnMode::Integer;

	//check big integer
	qint64 bigIntValue = locale.toLongLong(valueString, &ok);
	DEBUG(Q_FUNC_INFO << ", " << STDSTRING(valueString) << " : toBigInt " << bigIntValue << " ?: " << ok);
	Q_UNUSED(bigIntValue)
	if (ok || isNan(valueString))
		return AbstractColumn::ColumnMode::BigInt;

	//try to convert to a double
	auto mode = AbstractColumn::ColumnMode::Numeric;
	double value = locale.toDouble(valueString, &ok);
	DEBUG(Q_FUNC_INFO << ", " << STDSTRING(valueString) << " : toDouble " << value << " ?: " << ok);
	Q_UNUSED(value)

	//if not a number, check datetime. if that fails: string
	if (!ok) {
		QDateTime valueDateTime;
		if (dateTimeFormat.isEmpty()) {
			for (const auto& format : AbstractColumn::dateTimeFormats()) {
				valueDateTime = QDateTime::fromString(valueString, format);
				if (valueDateTime.isValid())
					break;
			}
		} else
			valueDateTime = QDateTime::fromString(valueString, dateTimeFormat);

		if (valueDateTime.isValid())
			mode = AbstractColumn::ColumnMode::DateTime;
		else {
			DEBUG(Q_FUNC_INFO << ", DATETIME invalid! String: " << STDSTRING(valueString) << " DateTime format: " << STDSTRING(dateTimeFormat))
			mode = AbstractColumn::ColumnMode::Text;
		}
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

AbstractFileFilter::FileType AbstractFileFilter::fileType(const QString& fileName) {
	QString fileInfo;
#ifndef HAVE_WINDOWS
	//check, if we can guess the file type by content
	QProcess proc;
	proc.start("file", QStringList() << "-b" << "-z" << fileName);
	if (!proc.waitForFinished(1000)) {
		proc.kill();
		DEBUG("ERROR: reading file type of file" << STDSTRING(fileName));
		return FileType::Binary;
	}
	fileInfo = proc.readLine();
#endif

	FileType fileType;
	QByteArray imageFormat = QImageReader::imageFormat(fileName);
	if (fileInfo.contains(QLatin1String("JSON")) || fileName.endsWith(QLatin1String("json"), Qt::CaseInsensitive)
		//json file can be compressed. add all formats supported by KFilterDev, \sa KCompressionDevice::CompressionType
		|| fileName.endsWith(QLatin1String("json.gz"), Qt::CaseInsensitive)
		|| fileName.endsWith(QLatin1String("json.bz2"), Qt::CaseInsensitive)
		|| fileName.endsWith(QLatin1String("json.lzma"), Qt::CaseInsensitive)
		|| fileName.endsWith(QLatin1String("json.xz"), Qt::CaseInsensitive)
		|| fileName.endsWith(QLatin1String("har"), Qt::CaseInsensitive) ) {
		//*.json files can be recognized as ASCII. so, do the check for the json-extension as first.
		fileType = FileType::JSON;
	} else if (fileInfo.contains(QLatin1String("ASCII"))
		|| fileName.endsWith(QLatin1String("txt"), Qt::CaseInsensitive)
		|| fileName.endsWith(QLatin1String("csv"), Qt::CaseInsensitive)
		|| fileName.endsWith(QLatin1String("dat"), Qt::CaseInsensitive)
		|| fileInfo.contains(QLatin1String("compressed data"))/* for gzipped ascii data */ ) {
		if (NgspiceRawAsciiFilter::isNgspiceAsciiFile(fileName))
			fileType = FileType::NgspiceRawAscii;
		else //probably ascii data
			fileType = FileType::Ascii;
#ifdef HAVE_HDF5
	} else if (fileInfo.contains(QLatin1String("Hierarchical Data Format"))
		|| fileName.endsWith(QLatin1String("h5"), Qt::CaseInsensitive)
		|| fileName.endsWith(QLatin1String("hdf"), Qt::CaseInsensitive)
		|| fileName.endsWith(QLatin1String("hdf5"), Qt::CaseInsensitive) )
		fileType = FileType::HDF5;
#endif
#ifdef HAVE_NETCDF
	else if (fileInfo.contains(QLatin1String("NetCDF Data Format"))
		|| fileName.endsWith(QLatin1String("nc"), Qt::CaseInsensitive)
		|| fileName.endsWith(QLatin1String("netcdf"), Qt::CaseInsensitive)
		|| fileName.endsWith(QLatin1String("cdf"), Qt::CaseInsensitive))
		fileType = FileType::NETCDF;
#endif
#ifdef HAVE_FITS
	else if (fileInfo.contains(QLatin1String("FITS image data"))
		|| fileName.endsWith(QLatin1String("fits"), Qt::CaseInsensitive)
		|| fileName.endsWith(QLatin1String("fit"), Qt::CaseInsensitive)
		|| fileName.endsWith(QLatin1String("fts"), Qt::CaseInsensitive))
		fileType = FileType::FITS;
#endif
#ifdef HAVE_ZIP
	else if (fileInfo.contains(QLatin1String("ROOT")) //can be "ROOT Data Format" or "ROOT file Version ??? (Compression: 1)"
		||  fileName.endsWith(QLatin1String("root"), Qt::CaseInsensitive)) // TODO find out file description
		fileType = FileType::ROOT;
#endif
	else if (fileInfo.contains("image") || fileInfo.contains("bitmap") || !imageFormat.isEmpty())
		fileType = FileType::Image;
	else if (NgspiceRawBinaryFilter::isNgspiceBinaryFile(fileName))
		fileType = FileType::NgspiceRawBinary;
	else
		fileType = FileType::Binary;

	return fileType;
}

/*!
  returns the list of all supported data file formats
*/
QStringList AbstractFileFilter::fileTypes() {
	return (QStringList() << i18n("ASCII data")
		<< i18n("Binary data")
		<< i18n("Image")
		<< i18n("Hierarchical Data Format 5 (HDF5)")
		<< i18n("Network Common Data Format (NetCDF)")
		<< i18n("Flexible Image Transport System Data Format (FITS)")
		<< i18n("JSON data")
		<< i18n("ROOT (CERN) Histograms")
		<< "Ngspice RAW ASCII"
		<< "Ngspice RAW Binary"
	);
}
