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
QString HDFFilter::readCurrentDataSet(const QString & fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode,  unsigned int lines){
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
	q(owner),currentDataSet("") {
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

	//TODO
        hid_t aspace = H5Aget_space(aid); // the dimensions of the attribute data
        hid_t atype  = H5Aget_type(aid);
        //readDatatype(atype);

        // ... read data with H5Aread etc.

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
                H5Aclose(aid);
        }

	return attr;
}

void HDFFilterPrivate::scanHDFDataType(hid_t tid, char *dataSetName, QTreeWidgetItem* parentItem) {
	H5T_class_t typeClass = H5Tget_class(tid);
	QStringList typeProps;

	typeProps<<translateHDFClass(typeClass);

        size_t size = H5Tget_size(tid);
	typeProps<<" ("<<QString::number(size)<<") ";

	H5T_order_t order = H5Tget_order(tid);
	typeProps<<translateHDFOrder(order);
	QString attr = scanHDFAttrs(tid).join(" ");

	char link[MAXNAMELENGTH];
        H5Iget_name(tid, link, MAXNAMELENGTH);

	QTreeWidgetItem *dataTypeItem = new QTreeWidgetItem((QTreeWidget*)0, QStringList()<<QString(dataSetName)<<QString(link)<<i18n("data type")<<typeProps.join("")<<attr);
	dataTypeItem->setIcon(0,QIcon(KIcon("accessories-calculator")));
	parentItem->addChild(dataTypeItem);

}

void HDFFilterPrivate::scanHDFDataSet(hid_t did, char *dataSetName, QTreeWidgetItem* parentItem) {
	QString attr = scanHDFAttrs(did).join(" ");

	char link[MAXNAMELENGTH];
        H5Iget_name(did, link, MAXNAMELENGTH);

	QStringList dataSetProps;
	hsize_t size = H5Dget_storage_size(did);
	hid_t datatype  = H5Dget_type(did);
	H5T_class_t dclass = H5Tget_class(datatype);
	H5T_order_t order = H5Tget_order(datatype);
	size_t typeSize  = H5Tget_size(datatype);

	dataSetProps<<translateHDFClass(dclass)<<" ("<<QString::number(typeSize)<<") ";
	dataSetProps<<translateHDFOrder(order);

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

	QTreeWidgetItem *dataSetItem = new QTreeWidgetItem((QTreeWidget*)0, QStringList()<<QString(dataSetName)<<QString(link)<<i18n("data set")<<dataSetProps.join("")<<attr);
	dataSetItem->setIcon(0,QIcon(KIcon("x-office-spreadsheet")));
	dataSetItem->setBackground(0,QBrush(QColor(192,255,192)));
	parentItem->addChild(dataSetItem);
}

void HDFFilterPrivate::scanHDFLink(hid_t gid, char *linkName, QTreeWidgetItem* parentItem) {
        char target[MAXNAMELENGTH];
        H5Gget_linkval(gid, linkName, MAXNAMELENGTH, target) ;
	//TODO: check for broken links

	QTreeWidgetItem *linkItem = new QTreeWidgetItem((QTreeWidget*)0, QStringList()<<QString(linkName)<<" "<<i18n("symbolic link")<<QString(target));
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
	// parse file fileName
	QByteArray bafileName = fileName.toLatin1();
	hid_t file = H5Fopen(bafileName.data(), H5F_ACC_RDONLY, H5P_DEFAULT);
	char rootName[]="/";
	hid_t group = H5Gopen(file, rootName, H5P_DEFAULT);
	multiLinkList.clear();
	scanHDFGroup(group,rootName, rootItem);
	H5Fclose(file);
#endif
}

/*!
    reads the content of the date set in the file \c fileName to a string (for preview) or to the data source.
*/
QString HDFFilterPrivate::readCurrentDataSet(const QString & fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode mode, unsigned int lines){
	QStringList dataString;

	if(currentDataSet.isEmpty())
		return QString("No data set selected");
#ifdef QT_DEBUG
	else
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
	if( rank != 2) {
		dataString<<"rank ="<<QString::number(rank)<<"not supported";
		qDebug()<<dataString.join(" ");
		H5Sclose(dataspace);
		H5Tclose(datatype);
		H5Dclose(dataset);
		H5Fclose(file);
		return dataString.join("");
	}

	hsize_t dims_out[2];
    	H5Sget_simple_extent_dims(dataspace, dims_out, NULL);
	unsigned int rows = dims_out[0];
	unsigned int cols = dims_out[1];

#ifdef QT_DEBUG
	H5T_order_t order = H5Tget_order(datatype);
	qDebug()<<translateHDFClass(dclass)<<"("<<typeSize<<")"<<translateHDFOrder(order)<<","<<rows<<"x"<<cols;
#endif
	//TODO: use start/end row/col

	if (dataSource != NULL) {
		int columnOffset = dataSource->resize(mode,QStringList(),cols);
		
		//TODO
	}
	
	// read data
	if (dclass == H5T_INTEGER) {
		int** data_out = (int**) malloc(rows*sizeof(int*));
		data_out[0] = (int*)malloc( cols*rows*sizeof(int) );
		for (unsigned int i=1; i < rows; i++) data_out[i] = data_out[0]+i*cols;

	  	H5Dread(dataset, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, &data_out[0][0]);
		for (unsigned int i=0; i < qMin(rows,lines); i++) {
			for (unsigned int j=0; j < cols; j++) {
				dataString<<QString::number(data_out[i][j])<<" ";
			}
			dataString<<"\n";
		}

		free(data_out[0]);
		free(data_out);
	} else if (dclass == H5T_FLOAT) {
		if(typeSize == 4) {
			float** data_out = (float**) malloc(rows*sizeof(float*));
			data_out[0] = (float*)malloc( cols*rows*sizeof(float) );
			for (unsigned int i=1; i < rows; i++) data_out[i] = data_out[0]+i*cols;

		  	H5Dread(dataset, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, &data_out[0][0]);
			for (unsigned int i=0; i < qMin(rows,lines); i++) {
				for (unsigned int j=0; j < cols; j++)
					dataString<<QString::number(data_out[i][j])<<" ";
				dataString<<"\n";
			}

			free(data_out[0]);
			free(data_out);
		}else if(typeSize == 8) {
			double** data_out = (double**) malloc(rows*sizeof(double*));
			data_out[0] = (double*)malloc( cols*rows*sizeof(double) );
			for (unsigned int i=1; i < rows; i++) data_out[i] = data_out[0]+i*cols;

		  	H5Dread(dataset, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL,H5P_DEFAULT, &data_out[0][0]);
			for (unsigned int i=0; i < qMin(rows,lines); i++) {
				for (unsigned int j=0; j < cols; j++)
					dataString<<QString::number(data_out[i][j])<<" ";
				dataString<<"\n";
			}

			free(data_out[0]);
			free(data_out);
		}else {
			dataString<<" data type size"<<QString::number(typeSize)<<"not supported";
			qDebug()<<dataString.join(" ");
		}
	} else {
		dataString<<translateHDFClass(dclass)<<"data class not supported";
		qDebug()<<dataString.join(" ");
	}

	H5Sclose(dataspace);
	H5Tclose(datatype);
	H5Dclose(dataset);
	H5Fclose(file);

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
/*	// same as readCurrentDataSet()
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
*/

	//TODO
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
