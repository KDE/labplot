/***************************************************************************
    File                 : PlotArea.cpp
    Project              : LabPlot
    Description          : Plot area (for background filling and clipping).
    --------------------------------------------------------------------
    Copyright            : (C) 2011-2020 by Alexander Semke (alexander.semke@web.de)
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

#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/PlotArea.h"
#include "backend/worksheet/plots/PlotAreaPrivate.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/macros.h"

#include <QPainter>
#include <QPalette>
#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>

/**
 * \class PlotArea
 * \brief Plot area (for background filling and clipping).
 *
 * \ingroup worksheet
 */

PlotArea::PlotArea(const QString &name, CartesianPlot* parent) : WorksheetElement(name, AspectType::PlotArea),
	d_ptr(new PlotAreaPrivate(this)),
	m_parent(parent) {

	init();
}

PlotArea::PlotArea(const QString &name, CartesianPlot* parent, PlotAreaPrivate *dd)
	: WorksheetElement(name, AspectType::PlotArea),
	  d_ptr(dd),
	  m_parent(parent) {

	init();
}

//no need to delete the d-pointer here - it inherits from QGraphicsItem
//and is deleted during the cleanup in QGraphicsScene
PlotArea::~PlotArea() = default;

void PlotArea::init() {
	Q_D(PlotArea);

	setHidden(true);//we don't show PlotArea aspect in the model view.
	d->rect = QRectF(0, 0, 1, 1);
	d->setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);

	KConfig config;
	KConfigGroup group = config.group("PlotArea");

	//Background
	d->backgroundType = (PlotArea::BackgroundType) group.readEntry("BackgroundType", static_cast<int>(BackgroundType::Color));
	d->backgroundColorStyle = (PlotArea::BackgroundColorStyle) group.readEntry("BackgroundColorStyle", static_cast<int>(BackgroundColorStyle::SingleColor));
	d->backgroundImageStyle = (PlotArea::BackgroundImageStyle) group.readEntry("BackgroundImageStyle", static_cast<int>(BackgroundImageStyle::Scaled));
	d->backgroundBrushStyle = (Qt::BrushStyle) group.readEntry("BackgroundBrushStyle", static_cast<int>(Qt::SolidPattern));
	d->backgroundFileName = group.readEntry("BackgroundFileName", QString());
	d->backgroundFirstColor = group.readEntry("BackgroundFirstColor", QColor(Qt::white));
	d->backgroundSecondColor = group.readEntry("BackgroundSecondColor", QColor(Qt::black));
	d->backgroundOpacity = group.readEntry("BackgroundOpacity", 1.0);

	//Border
	d->borderPen = QPen(group.readEntry("BorderColor", QColor(Qt::black)),
	                    group.readEntry("BorderWidth", Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Point)),
	                    (Qt::PenStyle) group.readEntry("BorderStyle", (int)Qt::SolidLine));
	d->borderCornerRadius = group.readEntry("BorderCornerRadius", 0.0);
	d->borderOpacity = group.readEntry("BorderOpacity", 1.0);
}

QGraphicsItem *PlotArea::graphicsItem() const {
	return d_ptr;
}

STD_SWAP_METHOD_SETTER_CMD_IMPL(PlotArea, SetVisible, bool, swapVisible)
void PlotArea::setVisible(bool on) {
	Q_D(PlotArea);
	exec(new PlotAreaSetVisibleCmd(d, on, on ? ki18n("%1: set visible") : ki18n("%1: set invisible")));
}

bool PlotArea::isVisible() const {
	Q_D(const PlotArea);
	return d->isVisible();
}

bool PlotArea::isHovered() const {
	return m_parent->isHovered();
}

bool PlotArea::isSelected() const {
	return m_parent->isSelected();
}

void PlotArea::handleResize(double horizontalRatio, double verticalRatio, bool pageResize) {
	DEBUG("PlotArea::handleResize()");
	Q_D(PlotArea);
	Q_UNUSED(pageResize);

	d->rect.setWidth(d->rect.width()*horizontalRatio);
	d->rect.setHeight(d->rect.height()*verticalRatio);

	// TODO: scale line width
}

void PlotArea::retransform() {
}

/* ============================ getter methods ================= */
BASIC_SHARED_D_READER_IMPL(PlotArea, bool, clippingEnabled, clippingEnabled())
BASIC_SHARED_D_READER_IMPL(PlotArea, QRectF, rect, rect)

BASIC_SHARED_D_READER_IMPL(PlotArea, PlotArea::BackgroundType, backgroundType, backgroundType)
BASIC_SHARED_D_READER_IMPL(PlotArea, PlotArea::BackgroundColorStyle, backgroundColorStyle, backgroundColorStyle)
BASIC_SHARED_D_READER_IMPL(PlotArea, PlotArea::BackgroundImageStyle, backgroundImageStyle, backgroundImageStyle)
BASIC_SHARED_D_READER_IMPL(PlotArea, Qt::BrushStyle, backgroundBrushStyle, backgroundBrushStyle)
BASIC_SHARED_D_READER_IMPL(PlotArea, QColor, backgroundFirstColor, backgroundFirstColor)
BASIC_SHARED_D_READER_IMPL(PlotArea, QColor, backgroundSecondColor, backgroundSecondColor)
BASIC_SHARED_D_READER_IMPL(PlotArea, QString, backgroundFileName, backgroundFileName)
BASIC_SHARED_D_READER_IMPL(PlotArea, qreal, backgroundOpacity, backgroundOpacity)

BASIC_SHARED_D_READER_IMPL(PlotArea, QPen, borderPen, borderPen)
BASIC_SHARED_D_READER_IMPL(PlotArea, qreal, borderCornerRadius, borderCornerRadius)
BASIC_SHARED_D_READER_IMPL(PlotArea, qreal, borderOpacity, borderOpacity)


/* ============================ setter methods and undo commands ================= */

STD_SWAP_METHOD_SETTER_CMD_IMPL(PlotArea, SetClippingEnabled, bool, toggleClipping);
void PlotArea::setClippingEnabled(bool on) {
	Q_D(PlotArea);

	if (d->clippingEnabled() != on)
		exec(new PlotAreaSetClippingEnabledCmd(d, on, ki18n("%1: toggle clipping")));
}

/*!
 * sets plot area rect in scene coordinates.
 */
void PlotArea::setRect(const QRectF &newRect) {
	Q_D(PlotArea);
	d->setRect(newRect);
}

//Background
STD_SETTER_CMD_IMPL_F_S(PlotArea, SetBackgroundType, PlotArea::BackgroundType, backgroundType, update)
void PlotArea::setBackgroundType(BackgroundType type) {
	Q_D(PlotArea);
	if (type != d->backgroundType)
		exec(new PlotAreaSetBackgroundTypeCmd(d, type, ki18n("%1: background type changed")));
}

STD_SETTER_CMD_IMPL_F_S(PlotArea, SetBackgroundColorStyle, PlotArea::BackgroundColorStyle, backgroundColorStyle, update)
void PlotArea::setBackgroundColorStyle(BackgroundColorStyle style) {
	Q_D(PlotArea);
	if (style != d->backgroundColorStyle)
		exec(new PlotAreaSetBackgroundColorStyleCmd(d, style, ki18n("%1: background color style changed")));
}

STD_SETTER_CMD_IMPL_F_S(PlotArea, SetBackgroundImageStyle, PlotArea::BackgroundImageStyle, backgroundImageStyle, update)
void PlotArea::setBackgroundImageStyle(PlotArea::BackgroundImageStyle style) {
	Q_D(PlotArea);
	if (style != d->backgroundImageStyle)
		exec(new PlotAreaSetBackgroundImageStyleCmd(d, style, ki18n("%1: background image style changed")));
}

STD_SETTER_CMD_IMPL_F_S(PlotArea, SetBackgroundBrushStyle, Qt::BrushStyle, backgroundBrushStyle, update)
void PlotArea::setBackgroundBrushStyle(Qt::BrushStyle style) {
	Q_D(PlotArea);
	if (style != d->backgroundBrushStyle)
		exec(new PlotAreaSetBackgroundBrushStyleCmd(d, style, ki18n("%1: background brush style changed")));
}

STD_SETTER_CMD_IMPL_F_S(PlotArea, SetBackgroundFirstColor, QColor, backgroundFirstColor, update)
void PlotArea::setBackgroundFirstColor(const QColor &color) {
	Q_D(PlotArea);
	if (color != d->backgroundFirstColor)
		exec(new PlotAreaSetBackgroundFirstColorCmd(d, color, ki18n("%1: set background first color")));
}

STD_SETTER_CMD_IMPL_F_S(PlotArea, SetBackgroundSecondColor, QColor, backgroundSecondColor, update)
void PlotArea::setBackgroundSecondColor(const QColor &color) {
	Q_D(PlotArea);
	if (color != d->backgroundSecondColor)
		exec(new PlotAreaSetBackgroundSecondColorCmd(d, color, ki18n("%1: set background second color")));
}

STD_SETTER_CMD_IMPL_F_S(PlotArea, SetBackgroundFileName, QString, backgroundFileName, update)
void PlotArea::setBackgroundFileName(const QString& fileName) {
	Q_D(PlotArea);
	if (fileName != d->backgroundFileName)
		exec(new PlotAreaSetBackgroundFileNameCmd(d, fileName, ki18n("%1: set background image")));
}

STD_SETTER_CMD_IMPL_F_S(PlotArea, SetBackgroundOpacity, qreal, backgroundOpacity, update)
void PlotArea::setBackgroundOpacity(qreal opacity) {
	Q_D(PlotArea);
	if (opacity != d->backgroundOpacity)
		exec(new PlotAreaSetBackgroundOpacityCmd(d, opacity, ki18n("%1: set plot area opacity")));
}

//Border
STD_SETTER_CMD_IMPL_F_S(PlotArea, SetBorderPen, QPen, borderPen, update)
void PlotArea::setBorderPen(const QPen &pen) {
	Q_D(PlotArea);
	if (pen != d->borderPen)
		exec(new PlotAreaSetBorderPenCmd(d, pen, ki18n("%1: set plot area border")));
}

STD_SETTER_CMD_IMPL_F_S(PlotArea, SetBorderCornerRadius, qreal, borderCornerRadius, update)
void PlotArea::setBorderCornerRadius(qreal radius) {
	Q_D(PlotArea);
	if (radius != d->borderCornerRadius)
		exec(new PlotAreaSetBorderCornerRadiusCmd(d, radius, ki18n("%1: set plot area corner radius")));
}

STD_SETTER_CMD_IMPL_F_S(PlotArea, SetBorderOpacity, qreal, borderOpacity, update)
void PlotArea::setBorderOpacity(qreal opacity) {
	Q_D(PlotArea);
	if (opacity != d->borderOpacity)
		exec(new PlotAreaSetBorderOpacityCmd(d, opacity, ki18n("%1: set plot area border opacity")));
}

//#####################################################################
//################### Private implementation ##########################
//#####################################################################
PlotAreaPrivate::PlotAreaPrivate(PlotArea *owner) : q(owner) {
}

QString PlotAreaPrivate::name() const {
	return q->name();
}

bool PlotAreaPrivate::clippingEnabled() const {
	return (flags() & QGraphicsItem::ItemClipsChildrenToShape);
}

bool PlotAreaPrivate::toggleClipping(bool on) {
	bool oldValue = clippingEnabled();
	setFlag(QGraphicsItem::ItemClipsChildrenToShape, on);
	return oldValue;
}

bool PlotAreaPrivate::swapVisible(bool on) {
	bool oldValue = isVisible();
	setVisible(on);
	return oldValue;
}

void PlotAreaPrivate::setRect(const QRectF& r) {
	prepareGeometryChange();
	rect = mapRectFromScene(r);
}

QRectF PlotAreaPrivate::boundingRect () const {
	if (borderPen.style() != Qt::NoPen){
		const qreal width = rect.width();
		const qreal height = rect.height();
		const double penWidth = borderPen.width();
		return QRectF{-width/2 - penWidth/2, -height/2 - penWidth/2,
					width + penWidth, height + penWidth};
	} else
		return rect;

}

QPainterPath PlotAreaPrivate::shape() const {
	QPainterPath path;
	if (qFuzzyIsNull(borderCornerRadius))
		path.addRect(rect);
	else
		path.addRoundedRect(rect, borderCornerRadius, borderCornerRadius);

	return path;
}

void PlotAreaPrivate::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget *widget) {
// 	DEBUG("PlotAreaPrivate::paint()");
	Q_UNUSED(option)
	Q_UNUSED(widget)

	if (!isVisible())
		return;

	//draw the area
	painter->setOpacity(backgroundOpacity);
	painter->setPen(Qt::NoPen);
	if (backgroundType == PlotArea::BackgroundType::Color) {
		switch (backgroundColorStyle) {
		case PlotArea::BackgroundColorStyle::SingleColor: {
			painter->setBrush(QBrush(backgroundFirstColor));
			break;
		}
		case PlotArea::BackgroundColorStyle::HorizontalLinearGradient: {
			QLinearGradient linearGrad(rect.topLeft(), rect.topRight());
			linearGrad.setColorAt(0, backgroundFirstColor);
			linearGrad.setColorAt(1, backgroundSecondColor);
			painter->setBrush(QBrush(linearGrad));
			break;
		}
		case PlotArea::BackgroundColorStyle::VerticalLinearGradient: {
			QLinearGradient linearGrad(rect.topLeft(), rect.bottomLeft());
			linearGrad.setColorAt(0, backgroundFirstColor);
			linearGrad.setColorAt(1, backgroundSecondColor);
			painter->setBrush(QBrush(linearGrad));
			break;
		}
		case PlotArea::BackgroundColorStyle::TopLeftDiagonalLinearGradient: {
			QLinearGradient linearGrad(rect.topLeft(), rect.bottomRight());
			linearGrad.setColorAt(0, backgroundFirstColor);
			linearGrad.setColorAt(1, backgroundSecondColor);
			painter->setBrush(QBrush(linearGrad));
			break;
		}
		case PlotArea::BackgroundColorStyle::BottomLeftDiagonalLinearGradient: {
			QLinearGradient linearGrad(rect.bottomLeft(), rect.topRight());
			linearGrad.setColorAt(0, backgroundFirstColor);
			linearGrad.setColorAt(1, backgroundSecondColor);
			painter->setBrush(QBrush(linearGrad));
			break;
		}
		case PlotArea::BackgroundColorStyle::RadialGradient: {
			QRadialGradient radialGrad(rect.center(), rect.width()/2);
			radialGrad.setColorAt(0, backgroundFirstColor);
			radialGrad.setColorAt(1, backgroundSecondColor);
			painter->setBrush(QBrush(radialGrad));
			break;
		}
		}
	} else if (backgroundType == PlotArea::BackgroundType::Image) {
		if ( !backgroundFileName.trimmed().isEmpty() ) {
			QPixmap pix(backgroundFileName);
			switch (backgroundImageStyle) {
			case PlotArea::BackgroundImageStyle::ScaledCropped:
				pix = pix.scaled(rect.size().toSize(),Qt::KeepAspectRatioByExpanding,Qt::SmoothTransformation);
				painter->setBrush(QBrush(pix));
				painter->setBrushOrigin(pix.size().width()/2,pix.size().height()/2);
				painter->drawRoundedRect(rect, borderCornerRadius, borderCornerRadius);
				break;
			case PlotArea::BackgroundImageStyle::Scaled:
				pix = pix.scaled(rect.size().toSize(),Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
				painter->setBrush(QBrush(pix));
				painter->setBrushOrigin(pix.size().width()/2,pix.size().height()/2);
				painter->drawRoundedRect(rect, borderCornerRadius, borderCornerRadius);
				break;
			case PlotArea::BackgroundImageStyle::ScaledAspectRatio:
				pix = pix.scaled(rect.size().toSize(),Qt::KeepAspectRatio,Qt::SmoothTransformation);
				painter->setBrush(QBrush(pix));
				painter->setBrushOrigin(pix.size().width()/2,pix.size().height()/2);
				painter->drawRoundedRect(rect, borderCornerRadius, borderCornerRadius);
				break;
			case PlotArea::BackgroundImageStyle::Centered:
				painter->drawPixmap(QPointF(rect.center().x()-pix.size().width()/2,rect.center().y()-pix.size().height()/2),pix);
				break;
			case PlotArea::BackgroundImageStyle::Tiled:
				painter->setBrush(QBrush(pix));
				painter->drawRoundedRect(rect, borderCornerRadius, borderCornerRadius);
				break;
			case PlotArea::BackgroundImageStyle::CenterTiled:
				painter->setBrush(QBrush(pix));
				painter->setBrushOrigin(pix.size().width()/2,pix.size().height()/2);
				painter->drawRoundedRect(rect, borderCornerRadius, borderCornerRadius);
			}
		}
	} else if (backgroundType == PlotArea::BackgroundType::Pattern) {
		painter->setBrush(QBrush(backgroundFirstColor,backgroundBrushStyle));
	}

	if ( qFuzzyIsNull(borderCornerRadius) )
		painter->drawRect(rect);
	else
		painter->drawRoundedRect(rect, borderCornerRadius, borderCornerRadius);

	//draw the border
	if (borderPen.style() != Qt::NoPen && !q->isHovered() && !isSelected()) {
		painter->setPen(borderPen);
		painter->setBrush(Qt::NoBrush);
		painter->setOpacity(borderOpacity);
		if ( qFuzzyIsNull(borderCornerRadius) )
			painter->drawRect(rect);
		else
			painter->drawRoundedRect(rect, borderCornerRadius, borderCornerRadius);
	}

	double penWidth = 6.;
	QRectF rect = boundingRect();
	rect = QRectF(-rect.width()/2 + penWidth / 2, -rect.height()/2 + penWidth / 2,
				  rect.width() - penWidth, rect.height() - penWidth);

	if (q->isHovered() && !q->isSelected() && !q->isPrinting()) {
		painter->setPen(QPen(QApplication::palette().color(QPalette::Shadow), penWidth, Qt::SolidLine));
		painter->drawRect(rect);
	}

	if (q->isSelected() && !q->isPrinting()) {
		painter->setPen(QPen(QApplication::palette().color(QPalette::Highlight), penWidth, Qt::SolidLine));
		painter->drawRect(rect);
	}


// 	DEBUG("PlotAreaPrivate::paint() DONE");
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################

//! Save as XML
void PlotArea::save(QXmlStreamWriter* writer) const {
	Q_D(const PlotArea);

	writer->writeStartElement( "plotArea" );
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	writer->writeStartElement( "background" );
	writer->writeAttribute( "type", QString::number(static_cast<int>(d->backgroundType)) );
	writer->writeAttribute( "colorStyle", QString::number(static_cast<int>(d->backgroundColorStyle)) );
	writer->writeAttribute( "imageStyle", QString::number(static_cast<int>(d->backgroundImageStyle)) );
	writer->writeAttribute( "brushStyle", QString::number(d->backgroundBrushStyle) );
	writer->writeAttribute( "firstColor_r", QString::number(d->backgroundFirstColor.red()) );
	writer->writeAttribute( "firstColor_g", QString::number(d->backgroundFirstColor.green()) );
	writer->writeAttribute( "firstColor_b", QString::number(d->backgroundFirstColor.blue()) );
	writer->writeAttribute( "secondColor_r", QString::number(d->backgroundSecondColor.red()) );
	writer->writeAttribute( "secondColor_g", QString::number(d->backgroundSecondColor.green()) );
	writer->writeAttribute( "secondColor_b", QString::number(d->backgroundSecondColor.blue()) );
	writer->writeAttribute( "fileName", d->backgroundFileName );
	writer->writeAttribute( "opacity", QString::number(d->backgroundOpacity) );
	writer->writeEndElement();

	//border
	writer->writeStartElement( "border" );
	WRITE_QPEN(d->borderPen);
	writer->writeAttribute( "borderOpacity", QString::number(d->borderOpacity) );
	writer->writeAttribute( "borderCornerRadius", QString::number(d->borderCornerRadius) );
	writer->writeEndElement();

	writer->writeEndElement();
}

//! Load from XML
bool PlotArea::load(XmlStreamReader* reader, bool preview) {
	Q_D(PlotArea);

	if ( !readBasicAttributes(reader) )
		return false;

	KLocalizedString attributeWarning = ki18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs;
	QString str;

	while ( !reader->atEnd() ) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == "plotArea")
			break;

		if ( !reader->isStartElement() )
			continue;

		if (!preview && reader->name() == "comment") {
			if (!readCommentElement(reader)) return false;
		} else if (!preview && reader->name() == "background") {
			attribs = reader->attributes();

			str = attribs.value("type").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("type").toString());
			else
				d->backgroundType = PlotArea::BackgroundType(str.toInt());

			str = attribs.value("colorStyle").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("colorStyle").toString());
			else
				d->backgroundColorStyle = PlotArea::BackgroundColorStyle(str.toInt());

			str = attribs.value("imageStyle").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("imageStyle").toString());
			else
				d->backgroundImageStyle = PlotArea::BackgroundImageStyle(str.toInt());

			str = attribs.value("brushStyle").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("brushStyle").toString());
			else
				d->backgroundBrushStyle = Qt::BrushStyle(str.toInt());

			str = attribs.value("firstColor_r").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("firstColor_r").toString());
			else
				d->backgroundFirstColor.setRed(str.toInt());

			str = attribs.value("firstColor_g").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("firstColor_g").toString());
			else
				d->backgroundFirstColor.setGreen(str.toInt());

			str = attribs.value("firstColor_b").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("firstColor_b").toString());
			else
				d->backgroundFirstColor.setBlue(str.toInt());

			str = attribs.value("secondColor_r").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("secondColor_r").toString());
			else
				d->backgroundSecondColor.setRed(str.toInt());

			str = attribs.value("secondColor_g").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("secondColor_g").toString());
			else
				d->backgroundSecondColor.setGreen(str.toInt());

			str = attribs.value("secondColor_b").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("secondColor_b").toString());
			else
				d->backgroundSecondColor.setBlue(str.toInt());

			str = attribs.value("fileName").toString();
			d->backgroundFileName = str;

			str = attribs.value("opacity").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("opacity").toString());
			else
				d->backgroundOpacity = str.toDouble();
		} else if (!preview && reader->name() == "border") {
			attribs = reader->attributes();

			READ_QPEN(d->borderPen);

			str = attribs.value("borderOpacity").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("borderOpacity").toString());
			else
				d->borderOpacity = str.toDouble();

			str = attribs.value("borderCornerRadius").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("borderCornerRadius").toString());
			else
				d->borderCornerRadius = str.toDouble();
		} else { // unknown element
			reader->raiseWarning(i18n("unknown element '%1'", reader->name().toString()));
			if (!reader->skipToEndElement()) return false;
		}
	}

	return true;
}

void PlotArea::loadThemeConfig(const KConfig& config) {
	KConfigGroup group;
	if (config.hasGroup(QLatin1String("Theme")))
		group = config.group("CartesianPlot");
	else
		group = config.group("PlotArea");

	//background
	this->setBackgroundType((PlotArea::BackgroundType) group.readEntry("BackgroundType", static_cast<int>(PlotArea::BackgroundType::Color)));
	this->setBackgroundColorStyle((PlotArea::BackgroundColorStyle) group.readEntry("BackgroundColorStyle", static_cast<int>(PlotArea::BackgroundColorStyle::SingleColor)));
	this->setBackgroundImageStyle((PlotArea::BackgroundImageStyle) group.readEntry("BackgroundImageStyle", static_cast<int>(PlotArea::BackgroundImageStyle::Scaled)));
	this->setBackgroundBrushStyle((Qt::BrushStyle) group.readEntry("BackgroundBrushStyle", static_cast<int>(Qt::SolidPattern)));
	this->setBackgroundFirstColor(group.readEntry("BackgroundFirstColor", QColor(Qt::white)));
	this->setBackgroundSecondColor(group.readEntry("BackgroundSecondColor", QColor(Qt::black)));
	this->setBackgroundOpacity(group.readEntry("BackgroundOpacity", 1.0));

	//border
	QPen pen = QPen(group.readEntry("BorderColor", QColor(Qt::black)),
					group.readEntry("BorderWidth", Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Point)),
					(Qt::PenStyle)group.readEntry("BorderStyle", (int)Qt::SolidLine));
	this->setBorderPen(pen);
	this->setBorderCornerRadius(group.readEntry("BorderCornerRadius", 0.0));
	this->setBorderOpacity(group.readEntry("BorderOpacity", 1.0));
}

void PlotArea::saveThemeConfig(const KConfig& config) {
	KConfigGroup group = config.group("CartesianPlot");

	group.writeEntry("BackgroundBrushStyle",(int) this->backgroundBrushStyle());
	group.writeEntry("BackgroundColorStyle",(int) this->backgroundColorStyle());
	group.writeEntry("BackgroundFirstColor",(QColor) this->backgroundFirstColor());
	group.writeEntry("BackgroundImageStyle",(int) this->backgroundImageStyle());
	group.writeEntry("BackgroundOpacity", this->backgroundOpacity());
	group.writeEntry("BackgroundSecondColor",(QColor) this->backgroundSecondColor());
	group.writeEntry("BackgroundType",(int) this->backgroundType());

	group.writeEntry("BorderColor",(QColor) this->borderPen().color());
	group.writeEntry("BorderCornerRadius", this->borderCornerRadius());
	group.writeEntry("BorderOpacity", this->borderOpacity());
	group.writeEntry("BorderStyle", (int) this->borderPen().style());
	group.writeEntry("BorderWidth", this->borderPen().widthF());
}
