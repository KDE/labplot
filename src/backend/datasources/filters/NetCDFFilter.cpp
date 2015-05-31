/***************************************************************************
File                 : NetCDFFilter.cpp
Project              : LabPlot
Description          : NetCDF I/O-filter
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
#include "backend/datasources/filters/NetCDFFilter.h"
#include "backend/datasources/filters/NetCDFFilterPrivate.h"
#include "backend/datasources/FileDataSource.h"
#include "backend/core/column/Column.h"

#include <math.h>

#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <KLocale>
#include <KIcon>

/*!
	\class NetCDFFilter
	\brief Manages the import/export of data from/to a NetCDF file.

	\ingroup datasources
*/
NetCDFFilter::NetCDFFilter():AbstractFileFilter(), d(new NetCDFFilterPrivate(this)){

}

NetCDFFilter::~NetCDFFilter(){
	delete d;
}

/*!
  parses the content of the file \c fileName.
*/
void NetCDFFilter::parse(const QString & fileName, QTreeWidgetItem* rootItem){
	d->parse(fileName, rootItem);
}

/*!
  reads the content of the data set \c dataSet from file \c fileName.
*/
/*QString NetCDFFilter::readCurrentDataSet(const QString & fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode,  int lines){
	return d->readCurrentDataSet(fileName, dataSource, importMode, lines);
}*/

/*!
  reads the content of the file \c fileName to the data source \c dataSource.
*/
void NetCDFFilter::read(const QString & fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode){
	d->read(fileName, dataSource, importMode);
}

/*!
writes the content of the data source \c dataSource to the file \c fileName.
*/
void NetCDFFilter::write(const QString & fileName, AbstractDataSource* dataSource){
 	d->write(fileName, dataSource);
// 	emit()
}

///////////////////////////////////////////////////////////////////////
/*!
  loads the predefined filter settings for \c filterName
*/
void NetCDFFilter::loadFilterSettings(const QString& filterName){
    Q_UNUSED(filterName);
}

/*!
  saves the current settings as a new filter with the name \c filterName
*/
void NetCDFFilter::saveFilterSettings(const QString& filterName) const{
    Q_UNUSED(filterName);
}

///////////////////////////////////////////////////////////////////////

/*void NetCDFFilter::setCurrentDataSet(QString ds){
	d->currentDataSet = ds;
}

QString NetCDFFilter::currentDataSet() const{
	return d->currentDataSet;
}*/

void NetCDFFilter::setStartRow(const int s) {
        d->startRow = s;
}

int NetCDFFilter::startRow() const{
        return d->startRow;
}

void NetCDFFilter::setEndRow(const int e) {
        d->endRow = e;
}

int NetCDFFilter::endRow() const{
        return d->endRow;
}

void NetCDFFilter::setStartColumn(const int c){
	d->startColumn=c;
}

int NetCDFFilter::startColumn() const{
	return d->startColumn;
}

void NetCDFFilter::setEndColumn(const int c){
	d->endColumn=c;
}

int NetCDFFilter::endColumn() const{
	return d->endColumn;
}

void NetCDFFilter::setAutoModeEnabled(bool b){
	d->autoModeEnabled = b;
}

bool NetCDFFilter::isAutoModeEnabled() const{
	return d->autoModeEnabled;
}
//#####################################################################
//################### Private implementation ##########################
//#####################################################################

NetCDFFilterPrivate::NetCDFFilterPrivate(NetCDFFilter* owner) : 
	q(owner),startRow(1), endRow(-1),startColumn(1),endColumn(-1) {
}

#ifdef HAVE_NETCDF
void NetCDFFilterPrivate::handleError(int status, QString function) {
	if (status != NC_NOERR) {
		qDebug()<<"ERROR:"<<function<<"() - "<<nc_strerror(status);
	}
}

QString NetCDFFilterPrivate::translateDataType(nc_type type) {
	QString typeString;

	switch(type) {
	case NC_BYTE:
		typeString="BYTE";
		break;
	case NC_UBYTE:
		typeString="UBYTE";
		break;
	case NC_CHAR:
		typeString="CHAR";
		break;
	case NC_SHORT:
		typeString="SHORT";
		break;
	case NC_USHORT:
		typeString="USHORT";
		break;
	case NC_INT:
		typeString="INT";
		break;
	case NC_UINT:
		typeString="UINT";
		break;
//	case NC_LONG:
//		typeString="LONG";
//		break;
	case NC_INT64:
		typeString="INT64";
		break;
	case NC_UINT64:
		typeString="UINT64";
		break;
	case NC_FLOAT:
		typeString="FLOAT";
		break;
	case NC_DOUBLE:
		typeString="DOUBLE";
		break;
	case NC_STRING:
		typeString="STRING";
		break;
	default:
		typeString="UNKNOWN";
	}

	return typeString;
}

void NetCDFFilterPrivate::scanAttrs(int ncid, int varid, int nattr, QTreeWidgetItem* parentItem) {
	int status;

	char name[NC_MAX_NAME + 1];
	nc_type type;
	size_t len;
	for(int i=0;i<nattr;i++) {
		status = nc_inq_attname(ncid,varid,i,name);
		handleError(status,"nc_inq_attname");

		status = nc_inq_att(ncid, varid, name, &type, &len);
		handleError(status,"nc_inq_att");
		qDebug()<<"	attr"<<i+1<<": name/type/len="<<name<<translateDataType(type)<<len;

		//read attribute
		QStringList valueString;
		switch(type) {
		case NC_BYTE: {
			signed char *value = (signed char *)malloc(len*sizeof(signed char));
			status = nc_get_att_schar(ncid, varid, name, value);
			handleError(status,"nc_get_att_schar");
			for(unsigned int l=0;l<len;l++)
				valueString<<QString::number(value[l]);
			free(value);
			break;
		}
		case NC_UBYTE: {
			unsigned char *value = (unsigned char *)malloc(len*sizeof(unsigned char));
			status = nc_get_att_uchar(ncid, varid, name, value);
			handleError(status,"nc_get_att_uchar");
			for(unsigned int l=0;l<len;l++)
				valueString<<QString::number(value[l]);
			free(value);
			break;
		}
		case NC_CHAR: {
			char *value = (char *)malloc((len+1)*sizeof(char));
			status = nc_get_att_text(ncid, varid, name, value);
			handleError(status,"nc_get_att_text");
			value[len]=0;
			valueString<<value;
			free(value);
			break;
		}
		case NC_SHORT: {
			short *value = (short *)malloc(len*sizeof(short));
			status = nc_get_att_short(ncid, varid, name, value);
			handleError(status,"nc_get_att_short");
			for(unsigned int l=0;l<len;l++)
				valueString<<QString::number(value[l]);
			free(value);
			break;
		}
		case NC_USHORT: {
			unsigned short *value = (unsigned short *)malloc(len*sizeof(unsigned short));
			status = nc_get_att_ushort(ncid, varid, name, value);
			handleError(status,"nc_get_att_ushort");
			for(unsigned int l=0;l<len;l++)
				valueString<<QString::number(value[l]);
			free(value);
			break;
		}
		case NC_INT: {
			int *value = (int *)malloc(len*sizeof(int));
			status = nc_get_att_int(ncid, varid, name, value);
			handleError(status,"nc_get_att_int");
			for(unsigned int l=0;l<len;l++)
				valueString<<QString::number(value[l]);
			free(value);
			break;
		}
		case NC_UINT: {
			unsigned int *value = (unsigned int *)malloc(len*sizeof(unsigned int));
			status = nc_get_att_uint(ncid, varid, name, value);
			handleError(status,"nc_get_att_uint");
			for(unsigned int l=0;l<len;l++)
				valueString<<QString::number(value[l]);
			free(value);
			break;
		}
		case NC_INT64: {
			long long *value = (long long *)malloc(len*sizeof(long long));
			status = nc_get_att_longlong(ncid, varid, name, value);
			handleError(status,"nc_get_att_longlong");
			for(unsigned int l=0;l<len;l++)
				valueString<<QString::number(value[l]);
			free(value);
			break;
		}
		case NC_UINT64: {
			unsigned long long *value = (unsigned long long *)malloc(len*sizeof(unsigned long long));
			status = nc_get_att_ulonglong(ncid, varid, name, value);
			handleError(status,"nc_get_att_ulonglong");
			for(unsigned int l=0;l<len;l++)
				valueString<<QString::number(value[l]);
			free(value);
			break;
		}
		case NC_FLOAT: {
			float *value = (float *)malloc(len*sizeof(float));
			status = nc_get_att_float(ncid, varid, name, value);
			handleError(status,"nc_get_att_float");
			for(unsigned int l=0;l<len;l++)
				valueString<<QString::number(value[l]);
			free(value);
			break;
		}
		case NC_DOUBLE: {
			double *value = (double *)malloc(len*sizeof(double));
			status = nc_get_att_double(ncid, varid, name, value);
			handleError(status,"nc_get_att_double");
			for(unsigned int l=0;l<len;l++)
				valueString<<QString::number(value[l]);
			free(value);
			break;
		}
		default:
			valueString<<"not supported";
		}

		QStringList props;
		props<<translateDataType(type)<<" ("<<QString::number(len)<<")";
		QTreeWidgetItem *attrItem = new QTreeWidgetItem((QTreeWidget*)0, QStringList()<<QString(name)<<props.join("")<<valueString.join(", "));
		attrItem->setIcon(0,QIcon(KIcon("accessories-calculator")));
		parentItem->addChild(attrItem);
	}
}

void NetCDFFilterPrivate::scanDims(int ncid, int ndims, QTreeWidgetItem* parentItem) {
	int status;

	int ulid;
	status = nc_inq_unlimdim(ncid,&ulid);
	handleError(status,"nc_inq_att");

	char name[NC_MAX_NAME + 1];
	size_t len;
	for(int i=0;i<ndims;i++) {
		status = nc_inq_dim(ncid, i, name, &len);
		handleError(status,"nc_inq_att");
		qDebug()<<"	dim"<<i+1<<": name/len="<<name<<len;
		
		QStringList props;
		props<<"length = "<<QString::number(len);
		QString value;
		if(i == ulid)
			value="unlimited";
		QTreeWidgetItem *attrItem = new QTreeWidgetItem((QTreeWidget*)0, QStringList()<<QString(name)<<props.join("")<<value);
		attrItem->setIcon(0,QIcon(KIcon("accessories-calculator")));
		parentItem->addChild(attrItem);
	}
}

void NetCDFFilterPrivate::scanVars(int ncid, int nvars, QTreeWidgetItem* parentItem) {
	int status;

	char name[NC_MAX_NAME + 1];
	nc_type type;
	int ndims, nattrs;
	int dimids[NC_MAX_VAR_DIMS];

	for(int i=0;i<nvars;i++) {
		status = nc_inq_var(ncid, i, name, &type, &ndims, dimids, &nattrs);
		handleError(status,"nc_inq_att");

		qDebug()<<"	var"<<i+1<<": name/type="<<name<<translateDataType(type);
		qDebug()<<"		ndims/nattr"<<ndims<<nattrs;

		QStringList props;
		props<<translateDataType(type);
		char dname[NC_MAX_NAME + 1];
		size_t dlen;
		for(int j=0;j<ndims;j++) {
			status = nc_inq_dim(ncid, dimids[j], dname, &dlen);
			if(j==0)
				props<<" ("<<QString::number(dlen);
			else
				props<<"x"<<QString::number(dlen);
		}
		props<<")";

		QTreeWidgetItem *varItem = new QTreeWidgetItem((QTreeWidget*)0, QStringList()<<QString(name)<<props.join(""));
		varItem->setIcon(0,QIcon(KIcon("x-office-spreadsheet")));
		varItem->setBackground(0,QBrush(QColor(192,255,192)));
		parentItem->addChild(varItem);

		scanAttrs(ncid,i,nattrs,varItem);
	}
}
#endif

/*!
    parses the content of the file \c fileName and fill the tree using rootItem.
*/
void NetCDFFilterPrivate::parse(const QString & fileName, QTreeWidgetItem* rootItem) {
#ifdef HAVE_NETCDF
	QByteArray bafileName = fileName.toLatin1();

	int status, ncid;
	status = nc_open(bafileName.data(), NC_NOWRITE, &ncid);
	handleError(status,"nc_open");
	
	int ndims, nvars, nattr, uldid;
	status = nc_inq(ncid, &ndims, &nvars, &nattr, &uldid);
	handleError(status,"nc_inq");
	qDebug()<<" nattr/ndims/nvars ="<<nattr<<ndims<<nvars;

	QTreeWidgetItem *attrItem = new QTreeWidgetItem((QTreeWidget*)0, QStringList()<<QString("Attributes"));
	attrItem->setIcon(0,QIcon(KIcon("folder")));
	rootItem->addChild(attrItem);
	scanAttrs(ncid,NC_GLOBAL,nattr,attrItem);

	QTreeWidgetItem *dimItem = new QTreeWidgetItem((QTreeWidget*)0, QStringList()<<QString("Dimensions"));
	dimItem->setIcon(0,QIcon(KIcon("folder")));
	rootItem->addChild(dimItem);
	scanDims(ncid,ndims,dimItem);

	QTreeWidgetItem *varItem = new QTreeWidgetItem((QTreeWidget*)0, QStringList()<<QString("Variables"));
	varItem->setIcon(0,QIcon(KIcon("folder")));
	rootItem->addChild(varItem);
	scanVars(ncid,nvars,varItem);

	//TODO: scan NetCDF file
#else
	Q_UNUSED(fileName)
	Q_UNUSED(rootItem)
#endif
}

/*!
    reads the content of the file \c fileName to the data source \c dataSource.
    Uses the settings defined in the data source.
*/
void NetCDFFilterPrivate::read(const QString & fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode mode){
	Q_UNUSED(fileName);
	Q_UNUSED(dataSource);
	Q_UNUSED(mode);
	//TODO

/*	if(currentDataSet.isEmpty()) {
		qDebug()<<" No data set selected";
		return;
	}

#ifdef QT_DEBUG
	else
		qDebug()<<" current data set ="<<currentDataSet;
#endif
*/
//	readCurrentDataSet(fileName,dataSource,mode);
}

/*!
    writes the content of \c dataSource to the file \c fileName.
*/
void NetCDFFilterPrivate::write(const QString & fileName, AbstractDataSource* dataSource){
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
void NetCDFFilter::save(QXmlStreamWriter* writer) const {
	writer->writeStartElement("hdfFilter");
	writer->writeAttribute("autoMode", QString::number(d->autoModeEnabled) );
	writer->writeEndElement();
}

/*!
  Loads from XML.
*/
bool NetCDFFilter::load(XmlStreamReader* reader) {
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
