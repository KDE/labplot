/***************************************************************************
File                 : HDF5Filter.cpp
Project              : LabPlot
Description          : HDF5 I/O-filter
--------------------------------------------------------------------
Copyright            : (C) 2015-2018 by Stefan Gerlach (stefan.gerlach@uni.kn)
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

#include "backend/datasources/filters/HDF5Filter.h"
#include "backend/datasources/filters/HDF5FilterPrivate.h"
#include "backend/datasources/LiveDataSource.h"
#include "backend/core/column/Column.h"

#include <KLocalizedString>
#include <QTreeWidgetItem>
#include <QProcess>
#include <QFile>

/*!
	\class HDF5Filter
	\brief Manages the import/export of data from/to a HDF5 file.

	\ingroup datasources
*/
HDF5Filter::HDF5Filter():AbstractFileFilter(FileType::HDF5), d(new HDF5FilterPrivate(this)) {}

HDF5Filter::~HDF5Filter() = default;

/*!
  parses the content of the file \c fileName.
*/
void HDF5Filter::parse(const QString & fileName, QTreeWidgetItem* rootItem) {
	d->parse(fileName, rootItem);
}

/*!
  reads the content of the data set \c dataSet from file \c fileName.
*/
QVector<QStringList> HDF5Filter::readCurrentDataSet(const QString& fileName, AbstractDataSource* dataSource, bool &ok, AbstractFileFilter::ImportMode importMode, int lines) {
	return d->readCurrentDataSet(fileName, dataSource, ok, importMode, lines);
}

/*!
  reads the content of the file \c fileName to the data source \c dataSource.
*/
void HDF5Filter::readDataFromFile(const QString& fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode mode) {
	d->readDataFromFile(fileName, dataSource, mode);
}

/*!
writes the content of the data source \c dataSource to the file \c fileName.
*/
void HDF5Filter::write(const QString& fileName, AbstractDataSource* dataSource) {
	d->write(fileName, dataSource);
}

///////////////////////////////////////////////////////////////////////
/*!
  loads the predefined filter settings for \c filterName
*/
void HDF5Filter::loadFilterSettings(const QString& filterName) {
	Q_UNUSED(filterName);
}

/*!
  saves the current settings as a new filter with the name \c filterName
*/
void HDF5Filter::saveFilterSettings(const QString& filterName) const {
	Q_UNUSED(filterName);
}

///////////////////////////////////////////////////////////////////////

void HDF5Filter::setCurrentDataSetName(const QString& ds) {
	d->currentDataSetName = ds;
}

const QString HDF5Filter::currentDataSetName() const {
	return d->currentDataSetName;
}

void HDF5Filter::setStartRow(const int s) {
	d->startRow = s;
}

int HDF5Filter::startRow() const {
	return d->startRow;
}

void HDF5Filter::setEndRow(const int e) {
	d->endRow = e;
}

int HDF5Filter::endRow() const {
	return d->endRow;
}

void HDF5Filter::setStartColumn(const int c) {
	d->startColumn = c;
}

int HDF5Filter::startColumn() const {
	return d->startColumn;
}

void HDF5Filter::setEndColumn(const int c) {
	d->endColumn = c;
}

int HDF5Filter::endColumn() const {
	return d->endColumn;
}

QString HDF5Filter::fileInfoString(const QString& fileName) {
	DEBUG("HDF5Filter::fileInfoString()");
	QString info;
#ifdef HAVE_HDF5
	DEBUG("fileName = " << qPrintable(fileName));

	// check file type first
	htri_t isHdf5 = H5Fis_hdf5(qPrintable(fileName));
	if (isHdf5 == 0) {
		DEBUG(qPrintable(fileName) << " is not a HDF5 file! isHdf5 = " << isHdf5 << " Giving up.");
		return i18n("Not a HDF5 file");
	}
	if (isHdf5 < 0) {
		DEBUG("H5Fis_hdf5() failed on " << qPrintable(fileName) << "! Giving up.");
		return i18n("Failed checking file");
	}

	// open file
	hid_t file = H5Fopen(qPrintable(fileName), H5F_ACC_RDONLY, H5P_DEFAULT);
	HDF5FilterPrivate::handleError((int)file, "H5Fopen", fileName);
	if (file < 0) {
		DEBUG("Opening file " << qPrintable(fileName) << " failed! Giving up.");
		return i18n("Failed opening HDF5 file");
	}

	hsize_t size;
	herr_t status = H5Fget_filesize(file, &size);
	if (status >= 0) {
		info += i18n("File size: %1 bytes", QString::number(size));
		info += QLatin1String("<br>");
	}

	hssize_t freesize = H5Fget_freespace(file);
	info += i18n("Free space: %1 bytes", QString::number(freesize));
	info += QLatin1String("<br>");
	info += QLatin1String("<br>");

	ssize_t objectCount;
	objectCount = H5Fget_obj_count(file, H5F_OBJ_FILE);
	info += i18n("Number of files: %1", QString::number(objectCount));
	info += QLatin1String("<br>");
	objectCount = H5Fget_obj_count(file, H5F_OBJ_DATASET);
	info += i18n("Number of data sets: %1", QString::number(objectCount));
	info += QLatin1String("<br>");
	objectCount = H5Fget_obj_count(file, H5F_OBJ_GROUP);
	info += i18n("Number of groups: %1", QString::number(objectCount));
	info += QLatin1String("<br>");
	objectCount = H5Fget_obj_count(file, H5F_OBJ_DATATYPE);
	info += i18n("Number of named datatypes: %1", QString::number(objectCount));
	info += QLatin1String("<br>");
	objectCount = H5Fget_obj_count(file, H5F_OBJ_ATTR);
	info += i18n("Number of attributes: %1", QString::number(objectCount));
	info += QLatin1String("<br>");
	objectCount = H5Fget_obj_count(file, H5F_OBJ_ALL);
	info += i18n("Number of all objects: %1", QString::number(objectCount));
	info += QLatin1String("<br>");

#ifdef HAVE_AT_LEAST_HDF5_1_10_0	// using H5Fget_info2 struct (see H5Fpublic.h)
	H5F_info2_t file_info;
	status = H5Fget_info2(file, &file_info);
	if (status >= 0) {
		info += QLatin1String("<br>");
		info += i18n("Version of superblock: %1", QString::number(file_info.super.version));
		info += QLatin1String("<br>");
		info += i18n("Size of superblock: %1 bytes", QString::number(file_info.super.super_size));
		info += QLatin1String("<br>");
		info += i18n("Size of superblock extension: %1 bytes", QString::number(file_info.super.super_ext_size));
		info += QLatin1String("<br>");
		info += i18n("Version of free-space manager: %1", QString::number(file_info.free.version));
		info += QLatin1String("<br>");
		info += i18n("Size of free-space manager metadata: %1 bytes", QString::number(file_info.free.meta_size));
		info += QLatin1String("<br>");
		info += i18n("Total size of free space: %1 bytes", QString::number(file_info.free.tot_space));
		info += QLatin1String("<br>");
		info += i18n("Version of shared object header: %1", QString::number(file_info.sohm.version));
		info += QLatin1String("<br>");
		info += i18n("Size of shared object header: %1 bytes", QString::number(file_info.sohm.hdr_size));
		info += QLatin1String("<br>");
		info += i18n("Size of all shared object header indexes: %1 bytes", QString::number(file_info.sohm.msgs_info.index_size));
		info += QLatin1String("<br>");
		info += i18n("Size of the heap: %1 bytes", QString::number(file_info.sohm.msgs_info.heap_size));
		info += QLatin1String("<br>");
	}
#else	// using H5Fget_info1 struct (named H5F_info_t in HDF5 1.8)
	H5F_info_t file_info;
	status = H5Fget_info(file, &file_info);
	if (status >= 0) {
		info += i18n("Size of superblock extension: %1 bytes", QString::number(file_info.super_ext_size));
		info += QLatin1String("<br>");
		info += i18n("Size of shared object header: %1 bytes", QString::number(file_info.sohm.hdr_size));
		info += QLatin1String("<br>");
		info += i18n("Size of all shared object header indexes: %1 bytes", QString::number(file_info.sohm.msgs_info.index_size));
		info += QLatin1String("<br>");
		info += i18n("Size of the heap: %1 bytes", QString::number(file_info.sohm.msgs_info.heap_size));
		info += QLatin1String("<br>");
	}
#endif

	// cache information
	//see https://support.hdfgroup.org/HDF5/doc/RM/RM_H5F.html
	info += QLatin1String("<br>");
	H5AC_cache_config_t config;
	config.version = H5AC__CURR_CACHE_CONFIG_VERSION;
	status = H5Fget_mdc_config(file, &config);
	if (status >= 0) {
		info += i18n("Cache config version: %1", QString::number(config.version));
		info += QLatin1String("<br>");
		info += i18n("Adaptive cache resize report function enabled: %1", config.rpt_fcn_enabled ? i18n("Yes") : i18n("No"));
		info += QLatin1String("<br>");
		info += i18n("Cache initial maximum size: %1 bytes", QString::number(config.initial_size));
		info += QLatin1String("<br>");
		info += i18n("Adaptive cache maximum size: %1 bytes", QString::number(config.max_size));
		info += QLatin1String("<br>");
		info += i18n("Adaptive cache minimum size: %1 bytes", QString::number(config.min_size));
		info += QLatin1String("<br>");
		//TODO: more settings
	}
	double hit_rate;
	status = H5Fget_mdc_hit_rate(file, &hit_rate);
	Q_UNUSED(status);
	info += i18n("Metadata cache hit rate: %1", QString::number(hit_rate));
	info += QLatin1String("<br>");
	//TODO: herr_t H5Fget_mdc_image_info(hid_t file_id, haddr_t *image_addr, hsize_t *image_len)
	size_t max_size, min_clean_size, cur_size;
	int cur_num_entries;
	status = H5Fget_mdc_size(file, &max_size, &min_clean_size, &cur_size, &cur_num_entries);
	if (status >= 0) {
		info += i18n("Current cache maximum size: %1 bytes", QString::number(max_size));
		info += QLatin1String("<br>");
		info += i18n("Current cache minimum clean size: %1 bytes", QString::number(min_clean_size));
		info += QLatin1String("<br>");
		info += i18n("Current cache size: %1 bytes", QString::number(cur_size));
		info += QLatin1String("<br>");
		info += i18n("Current number of entries in the cache: %1", QString::number(cur_num_entries));
		info += QLatin1String("<br>");
	}
	//TODO: 1.10 herr_t H5Fget_metadata_read_retry_info( hid_t file_id, H5F_retry_info_t *info )
	/* TODO: not available
	hbool_t atomicMode;
	status = H5Fget_mpi_atomicity(file, &atomicMode);
	if (status >= 0) {
		info += i18n("MPI file access atomic mode: %1", atomicMode ? i18n("Yes") : i18n("No"));
		info += QLatin1String("<br>");
	}*/
#ifdef HAVE_AT_LEAST_HDF5_1_10_0
	hbool_t is_enabled, is_currently_logging;
	status = H5Fget_mdc_logging_status(file, &is_enabled, &is_currently_logging);
	if (status >= 0) {
		info += i18n("Logging enabled: %1", is_enabled ? i18n("Yes") : i18n("No"));
		info += QLatin1String("<br>");
		info += i18n("Events are currently logged: %1", is_currently_logging ? i18n("Yes") : i18n("No"));
		info += QLatin1String("<br>");
	}
#endif
#ifdef HAVE_AT_LEAST_HDF5_1_10_1
	unsigned int accesses[2], hits[2], misses[2], evictions[2], bypasses[2];
	status = H5Fget_page_buffering_stats(file, accesses, hits, misses, evictions, bypasses);
	if (status >= 0) {
		info += i18n("Metadata/raw data page buffer accesses: %1 %2", QString::number(accesses[0]), QString::number(accesses[1]));
		info += QLatin1String("<br>");
		info += i18n("Metadata/raw data page buffer hits: %1 %2", QString::number(hits[0]), QString::number(hits[1]));
		info += QLatin1String("<br>");
		info += i18n("Metadata/raw data page buffer misses: %1 %2", QString::number(misses[0]), QString::number(misses[1]));
		info += QLatin1String("<br>");
		info += i18n("Metadata/raw data page buffer evictions: %1 %2", QString::number(evictions[0]), QString::number(evictions[1]));
		info += QLatin1String("<br>");
		info += i18n("Metadata/raw data accesses bypassing page buffer: %1 %2", QString::number(bypasses[0]), QString::number(bypasses[1]));
		info += QLatin1String("<br>");
	} else {
		info += i18n("Page buffer disabled");
		info += QLatin1String("<br>");
		DEBUG("H5Fget_page_buffering_stats() status = " << status);
	}
#endif
#else
	Q_UNUSED(fileName);
#endif
	return info;
}

/*!
 * Get file content in DDL (Data Description Language) format
 * uses "h5dump"
 */
QString HDF5Filter::fileDDLString(const QString& fileName) {
	DEBUG("HDF5Filter::fileDDLString()");

	QString DDLString;
#ifdef Q_OS_LINUX
	auto* proc = new QProcess();
	QStringList args;
	args << "-H" << fileName;
	proc->start( "h5dump", args);

	if (proc->waitForReadyRead(1000) == false)
		DDLString += i18n("Reading from file %1 failed.", fileName);
	else {
		DDLString += proc->readAll();
		DDLString.replace('\n', "<br>\n");
		DDLString.replace("\t","&nbsp;&nbsp;&nbsp;&nbsp;");
		//DEBUG("	DDL string: " << STDSTRING(DDLString));
	}
#else	//TODO: h5dump on Win, Mac
	Q_UNUSED(fileName)
#endif

	return DDLString;
}

//#####################################################################
//################### Private implementation ##########################
//#####################################################################

HDF5FilterPrivate::HDF5FilterPrivate(HDF5Filter* owner) : q(owner) {
#ifdef HAVE_HDF5
	m_status = 0;
#endif
}

#ifdef HAVE_HDF5
void HDF5FilterPrivate::handleError(int err, const QString& function, const QString& arg) {
#ifdef NDEBUG
	Q_UNUSED(err)
	Q_UNUSED(function)
	Q_UNUSED(arg)
#else
	if (err < 0) {
		DEBUG("ERROR " << err << ": " << STDSTRING(function) << "() - " << STDSTRING(arg));
	}
#endif
}

QString HDF5FilterPrivate::translateHDF5Order(H5T_order_t o) {
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

QString HDF5FilterPrivate::translateHDF5Type(hid_t t) {
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

QString HDF5FilterPrivate::translateHDF5Class(H5T_class_t c) {
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

QStringList HDF5FilterPrivate::readHDF5Compound(hid_t tid) {
	size_t typeSize = H5Tget_size(tid);

	QString line;
	line += QLatin1String("COMPOUND(") + QString::number(typeSize) + QLatin1String(") : (");
	int members = H5Tget_nmembers(tid);
	handleError(members, "H5Tget_nmembers");
	for (int i = 0; i < members; ++i) {
		H5T_class_t mclass = H5Tget_member_class(tid, i);
		handleError((int)mclass, "H5Tget_member_class");
		hid_t mtype = H5Tget_member_type(tid, i);
		handleError((int)mtype, "H5Tget_member_type");
		size_t size = H5Tget_size(mtype);
		handleError((int)size, "H5Tget_size");
		QString typeString = translateHDF5Class(mclass);
		if (mclass == H5T_INTEGER || mclass == H5T_FLOAT)
			typeString = translateHDF5Type(mtype);
		line += H5Tget_member_name(tid, i) + QLatin1String("[") + typeString + QLatin1String("(") + QString::number(size) + QLatin1String(")]");
		if (i == members-1)
			line += QLatin1String(")");
		else
			line += QLatin1String(",");
		m_status = H5Tclose(mtype);
		handleError(m_status, "H5Tclose");
	}

	QStringList dataString;
	dataString << line;

	return dataString;
}

template <typename T>
QStringList HDF5FilterPrivate::readHDF5Data1D(hid_t dataset, hid_t type, int rows, int lines, void* dataContainer) {
	DEBUG("readHDF5Data1D() rows = " << rows << ", lines = " << lines);
	QStringList dataString;

	// we read all rows of data
	T* data = new T[rows];

	m_status = H5Dread(dataset, type, H5S_ALL, H5S_ALL, H5P_DEFAULT, data);
	handleError(m_status, "H5Dread");
	DEBUG(" startRow = " << startRow << ", endRow = " << endRow);
	DEBUG("	dataContainer = " << dataContainer);
	for (int i = startRow - 1; i < qMin(endRow, lines + startRow - 1); ++i) {
		if (dataContainer)	// read to data source
			static_cast<QVector<double>*>(dataContainer)->operator[](i-startRow+1) = data[i];
		else				// for preview
			dataString << QString::number(static_cast<double>(data[i]));
	}
	delete[] data;

	return dataString;
}

QStringList HDF5FilterPrivate::readHDF5CompoundData1D(hid_t dataset, hid_t tid, int rows, int lines, std::vector<void*>& dataContainer) {
	DEBUG("HDF5FilterPrivate::readHDF5CompoundData1D()");
	DEBUG(" dataContainer size = " << dataContainer.size());
	int members = H5Tget_nmembers(tid);
	handleError(members, "H5Tget_nmembers");
	DEBUG(" # members = " << members);

	QStringList dataString;
	if (!dataContainer[0]) {
		for (int i = 0; i < qMin(rows, lines); ++i)
			dataString <<  QLatin1String("(");
		dataContainer.resize(members);	// avoid "index out of range" for preview
	}

	for (int m = 0; m < members; ++m) {
		hid_t mtype = H5Tget_member_type(tid, m);
		handleError((int)mtype, "H5Tget_member_type");
		size_t msize = H5Tget_size(mtype);
		handleError((int)msize, "H5Tget_size");
		hid_t ctype = H5Tcreate(H5T_COMPOUND, msize);
		handleError((int)ctype, "H5Tcreate");
		m_status = H5Tinsert(ctype, H5Tget_member_name(tid, m), 0, mtype);
		handleError(m_status, "H5Tinsert");

		QStringList mdataString;
		if (H5Tequal(mtype, H5T_STD_I8LE) || H5Tequal(mtype, H5T_STD_I8BE))
			mdataString = readHDF5Data1D<qint8>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, lines, dataContainer[m]);
		else if (H5Tequal(mtype, H5T_NATIVE_CHAR)) {
			switch (sizeof(H5T_NATIVE_CHAR)) {
			case 1:
				mdataString = readHDF5Data1D<qint8>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, lines, dataContainer[m]);
				break;
			case 2:
				mdataString = readHDF5Data1D<qint16>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, lines, dataContainer[m]);
				break;
			case 4:
				mdataString = readHDF5Data1D<qint32>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, lines, dataContainer[m]);
				break;
			case 8:
				mdataString = readHDF5Data1D<qint64>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, lines, dataContainer[m]);
				break;
			}
		} else if (H5Tequal(mtype, H5T_STD_U8LE) || H5Tequal(mtype, H5T_STD_U8BE))
			mdataString = readHDF5Data1D<uint8_t>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, lines, dataContainer[m]);
		else if (H5Tequal(mtype, H5T_NATIVE_UCHAR)) {
			switch (sizeof(H5T_NATIVE_UCHAR)) {
			case 1:
				mdataString = readHDF5Data1D<uint8_t>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, lines, dataContainer[m]);
				break;
			case 2:
				mdataString = readHDF5Data1D<uint16_t>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, lines, dataContainer[m]);
				break;
			case 4:
				mdataString = readHDF5Data1D<uint32_t>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, lines, dataContainer[m]);
				break;
			case 8:
				mdataString = readHDF5Data1D<uint64_t>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, lines, dataContainer[m]);
				break;
			}
		} else if (H5Tequal(mtype, H5T_STD_I16LE) || H5Tequal(mtype, H5T_STD_I16BE) || H5Tequal(mtype, H5T_NATIVE_SHORT))
			mdataString = readHDF5Data1D<short>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, lines, dataContainer[m]);
		else if (H5Tequal(mtype, H5T_STD_U16LE) || H5Tequal(mtype, H5T_STD_U16BE) || H5Tequal(mtype, H5T_NATIVE_SHORT))
			mdataString = readHDF5Data1D<unsigned short>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, lines, dataContainer[m]);
		else if (H5Tequal(mtype, H5T_STD_I32LE) || H5Tequal(mtype, H5T_STD_I32BE) || H5Tequal(mtype, H5T_NATIVE_INT))
			mdataString = readHDF5Data1D<int>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, lines, dataContainer[m]);
		else if (H5Tequal(mtype, H5T_STD_U32LE) || H5Tequal(mtype, H5T_STD_U32BE) || H5Tequal(mtype, H5T_NATIVE_UINT))
			mdataString = readHDF5Data1D<unsigned int>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, lines, dataContainer[m]);
		else if (H5Tequal(mtype, H5T_NATIVE_LONG))
			mdataString = readHDF5Data1D<long>(dataset, ctype, rows, lines, dataContainer[m]);
		else if (H5Tequal(mtype, H5T_NATIVE_ULONG))
			mdataString = readHDF5Data1D<unsigned long>(dataset, ctype, rows, lines, dataContainer[m]);
		else if (H5Tequal(mtype, H5T_STD_I64LE) || H5Tequal(mtype, H5T_STD_I64BE) || H5Tequal(mtype, H5T_NATIVE_LLONG))
			mdataString = readHDF5Data1D<long long>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, lines, dataContainer[m]);
		else if (H5Tequal(mtype, H5T_STD_U64LE) || H5Tequal(mtype, H5T_STD_U64BE) || H5Tequal(mtype, H5T_NATIVE_ULLONG))
			mdataString = readHDF5Data1D<unsigned long long>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, lines, dataContainer[m]);
		else if (H5Tequal(mtype, H5T_IEEE_F32LE) || H5Tequal(mtype, H5T_IEEE_F32BE))
			mdataString = readHDF5Data1D<float>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, lines, dataContainer[m]);
		else if (H5Tequal(mtype, H5T_IEEE_F64LE) || H5Tequal(mtype, H5T_IEEE_F64BE))
			mdataString = readHDF5Data1D<double>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, lines, dataContainer[m]);
		else if (H5Tequal(mtype, H5T_NATIVE_LDOUBLE))
			mdataString = readHDF5Data1D<long double>(dataset, ctype, rows, lines, dataContainer[m]);
		else {
			if (dataContainer[m]) {
				for (int i = startRow-1; i < qMin(endRow, lines + startRow - 1); ++i)
					static_cast<QVector<double>*>(dataContainer[m])->operator[](i - startRow + 1) = 0;
			} else {
				for (int i = 0; i < qMin(rows, lines); ++i)
					mdataString << QLatin1String("_");
			}
			H5T_class_t mclass = H5Tget_member_class(tid, m);
			handleError((int)mclass, "H5Tget_member_class");
			DEBUG("unsupported type of class " << STDSTRING(translateHDF5Class(mclass)));
		}

		if (!dataContainer[0]) {
			for (int i = 0; i < qMin(rows, lines); ++i) {
				dataString[i] +=  mdataString[i];
				if (m < members - 1)
					dataString[i] += QLatin1String(",");
			}
		}

		H5Tclose(ctype);
	}

	if (!dataContainer[0]) {
		for (int i = 0; i < qMin(rows, lines); ++i)
			dataString[i] +=  QLatin1String(")");
	}

	return dataString;
}

template <typename T>
QVector<QStringList> HDF5FilterPrivate::readHDF5Data2D(hid_t dataset, hid_t type, int rows, int cols, int lines, std::vector<void*>& dataPointer) {
	DEBUG("readHDF5Data2D() rows = " << rows << ", cols =" << cols << ", lines =" << lines);
	QVector<QStringList> dataStrings;

	if (rows == 0 || cols == 0)
		return dataStrings;

	T** data = (T**) malloc(rows*sizeof(T*));
	data[0] = (T*) malloc(cols*rows*sizeof(T));
	for (int i = 1; i < rows; ++i)
		data[i] = data[0] + i*cols;

	m_status = H5Dread(dataset, type, H5S_ALL, H5S_ALL, H5P_DEFAULT, &data[0][0]);
	handleError(m_status,"H5Dread");

	for (int i = 0; i < qMin(rows, lines); ++i) {
		QStringList line;
		line.reserve(cols);
		for (int j = 0; j < cols; ++j) {
			if (dataPointer[0])
				static_cast<QVector<double>*>(dataPointer[j-startColumn+1])->operator[](i-startRow+1) = data[i][j];
			else
				line << QString::number(static_cast<double>(data[i][j]));
		}
		dataStrings << line;
	}
	free(data[0]);
	free(data);

	QDEBUG(dataStrings);
	return dataStrings;
}

QVector<QStringList> HDF5FilterPrivate::readHDF5CompoundData2D(hid_t dataset, hid_t tid, int rows, int cols, int lines) {
	DEBUG("readHDF5CompoundData2D() rows =" << rows << "cols =" << cols << "lines =" << lines);

	int members = H5Tget_nmembers(tid);
	handleError(members, "H5Tget_nmembers");
	DEBUG(" # members =" << members);

	QVector<QStringList> dataStrings;
	for (int i = 0; i < qMin(rows, lines); ++i) {
		QStringList lineStrings;
		for (int j = 0; j < cols; ++j)
			lineStrings << QLatin1String("(");
		dataStrings << lineStrings;
	}

	//QStringList* data = new QStringList[members];
	for (int m = 0; m < members; ++m) {
		hid_t mtype = H5Tget_member_type(tid, m);
		handleError((int)mtype, "H5Tget_member_type");
		size_t msize = H5Tget_size(mtype);
		handleError((int)msize, "H5Tget_size");
		hid_t ctype = H5Tcreate(H5T_COMPOUND, msize);
		handleError((int)ctype, "H5Tcreate");
		m_status = H5Tinsert(ctype, H5Tget_member_name(tid, m), 0, mtype);
		handleError(m_status, "H5Tinsert");

		// dummy container for all data columns
		// initially contains one pointer set to NULL
		std::vector<void*> dummy(1, nullptr);
		QVector<QStringList> mdataStrings;
		if (H5Tequal(mtype, H5T_STD_I8LE) || H5Tequal(mtype, H5T_STD_I8BE))
			mdataStrings = readHDF5Data2D<qint8>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, cols, lines, dummy);
		else if (H5Tequal(mtype, H5T_NATIVE_CHAR)) {
			switch (sizeof(H5T_NATIVE_CHAR)) {
			case 1:
				mdataStrings = readHDF5Data2D<qint8>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, cols, lines, dummy);
				break;
			case 2:
				mdataStrings = readHDF5Data2D<qint16>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, cols, lines, dummy);
				break;
			case 4:
				mdataStrings = readHDF5Data2D<qint32>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, cols, lines, dummy);
				break;
			case 8:
				mdataStrings = readHDF5Data2D<qint64>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, cols, lines, dummy);
				break;
			}
		} else if (H5Tequal(mtype, H5T_STD_U8LE) || H5Tequal(mtype, H5T_STD_U8BE))
			mdataStrings = readHDF5Data2D<uint8_t>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, cols, lines, dummy);
		else if (H5Tequal(mtype, H5T_NATIVE_UCHAR)) {
			switch (sizeof(H5T_NATIVE_UCHAR)) {
			case 1:
				mdataStrings = readHDF5Data2D<uint8_t>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, cols, lines, dummy);
				break;
			case 2:
				mdataStrings = readHDF5Data2D<uint16_t>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, cols, lines, dummy);
				break;
			case 4:
				mdataStrings = readHDF5Data2D<uint32_t>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, cols, lines, dummy);
				break;
			case 8:
				mdataStrings = readHDF5Data2D<uint64_t>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, cols, lines, dummy);
				break;
			}
		} else if (H5Tequal(mtype, H5T_STD_I16LE) || H5Tequal(mtype, H5T_STD_I16BE)|| H5Tequal(mtype, H5T_NATIVE_SHORT))
			mdataStrings = readHDF5Data2D<short>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, cols, lines, dummy);
		else if (H5Tequal(mtype, H5T_STD_U16LE) || H5Tequal(mtype, H5T_STD_U16BE) || H5Tequal(mtype, H5T_NATIVE_USHORT))
			mdataStrings = readHDF5Data2D<unsigned short>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, cols, lines, dummy);
		else if (H5Tequal(mtype, H5T_STD_I32LE) || H5Tequal(mtype, H5T_STD_I32BE) || H5Tequal(mtype, H5T_NATIVE_INT))
			mdataStrings = readHDF5Data2D<int>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, cols, lines, dummy);
		else if (H5Tequal(mtype, H5T_STD_U32LE) || H5Tequal(mtype, H5T_STD_U32BE) || H5Tequal(mtype, H5T_NATIVE_UINT))
			mdataStrings = readHDF5Data2D<unsigned int>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, cols, lines, dummy);
		else if (H5Tequal(mtype, H5T_NATIVE_LONG))
			mdataStrings = readHDF5Data2D<long>(dataset, ctype, rows, cols, lines, dummy);
		else if (H5Tequal(mtype, H5T_NATIVE_ULONG))
			mdataStrings = readHDF5Data2D<unsigned long>(dataset, ctype, rows, cols, lines, dummy);
		else if (H5Tequal(mtype, H5T_STD_I64LE) || H5Tequal(mtype, H5T_STD_I64BE) || H5Tequal(mtype, H5T_NATIVE_LLONG))
			mdataStrings = readHDF5Data2D<long long>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, cols, lines, dummy);
		else if (H5Tequal(mtype, H5T_STD_U64LE) || H5Tequal(mtype, H5T_STD_U64BE) || H5Tequal(mtype, H5T_NATIVE_ULLONG))
			mdataStrings = readHDF5Data2D<unsigned long long>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, cols, lines, dummy);
		else if (H5Tequal(mtype, H5T_IEEE_F32LE) || H5Tequal(mtype, H5T_IEEE_F32BE))
			mdataStrings = readHDF5Data2D<float>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, cols, lines, dummy);
		else if (H5Tequal(mtype, H5T_IEEE_F64LE) || H5Tequal(mtype, H5T_IEEE_F64BE))
			mdataStrings = readHDF5Data2D<double>(dataset, H5Tget_native_type(ctype, H5T_DIR_DEFAULT), rows, cols, lines, dummy);
		else if (H5Tequal(mtype, H5T_NATIVE_LDOUBLE))
			mdataStrings = readHDF5Data2D<long double>(dataset, ctype, rows, cols, lines, dummy);
		else {
			for (int i = 0; i < qMin(rows, lines); ++i) {
				QStringList lineString;
				for (int j = 0; j < cols; ++j)
					lineString << QLatin1String("_");
				mdataStrings << lineString;
			}
#ifndef NDEBUG
			H5T_class_t mclass = H5Tget_member_class(tid, m);
#endif
			DEBUG("unsupported class " << STDSTRING(translateHDF5Class(mclass)));
		}

		m_status = H5Tclose(ctype);
		handleError(m_status, "H5Tclose");

		for (int i = 0; i < qMin(rows, lines); i++) {
			for (int j = 0; j < cols; j++) {
				dataStrings[i][j] += mdataStrings[i][j];
				if (m < members-1)
					dataStrings[i][j] += QLatin1String(",");
			}
		}
	}

	for (int i = 0; i < qMin(rows, lines); ++i) {
		for (int j = 0; j < cols; ++j)
			dataStrings[i][j] += QLatin1String(")");
	}

	QDEBUG("dataStrings =" << dataStrings);
	return dataStrings;
}

QStringList HDF5FilterPrivate::readHDF5Attr(hid_t aid) {
	QStringList attr;

	char name[MAXNAMELENGTH];
	m_status = H5Aget_name(aid, MAXNAMELENGTH, name);
	handleError(m_status, "H5Aget_name");
	attr << QString(name);
	// DEBUG("	name =" << QString(name));

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
		m_status = H5Aread(aid, amem, buf);
		handleError(m_status, "H5Aread");
		attr << QLatin1String("=") << QString(buf);
		m_status = H5Tclose(amem);
		handleError(m_status, "H5Tclose");
	} else if (aclass == H5T_INTEGER) {
		if (H5Tequal(atype, H5T_STD_I8LE)) {
			qint8 value;
			m_status = H5Aread(aid, H5T_STD_I8LE, &value);
			handleError(m_status, "H5Aread");
			attr << QLatin1String("=") << QString::number(value);
		} else if (H5Tequal(atype, H5T_STD_I8BE)) {
			qint8 value;
			m_status = H5Aread(aid, H5T_STD_I8BE, &value);
			handleError(m_status, "H5Aread");
			attr << QLatin1String("=") << QString::number(value);
		} else if (H5Tequal(atype, H5T_NATIVE_CHAR)) {
			switch (sizeof(H5T_NATIVE_CHAR)) {
			case 1: {
					qint8 value;
					m_status = H5Aread(aid, H5T_NATIVE_CHAR, &value);
					handleError(m_status, "H5Aread");
					attr << QLatin1String("=") << QString::number(value);
					break;
				}
			case 2: {
					qint16 value;
					m_status = H5Aread(aid, H5T_NATIVE_CHAR, &value);
					handleError(m_status, "H5Aread");
					attr << QLatin1String("=") << QString::number(value);
					break;
				}
			case 4: {
					qint32 value;
					m_status = H5Aread(aid, H5T_NATIVE_CHAR, &value);
					handleError(m_status, "H5Aread");
					attr << QLatin1String("=") << QString::number(value);
					break;
				}
			case 8: {
					qint64 value;
					m_status = H5Aread(aid, H5T_NATIVE_CHAR, &value);
					handleError(m_status, "H5Aread");
					attr << QLatin1String("=") << QString::number(value);
					break;
				}
			default:
				DEBUG("unknown size " << sizeof(H5T_NATIVE_CHAR) << " of H5T_NATIVE_CHAR");
				return QStringList(QString());
			}
		} else if (H5Tequal(atype, H5T_STD_U8LE)) {
			uint8_t value;
			m_status = H5Aread(aid, H5T_STD_U8LE, &value);
			handleError(m_status, "H5Aread");
			attr << QLatin1String("=") << QString::number(value);
		} else if (H5Tequal(atype, H5T_STD_U8BE)) {
			uint8_t value;
			m_status = H5Aread(aid, H5T_STD_U8BE, &value);
			handleError(m_status, "H5Aread");
			attr << QLatin1String("=") << QString::number(value);
		} else if (H5Tequal(atype, H5T_NATIVE_UCHAR)) {
			switch (sizeof(H5T_NATIVE_UCHAR)) {
			case 1: {
					uint8_t value;
					m_status = H5Aread(aid, H5T_NATIVE_UCHAR, &value);
					handleError(m_status, "H5Aread");
					attr << QLatin1String("=") << QString::number(value);
					break;
				}
			case 2: {
					uint16_t value;
					m_status = H5Aread(aid, H5T_NATIVE_UCHAR, &value);
					handleError(m_status, "H5Aread");
					attr << QLatin1String("=") << QString::number(value);
					break;
				}
			case 4: {
					uint32_t value;
					m_status = H5Aread(aid, H5T_NATIVE_UCHAR, &value);
					handleError(m_status, "H5Aread");
					attr << QLatin1String("=") << QString::number(value);
					break;
				}
			case 8: {
					uint64_t value;
					m_status = H5Aread(aid, H5T_NATIVE_UCHAR, &value);
					handleError(m_status, "H5Aread");
					attr << QLatin1String("=") << QString::number(value);
					break;
				}
			default:
				DEBUG("unknown size " << sizeof(H5T_NATIVE_UCHAR) << " of H5T_NATIVE_UCHAR");
				return QStringList(QString());
			}
		} else if (H5Tequal(atype, H5T_STD_I16LE) || H5Tequal(atype, H5T_STD_I16BE) || H5Tequal(atype, H5T_NATIVE_SHORT)) {
			short value;
			m_status = H5Aread(aid, H5T_NATIVE_SHORT, &value);
			handleError(m_status, "H5Aread");
			attr << QLatin1String("=") << QString::number(value);
		} else if (H5Tequal(atype, H5T_STD_U16LE) || H5Tequal(atype, H5T_STD_U16BE) || H5Tequal(atype, H5T_NATIVE_USHORT)) {
			unsigned short value;
			m_status = H5Aread(aid, H5T_NATIVE_USHORT, &value);
			handleError(m_status, "H5Aread");
			attr << QLatin1String("=") << QString::number(value);
		} else if (H5Tequal(atype, H5T_STD_I32LE) || H5Tequal(atype, H5T_STD_I32BE) || H5Tequal(atype, H5T_NATIVE_INT)) {
			int value;
			m_status = H5Aread(aid, H5T_NATIVE_INT, &value);
			handleError(m_status, "H5Aread");
			attr << QLatin1String("=") << QString::number(value);
		} else if (H5Tequal(atype, H5T_STD_U32LE) || H5Tequal(atype, H5T_STD_U32BE) || H5Tequal(atype, H5T_NATIVE_UINT)) {
			unsigned int value;
			m_status = H5Aread(aid, H5T_NATIVE_UINT, &value);
			handleError(m_status, "H5Aread");
			attr << QLatin1String("=") << QString::number(value);
		} else if (H5Tequal(atype, H5T_NATIVE_LONG)) {
			long value;
			m_status = H5Aread(aid, H5T_NATIVE_LONG, &value);
			handleError(m_status, "H5Aread");
			attr << QLatin1String("=") << QString::number(value);
		} else if (H5Tequal(atype, H5T_NATIVE_ULONG)) {
			unsigned long value;
			m_status = H5Aread(aid, H5T_NATIVE_ULONG, &value);
			handleError(m_status, "H5Aread");
			attr << QLatin1String("=") << QString::number(value);
		} else if (H5Tequal(atype, H5T_STD_I64LE) || H5Tequal(atype, H5T_STD_I64BE) || H5Tequal(atype, H5T_NATIVE_LLONG)) {
			long long value;
			m_status = H5Aread(aid, H5T_NATIVE_LLONG, &value);
			handleError(m_status, "H5Aread");
			attr << QLatin1String("=") << QString::number(value);
		} else if (H5Tequal(atype, H5T_STD_U64LE) || H5Tequal(atype, H5T_STD_U64BE) || H5Tequal(atype, H5T_NATIVE_ULLONG)) {
			unsigned long long value;
			m_status = H5Aread(aid, H5T_NATIVE_ULLONG, &value);
			handleError(m_status, "H5Aread");
			attr << QLatin1String("=") << QString::number(value);
		} else
			attr<<" (unknown integer)";
	} else if (aclass == H5T_FLOAT) {
		if (H5Tequal(atype, H5T_IEEE_F32LE) || H5Tequal(atype, H5T_IEEE_F32BE)) {
			float value;
			m_status = H5Aread(aid, H5T_NATIVE_FLOAT, &value);
			handleError(m_status, "H5Aread");
			attr << QLatin1String("=") << QString::number(value);
		} else if (H5Tequal(atype, H5T_IEEE_F64LE) || H5Tequal(atype, H5T_IEEE_F64BE)) {
			double value;
			m_status = H5Aread(aid, H5T_NATIVE_DOUBLE, &value);
			handleError(m_status, "H5Aread");
			attr << QLatin1String("=") << QString::number(value);
		} else if (H5Tequal(atype, H5T_NATIVE_LDOUBLE)) {
			long double value;
			m_status = H5Aread(aid, H5T_NATIVE_LDOUBLE, &value);
			handleError(m_status, "H5Aread");
			attr << QLatin1String("=") << QString::number((double)value);
		} else
			attr<<" (unknown float)";
	}

	m_status = H5Tclose(atype);
	handleError(m_status, "H5Tclose");
	m_status = H5Sclose(aspace);
	handleError(m_status, "H5Sclose");

	return attr;
}

QStringList HDF5FilterPrivate::scanHDF5Attrs(hid_t oid) {
	QStringList attrList;

	int numAttr = H5Aget_num_attrs(oid);
	handleError(numAttr, "H5Aget_num_attrs");
	DEBUG("number of attr = " << numAttr);

	for (int i = 0; i < numAttr; ++i) {
		hid_t aid = H5Aopen_idx(oid, i);
		handleError((int)aid, "H5Aopen_idx");
		attrList << readHDF5Attr(aid);
		if (i < numAttr-1)
			attrList << QLatin1String(", ");
		m_status = H5Aclose(aid);
		handleError(m_status, "H5Aclose");
	}

	return attrList;
}

QStringList HDF5FilterPrivate::readHDF5DataType(hid_t tid) {
	H5T_class_t typeClass = H5Tget_class(tid);
	handleError((int)typeClass, "H5Tget_class");

	QStringList typeProps;
	QString typeString = translateHDF5Class(typeClass);
	if (typeClass == H5T_INTEGER || typeClass == H5T_FLOAT)
		typeString = translateHDF5Type(tid);
	typeProps<<typeString;

	size_t size = H5Tget_size(tid);
	typeProps << QLatin1String(" (") << QString::number(size) << QLatin1String(") ");

	H5T_order_t order = H5Tget_order(tid);
	handleError((int)order, "H5Tget_order");
	typeProps << translateHDF5Order(order);

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
			QDEBUG(readHDF5Compound(tid).join(QString()));
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

QStringList HDF5FilterPrivate::readHDF5PropertyList(hid_t pid) {
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
	for (int i = 0; i < nfilters; ++i) {
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
				//unsigned int szip_options_mask = cd_values[0];
				unsigned int szip_pixels_per_block = cd_values[1];

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
	m_status = H5Pget_alloc_time(pid, &at);
	handleError(m_status, "H5Pget_alloc_time");

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
	m_status = H5Pget_fill_time(pid, &ft);
	handleError(m_status, "H5Pget_fill_time");
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
	m_status = H5Pfill_value_defined(pid, &fvstatus);
	handleError(m_status, "H5Pfill_value_defined");
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

void HDF5FilterPrivate::scanHDF5DataType(hid_t tid, char *dataSetName, QTreeWidgetItem* parentItem) {
	QStringList typeProps = readHDF5DataType(tid);

	QString attr = scanHDF5Attrs(tid).join(" ");

	char link[MAXNAMELENGTH];
	m_status = H5Iget_name(tid, link, MAXNAMELENGTH);
	handleError(m_status, "H5Iget_name");

	auto* dataTypeItem = new QTreeWidgetItem(QStringList()<<QString(dataSetName)<<QString(link)<<i18n("data type")<<typeProps.join(QString())<<attr);
	dataTypeItem->setIcon(0, QIcon::fromTheme("accessories-calculator"));
	dataTypeItem->setFlags(Qt::ItemIsEnabled);
	parentItem->addChild(dataTypeItem);
}

void HDF5FilterPrivate::scanHDF5DataSet(hid_t did, char *dataSetName, QTreeWidgetItem* parentItem) {
	QString attr = scanHDF5Attrs(did).join(QString());

	char link[MAXNAMELENGTH];
	m_status = H5Iget_name(did, link, MAXNAMELENGTH);
	handleError(m_status, "H5Iget_name");

	QStringList dataSetProps;
	hsize_t size = H5Dget_storage_size(did);
	handleError((int)size, "H5Dget_storage_size");
	hid_t datatype  = H5Dget_type(did);
	handleError((int)datatype, "H5Dget_type");
	size_t typeSize  = H5Tget_size(datatype);
	handleError((int)typeSize, "H5Dget_size");

	dataSetProps << readHDF5DataType(datatype);

	hid_t dataspace = H5Dget_space(did);
	int rank = H5Sget_simple_extent_ndims(dataspace);
	handleError(rank, "H5Sget_simple_extent_ndims");
	unsigned int rows = 1, cols = 1, regs = 1;
	if (rank == 1) {
		hsize_t dims_out[1];
		m_status = H5Sget_simple_extent_dims(dataspace, dims_out, nullptr);
		handleError(m_status, "H5Sget_simple_extent_dims");
		rows = dims_out[0];
		dataSetProps << QLatin1String(", ") << QString::number(rows)
		             << QLatin1String(" (") << QString::number(size/typeSize) << QLatin1String(")");
	} else if (rank == 2) {
		hsize_t dims_out[2];
		m_status = H5Sget_simple_extent_dims(dataspace, dims_out, nullptr);
		handleError(m_status, "H5Sget_simple_extent_dims");
		rows = dims_out[0];
		cols = dims_out[1];
		dataSetProps << QLatin1String(", ") << QString::number(rows) << QLatin1String("x") << QString::number(cols)
		             << QLatin1String(" (") << QString::number(size/typeSize) << QLatin1String(")");
	} else if (rank == 3) {
		hsize_t dims_out[3];
		m_status = H5Sget_simple_extent_dims(dataspace, dims_out, nullptr);
		handleError(m_status, "H5Sget_simple_extent_dims");
		rows = dims_out[0];
		cols = dims_out[1];
		regs = dims_out[2];
		dataSetProps << QLatin1String(", ")
		             << QString::number(rows) << QLatin1String("x") << QString::number(cols) << QLatin1String("x") << QString::number(regs)
		             << QLatin1String(" (") << QString::number(size/typeSize) << QLatin1String(")");
	} else
		dataSetProps << QLatin1String(", ") << i18n("rank %1 not supported yet", rank);

	hid_t pid = H5Dget_create_plist(did);
	handleError((int)pid, "H5Dget_create_plist");
	dataSetProps << ", " << readHDF5PropertyList(pid).join(QString());

	auto* dataSetItem = new QTreeWidgetItem(QStringList()<<QString(dataSetName)<<QString(link)<<i18n("data set")<<dataSetProps.join(QString())<<attr);
	dataSetItem->setIcon(0, QIcon::fromTheme("x-office-spreadsheet"));
	for (int i = 0; i < dataSetItem->columnCount(); ++i) {
		if (rows > 0 && cols > 0 && regs > 0) {
			dataSetItem->setBackground(i, QColor(192,255,192));
			dataSetItem->setForeground(i, Qt::black);
			dataSetItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		} else
			dataSetItem->setFlags(Qt::NoItemFlags);
	}
	parentItem->addChild(dataSetItem);
}

void HDF5FilterPrivate::scanHDF5Link(hid_t gid, char *linkName, QTreeWidgetItem* parentItem) {
	char target[MAXNAMELENGTH];
	m_status = H5Gget_linkval(gid, linkName, MAXNAMELENGTH, target) ;
	handleError(m_status, "H5Gget_linkval");

	auto* linkItem = new QTreeWidgetItem(QStringList() << QString(linkName) << i18n("symbolic link") << i18n("link to %1", QFile::decodeName(target)));
	linkItem->setIcon(0, QIcon::fromTheme("emblem-symbolic-link"));
	linkItem->setFlags(Qt::ItemIsEnabled);
	parentItem->addChild(linkItem);
}

void HDF5FilterPrivate::scanHDF5Group(hid_t gid, char *groupName, QTreeWidgetItem* parentItem) {
	DEBUG("HDF5FilterPrivate::scanHDF5Group()");

	//check for hard link
	H5G_stat_t statbuf;
	m_status = H5Gget_objinfo(gid, ".", true, &statbuf);
	handleError(m_status, "H5Gget_objinfo");
	if (statbuf.nlink > 1) {
		if (m_multiLinkList.contains(statbuf.objno[0])) {
			auto* objectItem = new QTreeWidgetItem(QStringList()<<QString(groupName) << i18n("hard link"));
			objectItem->setIcon(0, QIcon::fromTheme("link"));
			objectItem->setFlags(Qt::ItemIsEnabled);
			parentItem->addChild(objectItem);
			return;
		} else {
			m_multiLinkList.append(statbuf.objno[0]);
			DEBUG(" group multiple links: "<<statbuf.objno[0]<<' '<<statbuf.objno[1]);
		}
	}

	char link[MAXNAMELENGTH];
	m_status = H5Iget_name(gid, link, MAXNAMELENGTH);
	handleError(m_status, "H5Iget_name");

	QString attr = scanHDF5Attrs(gid).join(" ");

	auto* groupItem = new QTreeWidgetItem(QStringList() << QString(groupName) << QString(link) << QLatin1String("group ") << attr);
	groupItem->setIcon(0, QIcon::fromTheme("folder"));
	groupItem->setFlags(Qt::ItemIsEnabled);
	parentItem->addChild(groupItem);

	hsize_t numObj;
	m_status = H5Gget_num_objs(gid, &numObj);
	handleError(m_status, "H5Gget_num_objs");

	for (unsigned int i = 0; i < numObj; ++i) {
		char memberName[MAXNAMELENGTH];
		m_status = H5Gget_objname_by_idx(gid, (hsize_t)i, memberName, (size_t)MAXNAMELENGTH );
		handleError(m_status, "H5Gget_objname_by_idx");

		int otype =  H5Gget_objtype_by_idx(gid, (size_t)i );
		handleError(otype, "H5Gget_objtype_by_idx");
		switch (otype) {
		case H5G_LINK: {
				scanHDF5Link(gid, memberName, groupItem);
				break;
			}
		case H5G_GROUP: {
				hid_t grpid = H5Gopen(gid, memberName, H5P_DEFAULT);
				handleError((int)grpid, "H5Gopen");
				scanHDF5Group(grpid, memberName, groupItem);
				m_status = H5Gclose(grpid);
				handleError(m_status, "H5Gclose");
				break;
			}
		case H5G_DATASET: {
				hid_t dsid = H5Dopen(gid, memberName, H5P_DEFAULT);
				handleError((int)dsid, "H5Dopen");
				scanHDF5DataSet(dsid, memberName, groupItem);
				m_status = H5Dclose(dsid);
				handleError(m_status, "H5Dclose");
				break;
			}
		case H5G_TYPE: {
				hid_t tid = H5Topen(gid, memberName, H5P_DEFAULT);
				handleError((int)tid, "H5Topen");
				scanHDF5DataType(tid, memberName, groupItem);
				m_status = H5Tclose(tid);
				handleError(m_status, "H5Tclose");
				break;
			}
		default:
			auto* objectItem = new QTreeWidgetItem(QStringList() << QString(memberName) << i18n("unknown"));
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
void HDF5FilterPrivate::parse(const QString& fileName, QTreeWidgetItem* rootItem) {
	DEBUG("HDF5FilterPrivate::parse()");
#ifdef HAVE_HDF5
	DEBUG("fileName = " << qPrintable(fileName));

	// check file type first
	htri_t isHdf5 = H5Fis_hdf5(qPrintable(fileName));
	if (isHdf5 == 0) {
		DEBUG(qPrintable(fileName) << " is not a HDF5 file! Giving up.");
		return;
	}
	if (isHdf5 < 0) {
		DEBUG("H5Fis_hdf5() failed on " << qPrintable(fileName) << "! Giving up.");
		return;
	}

	// open file
	hid_t file = H5Fopen(qPrintable(fileName), H5F_ACC_RDONLY, H5P_DEFAULT);
	handleError((int)file, "H5Fopen", fileName);
	if (file < 0) {
		DEBUG("Opening file " << qPrintable(fileName) << " failed! Giving up.");
		return;
	}
	char rootName[] = "/";
	hid_t group = H5Gopen(file, rootName, H5P_DEFAULT);
	handleError((int)group, "H5Gopen", rootName);
	// multiLinkList.clear(); crashes
	scanHDF5Group(group, rootName, rootItem);
	m_status = H5Gclose(group);
	handleError(m_status, "H5Gclose", QString());
	m_status = H5Fclose(file);
	handleError(m_status, "H5Fclose", QString());
#else
	DEBUG("HDF5 not available");
	Q_UNUSED(fileName)
	Q_UNUSED(rootItem)
#endif
}

/*!
    reads the content of the date set in the file \c fileName to a string (for preview) or to the data source.
*/
QVector<QStringList> HDF5FilterPrivate::readCurrentDataSet(const QString& fileName, AbstractDataSource* dataSource, bool &ok, AbstractFileFilter::ImportMode mode, int lines) {
	DEBUG("HDF5Filter::readCurrentDataSet()");
	QVector<QStringList> dataStrings;

	if (currentDataSetName.isEmpty()) {
		//return QString("No data set selected").replace(' ',QChar::Nbsp);
		ok = false;
		return dataStrings << (QStringList() << i18n("No data set selected"));
	}
	DEBUG(" current data set = " << STDSTRING(currentDataSetName));

#ifdef HAVE_HDF5
	hid_t file = H5Fopen(qPrintable(fileName), H5F_ACC_RDONLY, H5P_DEFAULT);
	handleError((int)file, "H5Fopen", fileName);
	hid_t dataset = H5Dopen2(file, qPrintable(currentDataSetName), H5P_DEFAULT);
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
	DEBUG(" rank = " << rank);

	int columnOffset = 0;			// offset to import data
	int actualRows = 0, actualCols = 0;	// rows and cols to read

	// dataContainer is used to store the data read from the dataSource
	// it contains the pointers of all columns
	// initially there is one pointer set to nullptr
	// check for dataContainer[0] != nullptr to decide if dataSource can be used
	std::vector<void*> dataContainer(1, nullptr);

	// rank= 0: single value, 1: vector, 2: matrix, 3: 3D data, ...
	switch (rank) {
	case 0: {	// single value
			actualCols = 1;

			switch (dclass) {
			case H5T_STRING: {
					char* data = (char *) malloc(typeSize * sizeof(char));
					hid_t memtype = H5Tcopy(H5T_C_S1);
					handleError((int)memtype, "H5Tcopy");
					m_status = H5Tset_size(memtype, typeSize);
					handleError(m_status, "H5Tset_size");

					m_status = H5Dread(dataset, memtype, H5S_ALL, H5S_ALL, H5P_DEFAULT, data);
					handleError(m_status, "H5Tread");
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
					dataStrings << (QStringList() << i18n("rank 0 not implemented yet for type %1", translateHDF5Class(dclass)));
					QDEBUG(dataStrings);
				}
			default:
				break;
			}
			break;
		}
	case 1: {	// vector
			hsize_t size, maxSize;
			m_status = H5Sget_simple_extent_dims(dataspace, &size, &maxSize);
			handleError(m_status, "H5Sget_simple_extent_dims");
			int rows = size;
			if (endRow == -1)
				endRow = rows;
			if (lines == -1)
				lines = endRow;
			actualRows = endRow - startRow + 1;
			actualCols = 1;
#ifndef NDEBUG
			H5T_order_t order = H5Tget_order(dtype);
			handleError((int)order, "H5Sget_order");
#endif
			QDEBUG(translateHDF5Class(dclass) << '(' << typeSize << ')' << translateHDF5Order(order)
			         << ", rows:" << rows << " max:" << maxSize);

			//TODO: support other modes
			QVector<AbstractColumn::ColumnMode> columnModes;
			columnModes.resize(actualCols);

			// use current data set name (without path) for column name
			QStringList vectorNames = {currentDataSetName.mid(currentDataSetName.lastIndexOf("/") + 1)};
			QDEBUG("	vector names = " << vectorNames)

			if (dataSource)
				columnOffset = dataSource->prepareImport(dataContainer, mode, actualRows, actualCols, vectorNames, columnModes);

			QStringList dataString;	// data saved in a list
			switch (dclass) {
			case H5T_STRING: {
					DEBUG("rank 1 H5T_STRING");
					hid_t memtype = H5Tcopy(H5T_C_S1);
					handleError((int)memtype, "H5Tcopy");

					char** data = (char **) malloc(rows * sizeof (char *));

					if (H5Tis_variable_str(dtype)) {
						m_status = H5Tset_size(memtype, H5T_VARIABLE);
						handleError((int)memtype, "H5Tset_size");
						m_status = H5Dread(dataset, memtype, H5S_ALL, H5S_ALL, H5P_DEFAULT, data);
						handleError(m_status, "H5Dread");
					} else {
						data[0] = (char *) malloc(rows * typeSize * sizeof (char));
						for (int i = 1; i < rows; ++i)
							data[i] = data[0] + i * typeSize;

						m_status = H5Tset_size(memtype, typeSize);
						handleError((int)memtype, "H5Tset_size");

						m_status = H5Dread(dataset, memtype, H5S_ALL, H5S_ALL, H5P_DEFAULT, data[0]);
						handleError(m_status, "H5Dread");
					}

					for (int i = startRow-1; i < qMin(endRow, lines + startRow - 1); ++i)
						dataString << data[i];

					free(data);
					break;
				}
			case H5T_INTEGER: {
					if (H5Tequal(dtype, H5T_STD_I8LE))
						dataString = readHDF5Data1D<qint8>(dataset, H5T_STD_I8LE, rows, lines, dataContainer[0]);
					else if (H5Tequal(dtype, H5T_STD_I8BE))
						dataString = readHDF5Data1D<qint8>(dataset, H5T_STD_I8BE, rows, lines, dataContainer[0]);
					else if (H5Tequal(dtype, H5T_NATIVE_CHAR)) {
						switch (sizeof(H5T_NATIVE_CHAR)) {
						case 1:
							dataString = readHDF5Data1D<qint8>(dataset, H5T_NATIVE_CHAR, rows, lines, dataContainer[0]);
							break;
						case 2:
							dataString = readHDF5Data1D<qint16>(dataset, H5T_NATIVE_CHAR, rows, lines, dataContainer[0]);
							break;
						case 4:
							dataString = readHDF5Data1D<qint32>(dataset, H5T_NATIVE_CHAR, rows, lines, dataContainer[0]);
							break;
						case 8:
							dataString = readHDF5Data1D<qint64>(dataset, H5T_NATIVE_CHAR, rows, lines, dataContainer[0]);
							break;
						}
					} else if (H5Tequal(dtype, H5T_STD_U8LE))
						dataString = readHDF5Data1D<uint8_t>(dataset, H5T_STD_U8LE, rows, lines, dataContainer[0]);
					else if (H5Tequal(dtype, H5T_STD_U8BE))
						dataString = readHDF5Data1D<uint8_t>(dataset, H5T_STD_U8BE, rows, lines, dataContainer[0]);
					else if (H5Tequal(dtype, H5T_NATIVE_UCHAR)) {
						switch (sizeof(H5T_NATIVE_UCHAR)) {
						case 1:
							dataString = readHDF5Data1D<uint8_t>(dataset, H5T_NATIVE_UCHAR, rows, lines, dataContainer[0]);
							break;
						case 2:
							dataString = readHDF5Data1D<uint16_t>(dataset, H5T_NATIVE_UCHAR, rows, lines, dataContainer[0]);
							break;
						case 4:
							dataString = readHDF5Data1D<uint32_t>(dataset, H5T_NATIVE_UCHAR, rows, lines, dataContainer[0]);
							break;
						case 8:
							dataString = readHDF5Data1D<uint64_t>(dataset, H5T_NATIVE_UCHAR, rows, lines, dataContainer[0]);
							break;
						}
					} else if (H5Tequal(dtype, H5T_STD_I16LE) || H5Tequal(dtype, H5T_STD_I16BE) || H5Tequal(dtype, H5T_NATIVE_SHORT))
						dataString = readHDF5Data1D<short>(dataset, H5T_NATIVE_SHORT, rows, lines, dataContainer[0]);
					else if (H5Tequal(dtype, H5T_STD_U16LE) || H5Tequal(dtype, H5T_STD_U16BE) || H5Tequal(dtype, H5T_NATIVE_USHORT))
						dataString = readHDF5Data1D<unsigned short>(dataset, H5T_NATIVE_USHORT, rows, lines, dataContainer[0]);
					else if (H5Tequal(dtype, H5T_STD_I32LE) || H5Tequal(dtype, H5T_STD_I32BE) || H5Tequal(dtype, H5T_NATIVE_INT))
						dataString = readHDF5Data1D<int>(dataset, H5T_NATIVE_INT, rows, lines, dataContainer[0]);
					else if (H5Tequal(dtype, H5T_STD_U32LE) || H5Tequal(dtype, H5T_STD_U32BE) || H5Tequal(dtype, H5T_NATIVE_UINT))
						dataString = readHDF5Data1D<unsigned int>(dataset, H5T_NATIVE_UINT, rows, lines, dataContainer[0]);
					else if (H5Tequal(dtype, H5T_NATIVE_LONG))
						dataString = readHDF5Data1D<long>(dataset, H5T_NATIVE_LONG, rows, lines, dataContainer[0]);
					else if (H5Tequal(dtype, H5T_NATIVE_ULONG))
						dataString = readHDF5Data1D<unsigned long>(dataset, H5T_NATIVE_ULONG, rows, lines, dataContainer[0]);
					else if (H5Tequal(dtype, H5T_STD_I64LE) || H5Tequal(dtype, H5T_STD_I64BE) || H5Tequal(dtype, H5T_NATIVE_LLONG))
						dataString = readHDF5Data1D<long long>(dataset, H5T_NATIVE_LLONG, rows, lines, dataContainer[0]);
					else if (H5Tequal(dtype, H5T_STD_U64LE) || H5Tequal(dtype, H5T_STD_U64BE) || H5Tequal(dtype, H5T_NATIVE_ULLONG))
						dataString = readHDF5Data1D<unsigned long long>(dataset, H5T_NATIVE_ULLONG, rows, lines, dataContainer[0]);
					else {
						ok = false;
						dataString = (QStringList() << i18n("unsupported integer type for rank 1"));
						QDEBUG(dataString);
					}
					break;
				}
			case H5T_FLOAT: {
					if (H5Tequal(dtype, H5T_IEEE_F32LE) || H5Tequal(dtype, H5T_IEEE_F32BE))
						dataString = readHDF5Data1D<float>(dataset, H5T_NATIVE_FLOAT, rows, lines, dataContainer[0]);
					else if (H5Tequal(dtype, H5T_IEEE_F64LE) || H5Tequal(dtype, H5T_IEEE_F64BE))
						dataString = readHDF5Data1D<double>(dataset, H5T_NATIVE_DOUBLE, rows, lines, dataContainer[0]);
					else if (H5Tequal(dtype, H5T_NATIVE_LDOUBLE))
						dataString = readHDF5Data1D<long double>(dataset, H5T_NATIVE_LDOUBLE, rows, lines, dataContainer[0]);
					else {
						ok = false;
						dataString = (QStringList() << i18n("unsupported float type for rank 1"));
						QDEBUG(dataString);
					}
					break;
				}
			case H5T_COMPOUND: {
					int members = H5Tget_nmembers(dtype);
					handleError(members, "H5Tget_nmembers");
					if (dataSource) {
						// re-create data pointer
						dataContainer.clear();
						dataSource->prepareImport(dataContainer, mode, actualRows, members, vectorNames, columnModes);
					} else
						dataStrings << readHDF5Compound(dtype);
					dataString = readHDF5CompoundData1D(dataset, dtype, rows, lines, dataContainer);
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
					dataString = (QStringList() << i18n("rank 1 not implemented yet for type %1", translateHDF5Class(dclass)));
					QDEBUG(dataString);
				}
			default:
				break;
			}

			if (!dataSource) {	// preview
				QDEBUG("dataString =" << dataString);
				DEBUG("	data string size = " << dataString.size());
				DEBUG("	rows = " << rows << ", lines = " << lines << ", actual rows = " << actualRows);
				for (int i = 0; i < qMin(actualRows, lines); ++i)
					dataStrings << (QStringList() << dataString[i]);
			}

			break;
		}
	case 2: {	// matrix
			hsize_t dims_out[2];
			m_status = H5Sget_simple_extent_dims(dataspace, dims_out, nullptr);
			handleError(m_status, "H5Sget_simple_extent_dims");
			int rows = dims_out[0];
			int cols = dims_out[1];

			if (endRow == -1)
				endRow = rows;
			if (lines == -1)
				lines = endRow;
			if (endColumn == -1)
				endColumn = cols;
			actualRows = endRow-startRow+1;
			actualCols = endColumn-startColumn+1;

#ifndef NDEBUG
			H5T_order_t order = H5Tget_order(dtype);
			handleError((int)order, "H5Tget_order");
#endif
			QDEBUG(translateHDF5Class(dclass) << '(' << typeSize << ')' << translateHDF5Order(order) << "," << rows << "x" << cols);
			DEBUG("startRow/endRow" << startRow << endRow);
			DEBUG("startColumn/endColumn" << startColumn << endColumn);
			DEBUG("actual rows/cols" << actualRows << actualCols);
			DEBUG("lines" << lines);

			//TODO: support other modes
			QVector<AbstractColumn::ColumnMode> columnModes;
			columnModes.resize(actualCols);

			// use current data set name (without path) append by "_" and column number for column names
			QStringList vectorNames;
			QString colName = currentDataSetName.mid(currentDataSetName.lastIndexOf("/") + 1);
			for (int i = 0; i < actualCols; i++)
				vectorNames << colName + QLatin1String("_") + QString::number(i + 1);
			QDEBUG("	vector names = " << vectorNames)

			if (dataSource)
				columnOffset = dataSource->prepareImport(dataContainer, mode, actualRows, actualCols, vectorNames, columnModes);

			// read data
			switch (dclass) {
			case H5T_INTEGER: {
					if (H5Tequal(dtype, H5T_STD_I8LE))
						dataStrings << readHDF5Data2D<qint8>(dataset, H5T_STD_I8LE, rows, cols, lines, dataContainer);
					else if (H5Tequal(dtype, H5T_STD_I8BE))
						dataStrings << readHDF5Data2D<qint8>(dataset, H5T_STD_I8BE, rows, cols, lines, dataContainer);
					else if (H5Tequal(dtype, H5T_NATIVE_CHAR)) {
						switch (sizeof(H5T_NATIVE_CHAR)) {
						case 1:
							dataStrings << readHDF5Data2D<qint8>(dataset, H5T_NATIVE_CHAR, rows, cols, lines, dataContainer);
							break;
						case 2:
							dataStrings << readHDF5Data2D<qint16>(dataset, H5T_NATIVE_CHAR, rows, cols, lines, dataContainer);
							break;
						case 4:
							dataStrings << readHDF5Data2D<qint32>(dataset, H5T_NATIVE_CHAR, rows, cols, lines, dataContainer);
							break;
						case 8:
							dataStrings << readHDF5Data2D<qint64>(dataset, H5T_NATIVE_CHAR, rows, cols, lines, dataContainer);
							break;
						}
					} else if (H5Tequal(dtype, H5T_STD_U8LE))
						dataStrings << readHDF5Data2D<uint8_t>(dataset, H5T_STD_U8LE, rows, cols, lines, dataContainer);
					else if (H5Tequal(dtype, H5T_STD_U8BE))
						dataStrings << readHDF5Data2D<uint8_t>(dataset, H5T_STD_U8BE, rows, cols, lines, dataContainer);
					else if (H5Tequal(dtype, H5T_NATIVE_UCHAR)) {
						switch (sizeof(H5T_NATIVE_UCHAR)) {
						case 1:
							dataStrings << readHDF5Data2D<uint8_t>(dataset, H5T_NATIVE_UCHAR, rows, cols, lines, dataContainer);
							break;
						case 2:
							dataStrings << readHDF5Data2D<uint16_t>(dataset, H5T_NATIVE_UCHAR, rows, cols, lines, dataContainer);
							break;
						case 4:
							dataStrings << readHDF5Data2D<uint32_t>(dataset, H5T_NATIVE_UCHAR, rows, cols, lines, dataContainer);
							break;
						case 8:
							dataStrings << readHDF5Data2D<uint64_t>(dataset, H5T_NATIVE_UCHAR, rows, cols, lines, dataContainer);
							break;
						}
					} else if (H5Tequal(dtype, H5T_STD_I16LE) || H5Tequal(dtype, H5T_STD_I16BE) || H5Tequal(dtype, H5T_NATIVE_SHORT))
						dataStrings << readHDF5Data2D<short>(dataset, H5T_NATIVE_SHORT, rows, cols, lines, dataContainer);
					else if (H5Tequal(dtype, H5T_STD_U16LE) || H5Tequal(dtype, H5T_STD_U16BE) || H5Tequal(dtype, H5T_NATIVE_USHORT))
						dataStrings << readHDF5Data2D<unsigned short>(dataset, H5T_NATIVE_USHORT, rows, cols, lines, dataContainer);
					else if (H5Tequal(dtype, H5T_STD_I32LE) || H5Tequal(dtype, H5T_STD_I32BE) || H5Tequal(dtype, H5T_NATIVE_INT))
						dataStrings << readHDF5Data2D<int>(dataset, H5T_NATIVE_INT, rows, cols, lines, dataContainer);
					else if (H5Tequal(dtype, H5T_STD_U32LE) || H5Tequal(dtype, H5T_STD_U32BE) || H5Tequal(dtype, H5T_NATIVE_UINT))
						dataStrings << readHDF5Data2D<unsigned int>(dataset, H5T_NATIVE_UINT, rows, cols, lines, dataContainer);
					else if (H5Tequal(dtype, H5T_NATIVE_LONG))
						dataStrings << readHDF5Data2D<long>(dataset, H5T_NATIVE_LONG, rows, cols, lines, dataContainer);
					else if (H5Tequal(dtype, H5T_NATIVE_ULONG))
						dataStrings << readHDF5Data2D<unsigned long>(dataset, H5T_NATIVE_ULONG, rows, cols, lines, dataContainer);
					else if (H5Tequal(dtype, H5T_STD_I64LE) || H5Tequal(dtype, H5T_STD_I64BE) || H5Tequal(dtype, H5T_NATIVE_LLONG))
						dataStrings << readHDF5Data2D<long long>(dataset, H5T_NATIVE_LLONG, rows, cols, lines, dataContainer);
					else if (H5Tequal(dtype, H5T_STD_U64LE) || H5Tequal(dtype, H5T_STD_U64BE) || H5Tequal(dtype, H5T_NATIVE_ULLONG))
						dataStrings << readHDF5Data2D<unsigned long long>(dataset, H5T_NATIVE_ULLONG, rows, cols, lines, dataContainer);
					else {
						ok = false;
						dataStrings << (QStringList() << i18n("unsupported integer type for rank 2"));
						QDEBUG(dataStrings);
					}
					break;
				}
			case H5T_FLOAT: {
					if (H5Tequal(dtype, H5T_IEEE_F32LE) || H5Tequal(dtype, H5T_IEEE_F32BE))
						dataStrings << readHDF5Data2D<float>(dataset, H5T_NATIVE_FLOAT, rows, cols, lines, dataContainer);
					else if (H5Tequal(dtype, H5T_IEEE_F64LE) || H5Tequal(dtype, H5T_IEEE_F64BE))
						dataStrings << readHDF5Data2D<double>(dataset, H5T_NATIVE_DOUBLE, rows, cols, lines, dataContainer);
					else if (H5Tequal(dtype, H5T_NATIVE_LDOUBLE))
						dataStrings << readHDF5Data2D<long double>(dataset, H5T_NATIVE_LDOUBLE, rows, cols, lines, dataContainer);
					else {
						ok = false;
						dataStrings << (QStringList() << i18n("unsupported float type for rank 2"));
						QDEBUG(dataStrings);
					}
					break;
				}
			case H5T_COMPOUND: {
					dataStrings << readHDF5Compound(dtype);
					QDEBUG(dataStrings);
					dataStrings << readHDF5CompoundData2D(dataset,dtype,rows,cols,lines);
					break;
				}
			case H5T_STRING: {
					// TODO: implement this
					ok = false;
					dataStrings << (QStringList() << i18n("rank 2 not implemented yet for type %1, size = %2", translateHDF5Class(dclass), typeSize));
					QDEBUG(dataStrings);
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
					dataStrings << (QStringList() << i18n("rank 2 not implemented yet for type %1", translateHDF5Class(dclass)));
					QDEBUG(dataStrings);
				}
			default:
				break;
			}
			break;
		}
	default: {	// 3D or more data
			ok = false;
			dataStrings << (QStringList() << i18n("rank %1 not implemented yet for type %2", rank, translateHDF5Class(dclass)));
			QDEBUG(dataStrings);
		}
	}

	m_status = H5Sclose(dataspace);
	handleError(m_status, "H5Sclose");
	m_status = H5Tclose(dtype);
	handleError(m_status, "H5Tclose");
	m_status = H5Dclose(dataset);
	handleError(m_status, "H5Dclose");
	m_status = H5Fclose(file);
	handleError(m_status, "H5Fclose");

	if (!dataSource)
		return dataStrings;

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
    reads the content of the file \c fileName to the data source \c dataSource.
    Uses the settings defined in the data source.
*/
void HDF5FilterPrivate::readDataFromFile(const QString& fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode mode) {
	DEBUG("HDF5Filter::readDataFromFile()");

	if (currentDataSetName.isEmpty()) {
		DEBUG("WARNING: No data set selected");
		return;
	}

	bool ok = true;
	readCurrentDataSet(fileName, dataSource, ok, mode);
}

/*!
    writes the content of \c dataSource to the file \c fileName.
*/
void HDF5FilterPrivate::write(const QString & fileName, AbstractDataSource* dataSource) {
	Q_UNUSED(fileName);
	Q_UNUSED(dataSource);
	//TODO: writing HDF5 not implemented yet
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################

/*!
  Saves as XML.
 */
void HDF5Filter::save(QXmlStreamWriter* writer) const {
	writer->writeStartElement("hdfFilter");
	writer->writeEndElement();
}

/*!
  Loads from XML.
*/
bool HDF5Filter::load(XmlStreamReader* reader) {
	Q_UNUSED(reader);
// 	KLocalizedString attributeWarning = ki18n("Attribute '%1' missing or empty, default value is used");
// 	QXmlStreamAttributes attribs = reader->attributes();
	return true;
}
