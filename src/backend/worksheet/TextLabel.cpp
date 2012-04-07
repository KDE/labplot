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
#include <QDebug>

/**
 * \class TextLabel
 * \brief A label supporting rendering of hml- and tex-formated textes.
 * 
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

	d->horizontalPosition = TextLabel::HorizontalCenter;
	d->verticalPosition = TextLabel::VerticalCenter;
	d->texUsed = false;
	d->rotationAngle = 0.0;
	d->position.setX(100);
	d->position.setY(100);
	d->positionOffset.setX(0);
	d->positionOffset.setY(0);
	d->scaleFactor = Worksheet::convertToSceneUnits(1, Worksheet::Point);
	d->staticText.setTextFormat(Qt::RichText);
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
CLASS_SHARED_D_READER_IMPL(TextLabel, QPointF, position, position);
BASIC_SHARED_D_READER_IMPL(TextLabel, TextLabel::HorizontalAlignment, horizontalAlignment, horizontalAlignment);
BASIC_SHARED_D_READER_IMPL(TextLabel, TextLabel::VerticalAlignment, verticalAlignment, verticalAlignment);
BASIC_SHARED_D_READER_IMPL(TextLabel, float, rotationAngle, rotationAngle);

/* ============================ setter methods and undo commands ================= */
STD_SETTER_CMD_IMPL_F(TextLabel, SetText, QString, text, retransform);
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

void TextLabelPrivate::retransform(){
	staticText.setText(text);
	
	//TODO
// 			switch (horizontalAlignment) {
// 			case TextLabel::hAlignLeft:
// 				pos.setX(textRect.left());
// 				break;
// 			case TextLabel::hAlignCenter:
// 				pos.setX(textRect.center().x());
// 				break;
// 			case TextLabel::hAlignRight:
// 				pos.setX(textRect.right());
// 				break;
// 		}
// 
// 		switch (verticalAlignment) {
// 			case TextLabel::vAlignTop:
// 				pos.setY(textRect.top());
// 				break;
// 			case TextLabel::vAlignCenter:
// 				pos.setY(textRect.center().y());
// 				break;
// 			case TextLabel::vAlignBottom:
// 				pos.setY(textRect.bottom());
// 				break;
// 		}
		
	positionOffset.setX(0);
	positionOffset.setY(0);
	
	boundingRectangle.setX(position.x());
	boundingRectangle.setY(position.y());
	boundingRectangle.setWidth(staticText.size().width()*scaleFactor);
	boundingRectangle.setHeight(staticText.size().height()*scaleFactor);
	
	recalcShapeAndBoundingRect();
}

//TODO
void TextLabelPrivate::updatePosition(){
	//determine the current position
// 	float x,y;
// 	QGraphicsItem* parent = dynamic_cast<AbstractWorksheetElement*>(q->parentAspect())->graphicsItem();
// 	if (!parent)
// 		return;
// 
// 	if (horizontalPosition == TextLabel::HorizontalLeft)
// 		x = parent->boundingRect().x();
// 	else if (horizontalPosition == TextLabel::HorizontalCenter)
// 		x = parent->boundingRect().width()/2;
// 		else if (horizontalPosition == TextLabel::HorizontalRight)
// 		x = parent->boundingRect().x() + parent->boundingRect().width();
// 	
// 	if (verticalPosition == TextLabel::VerticalTop)
// 		y = parent->boundingRect().y();
// 	else if (verticalPosition == TextLabel::VerticalCenter)
// 		y = (parent->boundingRect().y() + parent->boundingRect().height())/2;
// 	else if (verticalPosition == TextLabel::VerticalBottom)
// 		y = parent->boundingRect().y() + parent->boundingRect().height();
// 	
// 	position.setX(x);
// 	position.setY(y);
// 	
// 	update();
}

void TextLabelPrivate::updateTexImage(){
	bool status = TexRenderer::renderImageLaTeX(text, texImage);
	if (status)
		qDebug()<<"tex image created";
	else
		qDebug()<<"tex image not created";
	update();
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
  recalculates the outer bounds and the shape of the curve.
*/
void TextLabelPrivate::recalcShapeAndBoundingRect(){
	prepareGeometryChange();

	QMatrix matrix;
	matrix.rotate(rotationAngle);
	transformedBoundingRectangle = matrix.mapRect(boundingRectangle);
		
	labelShape = QPainterPath();
	labelShape.addRect(boundingRectangle);
	labelShape = matrix.map(labelShape);
	
	update();
}

void TextLabelPrivate::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget * widget){
	Q_UNUSED(option)
	Q_UNUSED(widget)

	painter->save();
	painter->translate(position);

	if (texUsed){
		QRectF rect = texImage.rect();
		painter->drawImage(rect, texImage, rect);
		return;
	}

	painter->scale(scaleFactor, scaleFactor);
	painter->rotate(rotationAngle);
 	painter->drawStaticText(positionOffset, staticText);
	
	if (isSelected()){
		painter->setPen(QPen(Qt::blue, 0, Qt::DashLine));
		painter->drawPath(labelShape);
  }
  
	painter->restore();
}
