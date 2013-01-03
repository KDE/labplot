/***************************************************************************
    File                 : CartesianPlotLegend.cpp
    Project              : LabPlot/SciDAVis
    Description          : Legend for the cartesian plot
    --------------------------------------------------------------------
    Copyright            : (C) 2013 Alexander Semke (alexander.semke*web.de)
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

/*!
  \class CartesianPlotLegend
  \brief Legend for the cartesian plot.
 
  \ingroup kdefrontend
*/

#include "CartesianPlotLegend.h"
#include "backend/worksheet/plots/cartesian/CartesianPlotLegendPrivate.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/core/plugin/PluginManager.h"
#include "backend/worksheet/AbstractCurveSymbol.h"
#include "backend/worksheet/interfaces.h"

#include <QGraphicsItem>
#include <QPainterPath>
#include <QPainter>
#include <QtDebug>

#ifndef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
#include <KIcon>
#include <KConfig>
#include <KConfigGroup>
#endif

CartesianPlotLegend::CartesianPlotLegend(CartesianPlot* plot, const QString &name)
		: AbstractWorksheetElement(name), d_ptr(new CartesianPlotLegendPrivate(this)), m_plot(plot) {
	init();
}

CartesianPlotLegend::CartesianPlotLegend(CartesianPlot* plot, const QString &name, CartesianPlotLegendPrivate *dd)
		: AbstractWorksheetElement(name), d_ptr(dd), m_plot(plot){
	init();
}

void CartesianPlotLegend::init(){
	Q_D(CartesianPlotLegend);
#ifndef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
	KConfig config;
	KConfigGroup group = config.group( "CartesianPlotLegend" );

	d->labelFont.setPointSizeF( Worksheet::convertToSceneUnits( 8, Worksheet::Point ) );
	d->labelColor = Qt::black;
	d->lineSymbolWidth =  group.readEntry("LineSymbolWidth", Worksheet::convertToSceneUnits(1, Worksheet::Centimeter));

	//Background
	d->backgroundType = (PlotArea::BackgroundType) group.readEntry("BackgroundType", (int) PlotArea::Color);
	d->backgroundColorStyle = (PlotArea::BackgroundColorStyle) group.readEntry("BackgroundColorStyle", (int) PlotArea::SingleColor);
	d->backgroundImageStyle = (PlotArea::BackgroundImageStyle) group.readEntry("BackgroundImageStyle", (int) PlotArea::Scaled);
	d->backgroundBrushStyle = (Qt::BrushStyle) group.readEntry("BackgroundBrushStyle", (int) Qt::SolidPattern);
	d->backgroundFileName = group.readEntry("BackgroundFileName", QString());
	d->backgroundFirstColor = group.readEntry("BackgroundFirstColor", QColor(Qt::white));
	d->backgroundSecondColor = group.readEntry("BackgroundSecondColor", QColor(Qt::black));
	d->backgroundOpacity = group.readEntry("BackgroundOpacity", 1.0);

	//Border
	d->borderPen = QPen(group.readEntry("BorderColor", QColor(Qt::black)),
											group.readEntry("BorderWidth", Worksheet::convertToSceneUnits(1.0, Worksheet::Point)),
											(Qt::PenStyle) group.readEntry("BorderStyle", (int)Qt::SolidLine));
	d->borderOpacity = group.readEntry("BorderOpacity", 1.0);

	//Layout
	d->layoutTopMargin =  group.readEntry("LayoutTopMargin", Worksheet::convertToSceneUnits(0.2, Worksheet::Centimeter));
	d->layoutBottomMargin = group.readEntry("LayoutBottomMargin", Worksheet::convertToSceneUnits(0.2, Worksheet::Centimeter));
	d->layoutLeftMargin = group.readEntry("LayoutLeftMargin", Worksheet::convertToSceneUnits(0.2, Worksheet::Centimeter));
	d->layoutRightMargin = group.readEntry("LayoutRightMargin", Worksheet::convertToSceneUnits(0.2, Worksheet::Centimeter));
	d->layoutVerticalSpacing = group.readEntry("LayoutVerticalSpacing", Worksheet::convertToSceneUnits(0.1, Worksheet::Centimeter));
	d->layoutHorizontalSpacing = group.readEntry("LayoutHorizontalSpacing", Worksheet::convertToSceneUnits(0.1, Worksheet::Centimeter));
	d->layoutColumnCount = group.readEntry("LayoutColumnCount", 1);
#endif	
	graphicsItem()->setFlag(QGraphicsItem::ItemIsSelectable, true);
	graphicsItem()->setFlag(QGraphicsItem::ItemIsMovable);
	graphicsItem()->setFlag(QGraphicsItem::ItemSendsGeometryChanges);
}

CartesianPlotLegend::~CartesianPlotLegend() {
	delete d_ptr;
}

/*!
	Returns an icon to be used in the project explorer.
*/
QIcon CartesianPlotLegend::icon() const{
	QIcon icon;
#ifndef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
		icon = KIcon("text-field");
#endif
	return icon;
}

//TODO
// STD_SWAP_METHOD_SETTER_CMD_IMPL(CartesianPlotLegend, SetVisible, bool, swapVisible)
void CartesianPlotLegend::setVisible(bool on){
	Q_D(CartesianPlotLegend);
// 	exec(new CartesianPlotLegendSetVisibleCmd(d, on, on ? tr("%1: set visible") : tr("%1: set invisible")));
}

bool CartesianPlotLegend::isVisible() const{
	Q_D(const CartesianPlotLegend);
	return d->isVisible();
}

QGraphicsItem *CartesianPlotLegend::graphicsItem() const{
	return d_ptr;
}

void CartesianPlotLegend::retransform(){
	d_ptr->retransform();
}

void CartesianPlotLegend::handlePageResize(double horizontalRatio, double verticalRatio){
	Q_D(const CartesianPlotLegend);
  //TODO	
	retransform();
}

//##############################################################################
//################################  getter methods  ############################
//##############################################################################

//##############################################################################
//######################  setter methods and undo commands  ####################
//##############################################################################


//##############################################################################
//#################################  SLOTS  ####################################
//##############################################################################


//##############################################################################
//######################### Private implementation #############################
//##############################################################################
CartesianPlotLegendPrivate::CartesianPlotLegendPrivate(CartesianPlotLegend *owner):q(owner) {
}

QString CartesianPlotLegendPrivate::name() const {
	return q->name();
}

QRectF CartesianPlotLegendPrivate::boundingRect() const {
	return rect;
}

/*!
  Returns the shape of the CartesianPlotLegend as a QPainterPath in local coordinates
*/
QPainterPath CartesianPlotLegendPrivate::shape() const {
	QPainterPath path;
	path.addRect(rect);
	return path;
}

bool CartesianPlotLegendPrivate::swapVisible(bool on){
	bool oldValue = isVisible();
	setVisible(on);
	return oldValue;
}

/*!
  recalculates the rectangular of the legend.
*/
void CartesianPlotLegendPrivate::retransform() {
	prepareGeometryChange();

	QList<XYCurve*> children = q->m_plot->children<XYCurve>();

	//determine the width of the legend
	float max=0;
	QFontMetrics fm(labelFont);
	float w;
	float h=fm.ascent();
	foreach (XYCurve* child, children) {
		w = fm.width(child->name());
		if (w>max)
			max = w;
	}
	rect.setWidth(layoutLeftMargin + layoutRightMargin + lineSymbolWidth + layoutHorizontalSpacing + max);

	//determine the height of the legend
	int count = children.size();
	rect.setHeight(layoutTopMargin + layoutBottomMargin + (count-1)*layoutVerticalSpacing + count*h);
}

/*!
  Reimplementation of QGraphicsItem::paint(). This function does the actual painting of the legend.
  \sa QGraphicsItem::paint().
*/
void CartesianPlotLegendPrivate::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget * widget) {
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

	//draw the curve names
	painter->setFont(labelFont);
	
	QList<XYCurve*> children = q->m_plot->children<XYCurve>();
	QFontMetrics fm(labelFont);

	CurveSymbolFactory* factory=0;
	foreach(QObject *plugin, PluginManager::plugins())
		factory = qobject_cast<CurveSymbolFactory*>(plugin);

	float h=fm.ascent();
	painter->save();
	painter->translate(layoutLeftMargin, layoutTopMargin+h);
	foreach (XYCurve* curve, children) {
		//curve's line
		if (curve->lineType() != XYCurve::NoLine){
			painter->setPen(curve->linePen());
			painter->setOpacity(curve->lineOpacity());
			painter->drawLine(0, -h/2, lineSymbolWidth, -h/2);
		}

		//curve's symbol
		if (curve->symbolsTypeId()!="none"){
			if (factory) {
				painter->setOpacity(curve->symbolsOpacity());
				AbstractCurveSymbol* symbol = factory->prototype(curve->symbolsTypeId())->clone();

				symbol->setSize(curve->symbolsSize());
				symbol->setAspectRatio(curve->symbolsAspectRatio());
				symbol->setBrush(curve->symbolsBrush());
				symbol->setPen(curve->symbolsPen());
				symbol->setRotationAngle(curve->symbolsRotationAngle());

				painter->translate(QPointF(lineSymbolWidth/2, -h/2));
				symbol->paint(painter, option, widget);
				painter->translate(-QPointF(lineSymbolWidth/2, -h/2));
			}
		}
  
		//curve's name
		painter->setPen(QPen(labelColor));
		painter->setOpacity(1.0);
		painter->drawText(QPoint(lineSymbolWidth + layoutHorizontalSpacing, 0), curve->name());
		painter->translate(0,layoutVerticalSpacing+h);
	}
	painter->restore();
	
	if (isSelected()){
		QPainterPath path = shape();
		painter->setPen(QPen(Qt::blue, 0, Qt::DashLine));
		painter->drawPath(path);
	}
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
//! Save as XML
void CartesianPlotLegend::save(QXmlStreamWriter* writer) const {
	Q_D(const CartesianPlotLegend);

    writer->writeStartElement( "cartesianPlotLegend" );
    writeBasicAttributes( writer );
    writeCommentElement( writer );

	writer->writeEndElement(); // close "cartesianPlotLegend" section
}

//! Load from XML
bool CartesianPlotLegend::load(XmlStreamReader* reader) {
	Q_D(CartesianPlotLegend);

    if(!reader->isStartElement() || reader->name() != "cartesianPlotLegend"){
        reader->raiseError(tr("no cartesian plot legend element found"));
        return false;
    }

    if (!readBasicAttributes(reader))
        return false;

    QString attributeWarning = tr("Attribute '%1' missing or empty, default value is used");
    QXmlStreamAttributes attribs;
    QString str;

    while (!reader->atEnd()){
        reader->readNext();
        if (reader->isEndElement() && reader->name() == "cartesianPlotLegend")
            break;

        if (!reader->isStartElement())
            continue;

        if (reader->name() == "comment"){
            if (!readCommentElement(reader)) return false;
		}else if (reader->name() == "general"){

		}
	}
	return true;
}
