/***************************************************************************
File                 : HDFFilter.cpp
Project              : LabPlot
Description          : HDF I/O-filter
--------------------------------------------------------------------
Copyright            : (C) 2015-2017 by Stefan Gerlach (stefan.gerlach@uni.kn)
Copyright            : (C) 2017 Alexander Semke (alexander.semke@web.de)
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

/* TODO:
	* Feature: implement missing data types and ranks
	* Performance: only fill dataPointer or dataStrings (not both)
*/

#include "backend/datasources/filters/HDFFilter.h"
#include "backend/datasources/filters/HDFFilterPrivate.h"
#include "backend/datasources/FileDataSource.h"
#include "backend/core/column/Column.h"

#include <QFile>
#include <QDebug>
#include <QTreeWidgetItem>
#include <KLocale>
#include <KIcon>
#include <cmath>

/*!
	\class HDFFilter
	\brief Manages the import/export of data from/to a HDF file.

	\ingroup datasources
*/
HDFFilter::HDFFilter():AbstractFileFilter(), d(new HDFFilterPrivate(this)) {
}

HDFFilter::~HDFFilter() {
	delete d;
}

/*!
  parses the content of the file \c fileName.
*/
void HDFFilter::parse(const QString & fileName, QTreeWidgetItem* rootItem) {
	d->parse(fileName, rootItem);
}

/*!
  reads the content of the data set \c dataSet from file \c fileName.
*/
QList<QStringList> HDFFilter::readCurrentDataSet(const QString & fileName, AbstractDataSource* dataSource, bool &ok, AbstractFileFilter::ImportMode importMode,  int lines) {
	return d->readCurrentDataSet(fileName, dataSource, ok, importMode, lines);
}

/*!
  reads the content of the file \c fileName to the data source \c dataSource.
*/
void HDFFilter::read(const QString & fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode) {
	d->read(fileName, dataSource, importMode);
}

/*!
writes the content of the data source \c dataSource to the file \c fileName.
*/
void HDFFilter::write(const QString & fileName, AbstractDataSource* dataSource) {
	d->write(fileName, dataSource);
}

///////////////////////////////////////////////////////////////////////
/*!
  loads the predefined filter settings for \c filterName
*/
void HDFFilter::loadFilterSettings(const QString& filterName) {
	Q_UNUSED(filterName);
}

/*!
  saves the current settings as a new filter with the name \c filterName
*/
void HDFFilter::saveFilterSettings(const QString& filterName) const {
	Q_UNUSED(filterName);
}

///////////////////////////////////////////////////////////////////////

void HDFFilter::setCurrentDataSetName(QString ds) {
	d->currentDataSetName = ds;
}

const QString HDFFilter::currentDataSetName() const {
	return d->currentDataSetName;
}

void HDFFilter::setStartRow(const int s) {
	d->startRow = s;
}

int HDFFilter::startRow() const {
	return d->startRow;
}

void HDFFilter::setEndRow(const int e) {
	d->endRow = e;
}

int HDFFilter::endRow() const {
	return d->endRow;
}

void HDFFilter::setStartColumn(const int c) {
	d->startColumn=c;
}

int HDFFilter::startColumn() const {
	return d->startColumn;
}

void HDFFilter::setEndColumn(const int c) {
	d->endColumn=c;
}

int HDFFilter::endColumn() const {
	return d->endColumn;
}

//#####################################################################
//################### Private implementation ##########################
//#####################################################################

HDFFilterPrivate::HDFFilterPrivate(HDFFilter* owner) :
	q(owner),currentDataSetName(""),startRow(1), endRow(-1), startColumn(1), endColumn(-1), status(0) {
}

#ifdef HAVE_HDF5
void HDFFilterPrivate::handleError(int err, QString function, QString arg) {
	if (err < 0)
		qDebug()<<"ERROR"<<err<<":"<<function<<"() - "<<arg;
}

QString HDFFilterPrivate::translateHDFOrder(H5T_order_t o) {
	QString order;
	switch (o) {
	case H5T_ORDER_LE:
		order = "LE";
		break;
	case H5T_ORDER_BE:
		order = "BE";
		break;
	case H5T_ORDER_VAX:
		order = "VAX";
		break;
	case H5T_ORDER_MIXED:
		order = "MIXED";
		break;
	case H5T_ORDER_NONE:
		order = "NONE";
		break;
	case H5T_ORDER_ERROR:
		order = "ERROR";
		break;
	}

	return order;
}

QString HDFFilterPrivate::translateHDFType(hid_t t) {
	QString type;

	if (H5Tequal(t, H5T_STD_I8LE) || H5Tequal(t, H5T_STD_I8BE))
		type = "CHAR";
	else if (H5Tequal(t, H5T_STD_U8LE) || H5Tequal(t, H5T_STD_U8BE))
		type = "UCHAR";
	else if (H5Tequal(t, H5T_STD_I16LE) || H5Tequal(t, H5T_STD_I16BE))
		type = "SHORT";
	else if (H5Tequal(t, H5T_STD_U16LE) || H5Tequal(t, H5T_STD_U16BE))
		type = "USHORT";
	else if (H5Tequal(t, H5T_STD_I32LE) || H5Tequal(t, H5T_STD_I32BE))
		type = "INT";
	else if (H5Tequal(t, H5T_STD_U32LE) || H5Tequal(t, H5T_STD_U32BE))
		type = "UINT";
	else if (H5Tequal(t, H5T_NATIVE_LONG))
		type = "LONG";
	else if (H5Tequal(t, H5T_NATIVE_ULONG))
		type = "ULONG";
	else if (H5Tequal(t, H5T_STD_I64LE) || H5Tequal(t, H5T_STD_I64BE))
		type = "LLONG";
	else if (H5Tequal(t, H5T_STD_U64LE) || H5Tequal(t, H5T_STD_U64BE))
		type = "ULLONG";
	else if (H5Tequal(t, H5T_IEEE_F32LE) || H5Tequal(t, H5T_IEEE_F32BE))
		type = "FLOAT";
	else if (H5Tequal(t, H5T_IEEE_F64LE) || H5Tequal(t, H5T_IEEE_F64BE))
		type = "DOUBLE";
	else if (H5Tequal(t, H5T_NATIVE_LDOUBLE))
		type = "LDOUBLE";
	else
		type = "UNKNOWN";

	return type;
}

QString HDFFilterPrivate::translateHDFClass(H5T_class_t c) {
	QString dclass;
	switch (c) {
	case H5T_INTEGER:
		dclass = "INTEGER";
		break;
	case H5T_FLOAT:
		dclass = "FLOAT";
		break;
	case H5T_STRING:
		dclass = "STRING";
		break;
	case H5T_BITFIELD:
		dclass = "BITFIELD";
		break;
	case H5T_OPAQUE:
		dclass = "OPAQUE";
		break;
	case H5T_COMPOUND:
		dclass = "COMPOUND";
		break;
	case H5T_ARRAY:
		dclass = "ARRAY";
		break;
	case H5T_ENUM:
		dclass = "ENUM";
		break;
	case H5T_REFERENCE:
		dclass = "REFERENCE";
		break;
	case H5T_VLEN:
		dclass = "VLEN";
		break;
	case H5T_TIME:
		dclass = "TIME";
		break;
	case H5T_NCLASSES:
		dclass = "NCLASSES";
		break;
	case H5T_NO_CLASS:
		dclass = "NOCLASS";
		break;
	}
	return dclass;
}

QStringList HDFFilterPrivate::readHDFCompound(hid_t tid) {
	size_t typeSize = H5Tget_size(tid);

	QString line;
	line += QLatin1String("COMPOUND(") + QString::number(typeSize) + QLatin1String(") : (");
	int members = H5Tget_nmembers(tid);
	handleError(members, "H5Tget_nmembers");
	for (int i=0; i < members; i++) {
		H5T_class_t mclass = H5Tget_member_class(tid, i);
		handleError((int)mclass, "H5Tget_member_class");
		hid_t mtype = H5Tget_member_type(tid, i);
		handleError((int)mtype, "H5Tget_member_type");
		size_t size = H5Tget_size(mtype);
		handleError((int)size, "H5Tget_size");
		QString typeString = translateHDFClass(mclass);
		if (mclass == H5T_INTEGER || mclass == H5T_FLOAT)
			typeString = translateHDFType(mtype);
		line += H5Tget_member_name(tid, i) + QLatin1String("[") + typeString + QLatin1String("(") + QString::number(size) + QLatin1String(")]");
		if (i == members-1)
			line += QLatin1String(")");
		else
			line += QLatin1String(",");
		status = H5Tclose(mtype);
		handleError(status, "H5Tclose");
	}

	QStringList dataString;
	dataString << line;

	return dataString;
}

template <typename T>
QStringList HDFFilterPrivate::readHDFData1D(hid_t dataset, hid_t type, int rows, int lines, QVector<double> *dataPointer) {
	DEBUG_LOG("readHDFData1D() rows =" << rows << "lines =" << lines);
	QStringList dataString;

	// we read all rows of data
	T* data = (T*) malloc(rows*sizeof(T));

	status = H5Dread(dataset, type, H5S_ALL, H5S_ALL, H5P_DEFAULT, data);
	handleError(status, "H5Dread");
	DEBUG_LOG(" startRow =" << startRow << "endRow =" << endRow);
	DEBUG_LOG("dataPointer =" << dataPointer);
	for (int i = startRow-1; i < qMin(endRow, lines+startRow-1); i++) {
		if (dataPointer != NULL)	// read to data source
			dataPointer->operator[](i-startRow+1) = data[i];
		else				// for preview
			dataString << QString::number(static_cast<double>(data[i]));
	}
	free(data);

	return dataString;
}

QStringList HDFFilterPrivate::readHDFCompoundData1D(hid_t dataset, hid_t tid, int rows, int lines, QVector< QVector<double>* >& dataPointer) {
	int members = H5Tget_nmembers(tid);
	handleError(members, "H5Tget_nmembers");

	QStringList* data = new QStringList[members];
	for (int m = 0; m < members; m++) {
		hid_t mtype = H5Tget_member_type(tid,m);
		handleError((int)mtype, "H5Tget_member_type");
		size_t msize = H5Tget_size(mtype);
		handleError((int)msize, "H5Tget_size");
		hid_t ctype = H5Tcreate(H5T_COMPOUND, msize);
		handleError((int)ctype, "H5Tcreate");
		status = H5Tinsert(ctype, H5Tget_member_name(tid,m), 0, mtype);
		handleError(status, "H5Tinsert");

		QVector<double>* dataP = NULL;
		if (dataPointer[0] != NULL)
			dataP = dataPointer[m];

		if (H5Tequal(mtype, H5T_STD_I8LE) || H5Tequal(mtype, H5T_STD_I8BE)) {
				data[m] = readHDFData1D<int8_t>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, lines, dataP);
		} else if (H5Tequal(mtype, H5T_NATIVE_CHAR)) {
			switch (sizeof(H5T_NATIVE_CHAR)) {
			case 1:
				data[m] = readHDFData1D<int8_t>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, lines, dataP);
				break;
			case 2:
				data[m] = readHDFData1D<int16_t>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, lines, dataP);
				break;
			case 4:
				data[m] = readHDFData1D<int32_t>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, lines, dataP);
				break;
			case 8:
				data[m] = readHDFData1D<int64_t>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, lines, dataP);
				break;
			}
		} else if (H5Tequal(mtype, H5T_STD_U8LE) || H5Tequal(mtype, H5T_STD_U8BE)) {
				data[m] = readHDFData1D<uint8_t>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, lines, dataP);
		} else if (H5Tequal(mtype, H5T_NATIVE_UCHAR)) {
			switch (sizeof(H5T_NATIVE_UCHAR)) {
			case 1:
				data[m] = readHDFData1D<uint8_t>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, lines, dataP);
				break;
			case 2:
				data[m] = readHDFData1D<uint16_t>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, lines, dataP);
				break;
			case 4:
				data[m] = readHDFData1D<uint32_t>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, lines, dataP);
				break;
			case 8:
				data[m] = readHDFData1D<uint64_t>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, lines, dataP);
				break;
			}
		} else if (H5Tequal(mtype, H5T_STD_I16LE) || H5Tequal(mtype, H5T_STD_I16BE) || H5Tequal(mtype, H5T_NATIVE_SHORT))
			data[m] = readHDFData1D<short>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, lines, dataP);
		else if (H5Tequal(mtype, H5T_STD_U16LE) || H5Tequal(mtype, H5T_STD_U16BE) || H5Tequal(mtype, H5T_NATIVE_SHORT))
			data[m] = readHDFData1D<unsigned short>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, lines, dataP);
		else if (H5Tequal(mtype, H5T_STD_I32LE) || H5Tequal(mtype, H5T_STD_I32BE) || H5Tequal(mtype, H5T_NATIVE_INT))
			data[m] = readHDFData1D<int>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, lines, dataP);
		else if (H5Tequal(mtype, H5T_STD_U32LE) || H5Tequal(mtype, H5T_STD_U32BE) || H5Tequal(mtype, H5T_NATIVE_UINT))
			data[m] = readHDFData1D<unsigned int>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, lines, dataP);
		else if (H5Tequal(mtype, H5T_NATIVE_LONG))
			data[m] = readHDFData1D<long>(dataset, ctype, rows, lines, dataP);
		else if (H5Tequal(mtype, H5T_NATIVE_ULONG))
			data[m] = readHDFData1D<unsigned long>(dataset, ctype, rows, lines, dataP);
		else if (H5Tequal(mtype, H5T_STD_I64LE) || H5Tequal(mtype, H5T_STD_I64BE) || H5Tequal(mtype, H5T_NATIVE_LLONG))
			data[m] = readHDFData1D<long long>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, lines, dataP);
		else if (H5Tequal(mtype, H5T_STD_U64LE) || H5Tequal(mtype, H5T_STD_U64BE) || H5Tequal(mtype, H5T_NATIVE_ULLONG))
			data[m] = readHDFData1D<unsigned long long>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, lines, dataP);
		else if (H5Tequal(mtype, H5T_IEEE_F32LE) || H5Tequal(mtype, H5T_IEEE_F32BE))
			data[m] = readHDFData1D<float>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, lines, dataP);
		else if (H5Tequal(mtype, H5T_IEEE_F64LE) || H5Tequal(mtype, H5T_IEEE_F64BE))
			data[m] = readHDFData1D<double>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, lines, dataP);
		else if (H5Tequal(mtype, H5T_NATIVE_LDOUBLE))
			data[m] = readHDFData1D<long double>(dataset, ctype, rows, lines, dataP);
		else {
			for (int i = 0; i < rows; i++)
				data[m] << QLatin1String("_");
			if (dataP != NULL) {
				for (int i = startRow-1; i < qMin(endRow, lines+startRow-1); i++)
					dataP->operator[](i-startRow+1) = 0;
			}
			H5T_class_t mclass = H5Tget_member_class(tid,m);
			handleError((int)mclass, "H5Tget_member_class");
			qDebug()<<"	not supported type of class"<<translateHDFClass(mclass);
		}

		H5Tclose(ctype);
	}

	// create dataString from data
	QStringList dataString;
	if (dataPointer[0] == NULL) {
		for (int i = 0; i < qMin(rows,lines); i++) {
			QString line;
			line += QLatin1String("(");

			for (int m = 0; m < members; m++) {
				if (i < data[m].size())
					line += data[m][i];
				if (m == members-1)
					line += QLatin1String(")");
				else
					line += QLatin1String(",");
			}
			dataString << line;
		}
	}

	delete[] data;

	return dataString;
}

template <typename T>
QList<QStringList> HDFFilterPrivate::readHDFData2D(hid_t dataset, hid_t type, int rows, int cols, int lines, QVector< QVector<double>* >& dataPointer) {
	DEBUG_LOG("readHDFData2D() rows =" << rows << "cols =" << cols << "lines =" << lines);
	QList<QStringList> dataStrings;

	T** data = (T**) malloc(rows*sizeof(T*));
	data[0] = (T*) malloc(cols*rows*sizeof(T));
	for (int i = 1; i < rows; i++)
		data[i] = data[0]+i*cols;

	status = H5Dread(dataset, type, H5S_ALL, H5S_ALL, H5P_DEFAULT, &data[0][0]);
	handleError(status,"H5Dread");

	for (int i = 0; i < qMin(rows, lines); i++) {
		QStringList line;
		line.reserve(cols);
		for (int j = 0; j < cols; j++) {
			if (dataPointer[0] != NULL)
				dataPointer[j-startColumn+1]->operator[](i-startRow+1) = data[i][j];
			else {
				line << QString::number(static_cast<double>(data[i][j]));
			}
		}
		dataStrings << line;
	}
	free(data[0]);
	free(data);

	DEBUG_LOG(dataStrings);
	return dataStrings;
}

QList<QStringList> HDFFilterPrivate::readHDFCompoundData2D(hid_t dataset, hid_t tid, int rows, int cols, int lines) {
	DEBUG_LOG("readHDFCompoundData2D() rows =" << rows << "cols =" << cols << "lines =" << lines);

	int members = H5Tget_nmembers(tid);
	handleError(members, "H5Tget_nmembers");
	DEBUG_LOG("members =" << members);

	QStringList* data = new QStringList[members];
	for (int m = 0; m < members; m++) {
		hid_t mtype = H5Tget_member_type(tid, m);
		handleError((int)mtype, "H5Tget_member_type");
		size_t msize = H5Tget_size(mtype);
		handleError((int)msize, "H5Tget_size");
		hid_t ctype = H5Tcreate(H5T_COMPOUND, msize);
		handleError((int)ctype, "H5Tcreate");
		status = H5Tinsert(ctype, H5Tget_member_name(tid, m), 0, mtype);
		handleError(status, "H5Tinsert");

		// dummy container for all data columns
		// initially contains one pointer set to NULL
		QVector< QVector<double>* > dummy(1, NULL);
		QList<QStringList> dataStrings;
		if (H5Tequal(mtype, H5T_STD_I8LE) || H5Tequal(mtype, H5T_STD_I8BE)) {
				dataStrings = readHDFData2D<int8_t>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, cols, lines, dummy);
		} else if (H5Tequal(mtype, H5T_NATIVE_CHAR)) {
			switch (sizeof(H5T_NATIVE_CHAR)) {
			case 1:
				dataStrings = readHDFData2D<int8_t>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, cols, lines, dummy);
				break;
			case 2:
				dataStrings = readHDFData2D<int16_t>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, cols, lines, dummy);
				break;
			case 4:
				dataStrings = readHDFData2D<int32_t>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, cols, lines, dummy);
				break;
			case 8:
				dataStrings = readHDFData2D<int64_t>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, cols, lines, dummy);
				break;
			}
		} else if (H5Tequal(mtype, H5T_STD_U8LE) || H5Tequal(mtype, H5T_STD_U8BE)) {
				dataStrings = readHDFData2D<uint8_t>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, cols, lines, dummy);
		} else if (H5Tequal(mtype, H5T_NATIVE_UCHAR)) {
			switch (sizeof(H5T_NATIVE_UCHAR)) {
			case 1:
				dataStrings = readHDFData2D<uint8_t>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, cols, lines, dummy);
				break;
			case 2:
				dataStrings = readHDFData2D<uint16_t>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, cols, lines, dummy);
				break;
			case 4:
				dataStrings = readHDFData2D<uint32_t>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, cols, lines, dummy);
				break;
			case 8:
				dataStrings = readHDFData2D<uint64_t>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, cols, lines, dummy);
				break;
			}
		} else if (H5Tequal(mtype, H5T_STD_I16LE) || H5Tequal(mtype, H5T_STD_I16BE)|| H5Tequal(mtype, H5T_NATIVE_SHORT))
			dataStrings = readHDFData2D<short>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, cols, lines, dummy);
		else if (H5Tequal(mtype, H5T_STD_U16LE) || H5Tequal(mtype, H5T_STD_U16BE) || H5Tequal(mtype, H5T_NATIVE_USHORT))
			dataStrings = readHDFData2D<unsigned short>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, cols, lines, dummy);
		else if (H5Tequal(mtype, H5T_STD_I32LE) || H5Tequal(mtype, H5T_STD_I32BE) || H5Tequal(mtype, H5T_NATIVE_INT))
			dataStrings = readHDFData2D<int>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, cols, lines, dummy);
		else if (H5Tequal(mtype, H5T_STD_U32LE) || H5Tequal(mtype, H5T_STD_U32BE) || H5Tequal(mtype, H5T_NATIVE_UINT))
			dataStrings = readHDFData2D<unsigned int>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, cols, lines, dummy);
		else if (H5Tequal(mtype, H5T_NATIVE_LONG))
			dataStrings = readHDFData2D<long>(dataset, ctype, rows, cols, lines, dummy);
		else if (H5Tequal(mtype, H5T_NATIVE_ULONG))
			dataStrings = readHDFData2D<unsigned long>(dataset, ctype, rows, cols, lines, dummy);
		else if (H5Tequal(mtype, H5T_STD_I64LE) || H5Tequal(mtype, H5T_STD_I64BE) || H5Tequal(mtype, H5T_NATIVE_LLONG))
			dataStrings = readHDFData2D<long long>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, cols, lines, dummy);
		else if (H5Tequal(mtype, H5T_STD_U64LE) || H5Tequal(mtype, H5T_STD_U64BE) || H5Tequal(mtype, H5T_NATIVE_ULLONG))
			dataStrings = readHDFData2D<unsigned long long>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, cols, lines, dummy);
		else if (H5Tequal(mtype, H5T_IEEE_F32LE) || H5Tequal(mtype, H5T_IEEE_F32BE))
			dataStrings = readHDFData2D<float>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, cols, lines, dummy);
		else if (H5Tequal(mtype, H5T_IEEE_F64LE) || H5Tequal(mtype, H5T_IEEE_F64BE))
			dataStrings = readHDFData2D<double>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, cols, lines, dummy);
		else if (H5Tequal(mtype, H5T_NATIVE_LDOUBLE))
			dataStrings = readHDFData2D<long double>(dataset, ctype, rows, cols, lines, dummy);
		else {
			//TODO
			/*for (int i = 0; i < rows; i++) {
				for (int j = 0; j < cols; j++)
					data[m] << QLatin1String("_");
			}
			*/
			H5T_class_t mclass = H5Tget_member_class(tid, m);
			qDebug() << "	not supported class" << translateHDFClass(mclass);
		}

		status = H5Tclose(ctype);
		handleError(status, "H5Tclose");

		// convert local dataStrings to QStringList[]
		for (int i = 0; i < rows; i++)
			data[m] << dataStrings[i].join(",");
	}

	QList<QStringList> dataStrings;
	for (int i = 0; i < qMin(rows, lines); i++) {
		DEBUG_LOG("row =" << i);
		QStringList lineString;
		for (int m = 0; m < members; m++)
			lineString << QLatin1String("(") + data[m][i] + QLatin1String(")");
		dataStrings << lineString;
	}

	delete[] data;

	DEBUG_LOG("dataStrings =" << dataStrings);
	return dataStrings;
}

QStringList HDFFilterPrivate::readHDFAttr(hid_t aid) {
	QStringList attr;

	char name[MAXNAMELENGTH];
	status = H5Aget_name(aid, MAXNAMELENGTH, name);
	handleError(status, "H5Aget_name");
	attr << QString(name);
	// DEBUG_LOG("	name =" << QString(name));

	hid_t aspace = H5Aget_space(aid); // the dimensions of the attribute data
	handleError((int)aspace, "H5Aget_space");
	hid_t atype  = H5Aget_type(aid);
	handleError((int)atype, "H5Aget_type");
	hid_t aclass = H5Tget_class(atype);
	handleError((int)aclass, "H5Aget_class");

	if (aclass == H5T_STRING) {
		char buf[MAXSTRINGLENGTH];	// buffer to read attr value
		hid_t amem = H5Tget_native_type(atype, H5T_DIR_ASCEND);
		handleError((int)amem, "H5Tget_native_type");
		status = H5Aread(aid, amem, buf);
		handleError(status, "H5Aread");
		attr << QLatin1String("=") << QString(buf);
		status = H5Tclose(amem);
		handleError(status, "H5Tclose");
	} else if (aclass == H5T_INTEGER) {
		if (H5Tequal(atype, H5T_STD_I8LE)) {
			int8_t value;
			status = H5Aread(aid, H5T_STD_I8LE, &value);
			handleError(status, "H5Aread");
			attr << QLatin1String("=") << QString::number(value);
		} else if (H5Tequal(atype, H5T_STD_I8BE)) {
			int8_t value;
			status = H5Aread(aid, H5T_STD_I8BE, &value);
			handleError(status, "H5Aread");
			attr << QLatin1String("=") << QString::number(value);
		} else if (H5Tequal(atype, H5T_NATIVE_CHAR)) {
			switch (sizeof(H5T_NATIVE_CHAR)) {
			case 1: {
				int8_t value;
				status = H5Aread(aid, H5T_NATIVE_CHAR, &value);
				handleError(status, "H5Aread");
				attr << QLatin1String("=") << QString::number(value);
				break;
			}
			case 2: {
				int16_t value;
				status = H5Aread(aid, H5T_NATIVE_CHAR, &value);
				handleError(status, "H5Aread");
				attr << QLatin1String("=") << QString::number(value);
				break;
			}
			case 4: {
				int32_t value;
				status = H5Aread(aid, H5T_NATIVE_CHAR, &value);
				handleError(status, "H5Aread");
				attr << QLatin1String("=") << QString::number(value);
				break;
			}
			case 8: {
				int64_t value;
				status = H5Aread(aid, H5T_NATIVE_CHAR, &value);
				handleError(status, "H5Aread");
				attr << QLatin1String("=") << QString::number(value);
				break;
			}
			default:
				qDebug()<<"unknown size of H5T_NATIVE_CHAR:" << sizeof(H5T_NATIVE_CHAR);
				return QStringList("");
			}
		} else if (H5Tequal(atype, H5T_STD_U8LE)) {
			uint8_t value;
			status = H5Aread(aid, H5T_STD_U8LE, &value);
			handleError(status, "H5Aread");
			attr << QLatin1String("=") << QString::number(value);
		} else if (H5Tequal(atype, H5T_STD_U8BE)) {
			uint8_t value;
			status = H5Aread(aid, H5T_STD_U8BE, &value);
			handleError(status, "H5Aread");
			attr << QLatin1String("=") << QString::number(value);
		} else if (H5Tequal(atype, H5T_NATIVE_UCHAR)) {
			switch (sizeof(H5T_NATIVE_UCHAR)) {
			case 1: {
				uint8_t value;
				status = H5Aread(aid, H5T_NATIVE_UCHAR, &value);
				handleError(status, "H5Aread");
				attr << QLatin1String("=") << QString::number(value);
				break;
			}
			case 2: {
				uint16_t value;
				status = H5Aread(aid, H5T_NATIVE_UCHAR, &value);
				handleError(status, "H5Aread");
				attr << QLatin1String("=") << QString::number(value);
				break;
			}
			case 4: {
				uint32_t value;
				status = H5Aread(aid, H5T_NATIVE_UCHAR, &value);
				handleError(status, "H5Aread");
				attr << QLatin1String("=") << QString::number(value);
				break;
			}
			case 8: {
				uint64_t value;
				status = H5Aread(aid, H5T_NATIVE_UCHAR, &value);
				handleError(status, "H5Aread");
				attr << QLatin1String("=") << QString::number(value);
				break;
			}
			default:
				qDebug() << "unknown size of H5T_NATIVE_UCHAR:" << sizeof(H5T_NATIVE_UCHAR);
				return QStringList("");
			}
		} else if (H5Tequal(atype, H5T_STD_I16LE) || H5Tequal(atype, H5T_STD_I16BE) || H5Tequal(atype, H5T_NATIVE_SHORT)) {
			short value;
			status = H5Aread(aid, H5T_NATIVE_SHORT, &value);
			handleError(status, "H5Aread");
			attr<<QLatin1String("=")<<QString::number(value);
		} else if (H5Tequal(atype, H5T_STD_U16LE) || H5Tequal(atype, H5T_STD_U16BE) || H5Tequal(atype, H5T_NATIVE_USHORT)) {
			unsigned short value;
			status = H5Aread(aid, H5T_NATIVE_USHORT, &value);
			handleError(status, "H5Aread");
			attr<<QLatin1String("=")<<QString::number(value);
		} else if (H5Tequal(atype, H5T_STD_I32LE) || H5Tequal(atype, H5T_STD_I32BE) || H5Tequal(atype, H5T_NATIVE_INT)) {
			int value;
			status = H5Aread(aid, H5T_NATIVE_INT, &value);
			handleError(status, "H5Aread");
			attr<<QLatin1String("=")<<QString::number(value);
		} else if (H5Tequal(atype, H5T_STD_U32LE) || H5Tequal(atype, H5T_STD_U32BE) || H5Tequal(atype, H5T_NATIVE_UINT)) {
			unsigned int value;
			status = H5Aread(aid, H5T_NATIVE_UINT, &value);
			handleError(status, "H5Aread");
			attr<<QLatin1String("=")<<QString::number(value);
		} else if (H5Tequal(atype, H5T_NATIVE_LONG)) {
			long value;
			status = H5Aread(aid, H5T_NATIVE_LONG, &value);
			handleError(status, "H5Aread");
			attr<<QLatin1String("=")<<QString::number(value);
		} else if (H5Tequal(atype, H5T_NATIVE_ULONG)) {
			unsigned long value;
			status = H5Aread(aid, H5T_NATIVE_ULONG, &value);
			handleError(status, "H5Aread");
			attr<<QLatin1String("=")<<QString::number(value);
		} else if (H5Tequal(atype, H5T_STD_I64LE) || H5Tequal(atype, H5T_STD_I64BE) || H5Tequal(atype, H5T_NATIVE_LLONG)) {
			long long value;
			status = H5Aread(aid, H5T_NATIVE_LLONG, &value);
			handleError(status, "H5Aread");
			attr<<QLatin1String("=")<<QString::number(value);
		} else if (H5Tequal(atype, H5T_STD_U64LE) || H5Tequal(atype, H5T_STD_U64BE) || H5Tequal(atype, H5T_NATIVE_ULLONG)) {
			unsigned long long value;
			status = H5Aread(aid, H5T_NATIVE_ULLONG, &value);
			handleError(status, "H5Aread");
			attr<<QLatin1String("=")<<QString::number(value);
		} else
			attr<<" (unknown integer)";
	} else if (aclass == H5T_FLOAT) {
		if (H5Tequal(atype, H5T_IEEE_F32LE) || H5Tequal(atype, H5T_IEEE_F32BE)) {
			float value;
			status = H5Aread(aid, H5T_NATIVE_FLOAT, &value);
			handleError(status, "H5Aread");
			attr<<QLatin1String("=")<<QString::number(value);
		} else if (H5Tequal(atype, H5T_IEEE_F64LE) || H5Tequal(atype, H5T_IEEE_F64BE)) {
			double value;
			status = H5Aread(aid, H5T_NATIVE_DOUBLE, &value);
			handleError(status, "H5Aread");
			attr << QLatin1String("=") << QString::number(value);
		} else if (H5Tequal(atype, H5T_NATIVE_LDOUBLE)) {
			long double value;
			status = H5Aread(aid, H5T_NATIVE_LDOUBLE, &value);
			handleError(status, "H5Aread");
			attr << QLatin1String("=") << QString::number((double)value);
		} else
			attr<<" (unknown float)";
	}

	status = H5Tclose(atype);
	handleError(status, "H5Tclose");
	status = H5Sclose(aspace);
	handleError(status, "H5Sclose");

	return attr;
}

QStringList HDFFilterPrivate::scanHDFAttrs(hid_t oid) {
	QStringList attrList;

	int numAttr = H5Aget_num_attrs(oid);
	handleError(numAttr, "H5Aget_num_attrs");
	DEBUG_LOG("number of attr =" << numAttr);

	for (int i = 0; i < numAttr; i++) {
		hid_t aid = H5Aopen_idx(oid, i);
		handleError((int)aid, "H5Aopen_idx");
		attrList << readHDFAttr(aid);
		if (i < numAttr-1)
			attrList << QLatin1String(", ");
		status = H5Aclose(aid);
		handleError(status, "H5Aclose");
	}

	return attrList;
}

QStringList HDFFilterPrivate::readHDFDataType(hid_t tid) {
	H5T_class_t typeClass = H5Tget_class(tid);
	handleError((int)typeClass, "H5Tget_class");

	QStringList typeProps;
	QString typeString = translateHDFClass(typeClass);
	if (typeClass == H5T_INTEGER || typeClass == H5T_FLOAT)
		typeString = translateHDFType(tid);
	typeProps<<typeString;

	size_t size = H5Tget_size(tid);
	typeProps << QLatin1String(" (") << QString::number(size) << QLatin1String(") ");

	H5T_order_t order = H5Tget_order(tid);
	handleError((int)order, "H5Tget_order");
	typeProps << translateHDFOrder(order);

	// type specific props
	switch (typeClass) {
	case H5T_STRING: {
			H5T_cset_t cset = H5Tget_cset (tid);
			handleError((int)cset, "H5Tget_cset");
			switch (cset) {
			case H5T_CSET_ASCII:
				typeProps<<QLatin1String(", ASCII");
				break;
			case H5T_CSET_ERROR:
				typeProps<<QLatin1String(", ERROR");
				break;
			case H5T_CSET_UTF8:
				typeProps<<QLatin1String(", UTF8");
				break;
			case H5T_CSET_RESERVED_2:
			case H5T_CSET_RESERVED_3:
			case H5T_CSET_RESERVED_4:
			case H5T_CSET_RESERVED_5:
			case H5T_CSET_RESERVED_6:
			case H5T_CSET_RESERVED_7:
			case H5T_CSET_RESERVED_8:
			case H5T_CSET_RESERVED_9:
			case H5T_CSET_RESERVED_10:
			case H5T_CSET_RESERVED_11:
			case H5T_CSET_RESERVED_12:
			case H5T_CSET_RESERVED_13:
			case H5T_CSET_RESERVED_14:
			case H5T_CSET_RESERVED_15:
				typeProps<<QLatin1String(", RESERVED");
				break;
			}
			H5T_str_t strpad = H5Tget_strpad(tid);
			handleError((int)strpad, "H5Tget_strpad");
			switch (strpad) {
			case H5T_STR_NULLTERM:
				typeProps<<QLatin1String(" NULLTERM");
				break;
			case H5T_STR_NULLPAD:
				typeProps<<QLatin1String(" NULLPAD");
				break;
			case H5T_STR_SPACEPAD:
				typeProps<<QLatin1String(" SPACEPAD");
				break;
			case H5T_STR_ERROR:
				typeProps<<QLatin1String(" ERROR");
				break;
			case H5T_STR_RESERVED_3:
			case H5T_STR_RESERVED_4:
			case H5T_STR_RESERVED_5:
			case H5T_STR_RESERVED_6:
			case H5T_STR_RESERVED_7:
			case H5T_STR_RESERVED_8:
			case H5T_STR_RESERVED_9:
			case H5T_STR_RESERVED_10:
			case H5T_STR_RESERVED_11:
			case H5T_STR_RESERVED_12:
			case H5T_STR_RESERVED_13:
			case H5T_STR_RESERVED_14:
			case H5T_STR_RESERVED_15:
				typeProps<<QLatin1String(" RESERVED");
				break;
			}
			break;
		}
	case H5T_COMPOUND: {
			// not shown in tree widget
			DEBUG_LOG(readHDFCompound(tid).join(""));
			break;
		}
	case H5T_ENUM: {
			//TODO
			break;
		}
	case H5T_INTEGER:
		//TODO
		break;
	case H5T_FLOAT:
		//TODO
		break;
	case H5T_TIME:
		//TODO
		break;
	case H5T_BITFIELD:
		//TODO
		break;
	case H5T_OPAQUE:
		//TODO
		break;
	case H5T_REFERENCE:
		//TODO
		break;
	case H5T_VLEN:
		//TODO
		break;
	case H5T_ARRAY:
		//TODO
		break;
	case H5T_NCLASSES:
		//TODO
		break;
	case H5T_NO_CLASS:
		break;
	}

	return typeProps;
}

QStringList HDFFilterPrivate::readHDFPropertyList(hid_t pid) {
	QStringList props;

	hsize_t chunk_dims_out[2];
	if (H5D_CHUNKED == H5Pget_layout(pid)) {
		int rank_chunk = H5Pget_chunk(pid, 2, chunk_dims_out);
		handleError(rank_chunk, "H5Pget_chunk");
		props << QLatin1String("chunk rank=") << QString::number(rank_chunk) << QLatin1String(", dimension=")
			<< QString::number(chunk_dims_out[0]) << QString::number(chunk_dims_out[1]);
	}

	int nfilters = H5Pget_nfilters(pid);
	handleError(nfilters, "H5Pget_nfilters");
	props << QLatin1String(" ") << QString::number(nfilters) << QLatin1String(" filter");
	for (int i = 0; i < nfilters; i++) {
		size_t cd_nelmts = 32;
		unsigned int filt_flags, filt_conf;
		unsigned int cd_values[32];
		char f_name[MAXNAMELENGTH];
		H5Z_filter_t filtn = H5Pget_filter(pid, (unsigned)i, &filt_flags, &cd_nelmts, cd_values,(size_t)MAXNAMELENGTH, f_name, &filt_conf);
		handleError((int)filtn, "H5Pget_filter");

		switch (filtn) {
		case H5Z_FILTER_DEFLATE:  /* AKA GZIP compression */
			props << QLatin1String(": DEFLATE level =") << QString::number(cd_values[0]);
			break;
		case H5Z_FILTER_SHUFFLE:
			props << QLatin1String(": SHUFFLE"); /* no parms */
			break;
		case H5Z_FILTER_FLETCHER32:
			props << QLatin1String(": FLETCHER32");  /* Error Detection Code */
			break;
		case H5Z_FILTER_SZIP: {
				//unsigned int szip_options_mask=cd_values[0];;
				unsigned int szip_pixels_per_block=cd_values[1];

				props << QLatin1String(": SZIP COMPRESSION - PIXELS_PER_BLOCK ") << QString::number(szip_pixels_per_block);
				break;
			}
		default:
			props << QLatin1String(": Unknown filter");
			break;
		}
	}

	props << QLatin1String(", ALLOC_TIME:");
	H5D_alloc_time_t at;
	status = H5Pget_alloc_time(pid, &at);
	handleError(status, "H5Pget_alloc_time");

	switch (at) {
	case H5D_ALLOC_TIME_EARLY:
		props << QLatin1String(" EARLY");
		break;
	case H5D_ALLOC_TIME_INCR:
		props << QLatin1String(" INCR");
		break;
	case H5D_ALLOC_TIME_LATE:
		props << QLatin1String(" LATE");
		break;
	case H5D_ALLOC_TIME_DEFAULT:
		props << QLatin1String(" DEFAULT");
		break;
	case H5D_ALLOC_TIME_ERROR:
		props << QLatin1String(" ERROR");
		break;
	}

	props << QLatin1String(", FILL_TIME:");
	H5D_fill_time_t ft;
	status = H5Pget_fill_time(pid, &ft);
	handleError(status, "H5Pget_fill_time");
	switch (ft) {
	case H5D_FILL_TIME_ALLOC:
		props << QLatin1String(" ALLOW");
		break;
	case H5D_FILL_TIME_NEVER:
		props << QLatin1String(" NEVER");
		break;
	case H5D_FILL_TIME_IFSET:
		props << QLatin1String(" IFSET");
		break;
	case H5D_FILL_TIME_ERROR:
		props << QLatin1String(" ERROR");
		break;
	}

	H5D_fill_value_t fvstatus;
	status = H5Pfill_value_defined(pid, &fvstatus);
	handleError(status, "H5Pfill_value_defined");
	if (fvstatus == H5D_FILL_VALUE_UNDEFINED)
		props << QLatin1String(" No fill value defined");
	else {
		/* TODO: Read  the fill value with H5Pget_fill_value.
		 * Fill value is the same data type as the dataset.
		 * (details not shown)
		 **/
	}

	return props;
}

void HDFFilterPrivate::scanHDFDataType(hid_t tid, char *dataSetName, QTreeWidgetItem* parentItem) {
	QStringList typeProps = readHDFDataType(tid);

	QString attr = scanHDFAttrs(tid).join(" ");

	char link[MAXNAMELENGTH];
	status = H5Iget_name(tid, link, MAXNAMELENGTH);
	handleError(status, "H5Iget_name");

	QTreeWidgetItem* dataTypeItem = new QTreeWidgetItem(QStringList()<<QString(dataSetName)<<QString(link)<<i18n("data type")<<typeProps.join("")<<attr);
	dataTypeItem->setIcon(0, KIcon("accessories-calculator"));
	dataTypeItem->setFlags(Qt::ItemIsEnabled);
	parentItem->addChild(dataTypeItem);
}

void HDFFilterPrivate::scanHDFDataSet(hid_t did, char *dataSetName, QTreeWidgetItem* parentItem) {
	QString attr = scanHDFAttrs(did).join("");

	char link[MAXNAMELENGTH];
	status = H5Iget_name(did, link, MAXNAMELENGTH);
	handleError(status, "H5Iget_name");

	QStringList dataSetProps;
	hsize_t size = H5Dget_storage_size(did);
	handleError((int)size, "H5Dget_storage_size");
	hid_t datatype  = H5Dget_type(did);
	handleError((int)datatype, "H5Dget_type");
	size_t typeSize  = H5Tget_size(datatype);
	handleError((int)typeSize, "H5Dget_size");

	dataSetProps << readHDFDataType(datatype);

	hid_t dataspace = H5Dget_space(did);
	int rank = H5Sget_simple_extent_ndims(dataspace);
	handleError(rank, "H5Sget_simple_extent_ndims");
	if (rank == 1) {
		hsize_t dims_out[1];
		status = H5Sget_simple_extent_dims(dataspace, dims_out, NULL);
		handleError(status, "H5Sget_simple_extent_dims");
		unsigned int rows = dims_out[0];
		dataSetProps << QLatin1String(", ") << QString::number(rows)
				<< QLatin1String(" (") << QString::number(size/typeSize) << QLatin1String(")");
	} else if (rank == 2) {
		hsize_t dims_out[2];
		status = H5Sget_simple_extent_dims(dataspace, dims_out, NULL);
		handleError(status, "H5Sget_simple_extent_dims");
		unsigned int rows = dims_out[0];
		unsigned int cols = dims_out[1];
		dataSetProps << QLatin1String(", ") << QString::number(rows) << QLatin1String("x") << QString::number(cols)
				<< QLatin1String(" (") << QString::number(size/typeSize) << QLatin1String(")");
	} else if (rank == 3) {
		hsize_t dims_out[3];
		status = H5Sget_simple_extent_dims(dataspace, dims_out, NULL);
		handleError(status, "H5Sget_simple_extent_dims");
		unsigned int rows = dims_out[0];
		unsigned int cols = dims_out[1];
		unsigned int regs = dims_out[2];
		dataSetProps << QLatin1String(", ")
			<< QString::number(rows) << QLatin1String("x") << QString::number(cols) << QLatin1String("x") << QString::number(regs)
			<< QLatin1String(" (") << QString::number(size/typeSize) << QLatin1String(")");
	}

	hid_t pid = H5Dget_create_plist(did);
	handleError((int)pid, "H5Dget_create_plist");
	dataSetProps << ", " << readHDFPropertyList(pid).join("");

	QTreeWidgetItem* dataSetItem = new QTreeWidgetItem(QStringList()<<QString(dataSetName)<<QString(link)<<i18n("data set")<<dataSetProps.join("")<<attr);
	dataSetItem->setIcon(0, KIcon("x-office-spreadsheet"));
	for (int i = 0; i < dataSetItem->columnCount(); i++) {
		dataSetItem->setBackground(i, QColor(192,255,192));
		dataSetItem->setForeground(i, Qt::black);
	}
	dataSetItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	parentItem->addChild(dataSetItem);
}

void HDFFilterPrivate::scanHDFLink(hid_t gid, char *linkName, QTreeWidgetItem* parentItem) {
	char target[MAXNAMELENGTH];
	status = H5Gget_linkval(gid, linkName, MAXNAMELENGTH, target) ;
	handleError(status, "H5Gget_linkval");

	QTreeWidgetItem* linkItem = new QTreeWidgetItem(QStringList()<<QString(linkName) << i18n("symbolic link") << i18n("link to") + QString(target));
	linkItem->setIcon(0, KIcon("emblem-symbolic-link"));
	linkItem->setFlags(Qt::ItemIsEnabled);
	parentItem->addChild(linkItem);
}

void HDFFilterPrivate::scanHDFGroup(hid_t gid, char *groupName, QTreeWidgetItem* parentItem) {

	//check for hard link
	H5G_stat_t statbuf;
	status = H5Gget_objinfo(gid, ".", true, &statbuf);
	handleError(status, "H5Gget_objinfo");
	if (statbuf.nlink > 1) {
		if (multiLinkList.contains(statbuf.objno[0])) {
			QTreeWidgetItem* objectItem = new QTreeWidgetItem(QStringList()<<QString(groupName) << i18n("hard link"));
			objectItem->setIcon(0, KIcon("link"));
			objectItem->setFlags(Qt::ItemIsEnabled);
			parentItem->addChild(objectItem);
			return;
		} else {
			multiLinkList.append(statbuf.objno[0]);
#ifndef NDEBUG
			qDebug()<<" group multiple links: "<<statbuf.objno[0]<<' '<<statbuf.objno[1];
#endif
		}
	}

	char link[MAXNAMELENGTH];
	status = H5Iget_name(gid, link, MAXNAMELENGTH);
	handleError(status, "H5Iget_name");

	QString attr = scanHDFAttrs(gid).join(" ");

	QTreeWidgetItem* groupItem = new QTreeWidgetItem(QStringList() << QString(groupName) << QString(link) << QLatin1String("group ")<<attr);
	groupItem->setIcon(0, KIcon("folder"));
	groupItem->setFlags(Qt::ItemIsEnabled);
	parentItem->addChild(groupItem);

	hsize_t numObj;
	status = H5Gget_num_objs(gid, &numObj);
	handleError(status, "H5Gget_num_objs");

	for (unsigned int i = 0; i < numObj; i++) {
		char memberName[MAXNAMELENGTH];
		status = H5Gget_objname_by_idx(gid, (hsize_t)i, memberName, (size_t)MAXNAMELENGTH );
		handleError(status, "H5Gget_objname_by_idx");

		int otype =  H5Gget_objtype_by_idx(gid, (size_t)i );
		handleError(otype, "H5Gget_objtype_by_idx");
		switch (otype) {
		case H5G_LINK: {
				scanHDFLink(gid, memberName, groupItem);
				break;
			}
		case H5G_GROUP: {
				hid_t grpid = H5Gopen(gid, memberName, H5P_DEFAULT);
				handleError((int)grpid, "H5Gopen");
				scanHDFGroup(grpid, memberName, groupItem);
				status = H5Gclose(grpid);
				handleError(status, "H5Gclose");
				break;
			}
		case H5G_DATASET: {
				hid_t dsid = H5Dopen(gid, memberName, H5P_DEFAULT);
				handleError((int)dsid, "H5Dopen");
				scanHDFDataSet(dsid, memberName, groupItem);
				status = H5Dclose(dsid);
				handleError(status, "H5Dclose");
				break;
			}
		case H5G_TYPE: {
				hid_t tid = H5Topen(gid, memberName, H5P_DEFAULT);
				handleError((int)tid, "H5Topen");
				scanHDFDataType(tid, memberName, groupItem);
				status = H5Tclose(tid);
				handleError(status, "H5Tclose");
				break;
			}
		default:
			QTreeWidgetItem* objectItem = new QTreeWidgetItem(QStringList() << QString(memberName) << i18n("unknown"));
			objectItem->setFlags(Qt::ItemIsEnabled);
			groupItem->addChild(objectItem);
			break;
		}
	}
}
#endif

/*!
    parses the content of the file \c fileName and fill the tree using rootItem.
*/
void HDFFilterPrivate::parse(const QString & fileName, QTreeWidgetItem* rootItem) {
#ifdef HAVE_HDF5
	QByteArray bafileName = fileName.toLatin1();
	hid_t file = H5Fopen(bafileName.data(), H5F_ACC_RDONLY, H5P_DEFAULT);
	handleError((int)file, "H5Fopen", fileName);
	char rootName[]="/";
	hid_t group = H5Gopen(file, rootName, H5P_DEFAULT);
	handleError((int)group, "H5Gopen", rootName);
	// CRASHES multiLinkList.clear();
	scanHDFGroup(group, rootName, rootItem);
	status = H5Gclose(group);
	handleError(status, "H5Gclose", "");
	status = H5Fclose(file);
	handleError(status, "H5Fclose", "");
#else
	Q_UNUSED(fileName)
	Q_UNUSED(rootItem)
#endif
}

/*!
    reads the content of the date set in the file \c fileName to a string (for preview) or to the data source.
*/
QList<QStringList> HDFFilterPrivate::readCurrentDataSet(const QString & fileName, AbstractDataSource* dataSource, bool &ok, AbstractFileFilter::ImportMode mode, int lines) {
	DEBUG_LOG("HDFFilter::readCurrentDataSet()");
	QList<QStringList> dataStrings;

	if (currentDataSetName.isEmpty()) {
		//return QString("No data set selected").replace(' ',QChar::Nbsp);
		ok = false;
		return dataStrings << (QStringList() << i18n("No data set selected"));
	}
	DEBUG_LOG(" current data set =" << currentDataSetName);

#ifdef HAVE_HDF5
	QByteArray bafileName = fileName.toLatin1();
	hid_t file = H5Fopen(bafileName.data(), H5F_ACC_RDONLY, H5P_DEFAULT);
	handleError((int)file, "H5Fopen", fileName);
	QByteArray badataSet = currentDataSetName.toLatin1();
	hid_t dataset = H5Dopen2(file, badataSet.data(), H5P_DEFAULT);
	handleError((int)file, "H5Dopen2", currentDataSetName);

	// Get datatype and dataspace
	hid_t dtype = H5Dget_type(dataset);
	handleError((int)dtype, "H5Dget_type");
	H5T_class_t dclass = H5Tget_class(dtype);
	handleError((int)dclass, "H5Dget_class");
	size_t typeSize = H5Tget_size(dtype);
	handleError((int)(typeSize-1), "H5Dget_size");

	hid_t dataspace = H5Dget_space(dataset);
	handleError((int)dataspace, "H5Dget_space");
	int rank = H5Sget_simple_extent_ndims(dataspace);
	handleError(rank, "H5Dget_simple_extent_ndims");
	DEBUG_LOG(" rank =" << rank);

	int columnOffset = 0;			// offset to import data
	int actualRows = 0, actualCols = 0;	// rows and cols to read

	// dataPointers is used to store the data read from the dataSource
	// it contains the pointers of all columns
	// initially there is one pointer set to NULL
	// check for dataPointers[0] != NULL to decide if dataSource can be used
	QVector<QVector<double>*> dataPointers(1, NULL);

	// rank= 0: single value, 1: vector, 2: matrix, 3: 3D data, ...
	switch (rank) {
	case 0: {
			actualRows = 1;
			actualCols = 1;

			switch (dclass) {
			case H5T_STRING: {
					char* data = (char *) malloc(typeSize * sizeof(char));
					hid_t memtype = H5Tcopy(H5T_C_S1);
					handleError((int)memtype, "H5Tcopy");
					status = H5Tset_size(memtype, typeSize);
					handleError(status, "H5Tset_size");

					status = H5Dread(dataset, memtype, H5S_ALL, H5S_ALL, H5P_DEFAULT, data);
					handleError(status, "H5Tread");
					dataStrings << (QStringList() << data);
					free(data);
					break;
			}
			case H5T_INTEGER:
			case H5T_FLOAT:
			case H5T_TIME:
			case H5T_BITFIELD:
			case H5T_OPAQUE:
			case H5T_COMPOUND:
			case H5T_REFERENCE:
			case H5T_ENUM:
			case H5T_VLEN:
			case H5T_ARRAY:
			case H5T_NO_CLASS:
			case H5T_NCLASSES: {
					ok = false;
					dataStrings << (QStringList() << i18n("rank 0 not implemented yet for type %1").arg(translateHDFClass(dclass)));
					qDebug() << dataStrings;
			}
			default:
				break;
			}
			break;
		}
	case 1: {
			hsize_t size, maxSize;
			status = H5Sget_simple_extent_dims(dataspace, &size, &maxSize);
			handleError(status, "H5Sget_simple_extent_dims");
			int rows = size;
			if (endRow == -1)
				endRow = rows;
			if (lines == -1)
				lines = endRow;
			actualRows = endRow-startRow+1;
			actualCols = 1;
#ifndef NDEBUG
			H5T_order_t order = H5Tget_order(dtype);
			handleError((int)order, "H5Sget_order");
			qDebug() << translateHDFClass(dclass) << "(" << typeSize << ")" << translateHDFOrder(order)
				<< ", rows:" << rows << " max:" << maxSize;
#endif
			if (dataSource != NULL)
				columnOffset = dataSource->create(dataPointers, mode, actualRows, actualCols);

			QStringList dataString;	// data saved in a list
			switch (dclass) {
			case H5T_STRING: {
					DEBUG_LOG("rank 1 H5T_STRING");
					hid_t memtype = H5Tcopy(H5T_C_S1);
					handleError((int)memtype, "H5Tcopy");

					char** data = (char **) malloc(rows * sizeof (char *));

					if (H5Tis_variable_str(dtype)) {
						status = H5Tset_size(memtype, H5T_VARIABLE);
						handleError((int)memtype, "H5Tset_size");
						status = H5Dread(dataset, memtype, H5S_ALL, H5S_ALL, H5P_DEFAULT, data);
						handleError(status, "H5Dread");
					} else {
						data[0] = (char *) malloc(rows * typeSize * sizeof (char));
						for (int i = 1; i < rows; i++)
							data[i] = data[0] + i * typeSize;

						status = H5Tset_size(memtype, typeSize);
						handleError((int)memtype, "H5Tset_size");

						status = H5Dread(dataset, memtype, H5S_ALL, H5S_ALL, H5P_DEFAULT, data[0]);
						handleError(status, "H5Dread");
					}

					for (int i = startRow-1; i < qMin(endRow, lines+startRow-1); i++)
						dataString << data[i];

					free(data);
					break;
				}
			case H5T_INTEGER: {
					if (H5Tequal(dtype, H5T_STD_I8LE)) {
						dataString = readHDFData1D<int8_t>(dataset, H5T_STD_I8LE, rows, lines, dataPointers[0]);
					} else if (H5Tequal(dtype, H5T_STD_I8BE)) {
						dataString = readHDFData1D<int8_t>(dataset, H5T_STD_I8BE, rows, lines, dataPointers[0]);
					} else if (H5Tequal(dtype, H5T_NATIVE_CHAR)) {
						switch (sizeof(H5T_NATIVE_CHAR)) {
						case 1:
							dataString = readHDFData1D<int8_t>(dataset, H5T_NATIVE_CHAR, rows, lines, dataPointers[0]);
							break;
						case 2:
							dataString = readHDFData1D<int16_t>(dataset, H5T_NATIVE_CHAR, rows, lines, dataPointers[0]);
							break;
						case 4:
							dataString = readHDFData1D<int32_t>(dataset, H5T_NATIVE_CHAR, rows, lines, dataPointers[0]);
							break;
						case 8:
							dataString = readHDFData1D<int64_t>(dataset, H5T_NATIVE_CHAR, rows, lines, dataPointers[0]);
							break;
						}
					} else if (H5Tequal(dtype, H5T_STD_U8LE)) {
						dataString = readHDFData1D<uint8_t>(dataset, H5T_STD_U8LE, rows, lines, dataPointers[0]);
					} else if (H5Tequal(dtype, H5T_STD_U8BE)) {
						dataString = readHDFData1D<uint8_t>(dataset, H5T_STD_U8BE, rows, lines, dataPointers[0]);
					} else if (H5Tequal(dtype, H5T_NATIVE_UCHAR)) {
						switch (sizeof(H5T_NATIVE_UCHAR)) {
						case 1:
							dataString = readHDFData1D<uint8_t>(dataset, H5T_NATIVE_UCHAR, rows, lines, dataPointers[0]);
							break;
						case 2:
							dataString = readHDFData1D<uint16_t>(dataset, H5T_NATIVE_UCHAR, rows, lines, dataPointers[0]);
							break;
						case 4:
							dataString = readHDFData1D<uint32_t>(dataset, H5T_NATIVE_UCHAR, rows, lines, dataPointers[0]);
							break;
						case 8:
							dataString = readHDFData1D<uint64_t>(dataset, H5T_NATIVE_UCHAR, rows, lines, dataPointers[0]);
							break;
						}
					} else if (H5Tequal(dtype, H5T_STD_I16LE) || H5Tequal(dtype, H5T_STD_I16BE) || H5Tequal(dtype, H5T_NATIVE_SHORT))
						dataString = readHDFData1D<short>(dataset, H5T_NATIVE_SHORT, rows, lines, dataPointers[0]);
					else if (H5Tequal(dtype, H5T_STD_U16LE) || H5Tequal(dtype, H5T_STD_U16BE) || H5Tequal(dtype, H5T_NATIVE_USHORT))
						dataString = readHDFData1D<unsigned short>(dataset, H5T_NATIVE_USHORT, rows, lines, dataPointers[0]);
					else if (H5Tequal(dtype, H5T_STD_I32LE) || H5Tequal(dtype, H5T_STD_I32BE) || H5Tequal(dtype, H5T_NATIVE_INT))
						dataString = readHDFData1D<int>(dataset, H5T_NATIVE_INT, rows, lines, dataPointers[0]);
					else if (H5Tequal(dtype, H5T_STD_U32LE) || H5Tequal(dtype, H5T_STD_U32BE) || H5Tequal(dtype, H5T_NATIVE_UINT))
						dataString = readHDFData1D<unsigned int>(dataset, H5T_NATIVE_UINT, rows, lines, dataPointers[0]);
					else if (H5Tequal(dtype, H5T_NATIVE_LONG))
						dataString = readHDFData1D<long>(dataset, H5T_NATIVE_LONG, rows, lines, dataPointers[0]);
					else if (H5Tequal(dtype, H5T_NATIVE_ULONG))
						dataString = readHDFData1D<unsigned long>(dataset, H5T_NATIVE_ULONG, rows, lines, dataPointers[0]);
					else if (H5Tequal(dtype, H5T_STD_I64LE) || H5Tequal(dtype, H5T_STD_I64BE) || H5Tequal(dtype, H5T_NATIVE_LLONG))
						dataString = readHDFData1D<long long>(dataset, H5T_NATIVE_LLONG, rows, lines, dataPointers[0]);
					else if (H5Tequal(dtype, H5T_STD_U64LE) || H5Tequal(dtype, H5T_STD_U64BE) || H5Tequal(dtype, H5T_NATIVE_ULLONG))
						dataString = readHDFData1D<unsigned long long>(dataset, H5T_NATIVE_ULLONG, rows, lines, dataPointers[0]);
					else {
						ok = false;
						dataString = (QStringList() << i18n("unsupported integer type for rank 1"));
						qDebug() << dataString;
					}


					break;
				}
			case H5T_FLOAT: {
					if (H5Tequal(dtype, H5T_IEEE_F32LE) || H5Tequal(dtype, H5T_IEEE_F32BE))
						dataString = readHDFData1D<float>(dataset, H5T_NATIVE_FLOAT, rows, lines, dataPointers[0]);
					else if (H5Tequal(dtype, H5T_IEEE_F64LE) || H5Tequal(dtype, H5T_IEEE_F64BE))
						dataString = readHDFData1D<double>(dataset, H5T_NATIVE_DOUBLE, rows, lines, dataPointers[0]);
					else if (H5Tequal(dtype, H5T_NATIVE_LDOUBLE))
						dataString = readHDFData1D<long double>(dataset, H5T_NATIVE_LDOUBLE, rows, lines, dataPointers[0]);
					else {
						ok = false;
						dataString = (QStringList() << i18n("unsupported float type for rank 1"));
						qDebug() << dataString;
					}
					break;
				}
			case H5T_COMPOUND: {
					int members = H5Tget_nmembers(dtype);
					handleError(members, "H5Tget_nmembers");
					if (dataSource != NULL) {
						// re-create data pointer
						dataPointers.clear();
						dataSource->create(dataPointers, mode, actualRows, members);
					} else
						dataStrings << readHDFCompound(dtype);
					dataString = readHDFCompoundData1D(dataset, dtype, rows, lines, dataPointers);
					break;
				}
			case H5T_TIME:
			case H5T_BITFIELD:
			case H5T_OPAQUE:
			case H5T_REFERENCE:
			case H5T_ENUM:
			case H5T_VLEN:
			case H5T_ARRAY:
			case H5T_NO_CLASS:
			case H5T_NCLASSES: {
					ok = false;
					dataString = (QStringList() << i18n("rank 1 not implemented yet for type %1").arg(translateHDFClass(dclass)));
					qDebug() << dataString;
				}
			default:
				break;
			}

			if (dataSource == NULL) {
				DEBUG_LOG("dataString =" << dataString);
				for (int i = 0; i < rows; i++)
					dataStrings << (QStringList() << dataString[i]);
			}

			break;
		}
	case 2: {
			hsize_t dims_out[2];
			status = H5Sget_simple_extent_dims(dataspace, dims_out, NULL);
			handleError(status, "H5Sget_simple_extent_dims");
			int rows = dims_out[0];
			int cols = dims_out[1];

			if (endRow == -1)
				endRow=rows;
			if (lines == -1)
				lines=endRow;
			if (endColumn == -1)
				endColumn=cols;
			actualRows = endRow-startRow+1;
			actualCols = endColumn-startColumn+1;

#ifndef NDEBUG
			H5T_order_t order = H5Tget_order(dtype);
			handleError((int)order, "H5Tget_order");
			qDebug()<<translateHDFClass(dclass)<<"("<<typeSize<<")"<<translateHDFOrder(order)<<","<<rows<<"x"<<cols;
			qDebug()<<"startRow/endRow"<<startRow<<endRow;
			qDebug()<<"startColumn/endColumn"<<startColumn<<endColumn;
			qDebug()<<"actual rows/cols"<<actualRows<<actualCols;
			qDebug()<<"lines"<<lines;
#endif

			if (dataSource != NULL)
				columnOffset = dataSource->create(dataPointers, mode, actualRows, actualCols);

			// read data
			switch (dclass) {
			case H5T_INTEGER: {
					if (H5Tequal(dtype, H5T_STD_I8LE)) {
						dataStrings << readHDFData2D<int8_t>(dataset, H5T_STD_I8LE, rows, cols, lines, dataPointers);
					} else if (H5Tequal(dtype, H5T_STD_I8BE)) {
						dataStrings << readHDFData2D<int8_t>(dataset, H5T_STD_I8BE, rows, cols, lines, dataPointers);
					} else if (H5Tequal(dtype, H5T_NATIVE_CHAR)) {
						switch (sizeof(H5T_NATIVE_CHAR)) {
						case 1:
							dataStrings << readHDFData2D<int8_t>(dataset, H5T_NATIVE_CHAR, rows, cols, lines, dataPointers);
							break;
						case 2:
							dataStrings << readHDFData2D<int16_t>(dataset, H5T_NATIVE_CHAR, rows, cols, lines, dataPointers);
							break;
						case 4:
							dataStrings << readHDFData2D<int32_t>(dataset, H5T_NATIVE_CHAR, rows, cols, lines, dataPointers);
							break;
						case 8:
							dataStrings << readHDFData2D<int64_t>(dataset, H5T_NATIVE_CHAR, rows, cols, lines, dataPointers);
							break;
						}
					} else if (H5Tequal(dtype, H5T_STD_U8LE)) {
						dataStrings << readHDFData2D<uint8_t>(dataset, H5T_STD_U8LE, rows, cols, lines, dataPointers);
					} else if (H5Tequal(dtype, H5T_STD_U8BE)) {
						dataStrings << readHDFData2D<uint8_t>(dataset, H5T_STD_U8BE, rows, cols, lines, dataPointers);
					} else if (H5Tequal(dtype, H5T_NATIVE_UCHAR)) {
						switch (sizeof(H5T_NATIVE_UCHAR)) {
						case 1:
							dataStrings << readHDFData2D<uint8_t>(dataset, H5T_NATIVE_UCHAR, rows, cols, lines, dataPointers);
							break;
						case 2:
							dataStrings << readHDFData2D<uint16_t>(dataset, H5T_NATIVE_UCHAR, rows, cols, lines, dataPointers);
							break;
						case 4:
							dataStrings << readHDFData2D<uint32_t>(dataset, H5T_NATIVE_UCHAR, rows, cols, lines, dataPointers);
							break;
						case 8:
							dataStrings << readHDFData2D<uint64_t>(dataset, H5T_NATIVE_UCHAR, rows, cols, lines, dataPointers);
							break;
						}
					} else if (H5Tequal(dtype, H5T_STD_I16LE) || H5Tequal(dtype, H5T_STD_I16BE) || H5Tequal(dtype, H5T_NATIVE_SHORT))
						dataStrings << readHDFData2D<short>(dataset, H5T_NATIVE_SHORT, rows, cols, lines, dataPointers);
					else if (H5Tequal(dtype, H5T_STD_U16LE) || H5Tequal(dtype, H5T_STD_U16BE) || H5Tequal(dtype, H5T_NATIVE_USHORT))
						dataStrings << readHDFData2D<unsigned short>(dataset, H5T_NATIVE_USHORT, rows, cols, lines, dataPointers);
					else if (H5Tequal(dtype, H5T_STD_I32LE) || H5Tequal(dtype, H5T_STD_I32BE) || H5Tequal(dtype, H5T_NATIVE_INT))
						dataStrings << readHDFData2D<int>(dataset, H5T_NATIVE_INT, rows, cols, lines, dataPointers);
					else if (H5Tequal(dtype, H5T_STD_U32LE) || H5Tequal(dtype, H5T_STD_U32BE) || H5Tequal(dtype, H5T_NATIVE_UINT))
						dataStrings << readHDFData2D<unsigned int>(dataset, H5T_NATIVE_UINT, rows, cols, lines, dataPointers);
					else if (H5Tequal(dtype, H5T_NATIVE_LONG))
						dataStrings << readHDFData2D<long>(dataset, H5T_NATIVE_LONG, rows, cols, lines, dataPointers);
					else if (H5Tequal(dtype, H5T_NATIVE_ULONG))
						dataStrings << readHDFData2D<unsigned long>(dataset, H5T_NATIVE_ULONG, rows, cols, lines, dataPointers);
					else if (H5Tequal(dtype, H5T_STD_I64LE) || H5Tequal(dtype, H5T_STD_I64BE) || H5Tequal(dtype, H5T_NATIVE_LLONG))
						dataStrings << readHDFData2D<long long>(dataset, H5T_NATIVE_LLONG, rows, cols, lines, dataPointers);
					else if (H5Tequal(dtype, H5T_STD_U64LE) || H5Tequal(dtype, H5T_STD_U64BE) || H5Tequal(dtype, H5T_NATIVE_ULLONG))
						dataStrings << readHDFData2D<unsigned long long>(dataset, H5T_NATIVE_ULLONG, rows, cols, lines, dataPointers);
					else {
						ok=false;
						dataStrings << (QStringList() << i18n("unsupported integer type for rank 2"));
						qDebug() << dataStrings;
					}
					break;
				}
			case H5T_FLOAT: {
					if (H5Tequal(dtype, H5T_IEEE_F32LE) || H5Tequal(dtype, H5T_IEEE_F32BE))
						dataStrings << readHDFData2D<float>(dataset, H5T_NATIVE_FLOAT, rows, cols, lines, dataPointers);
					else if (H5Tequal(dtype, H5T_IEEE_F64LE) || H5Tequal(dtype, H5T_IEEE_F64BE))
						dataStrings << readHDFData2D<double>(dataset, H5T_NATIVE_DOUBLE, rows, cols, lines, dataPointers);
					else if (H5Tequal(dtype, H5T_NATIVE_LDOUBLE))
						dataStrings << readHDFData2D<long double>(dataset, H5T_NATIVE_LDOUBLE, rows, cols, lines, dataPointers);
					else {
						ok = false;
						dataStrings << (QStringList() << i18n("unsupported float type for rank 2"));
						qDebug() << dataStrings;
					}
					break;
				}
			case H5T_COMPOUND: {
					dataStrings << readHDFCompound(dtype);
					qDebug() << dataStrings;
					dataStrings << readHDFCompoundData2D(dataset,dtype,rows,cols,lines);
					break;
				}
			case H5T_STRING: {
					// TODO: implement this
					ok = false;
					dataStrings << (QStringList() << i18n("rank 2 not implemented yet for type %1").arg(translateHDFClass(dclass))
						+ ", " + i18n("size = %1").arg(typeSize));
					qDebug() << dataStrings;
					break;
				}
			case H5T_TIME:
			case H5T_BITFIELD:
			case H5T_OPAQUE:
			case H5T_REFERENCE:
			case H5T_ENUM:
			case H5T_VLEN:
			case H5T_ARRAY:
			case H5T_NO_CLASS:
			case H5T_NCLASSES: {
					ok = false;
					dataStrings << (QStringList() << i18n("rank 2 not implemented yet for type %1").arg(translateHDFClass(dclass)));
					qDebug() << dataStrings;
				}
			default:
				break;
			}
			break;
		}
	default: {
			ok = false;
			dataStrings << (QStringList() << i18n("rank %1 not implemented yet for type %2").arg(rank).arg(translateHDFClass(dclass)));
			qDebug() << dataStrings;
		}
	}

	status = H5Sclose(dataspace);
	handleError(status, "H5Sclose");
	status = H5Tclose(dtype);
	handleError(status, "H5Tclose");
	status = H5Dclose(dataset);
	handleError(status, "H5Dclose");
	status = H5Fclose(file);
	handleError(status, "H5Fclose");

	if (!dataSource)
		return dataStrings;

	// make everything undo/redo-able again
	// set column comments in spreadsheet
	Spreadsheet* spreadsheet = dynamic_cast<Spreadsheet*>(dataSource);
	if (spreadsheet) {
		QString comment = i18np("numerical data, %1 element", "numerical data, %1 elements", actualRows);
		for (int n = 0; n < actualCols; n++) {
			Column* column = spreadsheet->column(columnOffset+n);
			column->setComment(comment);
			column->setUndoAware(true);
			if (mode == AbstractFileFilter::Replace) {
				column->setSuppressDataChangedSignal(false);
				column->setChanged();
			}
		}
		spreadsheet->setUndoAware(true);
		return dataStrings;
	}

	Matrix* matrix = dynamic_cast<Matrix*>(dataSource);
	if (matrix) {
		matrix->setSuppressDataChangedSignal(false);
		matrix->setChanged();
		matrix->setUndoAware(true);
	}

#else
	Q_UNUSED(fileName)
	Q_UNUSED(dataSource)
	Q_UNUSED(mode)
	Q_UNUSED(lines)
#endif

	return dataStrings;
}

/*!
    reads the content of the file \c fileName to the data source \c dataSource.
    Uses the settings defined in the data source.
*/
void HDFFilterPrivate::read(const QString & fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode mode) {
	DEBUG_LOG("HDFFilter::read()");
	if (currentDataSetName.isEmpty()) {
		qDebug() << i18n("No data set selected");
		return;
	}

	bool ok = true;
	readCurrentDataSet(fileName, dataSource, ok, mode);
}

/*!
    writes the content of \c dataSource to the file \c fileName.
*/
void HDFFilterPrivate::write(const QString & fileName, AbstractDataSource* dataSource) {
	Q_UNUSED(fileName);
	Q_UNUSED(dataSource);
	//TODO
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################

/*!
  Saves as XML.
 */
void HDFFilter::save(QXmlStreamWriter* writer) const {
	writer->writeStartElement("hdfFilter");
	writer->writeEndElement();
}

/*!
  Loads from XML.
*/
bool HDFFilter::load(XmlStreamReader* reader) {
	if (!reader->isStartElement() || reader->name() != "hdfFilter") {
		reader->raiseError(i18n("no hdf filter element found"));
		return false;
	}

	QString attributeWarning = i18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs = reader->attributes();
	return true;
}
