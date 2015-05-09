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
	q(owner) {
}

#ifdef HAVE_HDF5
QStringList HDFFilterPrivate::scanHDFAttrs(hid_t aid) {
	QStringList attr;

	char buf[MAXNAMELENGTH];
	H5Aget_name(aid, MAXNAMELENGTH, buf );
	attr<<QString(buf);

        hid_t aspace = H5Aget_space(aid); /* the dimensions of the attribute data */
        hid_t atype  = H5Aget_type(aid);
        //readDatatype(atype);

        /* ... read data with H5Aread etc. */

        H5Tclose(atype);
        H5Sclose(aspace);

	//TODO
	attr<<"test";

	return attr;
}

void HDFFilterPrivate::scanHDFDataType(hid_t tid, char *dataSetName, QTreeWidgetItem* parentItem) {
	H5T_class_t typeClass = H5Tget_class(tid);

	QStringList typeProps;
	switch(typeClass) {
        case H5T_INTEGER:
		typeProps<<"INTEGER";
                break;
        case H5T_FLOAT:
		typeProps<<"FLOAT";
                break;
        case H5T_STRING:
		typeProps<<"STRING";
                break;
        case H5T_BITFIELD:
		typeProps<<"BITFIELD";
                break;
        case H5T_OPAQUE:
		typeProps<<"OPAQUE";
                break;
        case H5T_COMPOUND:
		typeProps<<"COMPOUND";
                break;
        case H5T_ARRAY:
		typeProps<<"ARRAY";
                break;
        case H5T_ENUM:
		typeProps<<"ENUM";
                break;
        default:
		typeProps<<"UNKNOWN";
                break;
        }

        size_t size = H5Tget_size(tid);
	typeProps<<"("<<QString::number(size)<<")";

	H5T_order_t order = H5Tget_order(tid);
        switch(order) {
        case H5T_ORDER_LE:
		typeProps<<"LE";
                break;
        case H5T_ORDER_BE:
		typeProps<<"BE";
                break;
        case H5T_ORDER_VAX:
		typeProps<<"VAX";
                break;
        case H5T_ORDER_MIXED:
		typeProps<<"MIXED";
                break;
        case H5T_ORDER_NONE:
		typeProps<<"NONE";
                break;
        case H5T_ORDER_ERROR:
		typeProps<<"ERROR";
                break;
        }

	//TODO scanHDFAttrs(tid);
	
	QTreeWidgetItem *dataTypeItem = new QTreeWidgetItem((QTreeWidget*)0, QStringList()<<QString(dataSetName)<<typeProps.join(" ")<<"attributes");
	dataTypeItem->setIcon(0,QIcon(KIcon("accessories-calculator")));
	parentItem->addChild(dataTypeItem);

}

void HDFFilterPrivate::scanHDFDataSet(hid_t dsid, char *dataSetName, QTreeWidgetItem* parentItem) {
	//TODO: read attributes

	QTreeWidgetItem *dataSetItem = new QTreeWidgetItem((QTreeWidget*)0, QStringList()<<QString(dataSetName)<<"data set");
	dataSetItem->setIcon(0,QIcon(KIcon("x-office-spreadsheet")));
	parentItem->addChild(dataSetItem);

}

void HDFFilterPrivate::scanHDFLink(hid_t gid, char *linkName, QTreeWidgetItem* parentItem) {
        char target[MAXNAMELENGTH];
        H5Gget_linkval(gid, linkName, MAXNAMELENGTH, target) ;
	//TODO: check for broken links

	//TODO: read attributes

	QTreeWidgetItem *linkItem = new QTreeWidgetItem((QTreeWidget*)0, QStringList()<<QString(linkName)<<"symlink"<<QString(target));
	linkItem->setIcon(0,QIcon(KIcon("emblem-symbolic-link")));
	parentItem->addChild(linkItem);
}

void HDFFilterPrivate::scanHDFGroup(hid_t gid, char *groupName, QTreeWidgetItem* parentItem) {

	//check for hard link
	H5G_stat_t  statbuf;
	H5Gget_objinfo(gid, ".", TRUE, &statbuf);
	if (statbuf.nlink > 1) {
		if(multiLinkList.contains(statbuf.objno[0])) {
			QTreeWidgetItem *objectItem = new QTreeWidgetItem((QTreeWidget*)0, QStringList()<<QString(groupName)<<"hard link");
			objectItem->setIcon(0,QIcon(KIcon("link")));
                	parentItem->addChild(objectItem);
			return;
		} else {
			multiLinkList.append(statbuf.objno[0]);
			//qDebug()<<" group multiple links: "<<statbuf.objno[0]<<' '<<statbuf.objno[1];
		}
	}

	QTreeWidgetItem *groupItem = new QTreeWidgetItem((QTreeWidget*)0, QStringList()<<QString(groupName)<<"group");
	groupItem->setIcon(0,QIcon(KIcon("folder")));
	parentItem->addChild(groupItem);

	//TODO scanHDFAttrs(gid);

	hsize_t numObj;
	//TODO: check for errors
	herr_t err = H5Gget_num_objs(gid, &numObj);

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
			QTreeWidgetItem *objectItem = new QTreeWidgetItem((QTreeWidget*)0, QStringList()<<QString(memberName)<<"UNKNOWN");
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
    reads the content of the file \c fileName to the data source \c dataSource.
    Uses the settings defined in the data source.
*/
void HDFFilterPrivate::read(const QString & fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode mode){
	Q_UNUSED(fileName)
	Q_UNUSED(dataSource)
	Q_UNUSED(mode)
#ifdef QT_DEBUG
	qDebug()<<"HDFFilterPrivate::read()";
#endif	
	//TODO: how to get the selected data set?

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
