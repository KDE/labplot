/*
    File                 : ImageFilter.cpp
    Project              : LabPlot
    Description          : Image I/O-filter
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2015 Stefan Gerlach <stefan.gerlach@uni.kn>
    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "backend/datasources/filters/ImageFilter.h"
#include "backend/datasources/filters/ImageFilterPrivate.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/core/column/Column.h"
#include "backend/lib/macros.h"
#include "backend/lib/XmlStreamReader.h"

#include <QImage>
#include <KLocalizedString>

/*!
\class ImageFilter
\brief Manages the import/export of data from/to an image file.

\ingroup datasources
*/
ImageFilter::ImageFilter():AbstractFileFilter(FileType::Image), d(new ImageFilterPrivate(this)) {}

ImageFilter::~ImageFilter() = default;

/*!
returns the list of all predefined import formats.
*/
QStringList ImageFilter::importFormats() {
	return (QStringList()
		<< i18n("Matrix (grayscale)")
		<< i18n("XYZ (grayscale)")
		<< i18n("XYRGB")
	);
}

/*!
  reads the content of the file \c fileName to the data source \c dataSource.
*/
void ImageFilter::readDataFromFile(const QString& fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode) {
	d->readDataFromFile(fileName, dataSource, importMode);
}

/*!
writes the content of the data source \c dataSource to the file \c fileName.
*/
void ImageFilter::write(const QString & fileName, AbstractDataSource* dataSource) {
	d->write(fileName, dataSource);
// 	emit()
}

///////////////////////////////////////////////////////////////////////
/*!
  loads the predefined filter settings for \c filterName
*/
void ImageFilter::loadFilterSettings(const QString& filterName) {
	Q_UNUSED(filterName);
}

/*!
  saves the current settings as a new filter with the name \c filterName
*/
void ImageFilter::saveFilterSettings(const QString& filterName) const {
	Q_UNUSED(filterName);
}

///////////////////////////////////////////////////////////////////////
void ImageFilter::setImportFormat(const ImageFilter::ImportFormat f) {
	d->importFormat = f;
}

ImageFilter::ImportFormat ImageFilter::importFormat() const {
	return d->importFormat;
}

void ImageFilter::setStartRow(const int s) {
	d->startRow = s;
}

int ImageFilter::startRow() const {
	return d->startRow;
}

void ImageFilter::setEndRow(const int e) {
	d->endRow = e;
}

int ImageFilter::endRow() const {
	return d->endRow;
}

void ImageFilter::setStartColumn(const int s) {
	d->startColumn = s;
}

int ImageFilter::startColumn() const {
	return d->startColumn;
}

void ImageFilter::setEndColumn(const int e) {
	d->endColumn = e;
}

int ImageFilter::endColumn() const {
	return d->endColumn;
}

QString ImageFilter::fileInfoString(const QString& fileName) {
	DEBUG("ImageFilter::fileInfoString()");
	QString info;

	//TODO
	Q_UNUSED(fileName);

	return info;
}

//#####################################################################
//################### Private implementation ##########################
//#####################################################################

ImageFilterPrivate::ImageFilterPrivate(ImageFilter* owner) : q(owner) {}

/*!
    reads the content of the file \c fileName to the data source \c dataSource.
    Uses the settings defined in the data source.
*/
void ImageFilterPrivate::readDataFromFile(const QString& fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode mode) {
	QImage image = QImage(fileName);
	if (image.isNull() || image.format() == QImage::Format_Invalid) {
#ifdef QT_DEBUG
		qDebug()<<"failed to read image"<<fileName<<"or invalid image format";
#endif
		return;
	}

	int cols = image.width();
	int rows = image.height();

	// set range of rows
	if (endColumn == -1)
		endColumn = cols;
	if (endRow == -1)
		endRow = rows;
	int actualCols = 0, actualRows = 0;

	switch (importFormat) {
	case ImageFilter::ImportFormat::MATRIX:
		actualCols = endColumn-startColumn+1;
		actualRows = endRow-startRow+1;
		break;
	case ImageFilter::ImportFormat::XYZ:
		actualCols = 3;
		actualRows = (endColumn-startColumn+1)*(endRow-startRow+1);
		break;
	case ImageFilter::ImportFormat::XYRGB:
		actualCols = 5;
		actualRows = (endColumn-startColumn+1)*(endRow-startRow+1);
	}

	DEBUG("image format =" << image.format());
	DEBUG("image w/h =" << cols << rows);
	DEBUG("actual rows/cols =" << actualRows << actualCols);

	//make sure we have enough columns in the data source.
	int columnOffset = 0;
	std::vector<void*> dataContainer;

	//TODO: support other modes
	QVector<AbstractColumn::ColumnMode> columnModes;
	columnModes.resize(actualCols);

	//TODO: use given names?
	QStringList vectorNames;

	if (dataSource)
		columnOffset = dataSource->prepareImport(dataContainer, mode, actualRows, actualCols, vectorNames, columnModes);
	else {
		DEBUG("data source in image import not defined! Giving up.");
		return;
	}

	// read data
	switch (importFormat) {
	case ImageFilter::ImportFormat::MATRIX: {
			for (int i = 0; i < actualRows; ++i) {
				for (int j = 0; j < actualCols; ++j) {
					double value = qGray(image.pixel(j+startColumn-1, i+startRow-1));
					static_cast<QVector<double>*>(dataContainer[j])->operator[](i) = value;
				}
				emit q->completed(100*i/actualRows);
			}
			break;
		}
	case ImageFilter::ImportFormat::XYZ: {
			int currentRow = 0;
			for (int i = startRow-1; i < endRow; ++i) {
				for (int j = startColumn-1; j < endColumn; ++j) {
					QRgb color = image.pixel(j, i);
					static_cast<QVector<int>*>(dataContainer[0])->operator[](currentRow) = i+1;
					static_cast<QVector<int>*>(dataContainer[1])->operator[](currentRow) = j+1;
					static_cast<QVector<int>*>(dataContainer[2])->operator[](currentRow) = qGray(color);
					currentRow++;
				}
				emit q->completed(100*i/actualRows);
			}
			break;
		}
	case ImageFilter::ImportFormat::XYRGB: {
			int currentRow = 0;
			for (int i = startRow-1; i < endRow; ++i) {
				for ( int j = startColumn-1; j < endColumn; ++j) {
					QRgb color = image.pixel(j, i);
					static_cast<QVector<int>*>(dataContainer[0])->operator[](currentRow) = i+1;
					static_cast<QVector<int>*>(dataContainer[1])->operator[](currentRow) = j+1;
					static_cast<QVector<int>*>(dataContainer[2])->operator[](currentRow) = qRed(color);
					static_cast<QVector<int>*>(dataContainer[3])->operator[](currentRow) = qGreen(color);
					static_cast<QVector<int>*>(dataContainer[4])->operator[](currentRow) = qBlue(color);
					currentRow++;
				}
				emit q->completed(100*i/actualRows);
			}
			break;
		}
	}

	auto* spreadsheet = dynamic_cast<Spreadsheet*>(dataSource);
	if (spreadsheet) {
		QString comment = i18np("numerical data, %1 element", "numerical data, %1 elements", rows);
		for ( int n = 0; n < actualCols; ++n) {
			Column* column = spreadsheet->column(columnOffset+n);
			column->setComment(comment);
			column->setUndoAware(true);
			if (mode == AbstractFileFilter::ImportMode::Replace) {
				column->setSuppressDataChangedSignal(false);
				column->setChanged();
			}
		}
	}

	dataSource->finalizeImport();
	return;
}

/*!
    writes the content of \c dataSource to the file \c fileName.
*/
void ImageFilterPrivate::write(const QString & fileName, AbstractDataSource* dataSource) {
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
	Q_UNUSED(reader);
// 	KLocalizedString attributeWarning = ki18n("Attribute '%1' missing or empty, default value is used");
// 	QXmlStreamAttributes attribs = reader->attributes();

	return true;
}
