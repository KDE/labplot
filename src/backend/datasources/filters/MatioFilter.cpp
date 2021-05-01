/***************************************************************************
File                 : MatioFilter.cpp
Project              : LabPlot
Description          : Matio I/O-filter
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
#include "MatioFilter.h"
#include "MatioFilterPrivate.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/core/column/Column.h"
#include "backend/lib/macros.h"

#include <KLocalizedString>
#include <QProcess>

///////////// macros ///////////////////////////////////////////////

// see NetCDFFilter.cpp

//////////////////////////////////////////////////////////////////////

/*!
	\class MatioFilter
	\brief Manages the import/export of data from/to a Matio file.

	\ingroup datasources
*/
MatioFilter::MatioFilter():AbstractFileFilter(FileType::MATIO), d(new MatioFilterPrivate(this)) {}

MatioFilter::~MatioFilter() = default;

QVector<QStringList> MatioFilter::preview(const QString& fileName, int lines) {
	return d->preview(fileName, lines);
}

/*!
  parses the content of the file \c ileName.
*/
void MatioFilter::parse(const QString & fileName, QTreeWidgetItem* rootItem) {
	d->parse(fileName, rootItem);
}

/*!
  reads the content of the selected attribute from file \c fileName.
*/
/*QString MatioFilter::readAttribute(const QString & fileName, const QString & name, const QString & varName) {
	return d->readAttribute(fileName, name, varName);
}*/

/*!
  reads the content of the current variable from file \c fileName.
*/
/*QVector<QStringList> MatioFilter::readCurrentVar(const QString& fileName, AbstractDataSource* dataSource,
		AbstractFileFilter::ImportMode importMode, int lines) {
	return d->readCurrentVar(fileName, dataSource, importMode, lines);
}*/

/*!
  reads the content of the file \c fileName to the data source \c dataSource.
*/
void MatioFilter::readDataFromFile(const QString& fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode mode) {
	d->readDataFromFile(fileName, dataSource, mode);
}

/*!
writes the content of the data source \c dataSource to the file \c fileName.
*/
void MatioFilter::write(const QString & fileName, AbstractDataSource* dataSource) {
	d->write(fileName, dataSource);
// 	emit()
}

///////////////////////////////////////////////////////////////////////
/*!
  loads the predefined filter settings for \c filterName
*/
void MatioFilter::loadFilterSettings(const QString& filterName) {
	Q_UNUSED(filterName);
}

/*!
  saves the current settings as a new filter with the name \c filterName
*/
void MatioFilter::saveFilterSettings(const QString& filterName) const {
	Q_UNUSED(filterName);
}

///////////////////////////////////////////////////////////////////////

/*void MatioFilter::setCurrentVarName(const QString& ds) {
	d->currentVarName = ds;
}

const QString MatioFilter::currentVarName() const {
	return d->currentVarName;
}*/

void MatioFilter::setStartRow(const int s) {
	d->startRow = s;
}

int MatioFilter::startRow() const {
	return d->startRow;
}

void MatioFilter::setEndRow(const int e) {
	d->endRow = e;
}

int MatioFilter::endRow() const {
	return d->endRow;
}

void MatioFilter::setStartColumn(const int c) {
	d->startColumn = c;
}

int MatioFilter::startColumn() const {
	return d->startColumn;
}

void MatioFilter::setEndColumn(const int c) {
	d->endColumn = c;
}

int MatioFilter::endColumn() const {
	return d->endColumn;
}

QString MatioFilter::fileInfoString(const QString& fileName) {
	DEBUG(Q_FUNC_INFO << ", fileName = " << qPrintable(fileName))

	QString info;
	// see NetCDFFilter.cpp
#ifdef HAVE_MATIO

	//TODO: supports 7.3 (build with hdf5?)
#endif

	return info;
}

//#####################################################################
//################### Private implementation ##########################
//#####################################################################

MatioFilterPrivate::MatioFilterPrivate(MatioFilter* owner) : q(owner) {
#ifdef HAVE_MATIO
	m_status = 0;
#endif
}

#ifdef HAVE_MATIO
// helper functions
#endif

/*!
    parses the content of the file \c fileName and fill the tree using rootItem.
*/
void MatioFilterPrivate::parse(const QString & fileName, QTreeWidgetItem* rootItem) {
	DEBUG("MatioFilterPrivate::parse()");
#ifdef HAVE_MATIO
	DEBUG("fileName = " << qPrintable(fileName));

	// see NetCDFFIlter.cpp
#else
	Q_UNUSED(fileName)
	Q_UNUSED(rootItem)
#endif
}

/*!
 * generates the preview for the file \c fileName reading the provided number of \c lines.
 */
QVector<QStringList> MatioFilterPrivate::preview(const QString& fileName, int lines) {
	QVector<QStringList> dataStrings;

	//TODO
	return dataStrings;
}

/*!
    reads the content of the current selected variable from file \c fileName to the data source \c dataSource.
    Uses the settings defined in the data source.
*/
QVector<QStringList> MatioFilterPrivate::readDataFromFile(const QString& fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode mode) {
	QVector<QStringList> dataStrings;

	if (currentVarName.isEmpty()) {
		DEBUG(" No variable selected");
		return dataStrings;
	}

	//return readCurrentVar(fileName, dataSource, mode);
	return dataStrings;
}

/*!
    writes the content of \c dataSource to the file \c fileName.
*/
void MatioFilterPrivate::write(const QString & fileName, AbstractDataSource* dataSource) {
	Q_UNUSED(fileName);
	Q_UNUSED(dataSource);
	//TODO: writing Matio files not implemented yet
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################

/*!
  Saves as XML.
 */
void MatioFilter::save(QXmlStreamWriter* writer) const {
	writer->writeStartElement("matioFilter");
	writer->writeEndElement();
}

/*!
  Loads from XML.
*/
bool MatioFilter::load(XmlStreamReader* reader) {
	Q_UNUSED(reader);
// 	KLocalizedString attributeWarning = ki18n("Attribute '%1' missing or empty, default value is used");
// 	QXmlStreamAttributes attribs = reader->attributes();
	return true;
}
