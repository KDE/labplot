/*
	File                 : Image.cpp
	Project              : LabPlot
	Description          : Worksheet element to draw images
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2019-2022 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "Image.h"
#include "ImagePrivate.h"
#include "Worksheet.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
#include "backend/worksheet/Line.h"

#include <QBuffer>
#include <QFileInfo>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QIcon>
#include <QMenu>
#include <QPainter>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>

/**
 * \class Image
 * \brief A label supporting rendering of html- and tex-formatted texts.
 *
 * The label is aligned relative to the specified position.
 * The position can be either specified by providing the x- and y- coordinates
 * in parent's coordinate system, or by specifying one of the predefined position
 * flags (\c HorizontalPosition, \c VerticalPosition).
 */

Image::Image(const QString& name)
	: WorksheetElement(name, new ImagePrivate(this), AspectType::Image) {
	init();
}

Image::Image(const QString& name, ImagePrivate* dd)
	: WorksheetElement(name, dd, AspectType::Image) {
	init();
}

void Image::init() {
	Q_D(Image);

	KConfig config;
	KConfigGroup group = config.group(QStringLiteral("Image"));

	d->embedded = group.readEntry(QStringLiteral("embedded"), true);
	d->opacity = group.readEntry(QStringLiteral("opacity"), d->opacity);

	// geometry
	d->position.point.setX(group.readEntry(QStringLiteral("PositionXValue"), 0.));
	d->position.point.setY(group.readEntry(QStringLiteral("PositionYValue"), 0.));
	d->position.horizontalPosition =
		(WorksheetElement::HorizontalPosition)group.readEntry(QStringLiteral("PositionX"), (int)WorksheetElement::HorizontalPosition::Center);
	d->position.verticalPosition =
		(WorksheetElement::VerticalPosition)group.readEntry(QStringLiteral("PositionY"), (int)WorksheetElement::VerticalPosition::Center);
	d->horizontalAlignment =
		(WorksheetElement::HorizontalAlignment)group.readEntry(QStringLiteral("HorizontalAlignment"), (int)WorksheetElement::HorizontalAlignment::Center);
	d->verticalAlignment =
		(WorksheetElement::VerticalAlignment)group.readEntry(QStringLiteral("VerticalAlignment"), (int)WorksheetElement::VerticalAlignment::Center);
	d->setRotation(group.readEntry(QStringLiteral("Rotation"), d->rotation()));

	// border
	d->borderLine = new Line(QString());
	d->borderLine->setPrefix(QStringLiteral("Border"));
	d->borderLine->setHidden(true);
	addChild(d->borderLine);
	d->borderLine->init(group);
	connect(d->borderLine, &Line::updatePixmapRequested, [=] {
		d->update();
	});
	connect(d->borderLine, &Line::updateRequested, [=] {
		d->recalcShapeAndBoundingRect();
	});
}

// no need to delete the d-pointer here - it inherits from QGraphicsItem
// and is deleted during the cleanup in QGraphicsScene
Image::~Image() = default;

void Image::setParentGraphicsItem(QGraphicsItem* item) {
	Q_D(Image);
	d->setParentItem(item);
	d->updatePosition();
}

void Image::retransform() {
	Q_D(Image);
	d->retransform();
}

void Image::handleResize(double /*horizontalRatio*/, double /*verticalRatio*/, bool /*pageResize*/) {
	DEBUG(Q_FUNC_INFO);
	// 	Q_D(Image);

	// 	double ratio = 0;
	// 	if (horizontalRatio > 1.0 || verticalRatio > 1.0)
	// 		ratio = std::max(horizontalRatio, verticalRatio);
	// 	else
	// 		ratio = std::min(horizontalRatio, verticalRatio);
}

/*!
	Returns an icon to be used in the project explorer.
*/
QIcon Image::icon() const {
	return QIcon::fromTheme(QStringLiteral("viewimage"));
}

/* ============================ getter methods ================= */
BASIC_SHARED_D_READER_IMPL(Image, QString, fileName, fileName)
BASIC_SHARED_D_READER_IMPL(Image, bool, embedded, embedded)
BASIC_SHARED_D_READER_IMPL(Image, qreal, opacity, opacity)
BASIC_SHARED_D_READER_IMPL(Image, int, width, width)
BASIC_SHARED_D_READER_IMPL(Image, int, height, height)
BASIC_SHARED_D_READER_IMPL(Image, bool, keepRatio, keepRatio)

Line* Image::borderLine() const {
	Q_D(const Image);
	return d->borderLine;
}

/* ============================ setter methods and undo commands ================= */
STD_SETTER_CMD_IMPL_F_S(Image, SetFileName, QString, fileName, updateImage)
void Image::setFileName(const QString& fileName) {
	Q_D(Image);
	if (fileName != d->fileName)
		exec(new ImageSetFileNameCmd(d, fileName, ki18n("%1: set image")));
}

STD_SETTER_CMD_IMPL_S(Image, SetEmbedded, bool, embedded)
void Image::setEmbedded(bool embedded) {
	Q_D(Image);
	if (embedded != d->embedded)
		exec(new ImageSetEmbeddedCmd(d, embedded, ki18n("%1: embed image")));
}

STD_SETTER_CMD_IMPL_F_S(Image, SetOpacity, qreal, opacity, update)
void Image::setOpacity(qreal opacity) {
	Q_D(Image);
	if (opacity != d->opacity)
		exec(new ImageSetOpacityCmd(d, opacity, ki18n("%1: set border opacity")));
}

STD_SETTER_CMD_IMPL_F_S(Image, SetWidth, int, width, retransform)
void Image::setWidth(int width) {
	Q_D(Image);
	if (width != d->width) {
		exec(new ImageSetWidthCmd(d, width, ki18n("%1: set width")));
		d->scaleImage();
	}
}

STD_SETTER_CMD_IMPL_F_S(Image, SetHeight, int, height, retransform)
void Image::setHeight(int height) {
	Q_D(Image);
	if (height != d->height) {
		exec(new ImageSetHeightCmd(d, height, ki18n("%1: set height")));
		d->scaleImage();
	}
}

STD_SETTER_CMD_IMPL_S(Image, SetKeepRatio, bool, keepRatio)
void Image::setKeepRatio(bool keepRatio) {
	Q_D(Image);
	if (keepRatio != d->keepRatio)
		exec(new ImageSetKeepRatioCmd(d, keepRatio, ki18n("%1: change keep ratio")));
}

// ##############################################################################
// ####################### Private implementation ###############################
// ##############################################################################
ImagePrivate::ImagePrivate(Image* owner)
	: WorksheetElementPrivate(owner)
	, q(owner) {
	setFlag(QGraphicsItem::ItemIsSelectable);
	setFlag(QGraphicsItem::ItemIsMovable);
	setFlag(QGraphicsItem::ItemSendsGeometryChanges);
	setFlag(QGraphicsItem::ItemIsFocusable);
	setAcceptHoverEvents(true);

	// initial placeholder image
	image = QIcon::fromTheme(QStringLiteral("viewimage")).pixmap(width, height).toImage();
	imageScaled = image;
}

/*!
	calculates the position and the bounding box of the label. Called on geometry or text changes.
 */
void ImagePrivate::retransform() {
	const bool suppress = suppressRetransform || q->isLoading();
	trackRetransformCalled(suppress);
	if (suppress)
		return;

	int w = imageScaled.width();
	int h = imageScaled.height();
	m_boundingRectangle.setX(-w / 2);
	m_boundingRectangle.setY(-h / 2);
	m_boundingRectangle.setWidth(w);
	m_boundingRectangle.setHeight(h);

	updatePosition(); // needed, because CartesianPlot calls retransform if some operations are done
	updateBorder();
}

void ImagePrivate::updateImage() {
	if (!fileName.isEmpty()) {
		image = QImage(fileName);
		width = image.width();
		height = image.height();
	} else {
		width = Worksheet::convertToSceneUnits(2, Worksheet::Unit::Centimeter);
		height = Worksheet::convertToSceneUnits(3, Worksheet::Unit::Centimeter);
		image = QIcon::fromTheme(QStringLiteral("viewimage")).pixmap(width, height).toImage();
	}

	imageScaled = image;

	Q_EMIT q->widthChanged(width);
	Q_EMIT q->heightChanged(height);

	retransform();
}

void ImagePrivate::scaleImage() {
	if (keepRatio) {
		if (width != imageScaled.width()) {
			// width was changed -> rescale the height to keep the ratio
			if (imageScaled.width() != 0)
				height = imageScaled.height() * width / imageScaled.width();
			else
				height = 0;
			Q_EMIT q->heightChanged(height);
		} else if (height != imageScaled.height()) {
			// height was changed -> rescale the width to keep the ratio
			if (imageScaled.height() != 0)
				width = imageScaled.width() * height / imageScaled.height();
			else
				width = 0;
			Q_EMIT q->widthChanged(width);
		}
	}

	if (width != 0 && height != 0)
		imageScaled = image.scaled(width, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

	retransform();
}

void ImagePrivate::updateBorder() {
	borderShapePath = QPainterPath();
	borderShapePath.addRect(m_boundingRectangle);
	recalcShapeAndBoundingRect();
}

/*!
	Returns the outer bounds of the item as a rectangle.
 */
QRectF ImagePrivate::boundingRect() const {
	return transformedBoundingRectangle;
}

/*!
	Returns the shape of this item as a QPainterPath in local coordinates.
*/
QPainterPath ImagePrivate::shape() const {
	return imageShape;
}

/*!
  recalculates the outer bounds and the shape of the label.
*/
void ImagePrivate::recalcShapeAndBoundingRect() {
	prepareGeometryChange();

	QTransform matrix;
	imageShape = QPainterPath();
	if (borderLine->pen().style() != Qt::NoPen) {
		imageShape.addPath(WorksheetElement::shapeFromPath(borderShapePath, borderLine->pen()));
		transformedBoundingRectangle = matrix.mapRect(imageShape.boundingRect());
	} else {
		imageShape.addRect(m_boundingRectangle);
		transformedBoundingRectangle = matrix.mapRect(m_boundingRectangle);
	}

	imageShape = matrix.map(imageShape);

	Q_EMIT q->changed();
}

void ImagePrivate::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget*) {
	painter->save();

	// draw the image
	painter->setOpacity(opacity);
	painter->drawImage(m_boundingRectangle.topLeft(), imageScaled, imageScaled.rect());
	painter->restore();

	// draw the border
	if (borderLine->style() != Qt::NoPen) {
		painter->save();
		painter->setPen(borderLine->pen());
		painter->setBrush(Qt::NoBrush);
		painter->setOpacity(borderLine->opacity());
		painter->drawPath(borderShapePath);
		painter->restore();
	}

	const bool selected = isSelected();
	const bool hovered = (m_hovered && !selected);
	if ((hovered || selected) && !q->isPrinting()) {
		static double penWidth = 2.;
		const QRectF& br = boundingRect();
		const qreal width = br.width();
		const qreal height = br.height();
		const QRectF rect = QRectF(-width / 2 + penWidth / 2, -height / 2 + penWidth / 2, width - penWidth, height - penWidth);

		if (hovered)
			painter->setPen(QPen(QApplication::palette().color(QPalette::Shadow), penWidth));
		else
			painter->setPen(QPen(QApplication::palette().color(QPalette::Highlight), penWidth));

		painter->drawRect(rect);
	}
}

// ##############################################################################
// ##################  Serialization/Deserialization  ###########################
// ##############################################################################
//! Save as XML
void Image::save(QXmlStreamWriter* writer) const {
	Q_D(const Image);

	writer->writeStartElement(QStringLiteral("image"));
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	// general
	writer->writeStartElement(QStringLiteral("general"));
	if (d->embedded) {
		QFileInfo fi(d->fileName);
		writer->writeAttribute(QStringLiteral("fileName"), fi.fileName()); // save the actual file name only and not the whole path
	} else
		writer->writeAttribute(QStringLiteral("fileName"), d->fileName);

	writer->writeAttribute(QStringLiteral("embedded"), QString::number(d->embedded));
	writer->writeAttribute(QStringLiteral("opacity"), QString::number(d->opacity));
	writer->writeEndElement();

	// image data
	if (d->embedded && !d->image.isNull()) {
		writer->writeStartElement(QStringLiteral("data"));
		QByteArray data;
		QBuffer buffer(&data);
		buffer.open(QIODevice::WriteOnly);
		d->image.save(&buffer, "PNG");
		writer->writeCharacters(QLatin1String(data.toBase64()));
		writer->writeEndElement();
	}

	// geometry
	writer->writeStartElement(QStringLiteral("geometry"));
	WorksheetElement::save(writer);
	writer->writeAttribute(QStringLiteral("width"), QString::number(d->width));
	writer->writeAttribute(QStringLiteral("height"), QString::number(d->height));
	writer->writeAttribute(QStringLiteral("keepRatio"), QString::number(d->keepRatio));
	writer->writeEndElement();

	// border
	d->borderLine->save(writer);

	writer->writeEndElement(); // close "image" section
}

//! Load from XML
bool Image::load(XmlStreamReader* reader, bool preview) {
	if (!readBasicAttributes(reader))
		return false;

	Q_D(Image);
	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == QLatin1String("image"))
			break;

		if (!reader->isStartElement())
			continue;

		if (!preview && reader->name() == QLatin1String("comment")) {
			if (!readCommentElement(reader))
				return false;
		} else if (!preview && reader->name() == QLatin1String("general")) {
			attribs = reader->attributes();
			d->fileName = attribs.value(QStringLiteral("fileName")).toString();
			READ_INT_VALUE("embedded", embedded, bool);
			READ_DOUBLE_VALUE("opacity", opacity);
		} else if (reader->name() == QLatin1String("data")) {
			QByteArray ba = QByteArray::fromBase64(reader->readElementText().toLatin1());
			if (!d->image.loadFromData(ba))
				reader->raiseWarning(i18n("Failed to read image data"));
		} else if (!preview && reader->name() == QLatin1String("geometry")) {
			attribs = reader->attributes();

			READ_INT_VALUE("width", width, int);
			READ_INT_VALUE("height", height, int);
			READ_INT_VALUE("keepRatio", keepRatio, bool);

			WorksheetElement::load(reader, preview);
		} else if (!preview && reader->name() == QLatin1String("border")) {
			d->borderLine->load(reader, preview);
		} else { // unknown element
			reader->raiseUnknownElementWarning();
			if (!reader->skipToEndElement())
				return false;
		}
	}

	if (!preview) {
		if (!d->embedded)
			d->image = QImage(d->fileName);
		d->imageScaled = d->image.scaled(d->width, d->height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	}

	return true;
}

// ##############################################################################
// #########################  Theme management ##################################
// ##############################################################################
void Image::loadThemeConfig(const KConfig& config) {
	Q_D(Image);
	const auto& group = config.group(QStringLiteral("CartesianPlot"));
	d->borderLine->loadThemeConfig(group);
}

void Image::saveThemeConfig(const KConfig& config) {
	Q_D(Image);
	KConfigGroup group = config.group(QStringLiteral("Image"));
	d->borderLine->saveThemeConfig(group);
}
