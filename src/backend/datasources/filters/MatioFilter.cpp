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
void MatioFilter::parse(const QString & fileName) {
	d->parse(fileName);
}

/*!
  reads the content of the current variable from file \c fileName.
*/
QVector<QStringList> MatioFilter::readCurrentVar(const QString& fileName, AbstractDataSource* dataSource,
		AbstractFileFilter::ImportMode importMode, int lines) {
	return d->readCurrentVar(fileName, dataSource, importMode, lines);
}

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

void MatioFilter::setCurrentVarName(const QString& ds) {
	d->currentVarName = ds;
}

const QString MatioFilter::currentVarName() const {
	return d->currentVarName;
}
int MatioFilter::varCount() const {
	return d->varCount;
}
/*QStringList MatioFilter::varNames() const {
	return d->varNames;
}*/
QVector<QStringList> MatioFilter::varsInfo() const {
	return d->varsInfo;
}

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
#ifdef HAVE_MATIO
	mat_t *matfp = Mat_Open(qPrintable(fileName), MAT_ACC_RDONLY);

	if (!matfp)
		return i18n("Error getting file info");

	int version = Mat_GetVersion(matfp);
	const char *header = Mat_GetHeader(matfp);
	DEBUG(Q_FUNC_INFO << ", Header: " << header)
	info += header;
	info += QLatin1String("<br>");
	switch (version) {
	case MAT_FT_MAT73:
		info += i18n("Matlab version 7.3");
		break;
	case MAT_FT_MAT5:
		info += i18n("Matlab version 5");
		break;
	case MAT_FT_MAT4:
		info += i18n("Matlab version 4");
		break;
	case MAT_FT_UNDEFINED:
		info += i18n("Matlab version undefined");
	}
	info += QLatin1String("<br>");

        size_t n;
        char **dir = Mat_GetDir(matfp, &n);
	info += i18n("Number of variables: ") + QString::number(n);
	info += QLatin1String("<br>");
	if (dir && n < 10) {	// only show variable info when there are not too much
		info += i18n("Variables: ");
		for (size_t i = 0; i < n; ++i) {
			if (dir[i]) {
				info += "\"" + QString(dir[i]) + "\"";
				matvar_t* var = Mat_VarReadInfo(matfp, dir[i]);
				if (var)
					info += " (" + QString::number(Mat_VarGetNumberOfFields(var)) +  " fields, "
						+ QString::number(Mat_VarGetSize(var)) + " byte)";
				Mat_VarFree(var);
			}
		}
	}

	Mat_Close(matfp);
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

// helper functions
#ifdef HAVE_MATIO
// see matio.h
QString MatioFilterPrivate::className(matio_classes classType) {
	switch (classType) {
	case MAT_C_EMPTY:
		return i18n("Empty");
	case MAT_C_CELL:
		return i18n("Cell");
	case MAT_C_STRUCT:
		return i18n("Struct");
	case MAT_C_OBJECT:
		return i18n("Object");
	case MAT_C_CHAR:
		return i18n("Char");
	case MAT_C_SPARSE:
		return i18n("Sparse");
	case MAT_C_DOUBLE:
		return i18n("Double");
	case MAT_C_SINGLE:
		return i18n("Single");
	case MAT_C_INT8:
		return i18n("Int8");
	case MAT_C_UINT8:
		return i18n("UInt8");
	case MAT_C_INT16:
		return i18n("Int16");
	case MAT_C_UINT16:
		return i18n("UInt16");
	case MAT_C_INT32:
		return i18n("Int32");
	case MAT_C_UINT32:
		return i18n("UInt32");
	case MAT_C_INT64:
		return i18n("Int64");
	case MAT_C_UINT64:
		return i18n("UInt64");
	case MAT_C_FUNCTION:
		return i18n("Function");
	case MAT_C_OPAQUE:
		return i18n("Opaque");
	}

	return i18n("Undefined");
}
QString MatioFilterPrivate::typeName(matio_types dataType) {
	switch (dataType) {
	case MAT_T_UNKNOWN:
		return i18n("Unknown");
	case MAT_T_INT8:
		return i18n("Int8");
	case MAT_T_UINT8:
		return i18n("UInt8");
	case MAT_T_INT16:
		return i18n("Int16");
	case MAT_T_UINT16:
		return i18n("UInt16");
	case MAT_T_INT32:
		return i18n("Int32");
	case MAT_T_UINT32:
		return i18n("UInt32");
	case MAT_T_SINGLE:
		return i18n("Single");
	case MAT_T_DOUBLE:
		return i18n("Double");
	case MAT_T_INT64:
		return i18n("Int64");
	case MAT_T_UINT64:
		return i18n("UInt64");
	case MAT_T_MATRIX:
		return i18n("Matrix");
	case MAT_T_COMPRESSED:
		return i18n("Compressed");
	case MAT_T_UTF8:
		return i18n("UTF8");
	case MAT_T_UTF16:
		return i18n("UTF16");
	case MAT_T_UTF32:
		return i18n("UTF32");
	case MAT_T_STRING:
		return i18n("String");
	case MAT_T_CELL:
		return i18n("Cell");
	case MAT_T_STRUCT:
		return i18n("Struct");
	case MAT_T_ARRAY:
		return i18n("Array");
	case MAT_T_FUNCTION:
		return i18n("Function");
	}

	return i18n("Undefined");
}

#endif

/*!
    parses the content of the file \c fileName
*/
void MatioFilterPrivate::parse(const QString& fileName) {
#ifdef HAVE_MATIO
	DEBUG(Q_FUNC_INFO << ", fileName = " << qPrintable(fileName));

	mat_t *matfp = Mat_Open(qPrintable(fileName), MAT_ACC_RDONLY);
	if (!matfp) {
		DEBUG(Q_FUNC_INFO << ", ERROR getting file info")
		return;
	}

	// get names of all vars
        char **dir = Mat_GetDir(matfp, &varCount);
	DEBUG(Q_FUNC_INFO << ", found " << varCount << " vars")

	varsInfo.clear();
	for (size_t i = 0; i < varCount; ++i) {
		if (dir[i]) {
			QStringList info;

			//name
			info << QString(dir[i]);
			matvar_t* var = Mat_VarReadInfo(matfp, dir[i]);

			// rank
			const int rank = var->rank;
			info << QString::number(rank);

			// dims
			QString dims;
			for (int j = 0; j < rank; j++) {
				if (j > 0)
					dims += ", ";
				dims += QString::number(var->dims[j]);
			}
			info << dims;

			// class_type
			info << className(var->class_type);

			// data_type
			info << typeName(var->data_type);

			// complex/logical
			if (var->isComplex)
				info << i18n("Yes");
			else
				info << i18n("No");
			if (var->isLogical)
				info << i18n("Yes");
			else
				info << i18n("No");

			Mat_VarFree(var);
			varsInfo.append(info);
		}
	}
	Mat_Close(matfp);
#else
	Q_UNUSED(fileName)
#endif
}

/*!
 * generates the preview for the file \c fileName reading the provided number of \c lines.
 */
QVector<QStringList> MatioFilterPrivate::preview(const QString& fileName, int lines) {
	DEBUG(Q_FUNC_INFO)
	QVector<QStringList> dataStrings;

	//TODO
	// read lines lines of current var from filelName

	return dataStrings;
}

/*!
    reads the content of the current selected variable from file \c fileName to the data source \c dataSource.
    Uses the settings defined in the data source.
*/
QVector<QStringList> MatioFilterPrivate::readDataFromFile(const QString& fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode mode) {
	DEBUG(Q_FUNC_INFO)
	QVector<QStringList> dataStrings;

	if (currentVarName.isEmpty()) {
		DEBUG(Q_FUNC_INFO << ", no variable selected");
		return dataStrings;
	}

	return readCurrentVar(fileName, dataSource, mode);
}

/*!
    reads the content of the variable in the file \c fileName to a string (for preview) or to the data source.
*/
QVector<QStringList> MatioFilterPrivate::readCurrentVar(const QString& fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode mode, int lines) {
	QVector<QStringList> dataStrings;

	if (currentVarName.isEmpty()) {
		DEBUG(Q_FUNC_INFO << ", WARNING: current var name is empty!")
		return dataStrings << (QStringList() << i18n("No variable selected"));
	}
	DEBUG(Q_FUNC_INFO << ", current variable: " << STDSTRING(currentVarName));

#ifdef HAVE_MATIO
	mat_t *matfp = Mat_Open(qPrintable(fileName), MAT_ACC_RDONLY);
	if (!matfp)
		return dataStrings << (QStringList() << i18n("File not found"));

	// read info and data
	matvar_t* var = Mat_VarRead(matfp, qPrintable(currentVarName));
	if (!var)
		return dataStrings << (QStringList() << i18n("Variable not found"));

	if (var->rank == 2) {
		// read data
		//TODO: other types
		if (var->class_type != MAT_C_DOUBLE)
			return dataStrings << (QStringList() << i18n("Not implemented yet"));

		const double *data = static_cast<const double*>(var->data);
		const int sizeX = var->dims[0], sizeY = var->dims[1];
		for (int i = 0; i < sizeY; i++) {
			QStringList row;
		for (int j = 0; j < sizeX; j++) {
			row << QString::number(data[j + i*sizeX]);
		}
			dataStrings << row;
		}

		//TODO: handle sparse, struct, cell
		//TODO: handle other classes and types

	}
	//TODO
	if (var->rank > 2)
		return dataStrings << (QStringList() << i18n("Not implemented yet"));

	Mat_VarFree(var);
	Mat_Close(matfp);
#else
	Q_UNUSED(fileName)
	Q_UNUSED(dataSource)
	Q_UNUSED(mode)
	Q_UNUSED(lines)
#endif

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
