/***************************************************************************
File                 : ReadStatFilter.cpp
Project              : LabPlot
Description          : ReadStat I/O-filter
--------------------------------------------------------------------
Copyright            : (C) 2021 by Stefan Gerlach (stefan.gerlach@uni.kn)
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
#include "ReadStatFilter.h"
#include "ReadStatFilterPrivate.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/core/column/Column.h"
#include "backend/lib/macros.h"

#include <KLocalizedString>
#include <QProcess>

///////////// macros ///////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////

/*!
	\class ReadStatFilter
	\brief Manages the import/export of data from/to a ReadStat file.

	\ingroup datasources
*/
ReadStatFilter::ReadStatFilter():AbstractFileFilter(FileType::READSTAT), d(new ReadStatFilterPrivate(this)) {}

ReadStatFilter::~ReadStatFilter() = default;

/*!
  reads the content of the file \c fileName to the data source \c dataSource.
*/
void ReadStatFilter::readDataFromFile(const QString& fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode mode) {
	d->readDataFromFile(fileName, dataSource, mode);
}

/*!
writes the content of the data source \c dataSource to the file \c fileName.
*/
void ReadStatFilter::write(const QString & fileName, AbstractDataSource* dataSource) {
	d->write(fileName, dataSource);
// 	emit()
}

///////////////////////////////////////////////////////////////////////
/*!
  loads the predefined filter settings for \c filterName
*/
void ReadStatFilter::loadFilterSettings(const QString& filterName) {
	Q_UNUSED(filterName);
}

/*!
  saves the current settings as a new filter with the name \c filterName
*/
void ReadStatFilter::saveFilterSettings(const QString& filterName) const {
	Q_UNUSED(filterName);
}

///////////////////////////////////////////////////////////////////////
#ifdef HAVE_READSTAT
int ReadStatFilter::get_metadata(readstat_metadata_t *metadata, void *md) {
	*(readstat_metadata_t *)md = *metadata;

	return READSTAT_HANDLER_OK;
}
#endif

QString ReadStatFilter::fileInfoString(const QString& fileName) {
	DEBUG(Q_FUNC_INFO << ", file name = " << qPrintable(fileName))

	QString info;

#ifdef HAVE_READSTAT
	readstat_error_t error = READSTAT_OK;
	readstat_parser_t *parser = readstat_parser_init();
	
	readstat_set_metadata_handler(parser, &get_metadata);
	readstat_metadata_t metadata;

	if ( fileName.endsWith(QLatin1String(".dta")) )
		error = readstat_parse_dta(parser, qPrintable(fileName), &metadata);
	else if ( fileName.endsWith(QLatin1String(".sav")) || fileName.endsWith(QLatin1String(".zsav")) )
		error = readstat_parse_sav(parser, qPrintable(fileName), &metadata);
	else if (fileName.endsWith(QLatin1String(".por")) )
		error = readstat_parse_por(parser, qPrintable(fileName), &metadata);
	else if (fileName.endsWith(QLatin1String(".sas7bdat")) )
		error = readstat_parse_sas7bdat(parser, qPrintable(fileName), &metadata);
	else if (fileName.endsWith(QLatin1String(".sas7bcat")) )
		error = readstat_parse_sas7bcat(parser, qPrintable(fileName), &metadata);
	else if (fileName.endsWith(QLatin1String(".xpt")) || fileName.endsWith(QLatin1String(".xpt5"))  || fileName.endsWith(QLatin1String(".xpt8")) )
		error = readstat_parse_xport(parser, qPrintable(fileName), &metadata);
	else
		return i18n("Unknown file extension");

	readstat_parser_free(parser);

	if (error == READSTAT_OK) {
		info += i18n("Number of records: %1", QString::number((int64_t)metadata.row_count));
		info += QLatin1String("<br>");
		info += i18n("Number of variables: %1", QString::number((int64_t)metadata.var_count));
		info += QLatin1String("<br>");
		info += i18n("Creation time: %1", QDateTime::fromTime_t(metadata.creation_time).toString());
		info += QLatin1String("<br>");
		info += i18n("Modification time: %1", QDateTime::fromTime_t(metadata.modified_time).toString());
		info += QLatin1String("<br>");
		info += i18n("Format version: %1", QString::number((int64_t)metadata.file_format_version));
		info += QLatin1String("<br>");
		QString compress;
		switch (metadata.compression) {
		case READSTAT_COMPRESS_NONE:
			compress = "none";
			break;
		case READSTAT_COMPRESS_ROWS:
			compress = "rows";
			break;
		case READSTAT_COMPRESS_BINARY:
			compress = "binary";
			break;
		}
		info += i18n("Compression: %1", compress);
		info += QLatin1String("<br>");
		QString endian;
		switch (metadata.endianness) {
		case READSTAT_ENDIAN_NONE:
			endian = "none";
			break;
		case READSTAT_ENDIAN_LITTLE:
			endian = "little";
			break;
		case READSTAT_ENDIAN_BIG:
			endian = "big";
			break;
		}
		info += i18n("Endianess: %1", endian);
		info += QLatin1String("<br>");
		info += i18n("Table name: %1", QString(metadata.table_name));
		info += QLatin1String("<br>");
		info += i18n("File label: %1", QString(metadata.file_label));
		info += QLatin1String("<br>");
		info += i18n("File encoding: %1", QString(metadata.file_encoding));
		info += QLatin1String("<br>");
		info += i18n("64bit: %1", QString::number((unsigned int)metadata.is64bit));
		info += QLatin1String("<br>");
	} else {
		info += i18n("Error getting file info");
	}
#endif

	return info;
}

//#####################################################################
//################### Private implementation ##########################
//#####################################################################

ReadStatFilterPrivate::ReadStatFilterPrivate(ReadStatFilter* owner) : q(owner) {
#ifdef HAVE_READSTAT
	m_status = 0;
#endif
}

/*!
    reads the content of the current selected variable from file \c fileName to the data source \c dataSource.
    Uses the settings defined in the data source.
*/
QVector<QStringList> ReadStatFilterPrivate::readDataFromFile(const QString& fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode mode) {
	QVector<QStringList> dataStrings;

//	if (currentVarName.isEmpty()) {
		DEBUG(" No variable selected");
		return dataStrings;
//	}

	Q_UNUSED(fileName)
	Q_UNUSED(dataSource)
	Q_UNUSED(mode)
//	return readCurrentVar(fileName, dataSource, mode);
}

/*!
    writes the content of \c dataSource to the file \c fileName.
*/
void ReadStatFilterPrivate::write(const QString & fileName, AbstractDataSource* dataSource) {
	Q_UNUSED(fileName);
	Q_UNUSED(dataSource);
	//TODO: writing ReadStat files not implemented yet
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################

/*!
  Saves as XML.
 */
void ReadStatFilter::save(QXmlStreamWriter* writer) const {
	writer->writeStartElement("readstatFilter");
	writer->writeEndElement();
}

/*!
  Loads from XML.
*/
bool ReadStatFilter::load(XmlStreamReader* reader) {
	Q_UNUSED(reader);
// 	KLocalizedString attributeWarning = ki18n("Attribute '%1' missing or empty, default value is used");
// 	QXmlStreamAttributes attribs = reader->attributes();
	return true;
}
