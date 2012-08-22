/***************************************************************************
    File                 : TextLabel.cpp
    Project              : LabPlot/SciDAVis
    Description          : A one-line text label supporting floating point font sizes.
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs*gmx.net)
    Copyright            : (C) 2012 Alexander Semke (alexander.semke*web.de)
                           (replace * with @ in the email addresses) 
                           
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

#include "TextLabel.h"
#include "Worksheet.h"
#include "TextLabelPrivate.h"
#include "../lib/commandtemplates.h"
#include "lib/XmlStreamReader.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QPainter>
 #include <QGraphicsScene>
#include <QDebug>

#ifndef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
#include <KIcon>
#endif

/**
 * \class TextLabel
 * \brief A label supporting rendering of html- and tex-formated textes.
 * 
 * The label is aligned relative to the specified position. The position can be either specified by providing the x- and y- coordinates 
 * in parent's coordinate system, or by specifing one of the predefined position flags (\ca HorizontalPosition, \ca VerticalPosition).
 */


TextLabel::TextLabel(const QString &name):AbstractWorksheetElement(name), d_ptr(new TextLabelPrivate(this)){
	init();
}

TextLabel::TextLabel(const QString &name, TextLabelPrivate *dd):AbstractWorksheetElement(name), d_ptr(dd) {
	init();
}

void TextLabel::init() {
	Q_D(TextLabel);

	graphicsItem()->setFlag(QGraphicsItem::ItemIsSelectable);
	graphicsItem()->setFlag(QGraphicsItem::ItemIsMovable);
	graphicsItem()->setFlag(QGraphicsItem::ItemSendsGeometryChanges);

	d->teXUsed = false;
	d->teXFontSize = 12;
	d->teXFontColor = Qt::black;
	d->staticText.setTextFormat(Qt::RichText);

	d->horizontalPosition = TextLabel::hPositionCustom;
	d->verticalPosition = TextLabel::vPositionCustom;
	d->position.setX( Worksheet::convertToSceneUnits(1, Worksheet::Centimeter) );
	d->position.setY( Worksheet::convertToSceneUnits(1, Worksheet::Centimeter) );
	
	d->horizontalAlignment= TextLabel::hAlignCenter;
	d->verticalAlignment= TextLabel::vAlignCenter;

	d->rotationAngle = 0.0;
	
	//scaling:
	//we need to scale from the font size specified in points to scene units.
	//furhermore, we create the tex-image in a higher resolution then usual desktop resolution
	// -> take this into account
	d->scaleFactor = Worksheet::convertToSceneUnits(1, Worksheet::Point);
	d->teXImageResolution = 300;
	d->teXImageScaleFactor = QApplication::desktop()->physicalDpiX()/float(d->teXImageResolution)*d->scaleFactor;
	
	d->cancelItemChangeEvent = false;
}

TextLabel::~TextLabel() {
// 	delete d_ptr;
}

QGraphicsItem* TextLabel::graphicsItem() const{
	return d_ptr;
}

void TextLabel::retransform(){
	Q_D(TextLabel);
	d->retransform();
}

void TextLabel::handlePageResize(double horizontalRatio, double verticalRatio){
	Q_D(TextLabel);
	d->scaleFactor = Worksheet::convertToSceneUnits(1, Worksheet::Point);
}

/*!
	Returns an icon to be used in the project explorer.
*/
QIcon TextLabel::icon() const{
#ifndef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
	return  KIcon("draw-text");
#endif
}

/* ============================ getter methods ================= */
CLASS_SHARED_D_READER_IMPL(TextLabel, QString, text, text)
CLASS_SHARED_D_READER_IMPL(TextLabel, bool, teXUsed, teXUsed);
CLASS_SHARED_D_READER_IMPL(TextLabel, qreal, teXFontSize, teXFontSize);
CLASS_SHARED_D_READER_IMPL(TextLabel, QColor, teXFontColor, teXFontColor);
BASIC_SHARED_D_READER_IMPL(TextLabel, TextLabel::HorizontalPosition, horizontalPosition, horizontalPosition);
BASIC_SHARED_D_READER_IMPL(TextLabel, TextLabel::VerticalPosition, verticalPosition, verticalPosition);
CLASS_SHARED_D_READER_IMPL(TextLabel, QPointF, position, position);
BASIC_SHARED_D_READER_IMPL(TextLabel, TextLabel::HorizontalAlignment, horizontalAlignment, horizontalAlignment);
BASIC_SHARED_D_READER_IMPL(TextLabel, TextLabel::VerticalAlignment, verticalAlignment, verticalAlignment);
BASIC_SHARED_D_READER_IMPL(TextLabel, float, rotationAngle, rotationAngle);

/* ============================ setter methods and undo commands ================= */
STD_SETTER_CMD_IMPL_F(TextLabel, SetText, QString, text, updateText);
void TextLabel::setText(const QString &text) {
	Q_D(TextLabel);
	if (text != d->text)
		exec(new TextLabelSetTextCmd(d, text, tr("%1: set label text")));
}

STD_SETTER_CMD_IMPL_F(TextLabel, SetTeXUsed, bool, teXUsed, updateTeXImage);
void TextLabel::setTeXUsed(const bool tex) {
	Q_D(TextLabel);
	if (tex != d->teXUsed)
		exec(new TextLabelSetTeXUsedCmd(d, tex, tr("%1: use TeX syntax")));
}

STD_SETTER_CMD_IMPL_F(TextLabel, SetTeXFontSize, qreal, teXFontSize, updateTeXImage);
void TextLabel::setTeXFontSize(const qreal fontSize) {
	Q_D(TextLabel);
	if (fontSize != d->teXFontSize)
		exec(new TextLabelSetTeXFontSizeCmd(d, fontSize, tr("%1: set TeX font size")));
}

STD_SETTER_CMD_IMPL_F(TextLabel, SetTeXFontColor, QColor, teXFontColor, updateTeXImage);
void TextLabel::setTeXFontColor(const QColor fontColor) {
	Q_D(TextLabel);
	if (fontColor != d->teXFontColor)
		exec(new TextLabelSetTeXFontColorCmd(d, fontColor, tr("%1: set TeX font color")));
}

STD_SETTER_CMD_IMPL_F(TextLabel, SetPosition, QPointF, position, retransform);
void TextLabel::setPosition(const QPointF& pos) {
	Q_D(TextLabel);
	if (pos != d->position)
		exec(new TextLabelSetPositionCmd(d, pos, tr("%1: set position")));
}

STD_SETTER_CMD_IMPL_F(TextLabel, SetHorizontalPosition, TextLabel::HorizontalPosition, horizontalPosition, retransform);
void TextLabel::setHorizontalPosition(const TextLabel::HorizontalPosition hPos){
	Q_D(TextLabel);
	if (hPos != d->horizontalPosition)
		exec(new TextLabelSetHorizontalPositionCmd(d, hPos, tr("%1: set horizontal position")));
}

STD_SETTER_CMD_IMPL_F(TextLabel, SetVerticalPosition, TextLabel::VerticalPosition, verticalPosition, retransform);
void TextLabel::setVerticalPosition(const TextLabel::VerticalPosition vPos){
	Q_D(TextLabel);
	if (vPos != d->verticalPosition)
		exec(new TextLabelSetVerticalPositionCmd(d, vPos, tr("%1: set vertical position")));
}

STD_SETTER_CMD_IMPL_F(TextLabel, SetRotationAngle, float, rotationAngle, recalcShapeAndBoundingRect);
void TextLabel::setRotationAngle(float angle) {
	Q_D(TextLabel);
	if (angle != d->rotationAngle)
		exec(new TextLabelSetRotationAngleCmd(d, angle, tr("%1: set rotation angle")));
}

STD_SETTER_CMD_IMPL_F(TextLabel, SetHorizontalAlignment, TextLabel::HorizontalAlignment, horizontalAlignment, retransform);
void TextLabel::setHorizontalAlignment(const TextLabel::HorizontalAlignment hAlign){
	Q_D(TextLabel);
	if (hAlign != d->horizontalAlignment)
		exec(new TextLabelSetHorizontalAlignmentCmd(d, hAlign, tr("%1: set horizontal alignment")));
}

STD_SETTER_CMD_IMPL_F(TextLabel, SetVerticalAlignment, TextLabel::VerticalAlignment, verticalAlignment, retransform);
void TextLabel::setVerticalAlignment(const TextLabel::VerticalAlignment vAlign){
	Q_D(TextLabel);
	if (vAlign != d->verticalAlignment)
		exec(new TextLabelSetVerticalAlignmentCmd(d, vAlign, tr("%1: set vertical alignment")));
}

STD_SWAP_METHOD_SETTER_CMD_IMPL(TextLabel, SetVisible, bool, swapVisible);
void TextLabel::setVisible(bool on) {
	Q_D(TextLabel);
	exec(new TextLabelSetVisibleCmd(d, on, on ? tr("%1: set visible") : tr("%1: set invisible")));
}

bool TextLabel::isVisible() const {
	Q_D(const TextLabel);
	return d->isVisible();
}

void TextLabel::updateTeXImage(){
	Q_D(TextLabel);
	d->updateTeXImage();
}

//################################################################
//################### Private implementation ##########################
//################################################################
TextLabelPrivate::TextLabelPrivate(TextLabel *owner) : q(owner){
}

QString TextLabelPrivate::name() const{
	return q->name();
}

/*!
	calculates the position and the bounding box of the label. Called on geometry or text changes.
 */
void TextLabelPrivate::retransform(){
	if (horizontalPosition != TextLabel::hPositionCustom || verticalPosition != TextLabel::vPositionCustom)
		updatePosition();
	
	float x = position.x();
	float y = position.y();

	//determine the size of the label in scene units.
	float w, h;
	if (teXUsed){
		w = teXImage.width()*teXImageScaleFactor;
		h = teXImage.height()*teXImageScaleFactor;
	}
	else {
		w = staticText.size().width()*scaleFactor;
		h = staticText.size().height()*scaleFactor;
	}

	//depending on the alignment, calculate the new GraphicsItem's position in parent's coordinate system
	QPointF itemPos;
	switch (horizontalAlignment) {
		case TextLabel::hAlignLeft:
			itemPos.setX( x - w/2 );
			break;
		case TextLabel::hAlignCenter:
			itemPos.setX( x );
			break;
		case TextLabel::hAlignRight:
			itemPos.setX( x +w/2);
			break;
	}

	switch (verticalAlignment) {
		case TextLabel::vAlignTop:
			itemPos.setY( y - h/2 );
			break;
		case TextLabel::vAlignCenter:
			itemPos.setY( y );
			break;
		case TextLabel::vAlignBottom:
			itemPos.setY( y + h/2 );
			break;
	}

	cancelItemChangeEvent=true;
	setPos(itemPos);
	cancelItemChangeEvent=false;

	boundingRectangle.setX(-w/2);
	boundingRectangle.setY(-h/2);
	boundingRectangle.setWidth(w);
	boundingRectangle.setHeight(h);

	recalcShapeAndBoundingRect();

	emit(q->changed());
}

/*!
	calculates the position of the label, when the position relative to the parent was specified (left, right, etc.)
*/
void TextLabelPrivate::updatePosition(){
	//determine the parent item
	QRectF parentRect;
	QGraphicsItem* parent = parentItem();
	if (parent){
		parentRect = parent->boundingRect();
	}else{
		if (!scene())
			return;

		parentRect = scene()->sceneRect();
	}

	if (horizontalPosition != TextLabel::hPositionCustom){
		if (horizontalPosition == TextLabel::hPositionLeft)
			position.setX( parentRect.x() );
		else if (horizontalPosition == TextLabel::hPositionCenter)
			position.setX( parentRect.x() + parentRect.width()/2 );
		else if (horizontalPosition == TextLabel::hPositionRight)
			position.setX( parentRect.x() + parentRect.width() );
	}

	if (verticalPosition != TextLabel::vPositionCustom){
		if (verticalPosition == TextLabel::vPositionTop)
			position.setY( parentRect.y() );
		else if (verticalPosition == TextLabel::vPositionCenter)
			position.setY( parentRect.y() + parentRect.height()/2 );
		else if (verticalPosition == TextLabel::vPositionBottom)
			position.setY( parentRect.y() + parentRect.height() );
	}
}

/*!
	updates the static text.
 */
void TextLabelPrivate::updateText(){
	staticText.setText(text);
	
	//the size of the label was most probably changed.
	//call retransform() to recalculate the position and the bounding box of the label
	retransform();
}

void TextLabelPrivate::updateTeXImage(){
	bool status = TeXRenderer::renderImageLaTeX(text, teXImage, teXFontColor, teXFontSize, teXImageResolution);
	if (!status)
		qDebug()<<"TeX image not created";

	//the size of the tex image was most probably changed.
	//call retransform() to recalculate the position and the bounding box of the label
	retransform();
}

bool TextLabelPrivate::swapVisible(bool on){
	bool oldValue = isVisible();
	setVisible(on);
	emit(q->changed());
	return oldValue;
}

/*!
	Returns the outer bounds of the item as a rectangle.
 */
QRectF TextLabelPrivate::boundingRect() const{
	return transformedBoundingRectangle;
}

/*!
	Returns the shape of this item as a QPainterPath in local coordinates.
*/
QPainterPath TextLabelPrivate::shape() const{
	return labelShape;
}

/*!
  recalculates the outer bounds and the shape of the label.
*/
void TextLabelPrivate::recalcShapeAndBoundingRect(){
	prepareGeometryChange();

	QMatrix matrix;
	matrix.rotate(rotationAngle);
	transformedBoundingRectangle = matrix.mapRect(boundingRectangle);

	labelShape = QPainterPath();
	labelShape.addRect(boundingRectangle);
	labelShape = matrix.map(labelShape);
}

void TextLabelPrivate::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget * widget){
	Q_UNUSED(option)
	Q_UNUSED(widget)

	//draw the selection box before scaling since it already has the proper size.
	if (isSelected()){
		painter->setPen(QPen(Qt::blue, 0, Qt::DashLine));
		painter->drawPath(labelShape);
	}
	
	painter->rotate(rotationAngle);

	if (teXUsed){
		painter->scale(teXImageScaleFactor, teXImageScaleFactor);
		float w = teXImage.width();
		float h = teXImage.height();
		painter->translate(-w/2,-h/2);
		painter->setRenderHint(QPainter::SmoothPixmapTransform);
		QRectF rect = teXImage.rect();
		painter->drawImage(rect, teXImage, rect);
	}else{
		painter->scale(scaleFactor, scaleFactor);
		float w = staticText.size().width();
		float h = staticText.size().height();
 		painter->drawStaticText(QPoint(-w/2,-h/2), staticText);
	}
}

QVariant TextLabelPrivate::itemChange(GraphicsItemChange change, const QVariant &value){
	if (cancelItemChangeEvent)
		return value;
	
	if (change == QGraphicsItem::ItemPositionChange) {
		QPointF itemPos = value.toPointF();//item's center point in parent's coordinates
		float x = itemPos.x();
		float y = itemPos.y();
		float w, h;
		if (teXUsed){
			w = teXImage.width()*scaleFactor;
			h = teXImage.height()*scaleFactor;
		}
		else {
			w = staticText.size().width()*scaleFactor;
			h = staticText.size().height()*scaleFactor;
		}

		//depending on the alignment, calculate the new position
		switch (horizontalAlignment) {
			case TextLabel::hAlignLeft:
				position.setX( x + w/2 );
				break;
			case TextLabel::hAlignCenter:
				position.setX( x );
				break;
			case TextLabel::hAlignRight:
				position.setX( x - w/2 );
				break;
		}

		switch (verticalAlignment) {
			case TextLabel::vAlignTop:
				position.setY( y + h/2 );
				break;
			case TextLabel::vAlignCenter:
				position.setY( y );
				break;
			case TextLabel::vAlignBottom:
				position.setY( y - h/2 );
				break;
		}

		//item was moved -> change the position flag to "custom"
		horizontalPosition = TextLabel::hPositionCustom;
		verticalPosition = TextLabel::vPositionCustom;

		 emit q->positionChanged(position);
     }

	return QGraphicsItem::itemChange(change, value);
 }
 
//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
//! Save as XML
void TextLabel::save(QXmlStreamWriter* writer) const{
	Q_D(const TextLabel);

    writer->writeStartElement( "textLabel" );
    writeBasicAttributes(writer);
    writeCommentElement(writer);

	//geometry
    writer->writeStartElement( "geometry" );
    writer->writeAttribute( "x", QString::number(d->position.x()) );
    writer->writeAttribute( "y", QString::number(d->position.y()) );
    writer->writeAttribute( "horizontalPosition", QString::number(d->horizontalPosition) );
	writer->writeAttribute( "verticalPosition", QString::number(d->verticalPosition) );
	writer->writeAttribute( "horizontalAlignment", QString::number(d->horizontalAlignment) );
	writer->writeAttribute( "verticalAlignment", QString::number(d->verticalAlignment) );
	writer->writeAttribute( "rotationAngle", QString::number(d->rotationAngle) );
    writer->writeEndElement();
	
	writer->writeStartElement( "text" );
	writer->writeCharacters( d->text );
	writer->writeEndElement();
	
	writer->writeStartElement( "format" );
    writer->writeAttribute( "teXUsed", QString::number(d->teXUsed) );
	writer->writeAttribute( "teXFontSize", QString::number(d->teXFontSize) );
	writer->writeAttribute( "teXFontColor_r", QString::number(d->teXFontColor.red()) );
	writer->writeAttribute( "teXFontColor_g", QString::number(d->teXFontColor.green()) );
	writer->writeAttribute( "teXFontColor_b", QString::number(d->teXFontColor.blue()) );
	writer->writeEndElement();
	
    writer->writeEndElement(); // close "textLabel" section
}

//! Load from XML
bool TextLabel::load(XmlStreamReader* reader){
	Q_D(TextLabel);

    if(!reader->isStartElement() || reader->name() != "textLabel"){
        reader->raiseError(tr("no textLabel element found"));
        return false;
    }

    if (!readBasicAttributes(reader))
        return false;

    QString attributeWarning = tr("Attribute '%1' missing or empty, default value is used");
    QXmlStreamAttributes attribs;
    QString str;
    QRectF rect;

    while (!reader->atEnd()){
        reader->readNext();
        if (reader->isEndElement() && reader->name() == "textLabel")
            break;

        if (!reader->isStartElement())
            continue;

        if (reader->name() == "comment"){
            if (!readCommentElement(reader)) return false;
		}else if (reader->name() == "geometry"){
            attribs = reader->attributes();

            str = attribs.value("x").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'x'"));
            else
                d->position.setX(str.toDouble());

            str = attribs.value("y").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'y'"));
            else
                d->position.setY(str.toDouble());

            str = attribs.value("horizontalPosition").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'horizontalPosition'"));
            else
                d->horizontalPosition = (TextLabel::HorizontalPosition)str.toInt();

            str = attribs.value("verticalPosition").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'verticalPosition'"));
            else
                d->verticalPosition = (TextLabel::VerticalPosition)str.toInt();

			str = attribs.value("horizontalAlignment").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'horizontalAlignment'"));
            else
                d->horizontalAlignment = (TextLabel::HorizontalAlignment)str.toInt();

            str = attribs.value("verticalAlignment").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'verticalAlignment'"));
            else
                d->verticalAlignment = (TextLabel::VerticalAlignment)str.toInt();
		}else if (reader->name() == "text"){
			d->text = reader->readElementText();
		}else if (reader->name() == "format"){
			attribs = reader->attributes();

			str = attribs.value("teXUsed").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'teXUsed'"));
            else
                d->teXUsed = str.toInt();
			
			str = attribs.value("teXFontSize").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'teXFontSize'"));
            else
                d->teXFontSize = str.toInt();
			
			str = attribs.value("teXFontColor_r").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'teXFontColor_r'"));
            else
                d->teXFontColor.setRed( str.toInt() );

			str = attribs.value("teXFontColor_g").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'teXFontColor_g'"));
            else
                d->teXFontColor.setGreen( str.toInt() );
			
			str = attribs.value("teXFontColor_b").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'teXFontColor_b'"));
            else
                d->teXFontColor.setBlue( str.toInt() );			
        }else{ // unknown element
            reader->raiseWarning(tr("unknown element '%1'").arg(reader->name().toString()));
            if (!reader->skipToEndElement()) return false;
        }
    }

	//TODO we need to call retransform() or updateText() here, but these functions don't properly behave yet.
    return true;
}
