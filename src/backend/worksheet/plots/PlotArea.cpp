/***************************************************************************
    File                 : PlotArea.cpp
    Project              : LabPlot/SciDAVis
    Description          : Plot area (for background filling and clipping).
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs*gmx.net)
    Copyright            : (C) 2011-2012 by Alexander Semke (alexander.semke*web.de)
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

#include "worksheet/Worksheet.h"
#include "worksheet/plots/PlotArea.h"
#include "worksheet/plots/PlotAreaPrivate.h"
#include "worksheet/plots/AbstractCoordinateSystem.h"
#include "worksheet/plots/AbstractPlot.h"
#include "lib/commandtemplates.h"
#include "lib/macros.h"
#include <QPainter>
#include <QDebug>
#ifndef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
#include <KConfig>
#include <KConfigGroup>
#endif

/**
 * \class PlotArea
 * \brief Plot area (for background filling and clipping).
 *
 * \ingroup worksheet
 */

PlotArea::PlotArea(const QString &name):AbstractWorksheetElement(name),
		d_ptr(new PlotAreaPrivate(this)){
	init();
}

PlotArea::PlotArea(const QString &name, PlotAreaPrivate *dd)
    : AbstractWorksheetElement(name), d_ptr(dd){
	init();
}

PlotArea::~PlotArea() {
}

void PlotArea::init(){
	Q_D(PlotArea);
	
	setHidden(true);//we don't show PlotArea aspect in the model view.
	d->rect = QRectF(0, 0, 1, 1);
	d->setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);

#ifndef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
	KConfig config;
	KConfigGroup group = config.group( "PlotArea" );

	//Background
	d->backgroundType = (PlotArea::BackgroundType) group.readEntry("BackgroundType", (int)PlotArea::Color);
	d->backgroundColorStyle = (PlotArea::BackgroundColorStyle) group.readEntry("BackgroundColorStyle", (int) PlotArea::SingleColor);
	d->backgroundImageStyle = (PlotArea::BackgroundImageStyle) group.readEntry("BackgroundImageStyle", (int) PlotArea::Scaled);
	d->backgroundBrushStyle = (Qt::BrushStyle) group.readEntry("BackgroundBrushStyle", (int) Qt::SolidPattern);
	d->backgroundFileName = group.readEntry("BackgroundFileName", QString());
	d->backgroundFirstColor = group.readEntry("BackgroundFirstColor", QColor(Qt::white));
	d->backgroundSecondColor = group.readEntry("BackgroundSecondColor", QColor(Qt::black));
	d->backgroundOpacity = group.readEntry("BackgroundOpacity", 1.0);
	//Border
	d->borderPen = QPen(group.readEntry("BorderColor", QColor(Qt::black)), 
											group.readEntry("BorderWidth", 0.0), 
											(Qt::PenStyle) group.readEntry("BorderStyle", (int)Qt::SolidLine));
	d->borderOpacity = group.readEntry("BorderOpacity", 1.0);
#endif
}

QGraphicsItem *PlotArea::graphicsItem() const{
	return d_ptr;
}

STD_SWAP_METHOD_SETTER_CMD_IMPL(PlotArea, SetVisible, bool, swapVisible);
void PlotArea::setVisible(bool on){
	Q_D(PlotArea);
	exec(new PlotAreaSetVisibleCmd(d, on, on ? tr("%1: set visible") : tr("%1: set invisible")));
}

bool PlotArea::isVisible() const{
	Q_D(const PlotArea);
	return d->isVisible();
}

void PlotArea::handlePageResize(double horizontalRatio, double verticalRatio){
	Q_D(PlotArea);

	//TODO: doesn't work properly
	d->rect.setWidth(d->rect.width()*horizontalRatio);
	d->rect.setHeight(d->rect.height()*verticalRatio);
	
	// TODO: scale line width
	BaseClass::handlePageResize(horizontalRatio, verticalRatio);
}

void PlotArea::retransform(){

}


/* ============================ accessor documentation ================= */
/**
  \fn PlotArea::BASIC_D_ACCESSOR_DECL(bool, clippingEnabled, ClippingEnabled);
  \brief Set/get whether clipping is enabled.
*/
/**
  \fn PlotArea::CLASS_D_ACCESSOR_DECL(QRectF, rect, Rect);
  \brief Set/get the plot area rectangle.
*/

/* ============================ getter methods ================= */
BASIC_SHARED_D_READER_IMPL(PlotArea, bool, clippingEnabled, clippingEnabled());
CLASS_SHARED_D_READER_IMPL(PlotArea, QRectF, rect, rect);

BASIC_SHARED_D_READER_IMPL(PlotArea, PlotArea::BackgroundType, backgroundType, backgroundType);
BASIC_SHARED_D_READER_IMPL(PlotArea, PlotArea::BackgroundColorStyle, backgroundColorStyle, backgroundColorStyle);
BASIC_SHARED_D_READER_IMPL(PlotArea, PlotArea::BackgroundImageStyle, backgroundImageStyle, backgroundImageStyle);
CLASS_SHARED_D_READER_IMPL(PlotArea, Qt::BrushStyle, backgroundBrushStyle, backgroundBrushStyle);
CLASS_SHARED_D_READER_IMPL(PlotArea, QColor, backgroundFirstColor, backgroundFirstColor);
CLASS_SHARED_D_READER_IMPL(PlotArea, QColor, backgroundSecondColor, backgroundSecondColor);
CLASS_SHARED_D_READER_IMPL(PlotArea, QString, backgroundFileName, backgroundFileName);
BASIC_SHARED_D_READER_IMPL(PlotArea, qreal, backgroundOpacity, backgroundOpacity);

CLASS_SHARED_D_READER_IMPL(PlotArea, QPen, borderPen, borderPen);
BASIC_SHARED_D_READER_IMPL(PlotArea, qreal, borderOpacity, borderOpacity);


/* ============================ setter methods and undo commands ================= */

STD_SWAP_METHOD_SETTER_CMD_IMPL(PlotArea, SetClippingEnabled, bool, toggleClipping);
void PlotArea::setClippingEnabled(bool on) {
	Q_D(PlotArea);

	if (d->clippingEnabled() != on)
		exec(new PlotAreaSetClippingEnabledCmd(d, on, tr("%1: toggle clipping")));
}

/*!
 * sets plot area rect in scene coordinates.
 */
void PlotArea::setRect(const QRectF &newRect) {
	Q_D(PlotArea);
	d->setRect(newRect);
}

//Background
STD_SETTER_CMD_IMPL_F(PlotArea, SetBackgroundType, PlotArea::BackgroundType, backgroundType, update);
void PlotArea::setBackgroundType(BackgroundType type) {
	Q_D(PlotArea);
	if (type != d->backgroundType)
		exec(new PlotAreaSetBackgroundTypeCmd(d, type, tr("%1: background type changed")));
}

STD_SETTER_CMD_IMPL_F(PlotArea, SetBackgroundColorStyle, PlotArea::BackgroundColorStyle, backgroundColorStyle, update);
void PlotArea::setBackgroundColorStyle(BackgroundColorStyle style) {
	Q_D(PlotArea);
	if (style != d->backgroundColorStyle)
		exec(new PlotAreaSetBackgroundColorStyleCmd(d, style, tr("%1: background color style changed")));
}

STD_SETTER_CMD_IMPL_F(PlotArea, SetBackgroundImageStyle, PlotArea::BackgroundImageStyle, backgroundImageStyle, update);
void PlotArea::setBackgroundImageStyle(PlotArea::BackgroundImageStyle style) {
	Q_D(PlotArea);
	if (style != d->backgroundImageStyle)
		exec(new PlotAreaSetBackgroundImageStyleCmd(d, style, tr("%1: background image style changed")));
}

STD_SETTER_CMD_IMPL_F(PlotArea, SetBackgroundBrushStyle, Qt::BrushStyle, backgroundBrushStyle, update);
void PlotArea::setBackgroundBrushStyle(Qt::BrushStyle style) {
	Q_D(PlotArea);
	if (style != d->backgroundBrushStyle)
		exec(new PlotAreaSetBackgroundBrushStyleCmd(d, style, tr("%1: background brush style changed")));
}

STD_SETTER_CMD_IMPL_F(PlotArea, SetBackgroundFirstColor, QColor, backgroundFirstColor, update);
void PlotArea::setBackgroundFirstColor(const QColor &color) {
	Q_D(PlotArea);
	if (color!= d->backgroundFirstColor)
		exec(new PlotAreaSetBackgroundFirstColorCmd(d, color, tr("%1: set background first color")));
}

STD_SETTER_CMD_IMPL_F(PlotArea, SetBackgroundSecondColor, QColor, backgroundSecondColor, update);
void PlotArea::setBackgroundSecondColor(const QColor &color) {
	Q_D(PlotArea);
	if (color!= d->backgroundSecondColor)
		exec(new PlotAreaSetBackgroundSecondColorCmd(d, color, tr("%1: set background second color")));
}

STD_SETTER_CMD_IMPL_F(PlotArea, SetBackgroundFileName, QString, backgroundFileName, update);
void PlotArea::setBackgroundFileName(const QString& fileName) {
	Q_D(PlotArea);
	if (fileName!= d->backgroundFileName)
		exec(new PlotAreaSetBackgroundFileNameCmd(d, fileName, tr("%1: set background image")));
}

STD_SETTER_CMD_IMPL_F(PlotArea, SetBackgroundOpacity, qreal, backgroundOpacity, update);
void PlotArea::setBackgroundOpacity(qreal opacity) {
	Q_D(PlotArea);
	if (opacity != d->backgroundOpacity)
		exec(new PlotAreaSetBackgroundOpacityCmd(d, opacity, tr("%1: set plot area opacity")));
}

//Border
STD_SETTER_CMD_IMPL_F(PlotArea, SetBorderPen, QPen, borderPen, update);
void PlotArea::setBorderPen(const QPen &pen) {
	Q_D(PlotArea);
	if (pen != d->borderPen)
		exec(new PlotAreaSetBorderPenCmd(d, pen, tr("%1: set plot area border style")));
}

STD_SETTER_CMD_IMPL_F(PlotArea, SetBorderOpacity, qreal, borderOpacity, update);
void PlotArea::setBorderOpacity(qreal opacity) {
	Q_D(PlotArea);
	if (opacity != d->borderOpacity)
		exec(new PlotAreaSetBorderOpacityCmd(d, opacity, tr("%1: set plot area border opacity")));
}

//################################################################
//################### Private implementation ##########################
//################################################################
PlotAreaPrivate::PlotAreaPrivate(PlotArea *owner):q(owner){
}

PlotAreaPrivate::~PlotAreaPrivate() {
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

bool PlotAreaPrivate::swapVisible(bool on){
	bool oldValue = isVisible();
	setVisible(on);
	return oldValue;
}

void PlotAreaPrivate::setRect(const QRectF& r){
	prepareGeometryChange();
	rect = mapRectFromScene(r);
}

QRectF PlotAreaPrivate::boundingRect () const{
	return rect; 
}

QPainterPath PlotAreaPrivate::shape() const{
	QPainterPath path;
	path.addRect(rect);
	return path;
}

void PlotAreaPrivate::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
	Q_UNUSED(option)
	Q_UNUSED(widget)

	if (!isVisible())
		return;

	//draw the area
	painter->setOpacity(backgroundOpacity);
	painter->setPen(Qt::NoPen);
	if (backgroundType == PlotArea::Color){
		switch (backgroundColorStyle){
			case PlotArea::SingleColor:{
				painter->setBrush(QBrush(backgroundFirstColor));
				break;
			}
			case PlotArea::HorizontalLinearGradient:{
				QLinearGradient linearGrad(rect.topLeft(), rect.topRight());
				linearGrad.setColorAt(0, backgroundFirstColor);
				linearGrad.setColorAt(1, backgroundSecondColor);
				painter->setBrush(QBrush(linearGrad));
				break;
			}
			case PlotArea::VerticalLinearGradient:{
				QLinearGradient linearGrad(rect.topLeft(), rect.bottomLeft());
				linearGrad.setColorAt(0, backgroundFirstColor);
				linearGrad.setColorAt(1, backgroundSecondColor);
				painter->setBrush(QBrush(linearGrad));
				break;
			}
			case PlotArea::TopLeftDiagonalLinearGradient:{
				QLinearGradient linearGrad(rect.topLeft(), rect.bottomRight());
				linearGrad.setColorAt(0, backgroundFirstColor);
				linearGrad.setColorAt(1, backgroundSecondColor);
				painter->setBrush(QBrush(linearGrad));
				break;
			}
			case PlotArea::BottomLeftDiagonalLinearGradient:{
				QLinearGradient linearGrad(rect.bottomLeft(), rect.topRight());
				linearGrad.setColorAt(0, backgroundFirstColor);
				linearGrad.setColorAt(1, backgroundSecondColor);
				painter->setBrush(QBrush(linearGrad));
				break;
			}
			case PlotArea::RadialGradient:{
				QRadialGradient radialGrad(rect.center(), rect.width()/2);
				radialGrad.setColorAt(0, backgroundFirstColor);
				radialGrad.setColorAt(1, backgroundSecondColor);
				painter->setBrush(QBrush(radialGrad));
				break;
			}			
			default:
				painter->setBrush(QBrush(backgroundFirstColor));
		}
	}else if (backgroundType == PlotArea::Image){
		QPixmap pix(backgroundFileName);
		switch (backgroundImageStyle){
			case PlotArea::ScaledCropped:
				pix = pix.scaled(rect.size().toSize(),Qt::KeepAspectRatioByExpanding,Qt::SmoothTransformation);
				painter->drawPixmap(rect.topLeft(),pix);
				break;
			case PlotArea::Scaled:
				pix = pix.scaled(rect.size().toSize(),Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
				painter->drawPixmap(rect.topLeft(),pix);
				break;
			case PlotArea::ScaledAspectRatio:
				pix = pix.scaled(rect.size().toSize(),Qt::KeepAspectRatio,Qt::SmoothTransformation);
				painter->drawPixmap(rect.topLeft(),pix);
				break;
			case PlotArea::Centered:
				painter->drawPixmap(QPointF(rect.center().x()-pix.size().width()/2,rect.center().y()-pix.size().height()/2),pix);
				break;
			case PlotArea::Tiled:
				painter->drawTiledPixmap(rect,pix);
				break;
			case PlotArea::CenterTiled:
				painter->drawTiledPixmap(rect,pix,QPoint(rect.size().width()/2,rect.size().height()/2));
				break;
			default:
				painter->drawPixmap(rect.topLeft(),pix);
		}
	} else if (backgroundType == PlotArea::Pattern){
			painter->setBrush(QBrush(backgroundFirstColor,backgroundBrushStyle));
	}
	painter->drawRect(rect);

	//draw the border
	if (borderPen.style() != Qt::NoPen){
		painter->setPen(borderPen);
		painter->setBrush(Qt::NoBrush);
		painter->setOpacity(borderOpacity);
		painter->drawRect(rect);
	}
}
