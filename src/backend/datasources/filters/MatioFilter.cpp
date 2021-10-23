/*
    File                 : MatioFilter.cpp
    Project              : LabPlot
    Description          : Matio I/O-filter
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2021 Stefan Gerlach <stefan.gerlach@uni.kn>
    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "MatioFilter.h"
#include "MatioFilterPrivate.h"
#include "backend/matrix/Matrix.h"
#include "backend/core/column/Column.h"
#include "backend/lib/macros.h"
#include "backend/lib/trace.h"
#include "backend/lib/XmlStreamReader.h"

#include <KLocalizedString>
#include <QProcess>

///////////// macros ///////////////////////////////////////////////

// see NetCDFFilter.cpp
// type - var data type, dtype - container data type
#define MAT_READ_VAR(type, dtype) \
	{ \
	if (var->isComplex) { \
		mat_complex_split_t* complex_data = (mat_complex_split_t*)var->data; \
		type* re = (type*)complex_data->Re; \
		type* im = (type*)complex_data->Im; \
		if (dataSource) { \
			for (size_t i = 0; i < actualRows; i++) \
				for (size_t j = 0; j < actualCols/2; j++) { \
					const size_t index = i + startRow - 1 + (j + startColumn - 1) * rows; \
					static_cast<QVector<dtype>*>(dataContainer[(int)(2*j)])->operator[](i) = re[index]; \
					static_cast<QVector<dtype>*>(dataContainer[(int)(2*j + 1)])->operator[](i) = im[index]; \
				} \
		} else { /* preview */ \
			QStringList header; \
			for (size_t j = 0; j < actualCols/2; j++) { \
				header << QLatin1String("Re ") + QString::number(j+1) << QLatin1String("Im ") + QString::number(j+1); \
			} \
			dataStrings << header; \
			for (size_t i = 0; i < qMin(actualRows, lines); i++) { \
				QStringList row; \
				for (size_t j = 0; j < actualCols/2; j++) { \
					const size_t index = i + startRow - 1 + (j + startColumn - 1) * rows; \
					row << QString::number(re[index]) << QString::number(im[index]); \
				} \
				dataStrings << row; \
			} \
		} \
	} else { \
		const type *data = static_cast<const type*>(var->data); \
		if (dataSource) { \
			for (size_t i = 0; i < actualRows; i++) \
				for (size_t j = 0; j < actualCols; j++) { \
					const size_t index = i + startRow - 1 + (j + startColumn - 1) * rows; \
					static_cast<QVector<dtype>*>(dataContainer[(int)j + (dynamic_cast<Matrix*>(dataSource) ? columnOffset : 0)])->operator[](i) = data[index]; \
				} \
		} else { /* preview */ \
			for (size_t i = 0; i < qMin(actualRows, lines); i++) { \
				QStringList row; \
				for (size_t j = 0; j < actualCols; j++) \
					row << QString::number(data[i + startRow - 1 + (j + startColumn - 1) * rows]); \
				dataStrings << row; \
			} \
		} \
	} \
	}

// type - cell data type, dtype - container data type
// TODO: complex
#define MAT_READ_CELL(type, dtype) \
	{ \
	const type* data = (const type*)cell->data; \
	if (dataSource) { \
		if (i + startRow - 1 < cellsize) \
			static_cast<QVector<dtype>*>(dataContainer[j])->operator[](i) = data[i + startRow - 1]; \
		else \
			static_cast<QVector<dtype>*>(dataContainer[j])->operator[](i) = qQNaN(); \
	} else { /* preview */ \
		if (i + startRow - 1 < cellsize) \
			row << QString::number(data[i + startRow - 1]); \
		else \
			row << QString(); \
	} \
	}

// type - sparse data type, dtype - container data type
#define MAT_READ_SPARSE(type, dtype) \
	{ \
	/* set default values */ \
	QVector<QVector<type>> matrix; /* for preview */ \
	if (dataSource) { \
		for (size_t i = 0; i < actualRows; i++) \
			for (size_t j = 0; j < actualCols; j++) \
				static_cast<QVector<dtype>*>(dataContainer[j])->operator[](i) = 0; \
	} else { /* preview (full matrix need to store values) */ \
		for (size_t i = 0; i < actualEndRow; i++) { \
			QVector<type> tmp; \
			for (size_t j = 0; j < actualEndColumn; j++) \
				tmp.append(0); \
			matrix.append(tmp); \
		} \
	} \
	if (var->isComplex) { \
		mat_complex_split_t* complex_data = (mat_complex_split_t*)sparse->data; \
		type* re = (type*)complex_data->Re; \
		type* im = (type*)complex_data->Im; \
		if (dataSource) { \
			for (size_t i = 0; i < qMin((size_t)sparse->njc - 1, actualCols/2); i++) \
				for (size_t j = sparse->jc[i]; j < (size_t)sparse->jc[i + 1] && j < (size_t)sparse->ndata; j++) { \
					if (sparse->ir[j] >= (size_t)startRow - 1 && sparse->ir[j] < actualEndRow) { /* only read requested rows */ \
						static_cast<QVector<dtype>*>(dataContainer[(int)2*i])->operator[](sparse->ir[j] - startRow + 1) \
								= *(re + j * stride/sizeof(type)); \
						static_cast<QVector<dtype>*>(dataContainer[(int)2*i+1])->operator[](sparse->ir[j] - startRow + 1) \
								= *(im + j * stride/sizeof(type)); \
					} \
				} \
		} else { /* preview */ \
			for (size_t i = 0; i < qMin((size_t)sparse->njc - 1, actualEndColumn/2 + 1); i++) \
				for (size_t j = sparse->jc[i]; j < (size_t)sparse->jc[i + 1] && j < (size_t)sparse->ndata; j++) { \
					if (sparse->ir[j] >= (size_t)startRow - 1 && sparse->ir[j] < actualEndRow) { /* only read requested rows */ \
						if (2*i < actualEndColumn) /* Im may be last col */ \
							matrix[sparse->ir[j]][2*i] = *(re + j * stride/sizeof(type)); \
						if (2*i+1 < actualEndColumn) /* Re may be last col */ \
							matrix[sparse->ir[j]][2*i + 1] = *(im + j * stride/sizeof(type)); \
					} \
				} \
		} \
	} else { /* real */ \
		type* data = (type*)sparse->data; \
		if (dataSource) { \
			for (size_t i = startColumn - 1; i < qMin((size_t)sparse->njc - 1, actualEndColumn); i++) \
				for (size_t j = sparse->jc[i]; j < (size_t)sparse->jc[i + 1] && j < (size_t)sparse->ndata; j++) { \
					if (sparse->ir[j] >= (size_t)startRow - 1 && sparse->ir[j] < actualEndRow) /* only read requested rows */ \
						static_cast<QVector<dtype>*>(dataContainer[(int)i - startColumn + 1])->operator[](sparse->ir[j] - startRow + 1) \
								= *(data + j * stride/sizeof(type)); \
			} \
		} else { /* preview */ \
			for (size_t i = 0; i < qMin((size_t)sparse->njc - 1, actualEndColumn); i++) \
				for (size_t j = sparse->jc[i]; j < (size_t)sparse->jc[i + 1] && j < (size_t)sparse->ndata; j++) \
					if (sparse->ir[j] < actualEndRow) /* don't read beyond last row */ \
						matrix[sparse->ir[j]][i] = *(data + j * stride/sizeof(type)); \
		} \
	} \
	if (!dataSource) { /* preview */ \
		for (size_t i = startRow - 1; i < qMin(actualEndRow, lines); i++) { \
			QStringList row; \
			for (size_t j = startColumn - 1; j < actualEndColumn; j++) \
				row << QString::number(matrix[i][j]); \
			dataStrings << row; \
		} \
	} \
	}

// type - struct data type
#define MAT_READ_STRUCT(type) \
	{ \
	if (fields[i]->isComplex) { \
		mat_complex_split_t* complex_data = (mat_complex_split_t*)fields[i]->data; \
		type* re = (type*)complex_data->Re; \
		type* im = (type*)complex_data->Im; \
		\
		DEBUG(Q_FUNC_INFO << "  rank = 2 (" << fields[i]->dims[0] << " x " << fields[i]->dims[1] << ")") \
		if (dataSource) { \
			for (size_t j = 0; j < actualRows; j++) { \
				static_cast<QVector<type>*>(dataContainer[colIndex])->operator[](j) = re[j + startRow - 1]; \
				static_cast<QVector<type>*>(dataContainer[colIndex+1])->operator[](j) = im[j + startRow - 1]; \
			} \
		} else { /* preview */ \
			for (size_t j = 0; j < qMin(actualRows, lines); j++) { \
				/* TODO: use when complex column mode is supported */ \
				/* if (im[j] < 0) \
					dataStrings[j+1][field] = QString::number(re[j]) + QLatin1String(" - ") \
								+ QString::number(fabs(im[j])) + QLatin1String("i"); \
				else if (im[j] == 0) \
					dataStrings[j+1][field] = QString::number(re[j]); \
				else if (re[j] == 0) \
					dataStrings[j+1][field] = QString::number(im[j]) + QLatin1String("i"); \
				else \
					dataStrings[j+1][field] = QString::number(re[j]) + QLatin1String(" + ") \
								+ QString::number(im[j]) + QLatin1String("i"); \
				*/ \
				dataStrings[j+1][colIndex] = QString::number(re[j + startRow - 1]); \
				dataStrings[j+1][colIndex+1] = QString::number(im[j + startRow - 1]); \
			} \
		} \
		colIndex++;	/* complex uses two columns atm */ \
	} else { /* real */ \
		type* data = (type*)fields[i]->data; \
		DEBUG(Q_FUNC_INFO << "  rank = 2 (" << fields[i]->dims[0] << " x " << fields[i]->dims[1] << ")") \
		if (dataSource) { \
			for (size_t j = 0; j < actualRows; j++) \
				static_cast<QVector<type>*>(dataContainer[colIndex])->operator[](j) = data[j + startRow - 1]; \
		} else { /* preview */ \
			for (size_t j = 0; j < qMin(actualRows, lines); j++) \
				dataStrings[j+1][colIndex] = QString::number(data[j + startRow - 1]); \
		} \
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
	return d->readCurrentVar(fileName, dataSource, importMode, (size_t)lines);
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
void MatioFilter::loadFilterSettings(const QString& /*filterName*/) {
}

/*!
  saves the current settings as a new filter with the name \c filterName
*/
void MatioFilter::saveFilterSettings(const QString& /*filterName*/) const {
}

///////////////////////////////////////////////////////////////////////

void MatioFilter::setCurrentVarName(const QString& name) {
	d->currentVarName = name;
	d->selectedVarNames = QStringList() << name;
}
void MatioFilter::setSelectedVarNames(const QStringList& names) {
	d->currentVarName = names.first();
	d->selectedVarNames = names;
}
const QStringList MatioFilter::selectedVarNames() const {
	return d->selectedVarNames;
}
size_t MatioFilter::varCount() const {
	return d->varCount;
}
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

AbstractColumn::ColumnMode MatioFilterPrivate::classMode(matio_classes classType) {
	switch (classType) {
	case MAT_C_INT8:
	case MAT_C_UINT8:
	case MAT_C_INT16:
	case MAT_C_UINT16:
	case MAT_C_INT32:
	case MAT_C_UINT32:
		return AbstractColumn::ColumnMode::Integer;
		break;
	case MAT_C_INT64:
	case MAT_C_UINT64:
		return AbstractColumn::ColumnMode::BigInt;
		break;
	case MAT_C_CHAR:
		return AbstractColumn::ColumnMode::Text;
		break;
	case MAT_C_EMPTY:
	case MAT_C_CELL:
	case MAT_C_STRUCT:
	case MAT_C_OBJECT:
	case MAT_C_SPARSE:
	case MAT_C_DOUBLE:
	case MAT_C_SINGLE:
	case MAT_C_FUNCTION:
	case MAT_C_OPAQUE:
		break;
	}

	return AbstractColumn::ColumnMode::Double;

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

	return AbstractColumn::ColumnMode::Double;
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
			// Mat_VarReadInfo() does not determine the data type of sparse
			// Don't use Mat_VarRead(matfp, dir[i]). It searches the whole file for every var
			matvar_t* var = Mat_VarReadNext(matfp);

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
    (Not used for preview)
    Uses the settings defined in the data source.
*/
void MatioFilterPrivate::readDataFromFile(const QString& fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode mode) {
	PERFTRACE(Q_FUNC_INFO);
	QVector<QStringList> dataStrings;

	if (currentVarName.isEmpty()) {
		DEBUG(Q_FUNC_INFO << ", no variable selected");
		return;
	}

	QDEBUG(Q_FUNC_INFO << ", selected var names:" << selectedVarNames)
#ifdef HAVE_MATIO
	//open file only once
	if (!selectedVarNames.isEmpty())
		matfp = Mat_Open(qPrintable(fileName), MAT_ACC_RDONLY);
#endif
	for (const auto& var : selectedVarNames) {
		currentVarName = var;
		readCurrentVar(fileName, dataSource, mode);
		mode = AbstractFileFilter::ImportMode::Append;	// append other vars
	}
#ifdef HAVE_MATIO
	if (matfp) {	// only if opened
		Mat_Close(matfp);
		matfp = NULL;
	}
#endif
}

/*!
    reads the content of the current variable in the file \c fileName to a string (for preview) or to the data source.
*/
QVector<QStringList> MatioFilterPrivate::readCurrentVar(const QString& fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode mode, size_t lines) {
	PERFTRACE(Q_FUNC_INFO);
	QVector<QStringList> dataStrings;

	if (currentVarName.isEmpty()) {
		DEBUG(Q_FUNC_INFO << ", WARNING: current var name is empty!")
		return dataStrings << (QStringList() << i18n("No variable selected"));
	}
	DEBUG(Q_FUNC_INFO << ", current variable: " << STDSTRING(currentVarName));

#ifdef HAVE_MATIO
	bool openedFile = false;
	if (!matfp) {	// file not open
		matfp = Mat_Open(qPrintable(fileName), MAT_ACC_RDONLY);
		openedFile = true;
	}
	if (!matfp)	// open failed
		return dataStrings << (QStringList() << i18n("File not found"));

	// read info and data
	matvar_t* var = Mat_VarReadNext(matfp);	// try next first (faster)
	if (QString(var->name) != currentVarName)
		var = Mat_VarRead(matfp, qPrintable(currentVarName));
	//else
	//	DEBUG(Q_FUNC_INFO << ", was NEXT!")
	if (!var)
		return dataStrings << (QStringList() << i18n("Variable not found"));
	if (!var->data)
		return dataStrings << (QStringList() << i18n("Variable contains no data"));

	//DEBUG(Q_FUNC_INFO << ", start/end row = " << startRow << '/' << endRow)
	//DEBUG(Q_FUNC_INFO << ", start/end col = " << startColumn << '/' << endColumn)

	size_t actualRows = 0, actualCols = 0;
	int columnOffset = 0;
	std::vector<void*> dataContainer;
	QStringList vectorNames;
	if (var->rank == 2) {	// rank is always >= 2
		// read data
		size_t rows = var->dims[0], cols = var->dims[1];
		if (var->class_type == MAT_C_CELL)
			cols = var->nbytes / var->data_size;
		else if (rows == 1) {	// only one row: read as column
			rows = cols;
			cols = 1;
		}
		size_t actualEndRow = (endRow == -1 || endRow > (int)rows) ? rows : endRow;
		actualRows = actualEndRow - startRow + 1;
		size_t actualEndColumn = (endColumn == -1 || endColumn > (int)cols) ? cols : endColumn;
		actualCols = actualEndColumn - startColumn + 1;
		if (var->class_type == MAT_C_STRUCT) {
			if (endRow != -1)
				actualRows = endRow - startRow + 1;
		} else if (var->class_type == MAT_C_CELL) {	// calculated later
			actualEndRow = 0;
			actualRows = 0;
		}
		//DEBUG(Q_FUNC_INFO << ", start row = " << startRow << ", actual end row = " << actualEndRow << ", actual rows = " << actualRows)
		//DEBUG(Q_FUNC_INFO << ", start col = " << startColumn << ", actual end col = " << actualEndColumn << ", actual cols = " << actualCols)

		if (lines == 0)
			lines = actualRows;
		// double the number of cols for complex data (not for CELL or STRUCT)
		if (var->isComplex && var->class_type != MAT_C_CELL && var->class_type != MAT_C_STRUCT) {
			if (endColumn == -1) {
				actualCols *= 2;
				actualEndColumn *= 2;
			}
			for (size_t j = startColumn - 1; j < actualEndColumn; j++) {
				if (j % 2)
					vectorNames << QLatin1String("Im ") + QString::number(j/2 + 1);
				else
					vectorNames << QLatin1String("Re ") + QString::number(j/2 + 1);
			}
		}

		// column modes
		QVector<AbstractColumn::ColumnMode> columnModes;
		columnModes.resize(actualCols);

		// A: set column modes
		switch (var->class_type) {
		case MAT_C_CHAR:
		case MAT_C_INT8:
		case MAT_C_UINT8:
		case MAT_C_INT16:
		case MAT_C_UINT16:
		case MAT_C_INT32:
		case MAT_C_UINT32:
			for (auto& col : columnModes)
				col = AbstractColumn::ColumnMode::Integer;
			break;
		case MAT_C_INT64:
		case MAT_C_UINT64:
			for (auto& col : columnModes)
				col = AbstractColumn::ColumnMode::BigInt;
			break;
		case MAT_C_DOUBLE:
		case MAT_C_SINGLE:
			for (auto& col : columnModes)
				col = AbstractColumn::ColumnMode::Double;
			break;
		case MAT_C_EMPTY:
			return dataStrings << (QStringList() << i18n("Empty"));
			break;
		case MAT_C_CELL: {
			DEBUG(Q_FUNC_INFO << ", found CELL. name = " << var->name << ", nbytes = " << var->nbytes << ", size = " << var->data_size)
			// Each element of the cell array can be a different type: one column per cell
			if (var->nbytes == 0 || var->data_size == 0 || var->data == nullptr)
				break;
			//const int ncells = var->nbytes / var->data_size;
			//DEBUG(Q_FUNC_INFO << ", found " << ncells << " cells")
			columnModes.resize(actualCols);

			// find out number of rows
			for (size_t i = 0; i < actualCols; i++) {
				matvar_t* cell = Mat_VarGetCell(var, i + startColumn - 1);
				if (cell->rank == 2 && cell->dims[0] <= 1) {	// read only rank 2 and cells with one row, omit strings
					if (rows < cell->dims[1] && cell->class_type != MAT_C_CHAR)	// find max row count
					       rows = cell->dims[1];

					if (cell->name)
						vectorNames << cell->name;
					else
						vectorNames << QLatin1String("Column ") + QString::number(i+1);

					auto mode = classMode(cell->class_type);
					if (dynamic_cast<Matrix*>(dataSource) && mode == AbstractColumn::ColumnMode::Text)	// text not supported for matrix
						mode = AbstractColumn::ColumnMode::Double;

					columnModes[i] = mode;
				}
			}
			// calculate from startRow and endRow
			actualEndRow = (endRow == -1 || endRow > (int)rows) ? rows : endRow;
			actualRows = actualEndRow - startRow + 1;
			DEBUG(Q_FUNC_INFO << ", start row = " << startRow <<  ", actual end row = " << actualEndRow << ", actual rows = " << actualRows)
			break;
		}
		case MAT_C_SPARSE:
			DEBUG(Q_FUNC_INFO << ", found SPARSE. name = " << var->name << ", type = " << typeName(var->data_type).toStdString()  << ", nbytes = " << var->nbytes << ", size = " << var->data_size)
			DEBUG(Q_FUNC_INFO << ", rank " << var->rank << ", dim = " << var->dims[0] << " x " << var->dims[1])

			if (dataSource) {
				columnModes.resize(actualCols);
				auto mode = typeMode(var->data_type);
				for (size_t i = 0; i < actualCols; i++)
					columnModes[i] = mode;
			}
			break;
		case MAT_C_STRUCT: {
			DEBUG(Q_FUNC_INFO << ", found STRUCT. name = " << var->name << ", nbytes = " << var->nbytes << ", size = " << var->data_size)
			DEBUG(Q_FUNC_INFO << ", data type = " << typeName(var->data_type).toStdString() << ", dims = " << var->dims[0] << " x " << var->dims[1])
			const int nelem = var->dims[0]*var->dims[1];
			const int nfields = Mat_VarGetNumberOfFields(var);
			DEBUG(Q_FUNC_INFO << ", nelements = " << nelem << ", nfields = " << nfields)
			if (endColumn == -1)
				endColumn = nfields;
			actualCols = endColumn - startColumn + 1;

			if (nfields <= 0)
				return dataStrings << (QStringList() << i18n("Struct contains no fields"));

			if (nelem < 1) {
				DEBUG(Q_FUNC_INFO << ", no elements")
				char *const *fieldnames = Mat_VarGetStructFieldnames(var);
				if (fieldnames) {
					for (int i = 0; i < nfields; i++ )
						DEBUG(Q_FUNC_INFO << ", field " << i << " name = " << fieldnames[i])
				}
			}

			//set actualRows
			matvar_t **fields = (matvar_t **)var->data;
			for (int i = startColumn - 1; i < qMin(nfields, endColumn); i++) {
				if (fields[i]->name) {
					//TODO: not needed when supporting complex column mode
					if (fields[i]->isComplex)
						vectorNames << fields[i]->name + QLatin1String(" - Re") << fields[i]->name + QLatin1String(" - Im");
					else
						vectorNames << fields[i]->name;
				} else
					vectorNames << QLatin1String("Column ") + QString::number(i);
			}
			for (int i = 0; i < nfields * nelem; i++) {
				const int field = i % nfields;
				if (field < startColumn - 1 || field > endColumn - 1)
					continue;

				if (fields[i]->rank == 2) {
					DEBUG(Q_FUNC_INFO << ", dims = " << fields[i]->dims[0] << " x " << fields[i]->dims[1])
					size_t size;
					if (fields[i]->class_type == MAT_C_CHAR)	// read as string
						size = fields[i]->dims[0];
					else {
						if (endRow == -1)
							size = fields[i]->dims[0] * fields[i]->dims[1] - startRow + 1;
						else
							size = qMin(fields[i]->dims[0] * fields[i]->dims[1], actualRows);
					}

					if (actualRows < size)
						actualRows = size;
				} else
					DEBUG(Q_FUNC_INFO << "  rank = " << fields[i]->rank)

				//TODO: not needed when supporting complex column mode
				if (fields[i]->isComplex)	// if complex: add column
					actualCols++;
			}
			DEBUG(Q_FUNC_INFO << ", Setting rows/cols to: " << actualRows << "/" << actualCols)
			if (dataSource) {
				columnModes.resize(actualCols);
				int index = 0;
				for (int i = startColumn - 1; i < qMin(nfields, endColumn); i++) {
					auto mode = classMode(fields[i]->class_type);
					if (dynamic_cast<Matrix*>(dataSource) && mode == AbstractColumn::ColumnMode::Text)	// text not supported for matrix
						mode = AbstractColumn::ColumnMode::Double;

					//TODO: not needed when supporting complex column mode
					if (fields[i]->isComplex)	// additional column for complex
						columnModes[index++] = mode;
					columnModes[index++] = mode;
				}
			} else {	// preview
				if (actualRows > lines)
					actualRows = lines;

				dataStrings.resize(actualRows + 1);	// + 1 for header
				for (auto& string : dataStrings) {
					string.reserve(actualCols);
					for (size_t j = 0; j < actualCols; j++)
						string << QString();
				}
			}
			break;
		}
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
		if (dataSource)
			columnOffset = dataSource->prepareImport(dataContainer, mode, actualRows, actualCols, vectorNames, columnModes);
		DEBUG(Q_FUNC_INFO << ", column offset = " << columnOffset)

		// B: read data
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

			//TODO: complex not supported yet

			for (size_t i = 0; i < actualCols; i++) {
				matvar_t* cell = Mat_VarGetCell(var, i + startColumn - 1);
				// cell->name can be NULL
				QString dims;
				for (int j = 0; j < cell->rank; j++)
					dims += QString::number(cell->dims[j]) + " ";
				DEBUG(Q_FUNC_INFO << ", cell " << i+1 << " : class = " << className(cell->class_type).toStdString()
						<< ", type = " << typeName(cell->data_type).toStdString() << ", rank = "
						<< cell->rank << ", dims = " << dims.toStdString() << ", nbytes = " << cell->nbytes << ", size = " << cell->data_size)
			}

			// read cell data (see MAT_READ_VAR)
			for (size_t i = 0; i < actualRows; i++) {
				QStringList row;
				for (size_t j = 0; j < actualCols; j++) {
					matvar_t* cell = Mat_VarGetCell(var, j + startColumn - 1);
					const size_t cellsize = cell->dims[1];
					if (cell->rank == 2 && cell->dims[0] <= 1) {	// read only rank 2 and cells with zero/one row
						switch (cell->class_type) {
						case MAT_C_CHAR:
							if (dataSource) {
								if (dynamic_cast<Matrix*>(dataSource)) {
									QDEBUG(Q_FUNC_INFO << ", WARNING: string import into matrix not supported.")
									continue;
								}
								if (i == 0) {	// first line
									if (cell->data_type == MAT_T_UINT16 || cell->data_type == MAT_T_INT16)
										static_cast<QVector<QString>*>(dataContainer[j])->operator[](0)
											= QString::fromUtf16((const mat_uint16_t*)cell->data);
									else if (cell->data_type == MAT_T_UTF8)
										static_cast<QVector<QString>*>(dataContainer[j])->operator[](0)
											= QString::fromUtf8((const char*)cell->data);
									else
										static_cast<QVector<QString>*>(dataContainer[j])->operator[](0)
											= QString((const char*)cell->data);
								}
							} else {	// preview
								if (i == 0) {	// first line
									if (cell->data_type == MAT_T_UINT16 || cell->data_type == MAT_T_INT16)
										 row << QString::fromUtf16((const mat_uint16_t*)cell->data);
									else if (cell->data_type == MAT_T_UTF8)
										row << QString::fromUtf8((const char*)cell->data);
									else
										row << QString((const char*)cell->data);
								} else
									row << QString();
							}
							break;
						case MAT_C_DOUBLE:
							MAT_READ_CELL(double, double);
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
						case MAT_C_CELL:
						case MAT_C_STRUCT:
						case MAT_C_OBJECT:
						case MAT_C_SPARSE:
						case MAT_C_FUNCTION:
						case MAT_C_OPAQUE:
							DEBUG(Q_FUNC_INFO << ", class type \"" << className(cell->class_type).toStdString() << "\" not supported yet")
							break;
						case MAT_C_EMPTY:
							break;
						}
					}
				}
				dataStrings << row;
			}
			break;
		}
		case MAT_C_SPARSE: {
			//TODO: not needed when supporting complex column mode
			if (var->isComplex && !dataSource) {	// header for preview
				QStringList row;
				for (size_t j = startColumn - 1; j < actualEndColumn; j++)
					if (j % 2)
						row << QLatin1String("Im ") + QString::number(j/2 + 1);
					else
						row << QLatin1String("Re ") + QString::number(j/2 + 1);
				dataStrings << row;
			}

			mat_sparse_t* sparse = (mat_sparse_t*)var->data;
			size_t stride = Mat_SizeOf(var->data_type);
			//DEBUG(Q_FUNC_INFO << ", stride = " << stride << ", njc = " << sparse->njc << ", ndata = " << sparse->ndata)

			switch (var->data_type) {
			case MAT_T_INT8:
				MAT_READ_SPARSE(qint8, int)
				break;
			case MAT_T_UINT8:
				MAT_READ_SPARSE(quint8, int)
				break;
			case MAT_T_INT16:
				MAT_READ_SPARSE(qint16, int)
				break;
			case MAT_T_UINT16:
				MAT_READ_SPARSE(quint16, int)
				break;
			case MAT_T_INT32:
				MAT_READ_SPARSE(qint32, int)
				break;
			case MAT_T_UINT32:
				MAT_READ_SPARSE(quint32, int)
				break;
			case MAT_T_INT64:
				MAT_READ_SPARSE(qint64, qint64)
				break;
			case MAT_T_UINT64:
				MAT_READ_SPARSE(quint64, qint64)
				break;
			case MAT_T_SINGLE:
				MAT_READ_SPARSE(float, double)
				break;
			case MAT_T_DOUBLE:
				MAT_READ_SPARSE(double, double)
				break;
			case MAT_T_MATRIX:
			case MAT_T_CELL:
			case MAT_T_STRUCT:
			case MAT_T_FUNCTION:
			case MAT_T_COMPRESSED:
			case MAT_T_UTF8:
			case MAT_T_UTF16:
			case MAT_T_UTF32:
			case MAT_T_STRING:
			case MAT_T_ARRAY:
			case MAT_T_UNKNOWN:
				DEBUG(Q_FUNC_INFO << ", data type " << var->data_type << " not supported yet")
				break;
			}

			break;
		}
		case MAT_C_STRUCT: {
			const int nelem = var->dims[0]*var->dims[1];
			const int nfields = Mat_VarGetNumberOfFields(var);

			if (nelem < 1) {
				DEBUG(Q_FUNC_INFO << ", WARNING: nr of elements is zero")
				break;
			}

			DEBUG(Q_FUNC_INFO << ", Reading data ...")
			matvar_t **fields = (matvar_t **)var->data;

			if (!dataSource)
				dataStrings[0] = vectorNames;

			int colIndex = 1;	// count cols (only needed since complex uses two cols atm)

			for (int i = 0; i < nfields * nelem; i++) {
				if (fields[i]->rank > 2) {
					DEBUG(Q_FUNC_INFO << "  rank = " << fields[i]->rank << " not supported")
					continue;
				}
				const int field = i % nfields;
				if (field < startColumn - 1 || field > endColumn - 1)
					continue;
				if (field == startColumn -1)
					colIndex = 0;
#ifndef NDEBUG
				const int elem = i/nfields;
#endif
				DEBUG(Q_FUNC_INFO << ", var " << i + 1 << "(field " << field + 1 << ", elem " << elem + 1 <<"): name = " << fields[i]->name
						<< ", type = " << className(fields[i]->class_type).toStdString())

				switch (fields[i]->class_type) {
				case MAT_C_INT8:
					MAT_READ_STRUCT(qint8);
					break;
				case MAT_C_UINT8:
					MAT_READ_STRUCT(quint8);
					break;
				case MAT_C_INT16:
					MAT_READ_STRUCT(qint16);
					break;
				case MAT_C_UINT16:
					MAT_READ_STRUCT(quint16);
					break;
				case MAT_C_INT32:
					MAT_READ_STRUCT(qint32);
					break;
				case MAT_C_UINT32:
					MAT_READ_STRUCT(quint32);
					break;
				case MAT_C_INT64:
					MAT_READ_STRUCT(qint64);
					break;
				case MAT_C_UINT64:
					MAT_READ_STRUCT(quint64);
					break;
				case MAT_C_SINGLE:
					MAT_READ_STRUCT(float);
					break;
				case MAT_C_DOUBLE:
					MAT_READ_STRUCT(double);
					break;
				case MAT_C_CHAR:
					if (dynamic_cast<Matrix*>(dataSource)) {
						QDEBUG(Q_FUNC_INFO << ", WARNING: string import into matrix not supported.")
						break;
					}
					if (fields[i]->data_type == MAT_T_UINT16 || fields[i]->data_type == MAT_T_INT16) {
						mat_uint16_t* data = (mat_uint16_t*)fields[i]->data;
						if (fields[i]->rank == 2) {
							DEBUG(Q_FUNC_INFO << "  rank = 2 (" << fields[i]->dims[0] << " x " << fields[i]->dims[1] << ")")
						} else
							DEBUG(Q_FUNC_INFO << "  rank = " << fields[i]->rank)
						DEBUG(Q_FUNC_INFO << ", UTF16 data: \"" << QString::fromUtf16(data).toStdString() << "\"")
						//TODO: row
						if (dataSource)
							static_cast<QVector<QString>*>(dataContainer[colIndex])->operator[](0) = QString::fromUtf16(data);
						else
							dataStrings[1][colIndex] = QString::fromUtf16(data);
					} else {
						char* data = (char*)fields[i]->data;
						if (fields[i]->data_type == MAT_T_UTF8) {
							DEBUG(Q_FUNC_INFO << ", UTF8 data: \"" << QString::fromUtf8(data).toStdString() << "\"")
							//TODO: row
							if (dataSource)
								static_cast<QVector<QString>*>(dataContainer[colIndex])->operator[](0) = QString::fromUtf8(data);
							else
								dataStrings[1][colIndex] = QString::fromUtf8(data);
						} else {
							DEBUG(Q_FUNC_INFO << ", STRING data: \"" << QString(data).toStdString() << "\"")
							//TODO: row
							if (dataSource)
								static_cast<QVector<QString>*>(dataContainer[colIndex])->operator[](0) = QString(data);
							else
								dataStrings[1][colIndex] = QString(data);
						}
					}
					break;
				case MAT_C_CELL:
				case MAT_C_STRUCT:
				case MAT_C_OBJECT:
				case MAT_C_SPARSE:
				case MAT_C_FUNCTION:
				case MAT_C_OPAQUE:
					DEBUG(Q_FUNC_INFO << ", not support struct field class type " << className(fields[i]->class_type).toStdString())
					break;
				case MAT_C_EMPTY:
					break;
				}

				colIndex++;
			}
			break;
		}
		case MAT_C_OBJECT:	// unsupported (s.a.)
		case MAT_C_FUNCTION:	// unsupported (s.a.)
		case MAT_C_OPAQUE:	// ???
			break;
		}
	}
	if (var->rank > 2)	// TODO
		return dataStrings << (QStringList() << i18n("Not implemented yet"));

	Mat_VarFree(var);
	if (openedFile) {
		Mat_Close(matfp);
		matfp = NULL;
	}

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
void MatioFilterPrivate::write(const QString& /*fileName*/, AbstractDataSource* /*dataSource*/) {
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
bool MatioFilter::load(XmlStreamReader*) {
// 	KLocalizedString attributeWarning = ki18n("Attribute '%1' missing or empty, default value is used");
// 	QXmlStreamAttributes attribs = reader->attributes();
	return true;
}
