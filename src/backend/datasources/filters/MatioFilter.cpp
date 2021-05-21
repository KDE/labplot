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
// type - var data type, dtype - container data type
#define MAT_READ_VAR(type, dtype) \
	{ \
	const type *data = static_cast<const type*>(var->data); \
	if (dataSource) { \
		for (int i = 0; i < actualRows; i++) \
			for (int j = 0; j < actualCols; j++) \
				static_cast<QVector<dtype>*>(dataContainer[(int)(j-(size_t)startColumn+1)])->operator[](i-startRow+1) = data[i + j*actualRows]; \
	} else { /* preview */ \
		for (int i = 0; i < actualRows; i++) { \
			QStringList row; \
			for (int j = 0; j < actualCols; j++) \
				row << QString::number(data[i + j*actualRows]); \
			dataStrings << row; \
		} \
	} \
	}

// type - cell data type, dtype - container data type
#define MAT_READ_CELL(type, dtype) \
	{ \
	const type* data = (const type*)cell->data; \
	if (dataSource) { \
		for (unsigned int j = 0; j < cell->dims[1]; j++) \
			static_cast<QVector<dtype>*>(dataContainer[j])->operator[](i) = data[j]; \
		for (int j = cell->dims[1]; j < actualCols; j++) /* reset not defined values */ \
			static_cast<QVector<dtype>*>(dataContainer[j])->operator[](i) = qQNaN(); \
	} else { /* preview */ \
		QStringList row; \
		if (cell->dims[1] == 0)	/* handle empty cells */ \
			row << QString(); \
		for (unsigned int j = 0; j < cell->dims[1]; j++) \
			row << QString::number(data[j]); \
		dataStrings << row; \
	} \
	}

//////////////////////////////////////////////////////////////////////

/*!
	\class MatioFilter
	\brief Manages the import/export of data from/to a Matio file.

	\ingroup datasources
*/
MatioFilter::MatioFilter():AbstractFileFilter(FileType::MATIO), d(new MatioFilterPrivate(this)) {}

MatioFilter::~MatioFilter() = default;

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
	if (dir && n < 10) {	// only show variable info when there are not too many
		info += i18n("Variables:");
		for (size_t i = 0; i < n; ++i) {
			if (dir[i]) {
				info += " \"" + QString(dir[i]) + "\"";
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

AbstractColumn::ColumnMode MatioFilterPrivate::typeMode(matio_types dataType) {
	switch (dataType) {
	case MAT_T_INT8:
	case MAT_T_UINT8:
	case MAT_T_INT16:
	case MAT_T_UINT16:
	case MAT_T_INT32:
	case MAT_T_UINT32:
		return AbstractColumn::ColumnMode::Integer;
		break;
	case MAT_T_INT64:
	case MAT_T_UINT64:
		return AbstractColumn::ColumnMode::BigInt;
		break;
	case MAT_T_SINGLE:
	case MAT_T_DOUBLE:
	case MAT_T_UNKNOWN:
	case MAT_T_MATRIX:
	case MAT_T_COMPRESSED:
	case MAT_T_UTF8:
	case MAT_T_UTF16:
	case MAT_T_UTF32:
	case MAT_T_STRING:
	case MAT_T_CELL:
	case MAT_T_STRUCT:
	case MAT_T_ARRAY:
	case MAT_T_FUNCTION:
		break;
	}

	return AbstractColumn::ColumnMode::Numeric;
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

	int actualRows = 0, actualCols = 0;
	int columnOffset = 0;
	std::vector<void*> dataContainer;
	if (var->rank == 2) {	// rank is always >= 2
		// read data
		actualRows = var->dims[0], actualCols = var->dims[1];
		if (actualRows == 1) { // only one row: read as column
			actualRows = actualCols;
			actualCols = 1;
		}

		// column modes
		QVector<AbstractColumn::ColumnMode> columnModes;
		columnModes.resize(actualCols);

		switch (var->class_type) {
		case MAT_C_CHAR:
		case MAT_C_INT8:
		case MAT_C_UINT8:
		case MAT_C_INT16:
		case MAT_C_UINT16:
		case MAT_C_INT32:
		case MAT_C_UINT32:
			for (int i = 0; i < actualCols; i++)
				columnModes[i] = AbstractColumn::ColumnMode::Integer;
			break;
		case MAT_C_INT64:
		case MAT_C_UINT64:
			for (int i = 0; i < actualCols; i++)
				columnModes[i] = AbstractColumn::ColumnMode::BigInt;
			break;
		case MAT_C_DOUBLE:
		case MAT_C_SINGLE:
			for (int i = 0; i < actualCols; i++)
				columnModes[i] = AbstractColumn::ColumnMode::Numeric;
			break;
		case MAT_C_EMPTY:
			return dataStrings << (QStringList() << i18n("Empty"));
			break;
		case MAT_C_CELL: {
			DEBUG(Q_FUNC_INFO << ", found CELL. name = " << var->name << ", nbytes = " << var->nbytes << ", size = " << var->data_size)
			// Each element of the cell array can be a different type: only import numbers atm
			if (var->nbytes == 0 || var->data_size == 0 || var->data == NULL)
				break;
			const int ncells = var->nbytes / var->data_size;
			DEBUG(Q_FUNC_INFO << ", found " << ncells << " cells")
			auto mode = AbstractColumn::ColumnMode::Integer;	// start from here
			unsigned int colCount = 0;
			for (int i = 0; i < ncells; i++) {
				matvar_t* cell = Mat_VarGetCell(var, i);
				if (cell->rank == 2 && cell->dims[0] == 1 && cell->class_type != MAT_C_CHAR) {	// read only rank 2 and cells with one row, omit strings
					if (colCount < cell->dims[1])	// find max column count
					       colCount = cell->dims[1];

					// promote type if found
					if (mode == AbstractColumn::ColumnMode::Integer && (cell->class_type == MAT_C_INT64 || cell->class_type == MAT_C_UINT64))
						mode = AbstractColumn::ColumnMode::BigInt;
					else if ((mode == AbstractColumn::ColumnMode::Integer || mode == AbstractColumn::ColumnMode::BigInt) && 
							(cell->class_type == MAT_C_DOUBLE || cell->class_type == MAT_C_SINGLE) )
						mode = AbstractColumn::ColumnMode::Numeric;
				}
			}
			DEBUG(Q_FUNC_INFO << ", set mode to " << AbstractColumn::modeName(mode).toStdString())
			if (dataSource) {
				//change data source settings
				actualRows = ncells;
				actualCols = colCount;
				columnModes.resize(actualCols);
			}
			break;
		}
		case MAT_C_SPARSE:
			DEBUG(Q_FUNC_INFO << ", found SPARSE. name = " << var->name << ", type = " << typeName(var->data_type).toStdString()  << ", nbytes = " << var->nbytes << ", size = " << var->data_size)
			DEBUG(Q_FUNC_INFO << ", rank " << var->rank << ", dim = " << var->dims[0] << " x " << var->dims[1])

			if (dataSource) {
				//change data source settings
				columnModes.resize(actualCols);
				auto mode = typeMode(var->data_type);
				for (int i = 0; i < actualCols; i++)
					columnModes[i] = mode;
			}
			break;
		case MAT_C_STRUCT:	//TODO: how to import? see users_guide
			DEBUG(Q_FUNC_INFO << ", found STRUCT. name = " << var->name << ", nbytes = " << var->nbytes << ", size = " << var->data_size)
			DEBUG(Q_FUNC_INFO << ", data type = " << typeName(var->data_type).toStdString())
			return dataStrings << (QStringList() << i18n("Not implemented yet"));
			break;
		case MAT_C_OBJECT:	// not available (not supported by matio yet)
			DEBUG(Q_FUNC_INFO << ", found OBJECT. name = " << var->name << ", nbytes = " << var->nbytes << ", size = " << var->data_size)
			return dataStrings << (QStringList() << i18n("Not implemented yet"));
			break;
		case MAT_C_FUNCTION:	// not available (not supported by matio yet)
			DEBUG(Q_FUNC_INFO << ", found FUNCTION. name = " << var->name << ", nbytes = " << var->nbytes << ", size = " << var->data_size)
			QDEBUG(Q_FUNC_INFO << ", data: " << (const char *)var->data)
		case MAT_C_OPAQUE:
			return dataStrings << (QStringList() << i18n("Not implemented yet"));
		}

		//prepare import
		QStringList vectorNames;
		if (dataSource)
			columnOffset = dataSource->prepareImport(dataContainer, mode, actualRows, actualCols, vectorNames, columnModes);

		// read data
		switch (var->class_type) {
		case MAT_C_CHAR:
			MAT_READ_VAR(char, int);
			break;
		case MAT_C_DOUBLE:
			MAT_READ_VAR(double, double);
			break;
		case MAT_C_SINGLE:
			MAT_READ_VAR(float, double);
			break;
		case MAT_C_INT8:
			MAT_READ_VAR(qint8, int);
			break;
		case MAT_C_UINT8:
			MAT_READ_VAR(quint8, int);
			break;
		case MAT_C_INT16:
			MAT_READ_VAR(qint16, int);
			break;
		case MAT_C_UINT16:
			MAT_READ_VAR(quint16, int);
			break;
		case MAT_C_INT32:
			MAT_READ_VAR(qint32, int);
			break;
		case MAT_C_UINT32:
			MAT_READ_VAR(quint32, int);
			break;
		case MAT_C_INT64:
			MAT_READ_VAR(qint64, qint64);
			break;
		case MAT_C_UINT64:
			MAT_READ_VAR(quint64, qint64);
			break;
		case MAT_C_EMPTY:
			break;
		case MAT_C_CELL: {
			if (var->nbytes == 0 || var->data_size == 0 || var->data == NULL)
				break;
			const int ncells = var->nbytes / var->data_size;
			for (int i = 0; i < ncells; i++) {
				matvar_t* cell = Mat_VarGetCell(var, i);
				// cell->name can be NULL
				QString dims;
				for (int j = 0; j < cell->rank; j++)
					dims += QString::number(cell->dims[j]) + " ";
				DEBUG(Q_FUNC_INFO << ", cell " << i << " : class = " << className(cell->class_type).toStdString()
						<< ", type = " << typeName(cell->data_type).toStdString() << ", rank = "
						<< cell->rank << ", dims = " << dims.toStdString() << ", nbytes = " << cell->nbytes << ", size = " << cell->data_size)

				// read cell data (see MAT_READ_VAR)
				if (cell->rank == 2 && cell->dims[0] <= 1) {	// read only rank 2 and cells with zero/one row
					switch(cell->class_type) {
					case MAT_C_CHAR:	// strings are not imported (yet)
						if (dataSource) {
							for (int j = 0; j < actualCols; j++)	// reset not defined values
								static_cast<QVector<double>*>(dataContainer[j])->operator[](i) = qQNaN();
						} else {
							QStringList row;
							// see mat.c for supported data types
							if (cell->data_type == MAT_T_UINT16 || cell->data_type == MAT_T_INT16)
								row << QString::fromUtf16((const mat_uint16_t*)cell->data);
							else if (cell->data_type == MAT_T_UTF8)
								row << QString::fromUtf8((const char*)cell->data);
							else
								row << QString((const char*)cell->data);
							dataStrings << row;
						}
						break;
					case MAT_C_DOUBLE:
						MAT_READ_CELL(double, double)
						break;
					case MAT_C_SINGLE:
						MAT_READ_CELL(float, double)
						break;
					case MAT_C_INT8:
						MAT_READ_CELL(qint8, int);
						break;
					case MAT_C_UINT8:
						MAT_READ_CELL(quint8, int);
						break;
					case MAT_C_INT16:
						MAT_READ_CELL(qint16, int);
						break;
					case MAT_C_UINT16:
						MAT_READ_CELL(quint16, int);
						break;
					case MAT_C_INT32:
						MAT_READ_CELL(qint32, int);
						break;
					case MAT_C_UINT32:
						MAT_READ_CELL(quint32, int);
						break;
					case MAT_C_INT64:
						MAT_READ_CELL(qint64, qint64);
						break;
					case MAT_C_UINT64:
						MAT_READ_CELL(quint64, qint64);
						break;
					case MAT_C_CELL:	// TODO ...
					case MAT_C_EMPTY:
					case MAT_C_STRUCT:
					case MAT_C_OBJECT:
					case MAT_C_SPARSE:
					case MAT_C_FUNCTION:
					case MAT_C_OPAQUE:
						break;
					}
				}
			}
			break;
		}
		case MAT_C_SPARSE: {
			mat_sparse_t* sparse = (mat_sparse_t*)var->data;
			size_t stride = Mat_SizeOf(var->data_type);
			//DEBUG(Q_FUNC_INFO << ", stride = " << stride << ", njc = " << sparse->njc << ", ndata = " << sparse->ndata)

			//TODO: complex
			//TODO: other columnModes (s.a.)

			double *data = (double*)sparse->data;
			if (dataSource) {
				// set default values
				for (int i = 0; i < actualRows; i++)
					for (int j = 0; j < actualCols; j++)
						static_cast<QVector<double>*>(dataContainer[j])->operator[](i) = 0;

				for (size_t i = 0; i < (size_t)sparse->njc - 1; i++)
					for (size_t j = sparse->jc[i]; j < (size_t)sparse->jc[i + 1] && j < (size_t)sparse->ndata; j++)
						//DEBUG(Q_FUNC_INFO << ", (" << sparse->ir[j] + 1 << "," << i + 1 << ") = " << *(double*)(data + j * stride))
						static_cast<QVector<double>*>(dataContainer[(int)(sparse->ir[j]-(size_t)startColumn+1)])->operator[](i-startRow+1)
							= *(data + j * stride/sizeof(double));
			} else {	// preview
				QVector<QVector<double>> matrix;
				// set default values
				for (size_t i = 0; i < var->dims[0]; i++) {
					QVector<double> tmp;
					for (size_t j = 0; j < var->dims[1]; j++)
						tmp.append(0);
					matrix.append(tmp);
				}

				for (size_t i = 0; i < (size_t)sparse->njc - 1; i++)
					for (size_t j = sparse->jc[i]; j < (size_t)sparse->jc[i + 1] && j < (size_t)sparse->ndata; j++)
						//DEBUG(Q_FUNC_INFO << ", (" << sparse->ir[j] + 1 << "," << i + 1 << ") = " << *(double*)(data + j * stride))
						matrix[sparse->ir[j]][i] = *(data + j * stride/sizeof(double));

				for (size_t i = 0; i < var->dims[1]; i++) {
					QStringList row;
					for (size_t j = 0; j < var->dims[0]; j++)
						row << QString::number(matrix[j][i]);
					dataStrings << row;
				}
			}

			break;
		}
		case MAT_C_STRUCT:	// s.a.
		case MAT_C_OBJECT:	// unsupported (s.a.)
		case MAT_C_FUNCTION:	// unsupported (s.a.)
		case MAT_C_OPAQUE:	// ???
			break;
		}
	}
	if (var->rank > 2)	// TODO
		return dataStrings << (QStringList() << i18n("Not implemented yet"));

	Mat_VarFree(var);
	Mat_Close(matfp);

	// TODO: why 1 row?
	if (dataSource)
		dataSource->finalizeImport(columnOffset, 1, actualCols, QString(), mode);
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
	//TODO: writing MAT files not implemented yet
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
