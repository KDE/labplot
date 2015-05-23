/***************************************************************************
File                 : HDFFilter.cpp
Project              : LabPlot
Description          : HDF I/O-filter
--------------------------------------------------------------------
Copyright            : (C) 2015 by Stefan Gerlach (stefan.gerlach@uni.kn)
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
#include "backend/datasources/filters/HDFFilter.h"
#include "backend/datasources/filters/HDFFilterPrivate.h"
#include "backend/datasources/FileDataSource.h"
#include "backend/core/column/Column.h"

#include <math.h>

#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <KLocale>
#include <KIcon>

/*!
	\class HDFFilter
	\brief Manages the import/export of data from/to a HDF file.

	\ingroup datasources
*/
HDFFilter::HDFFilter():AbstractFileFilter(), d(new HDFFilterPrivate(this)){

}

HDFFilter::~HDFFilter(){
	delete d;
}

/*!
  parses the content of the file \c fileName.
*/
void HDFFilter::parse(const QString & fileName, QTreeWidgetItem* rootItem){
	d->parse(fileName, rootItem);
}

/*!
  reads the content of the data set \c dataSet from file \c fileName.
*/
QString HDFFilter::readCurrentDataSet(const QString & fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode,  int lines){
	return d->readCurrentDataSet(fileName, dataSource, importMode, lines);
}

/*!
  reads the content of the file \c fileName to the data source \c dataSource.
*/
void HDFFilter::read(const QString & fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode){
	d->read(fileName, dataSource, importMode);
}

/*!
writes the content of the data source \c dataSource to the file \c fileName.
*/
void HDFFilter::write(const QString & fileName, AbstractDataSource* dataSource){
 	d->write(fileName, dataSource);
// 	emit()
}

///////////////////////////////////////////////////////////////////////
/*!
  loads the predefined filter settings for \c filterName
*/
void HDFFilter::loadFilterSettings(const QString& filterName){
    Q_UNUSED(filterName);
}

/*!
  saves the current settings as a new filter with the name \c filterName
*/
void HDFFilter::saveFilterSettings(const QString& filterName) const{
    Q_UNUSED(filterName);
}

///////////////////////////////////////////////////////////////////////

void HDFFilter::setCurrentDataSet(QString ds){
	d->currentDataSet = ds;
}

QString HDFFilter::currentDataSet() const{
	return d->currentDataSet;
}

void HDFFilter::setStartRow(const int s) {
        d->startRow = s;
}

int HDFFilter::startRow() const{
        return d->startRow;
}

void HDFFilter::setEndRow(const int e) {
        d->endRow = e;
}

int HDFFilter::endRow() const{
        return d->endRow;
}

void HDFFilter::setStartColumn(const int c){
	d->startColumn=c;
}

int HDFFilter::startColumn() const{
	return d->startColumn;
}

void HDFFilter::setEndColumn(const int c){
	d->endColumn=c;
}

int HDFFilter::endColumn() const{
	return d->endColumn;
}

void HDFFilter::setAutoModeEnabled(bool b){
	d->autoModeEnabled = b;
}

bool HDFFilter::isAutoModeEnabled() const{
	return d->autoModeEnabled;
}
//#####################################################################
//################### Private implementation ##########################
//#####################################################################

HDFFilterPrivate::HDFFilterPrivate(HDFFilter* owner) : 
	q(owner),currentDataSet(""),startRow(1), endRow(-1),startColumn(1),endColumn(-1) {
}

#ifdef HAVE_HDF5
QString HDFFilterPrivate::translateHDFOrder(H5T_order_t o) {
	QString order;
        switch(o) {
        case H5T_ORDER_LE:
		order="LE";
                break;
        case H5T_ORDER_BE:
		order="BE";
                break;
        case H5T_ORDER_VAX:
		order="VAX";
                break;
        case H5T_ORDER_MIXED:
		order="MIXED";
                break;
        case H5T_ORDER_NONE:
		order="NONE";
                break;
        case H5T_ORDER_ERROR:
		order="ERROR";
                break;
	default:
		order="UNKNOWN";
        }

	return order;
}

QString HDFFilterPrivate::translateHDFClass(H5T_class_t c) {
	QString dclass;
	switch(c) {
        case H5T_INTEGER:
		dclass="INTEGER";
                break;
        case H5T_FLOAT:
		dclass="FLOAT";
                break;
        case H5T_STRING:
		dclass="STRING";
                break;
        case H5T_BITFIELD:
		dclass="BITFIELD";
                break;
        case H5T_OPAQUE:
		dclass="OPAQUE";
                break;
        case H5T_COMPOUND:
		dclass="COMPOUND";
                break;
        case H5T_ARRAY:
		dclass="ARRAY";
                break;
        case H5T_ENUM:
		dclass="ENUM";
                break;
        default:
		dclass="UNKNOWN";
                break;
        }
	return dclass;
}

QStringList HDFFilterPrivate::readHDFAttr(hid_t aid) {
	QStringList attr;

        char name[MAXNAMELENGTH];
        H5Aget_name(aid, MAXNAMELENGTH, name );
	attr << QString(name);

        hid_t aspace = H5Aget_space(aid); // the dimensions of the attribute data
        hid_t atype  = H5Aget_type(aid);
        hid_t aclass = H5Tget_class(atype);

	if (aclass == H5T_STRING) {
		hid_t amem = H5Tget_native_type(atype, H5T_DIR_ASCEND);
		H5Aread(aid, amem, name);
		attr<<"="<<QString(name);
		H5Tclose(amem);
	}else if(aclass == H5T_INTEGER) {
		int value;
		H5Aread(aid, H5T_NATIVE_INT, &value);
		attr<<"="<<QString::number(value);
	}else if(aclass == H5T_FLOAT) {
		size_t typeSize = H5Tget_size(atype);
		if(typeSize == 4) {
			float value;
			H5Aread(aid, H5T_NATIVE_FLOAT, &value);
			attr<<"="<<QString::number(value);
		}else if (typeSize == 8) {
			double value;
			H5Aread(aid, H5T_NATIVE_DOUBLE, &value);
			attr<<"="<<QString::number(value);
		}
	}

        H5Tclose(atype);
        H5Sclose(aspace);

	return attr;
}

QStringList HDFFilterPrivate::scanHDFAttrs(hid_t oid) {
	QStringList attr;

	int numAttr = H5Aget_num_attrs(oid);

        for (int i = 0; i < numAttr; i++) {
                hid_t aid = H5Aopen_idx(oid, i);
                attr<<readHDFAttr(aid);
		if(i != numAttr-1)
			attr<<", ";
                H5Aclose(aid);
        }

	return attr;
}

QStringList HDFFilterPrivate::readHDFDataType(hid_t tid) {
	H5T_class_t typeClass = H5Tget_class(tid);

	QStringList typeProps;
	typeProps<<translateHDFClass(typeClass);

        size_t size = H5Tget_size(tid);
	typeProps<<" ("<<QString::number(size)<<") ";

	H5T_order_t order = H5Tget_order(tid);
	typeProps<<translateHDFOrder(order);

	// type specific props
	switch(typeClass) {
	case H5T_STRING: {
		H5T_cset_t cset = H5Tget_cset (tid);
		switch(cset) {
		case  H5T_CSET_ASCII:
			typeProps<<", ASCII";
		default:
			break;
		}
		H5T_str_t strpad = H5Tget_strpad(tid);
		switch(strpad) {
		case H5T_STR_NULLTERM:
			typeProps<<" NULLTERM";
			break;
		case H5T_STR_NULLPAD:
			typeProps<<" NULLPAD";
			break;
		case H5T_STR_SPACEPAD:
			typeProps<<" SPACEPAD";
			break;
		default:
			break;
		}
		break;
	}
	default:
		break;
	}

	return typeProps;
}

QStringList HDFFilterPrivate::readHDFPropertyList(hid_t pid) {
	QStringList props;

	hsize_t chunk_dims_out[2];
	if(H5D_CHUNKED == H5Pget_layout(pid)){
                int rank_chunk = H5Pget_chunk(pid, 2, chunk_dims_out);
		props<<"chunk rank="<<QString::number(rank_chunk)<<", dimension="<<QString::number(chunk_dims_out[0])<<QString::number(chunk_dims_out[1]);
        }

	int nfilters = H5Pget_nfilters(pid);
	props<<" "<<QString::number(nfilters)<<" filter";
        for (int i = 0; i < nfilters; i++) {
		size_t cd_nelmts = 32;
		unsigned int filt_flags, filt_conf;
		unsigned int cd_values[32];
		char f_name[MAXNAMELENGTH];
                H5Z_filter_t filtn = H5Pget_filter(pid, (unsigned)i, &filt_flags, &cd_nelmts, cd_values,(size_t)MAXNAMELENGTH, f_name, &filt_conf);

		switch (filtn) {
		case H5Z_FILTER_DEFLATE:  /* AKA GZIP compression */
			props<<": DEFLATE level ="<<QString::number(cd_values[0]);
			break;
		case H5Z_FILTER_SHUFFLE: 
			props<<": SHUFFLE"; /* no parms */
			break;
		case H5Z_FILTER_FLETCHER32:
			props<<": FLETCHER32";  /* Error Detection Code */
			break;
		case H5Z_FILTER_SZIP: {
			//unsigned int szip_options_mask=cd_values[0];;
			unsigned int szip_pixels_per_block=cd_values[1];

			props<<": SZIP COMPRESSION - PIXELS_PER_BLOCK "<<QString::number(szip_pixels_per_block);
			break;
		}
		default:
			props<<": Unknown filter";
			break;
		}
	}

	props<<", ALLOC_TIME:";
	H5D_alloc_time_t at;
        H5Pget_alloc_time(pid, &at);

        switch (at) {
	case H5D_ALLOC_TIME_EARLY:
		props<<" EARLY";
		break;
	case H5D_ALLOC_TIME_INCR:
		props<<" INCR";
		break;
	case H5D_ALLOC_TIME_LATE:
		props<<" LATE";
		break;
	default:
		props<<" unknown allocation policy";
		break;
        }

	props<<", FILL_TIME:";
        H5D_fill_time_t ft;
	H5Pget_fill_time(pid, &ft);
        switch ( ft ) {
	case H5D_FILL_TIME_ALLOC:
		props<<" ALLOW";
		break;
	case H5D_FILL_TIME_NEVER:
		props<<" NEVER";
		break;
	case H5D_FILL_TIME_IFSET:
		props<<" IFSET";
		break;
	default:
		props<<" unknown";
		break;
        }

	H5D_fill_value_t fvstatus;
	H5Pfill_value_defined(pid, &fvstatus);
        if (fvstatus == H5D_FILL_VALUE_UNDEFINED) {
		props<<" No fill value defined";
        } else {
                /* TODO: Read  the fill value with H5Pget_fill_value. 
                 * Fill value is the same data type as the dataset.
                 * (details not shown) 
                 **/
        }

	return props;
}

void HDFFilterPrivate::scanHDFDataType(hid_t tid, char *dataSetName, QTreeWidgetItem* parentItem) {
	QStringList typeProps=readHDFDataType(tid);

	QString attr = scanHDFAttrs(tid).join(" ");

	char link[MAXNAMELENGTH];
        H5Iget_name(tid, link, MAXNAMELENGTH);

	QTreeWidgetItem *dataTypeItem = new QTreeWidgetItem((QTreeWidget*)0, QStringList()<<QString(dataSetName)<<QString(link)<<i18n("data type")<<typeProps.join("")<<attr);
	dataTypeItem->setIcon(0,QIcon(KIcon("accessories-calculator")));
	parentItem->addChild(dataTypeItem);

}

void HDFFilterPrivate::scanHDFDataSet(hid_t did, char *dataSetName, QTreeWidgetItem* parentItem) {
	QString attr = scanHDFAttrs(did).join("");

	char link[MAXNAMELENGTH];
        H5Iget_name(did, link, MAXNAMELENGTH);

	QStringList dataSetProps;
	hsize_t size = H5Dget_storage_size(did);
	hid_t datatype  = H5Dget_type(did);
	size_t typeSize  = H5Tget_size(datatype);

	dataSetProps<<readHDFDataType(datatype);

	hid_t dataspace = H5Dget_space(did);
	int rank = H5Sget_simple_extent_ndims(dataspace);
	if(rank == 2) {
		hsize_t dims_out[2];
    		H5Sget_simple_extent_dims(dataspace, dims_out, NULL);
		unsigned int rows = dims_out[0];
		unsigned int cols = dims_out[1];
		dataSetProps<<", "<<QString::number(rows)<<"x"<<QString::number(cols)<<" ("<<QString::number(size/typeSize)<<")";
	}
	else if(rank == 3) {
		hsize_t dims_out[3];
    		H5Sget_simple_extent_dims(dataspace, dims_out, NULL);
		unsigned int rows = dims_out[0];
		unsigned int cols = dims_out[1];
		unsigned int regs = dims_out[2];
		dataSetProps<<", "<<QString::number(rows)<<"x"<<QString::number(cols)<<"x"<<QString::number(regs)<<" ("<<QString::number(size/typeSize)<<")";
	}

	hid_t pid = H5Dget_create_plist(did);
	dataSetProps<<", "<<readHDFPropertyList(pid).join("");

	QTreeWidgetItem *dataSetItem = new QTreeWidgetItem((QTreeWidget*)0, QStringList()<<QString(dataSetName)<<QString(link)<<i18n("data set")<<dataSetProps.join("")<<attr);
	dataSetItem->setIcon(0,QIcon(KIcon("x-office-spreadsheet")));
	dataSetItem->setBackground(0,QBrush(QColor(192,255,192)));
	parentItem->addChild(dataSetItem);
}

void HDFFilterPrivate::scanHDFLink(hid_t gid, char *linkName, QTreeWidgetItem* parentItem) {
        char target[MAXNAMELENGTH];
        H5Gget_linkval(gid, linkName, MAXNAMELENGTH, target) ;

	QTreeWidgetItem *linkItem = new QTreeWidgetItem((QTreeWidget*)0, QStringList()<<QString(linkName)<<" "<<i18n("symbolic link")<<i18n("link to ")+QString(target));
	linkItem->setIcon(0,QIcon(KIcon("emblem-symbolic-link")));
	parentItem->addChild(linkItem);
}

void HDFFilterPrivate::scanHDFGroup(hid_t gid, char *groupName, QTreeWidgetItem* parentItem) {

	//check for hard link
	H5G_stat_t  statbuf;
	H5Gget_objinfo(gid, ".", TRUE, &statbuf);
	if (statbuf.nlink > 1) {
		if(multiLinkList.contains(statbuf.objno[0])) {
			QTreeWidgetItem *objectItem = new QTreeWidgetItem((QTreeWidget*)0, QStringList()<<QString(groupName)<<" "<<i18n("hard link"));
			objectItem->setIcon(0,QIcon(KIcon("link")));
                	parentItem->addChild(objectItem);
			return;
		} else {
			multiLinkList.append(statbuf.objno[0]);
			//qDebug()<<" group multiple links: "<<statbuf.objno[0]<<' '<<statbuf.objno[1];
		}
	}

	char link[MAXNAMELENGTH];
        H5Iget_name(gid, link, MAXNAMELENGTH);

	QString attr = scanHDFAttrs(gid).join(" ");

	QTreeWidgetItem *groupItem = new QTreeWidgetItem((QTreeWidget*)0, QStringList()<<QString(groupName)<<QString(link)<<i18n("group")<<" "<<attr);
	groupItem->setIcon(0,QIcon(KIcon("folder")));
	parentItem->addChild(groupItem);

	hsize_t numObj;
	H5Gget_num_objs(gid, &numObj);

	for (unsigned int i = 0; i < numObj; i++) {
                char memberName[MAXNAMELENGTH];
                H5Gget_objname_by_idx(gid, (hsize_t)i, memberName, (size_t)MAXNAMELENGTH );

                int otype =  H5Gget_objtype_by_idx(gid, (size_t)i );
                switch(otype) {
                case H5G_LINK: {
                        scanHDFLink(gid,memberName, groupItem);
                        break;
                }
                case H5G_GROUP: {
                        hid_t grpid = H5Gopen(gid,memberName, H5P_DEFAULT);
                        scanHDFGroup(grpid, memberName, groupItem);
                        H5Gclose(grpid);
                        break;
                }
                case H5G_DATASET: {
                        hid_t dsid = H5Dopen(gid,memberName, H5P_DEFAULT);
                        scanHDFDataSet(dsid, memberName, groupItem);
                        H5Dclose(dsid);
                        break;
                }
                case H5G_TYPE: {
                        hid_t tid = H5Topen(gid,memberName, H5P_DEFAULT);
                        scanHDFDataType(tid, memberName, groupItem);
                        H5Tclose(tid);
			break;
                }
                default:
			QTreeWidgetItem *objectItem = new QTreeWidgetItem((QTreeWidget*)0, QStringList()<<QString(memberName)<<i18n("unknown"));
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
	char rootName[]="/";
	hid_t group = H5Gopen(file, rootName, H5P_DEFAULT);
	multiLinkList.clear();
	scanHDFGroup(group,rootName, rootItem);
	H5Gclose(group);
	H5Fclose(file);
#endif
}

/*!
    reads the content of the date set in the file \c fileName to a string (for preview) or to the data source.
*/
QString HDFFilterPrivate::readCurrentDataSet(const QString & fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode mode, int lines){
	QStringList dataString;

	if(currentDataSet.isEmpty())
		return QString("No data set selected");
#ifdef QT_DEBUG
	qDebug()<<" current data set ="<<currentDataSet;
#endif

	QByteArray bafileName = fileName.toLatin1();
	hid_t file = H5Fopen(bafileName.data(), H5F_ACC_RDONLY, H5P_DEFAULT);
	QByteArray badataSet = currentDataSet.toLatin1();
	hid_t dataset = H5Dopen2(file, badataSet.data(), H5P_DEFAULT);

	// Get datatype and dataspace
	hid_t datatype = H5Dget_type(dataset);
	H5T_class_t dclass = H5Tget_class(datatype);
	size_t typeSize = H5Tget_size(datatype);

	hid_t dataspace = H5Dget_space(dataset);
	int rank = H5Sget_simple_extent_ndims(dataspace);

	Spreadsheet* spreadsheet=0;
	int columnOffset = 0;	// offset to first column
	int actualRows=0, actualCols=0;	// rows and cols to read

	switch(rank) {
	case 0: {
		actualRows=1;
		actualCols=1;

		switch(dclass) {
		case H5T_STRING: {
			char* data = (char *) malloc(typeSize * sizeof (char *));
			hid_t memtype = H5Tcopy(H5T_C_S1);
			H5Tset_size(memtype, typeSize);

			H5Dread(dataset, memtype, H5S_ALL, H5S_ALL, H5P_DEFAULT, data);
			dataString<<data<<"\n";
			break;
		}
		//TODO: other types
		default: {
			dataString<<"rank = 0 not implemented yet for type "<<translateHDFClass(dclass);
			qDebug()<<dataString.join("");
		}
		}

		break;
	}
	case 1: {
		hsize_t size, maxSize;
		H5Sget_simple_extent_dims(dataspace, &size, &maxSize);
		int rows=size;
		if(endRow == -1)
			endRow=rows;
		if (lines == -1)
			lines=endRow;
		actualRows=endRow-startRow+1;
		actualCols=1;
#ifdef QT_DEBUG
		H5T_order_t order = H5Tget_order(datatype);
		qDebug()<<translateHDFClass(dclass)<<"("<<typeSize<<")"<<translateHDFOrder(order)<<", rows:"<<rows<<" max:"<<maxSize;
#endif
		switch(dclass) {
		case H5T_STRING: {
			char** data = (char **) malloc(rows * sizeof (char *));
			data[0] = (char *) malloc(rows * typeSize * sizeof (char));
			for (int i=1; i<rows; i++)
				data[i] = data[0] + i * typeSize;

			hid_t memtype = H5Tcopy(H5T_C_S1);
			H5Tset_size(memtype, typeSize);

			H5Dread(dataset, memtype, H5S_ALL, H5S_ALL, H5P_DEFAULT, data[0]);

			for (int i=startRow-1; i<qMin(endRow,lines+startRow-1); i++)
				dataString<<data[i]<<"\n";

			free(data[0]);
			free(data);
			break;
		}
		//TODO: other types
		default:
			dataString<<"rank = 1 not implemented yet for type "<<translateHDFClass(dclass);
			qDebug()<<dataString.join("");
		}
		break;
	}
	case 2: {
		hsize_t dims_out[2];
		H5Sget_simple_extent_dims(dataspace, dims_out, NULL);
		int rows = dims_out[0];
		int cols = dims_out[1];

		if(endRow == -1)
			endRow=rows;
		if (lines == -1)
			lines=endRow;
		if(endColumn == -1)
			endColumn=cols;
		actualRows=endRow-startRow+1;
		actualCols=endColumn-startColumn+1;

#ifdef QT_DEBUG
		H5T_order_t order = H5Tget_order(datatype);
		qDebug()<<translateHDFClass(dclass)<<"("<<typeSize<<")"<<translateHDFOrder(order)<<","<<rows<<"x"<<cols;
		qDebug()<<"startRow/endRow"<<startRow<<endRow;
		qDebug()<<"startColumn/endColumn"<<startColumn<<endColumn;
		qDebug()<<"actual rows"<<actualRows;
		qDebug()<<"actual cols"<<actualCols;
		qDebug()<<"lines"<<lines;
#endif

		QVector<QVector<double>*> dataPointers;

		if (dataSource != NULL) {
			if(dataSource->inherits("Spreadsheet")) {
				columnOffset = dataSource->resize(mode,QStringList(),actualCols);
				//qDebug()<<"column offset"<<columnOffset;
		
				// resize the spreadsheet
				spreadsheet = dynamic_cast<Spreadsheet*>(dataSource);
				if (mode==AbstractFileFilter::Replace) {
					spreadsheet->clear();
					spreadsheet->setRowCount(actualRows);
				}else{
					if (spreadsheet->rowCount() < actualRows)
						spreadsheet->setRowCount(actualRows);
				}
				for (int n=0; n<actualCols; n++ ){
					QVector<double>* vector = static_cast<QVector<double>* >(dataSource->child<Column>(columnOffset+n)->data());
					vector->reserve(actualRows);
					vector->resize(actualRows);
					dataPointers.push_back(vector);
				}
			} else if (dataSource->inherits("Matrix")) {
				Matrix* matrix = dynamic_cast<Matrix*>(dataSource);
				// resize the matrix
				if (mode==AbstractFileFilter::Replace) {
					matrix->clear();
					matrix->setDimensions(actualRows,actualCols);
				}else{
					if (matrix->rowCount() < actualRows)
						matrix->setDimensions(actualRows,actualCols);
					else
						matrix->setDimensions(matrix->rowCount(),actualCols);
				}

				QVector<QVector<double> >& matrixColumns = matrix->data();
				for ( int n=0; n<actualCols; n++ ){
					QVector<double>* vector = &matrixColumns[n];
					vector->reserve(actualRows);
					vector->resize(actualRows);
					dataPointers.push_back(vector);
				}
			}
		}

		// read data
		if (dclass == H5T_INTEGER) {
			int** data_out = (int**) malloc(rows*sizeof(int*));
			data_out[0] = (int*)malloc( cols*rows*sizeof(int) );
			for (int i=1; i < rows; i++) data_out[i] = data_out[0]+i*cols;

			H5Dread(dataset, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, &data_out[0][0]);
			for (int i=startRow-1; i < qMin(endRow,lines+startRow-1); i++) {
				for (int j=startColumn-1; j < endColumn; j++) {
					if (dataSource != NULL) {
						dataPointers[j-startColumn+1]->operator[](i-startRow+1) = data_out[i][j];
					} else {
						dataString<<QString::number(data_out[i][j])<<" ";
					}
				}
				dataString<<"\n";
			}

			free(data_out[0]);
			free(data_out);
		} else if (dclass == H5T_FLOAT) {
			if(typeSize == 4) {
				float** data_out = (float**) malloc(rows*sizeof(float*));
				data_out[0] = (float*)malloc( cols*rows*sizeof(float) );
				for (int i=1; i < rows; i++) data_out[i] = data_out[0]+i*cols;

				H5Dread(dataset, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, &data_out[0][0]);
				for (int i=startRow-1; i < qMin(endRow,lines+startRow-1); i++) {
					for (int j=startColumn-1; j < endColumn; j++) {
						if (dataSource != NULL) {
							dataPointers[j-startColumn+1]->operator[](i-startRow+1) = data_out[i][j];
						}else {
							dataString<<QString::number(data_out[i][j])<<" ";
						}
					}
					dataString<<"\n";
				}

				free(data_out[0]);
				free(data_out);
			}else if(typeSize == 8) {
				double** data_out = (double**) malloc(rows*sizeof(double*));
				data_out[0] = (double*)malloc( cols*rows*sizeof(double) );
				for (int i=1; i < rows; i++) data_out[i] = data_out[0]+i*cols;

				H5Dread(dataset, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL,H5P_DEFAULT, &data_out[0][0]);
				for (int i=startRow-1; i < qMin(endRow,lines+startRow-1); i++) {
					for (int j=startColumn-1; j < endColumn; j++) {
						if (dataSource != NULL) {
							dataPointers[j-startColumn+1]->operator[](i-startRow+1) = data_out[i][j];
						}else {
							dataString<<QString::number(data_out[i][j])<<" ";
						}
					}
					dataString<<"\n";
				}

				free(data_out[0]);
				free(data_out);
			}else {
				dataString<<"data type size"<<QString::number(typeSize)<<"not supported";
				qDebug()<<dataString.join(" ");
			}
		} else if (dclass == H5T_STRING) {
			//TODO
			dataString<<translateHDFClass(dclass)<<" not implemented yet";
			dataString<<", size ="<<QString::number(typeSize);
			qDebug()<<dataString.join("");
		} else {
			dataString<<translateHDFClass(dclass)<<" data class not supported";
			qDebug()<<dataString.join("");
		}
		break;
	}
	default:
		dataString<<"rank = "<<QString::number(rank)<<" not supported";
		qDebug()<<dataString.join("");
	}

	H5Sclose(dataspace);
	H5Tclose(datatype);
	H5Dclose(dataset);
	H5Fclose(file);

	// set column comments in spreadsheet
	if (dataSource != NULL && spreadsheet != NULL) {
		QString comment = i18np("numerical data, %1 element", "numerical data, %1 elements", actualRows);
		for ( int n=0; n<actualCols; n++ ){
			Column* column = spreadsheet->column(columnOffset+n);
			column->setComment(comment);
			column->setUndoAware(true);
			if (mode==AbstractFileFilter::Replace) {
				column->setSuppressDataChangedSignal(false);
				column->setChanged();
			}
		}
		spreadsheet->setUndoAware(true);
	}

	return dataString.join("");
}

/*!
    reads the content of the file \c fileName to the data source \c dataSource.
    Uses the settings defined in the data source.
*/
void HDFFilterPrivate::read(const QString & fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode mode){
	Q_UNUSED(dataSource)
	Q_UNUSED(mode)
#ifdef QT_DEBUG
	qDebug()<<"HDFFilterPrivate::read()";
#endif	
	
	if(currentDataSet.isEmpty()) {
		qDebug()<<" No data set selected";
		return;
	}
#ifdef QT_DEBUG
	else
		qDebug()<<" current data set ="<<currentDataSet;
#endif

	readCurrentDataSet(fileName,dataSource,mode);
}

/*!
    writes the content of \c dataSource to the file \c fileName.
*/
void HDFFilterPrivate::write(const QString & fileName, AbstractDataSource* dataSource){
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
	writer->writeAttribute("autoMode", QString::number(d->autoModeEnabled) );
	writer->writeEndElement();
}

/*!
  Loads from XML.
*/
bool HDFFilter::load(XmlStreamReader* reader) {
	if(!reader->isStartElement() || reader->name() != "hdfFilter"){
		reader->raiseError(i18n("no hdf filter element found"));
		return false;
	}

	QString attributeWarning = i18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs = reader->attributes();

	// read attributes
	QString str = attribs.value("autoMode").toString();
	if(str.isEmpty())
		reader->raiseWarning(attributeWarning.arg("'autoMode'"));
	else
		d->autoModeEnabled = str.toInt();

	return true;
}
