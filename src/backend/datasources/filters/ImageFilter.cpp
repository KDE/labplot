/***************************************************************************
File                 : ImageFilter.cpp
Project              : LabPlot
Description          : Image I/O-filter
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
#include "backend/datasources/filters/ImageFilter.h"
#include "backend/datasources/filters/ImageFilterPrivate.h"
#include "backend/datasources/FileDataSource.h"
#include "backend/core/column/Column.h"

#include <math.h>

#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <KLocale>

 /*!
	\class ImageFilter
	\brief Manages the import/export of data from/to an image file.

	\ingroup datasources
 */
ImageFilter::ImageFilter():AbstractFileFilter(), d(new ImageFilterPrivate(this)){

}

ImageFilter::~ImageFilter(){
	delete d;
}

/*!
  reads the content of the file \c fileName to the data source \c dataSource.
*/
void ImageFilter::read(const QString & fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode){
	d->read(fileName, dataSource, importMode);
}

/*!
writes the content of the data source \c dataSource to the file \c fileName.
*/
void ImageFilter::write(const QString & fileName, AbstractDataSource* dataSource){
 	d->write(fileName, dataSource);
// 	emit()
}

///////////////////////////////////////////////////////////////////////
/*!
  loads the predefined filter settings for \c filterName
*/
void ImageFilter::loadFilterSettings(const QString& filterName){
	Q_UNUSED(filterName);
}

/*!
  saves the current settings as a new filter with the name \c filterName
*/
void ImageFilter::saveFilterSettings(const QString& filterName) const{
	Q_UNUSED(filterName);
}

///////////////////////////////////////////////////////////////////////

void ImageFilter::setAutoModeEnabled(bool b){
	d->autoModeEnabled = b;
}

bool ImageFilter::isAutoModeEnabled() const{
	return d->autoModeEnabled;
}
//#####################################################################
//################### Private implementation ##########################
//#####################################################################

ImageFilterPrivate::ImageFilterPrivate(ImageFilter* owner) :
	q(owner){
}

/*!
    reads the content of the file \c fileName to the data source \c dataSource.
    Uses the settings defined in the data source.
*/
void ImageFilterPrivate::read(const QString & fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode mode){
#ifdef QT_DEBUG
	qDebug()<<"ImageFilterPrivate::read()";
#endif

	QFile file(fileName);
	if ( !file.exists() )
		return;

	if (!file.open(QIODevice::ReadOnly))
        	return;

	QDataStream in(&file);

	// see binary filter
}

/*!
    writes the content of \c dataSource to the file \c fileName.
*/
void ImageFilterPrivate::write(const QString & fileName, AbstractDataSource* dataSource){
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
void ImageFilter::save(QXmlStreamWriter* writer) const {
	writer->writeStartElement("imageFilter");
	writer->writeEndElement();
}

/*!
  Loads from XML.
*/
bool ImageFilter::load(XmlStreamReader* reader) {
	if(!reader->isStartElement() || reader->name() != "imageFilter"){
		reader->raiseError(i18n("no binary filter element found"));
		return false;
	}

	QString attributeWarning = i18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs = reader->attributes();

	return true;
}
