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

#include <QPainter>
 #include <QGraphicsScene>
#include <QDebug>

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

	d->texUsed = false;
	d->staticText.setTextFormat(Qt::RichText);

	d->horizontalPosition = TextLabel::hPositionCustom;
	d->verticalPosition = TextLabel::vPositionCustom;
	d->position.setX( Worksheet::convertToSceneUnits(1, Worksheet::Centimeter) );
	d->position.setY( Worksheet::convertToSceneUnits(1, Worksheet::Centimeter) );
	
	d->horizontalAlignment= TextLabel::hAlignCenter;
	d->verticalAlignment= TextLabel::vAlignCenter;

	d->rotationAngle = 0.0;
	d->scaleFactor = Worksheet::convertToSceneUnits(1, Worksheet::Point);
	
	d->cancelItemChangeEvent = false;
}

TextLabel::~TextLabel() {
	delete d_ptr;
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

/* ============================ getter methods ================= */
CLASS_SHARED_D_READER_IMPL(TextLabel, QString, text, text);
CLASS_SHARED_D_READER_IMPL(TextLabel, bool, texUsed, texUsed);
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

STD_SETTER_CMD_IMPL_F(TextLabel, SetTexUsed, bool, texUsed, updateTexImage);
void TextLabel::setTexUsed(const bool tex) {
	Q_D(TextLabel);
	if (tex != d->texUsed)
		exec(new TextLabelSetTexUsedCmd(d, tex, tr("%1: set use tex syntax")));
}

STD_SETTER_CMD_IMPL_F(TextLabel, SetTexFontSize, qreal, texFontSize, updateTexImage);
void TextLabel::setTexFontSize(const qreal fontSize) {
	Q_D(TextLabel);
	if (fontSize != d->texFontSize)
		exec(new TextLabelSetTexFontSizeCmd(d, fontSize, tr("%1: set tex font size")));
}

STD_SETTER_CMD_IMPL_F(TextLabel, SetPosition, QPointF, position, retransform);
void TextLabel::setPosition(const QPointF& pos) {
	Q_D(TextLabel);
	if (pos != d->position)
		exec(new TextLabelSetPositionCmd(d, pos, tr("%1: set position")));
}

STD_SETTER_CMD_IMPL_F(TextLabel, SetHorizontalPosition, TextLabel::HorizontalPosition, horizontalPosition, updatePosition);
void TextLabel::setHorizontalPosition(const TextLabel::HorizontalPosition hPos){
	Q_D(TextLabel);
	if (hPos != d->horizontalPosition)
		exec(new TextLabelSetHorizontalPositionCmd(d, hPos, tr("%1: set horizontal position")));
}

STD_SETTER_CMD_IMPL_F(TextLabel, SetVerticalPosition, TextLabel::VerticalPosition, verticalPosition, updatePosition);
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

void TextLabel::updateTexImage(){
	Q_D(TextLabel);
	d->updateTexImage();
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
	float x = position.x();
	float y = position.y();
	float w, h;

	if (texUsed){
		w = texImage.width()*scaleFactor;
		h = texImage.height()*scaleFactor;
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
}

/*!
	calculates the position of the label, when the position relative to the parent was specified (left, right, etc.)
*/
void TextLabelPrivate::updatePosition(){
	//determine the parent item
	QRectF parentRect;
	QGraphicsItem* parent = parentItem();
	if (parent){
		parentRect = mapRectFromParent( parent->boundingRect() );
	}else{
		if (!scene())
			return;

		parentRect = scene()->sceneRect();
	}

	if (horizontalPosition != TextLabel::hPositionCustom){
		if (horizontalPosition == TextLabel::hPositionlLeft)
			position.setX( parentRect.x() );
		else if (horizontalPosition == TextLabel::hPositionCenter)
			position.setX( parentRect.width()/2 );
		else if (horizontalPosition == TextLabel::hPositionRight)
			position.setX( parentRect.x() + parentRect.width() );
	}

	if (verticalPosition != TextLabel::vPositionCustom){
		if (verticalPosition == TextLabel::vPositionTop)
			position.setY( parentRect.y() );
		else if (verticalPosition == TextLabel::vPositionCenter)
			position.setY( (parentRect.y() + parentRect.height())/2 );
		else if (verticalPosition == TextLabel::vPositionBottom)
			position.setY( parentRect.y() + parentRect.height() );
	}
	
	//retransform the label if either  horiz. or vert. positions were changed
	if (horizontalPosition!=TextLabel::hPositionCustom || verticalPosition!=TextLabel::vPositionCustom)
		retransform();
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

void TextLabelPrivate::updateTexImage(){
	bool status = TexRenderer::renderImageLaTeX(text, texImage, texFontSize);
	if (!status)
		qDebug()<<"TeX image not created";
	
	//the size of the tex image was most probably changed.
	//call retransform() to recalculate the position and the bounding box of the label
	retransform();
}

bool TextLabelPrivate::swapVisible(bool on){
	bool oldValue = isVisible();
	setVisible(on);
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
	
	painter->scale(scaleFactor, scaleFactor);
	painter->rotate(rotationAngle);

	float w,h;
	if (texUsed){
		//painter->scale(.5,.5);
		w = texImage.width();
		h = texImage.height();
		painter->translate(-w/2,-h/2);
		QRectF rect = texImage.rect();
		painter->setRenderHint(QPainter::SmoothPixmapTransform);
		painter->drawImage(rect, texImage, rect);
		//painter->scale(2,2);
	}else{
		w = staticText.size().width();
		h = staticText.size().height();
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
		if (texUsed){
			w = texImage.width()*scaleFactor;
			h = texImage.height()*scaleFactor;
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
 
