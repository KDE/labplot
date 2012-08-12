/***************************************************************************
    File                 : CartesianPlot.cpp
    Project              : LabPlot/SciDAVis
    Description          : A plot containing decoration elements.
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

#include "CartesianPlot.h"
#include "CartesianCoordinateSystem.h"
#include "Axis.h"
#include "XYCurve.h"
#include "../PlotArea.h"
#include "../AbstractPlotPrivate.h"
#include "../../Worksheet.h"
#include "../../TextLabel.h"

#include "lib/XmlStreamReader.h"
#include <QDebug>
#include <QMenu>
 
#ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
#include <QIcon>
#else
#include <KIcon>
#include <KAction>
#include <KLocale>
#endif

#define SCALE_MIN CartesianCoordinateSystem::Scale::LIMIT_MIN
#define SCALE_MAX CartesianCoordinateSystem::Scale::LIMIT_MAX

/**
 * \class CartesianPlot
 * \brief A xy-plot.
 *
 * 
 */

class CartesianPlotPrivate:public AbstractPlotPrivate{
    public:
		CartesianPlotPrivate(CartesianPlot *owner);
		QString name() const;
		void setRect(const QRectF& r);
		QVariant itemChange(GraphicsItemChange change, const QVariant &value);
		virtual void retransform();
};

CartesianPlot::CartesianPlot(const QString &name):AbstractPlot(name, new CartesianPlotPrivate(this)){
	init();
}

CartesianPlot::CartesianPlot(const QString &name, CartesianPlotPrivate *dd):AbstractPlot(name, dd){
	init();
}

CartesianPlot::~CartesianPlot(){
	delete d_ptr;
	delete m_title;
	delete m_coordinateSystem;
	delete m_plotArea;
	delete addNewMenu;
}

void CartesianPlot::init(){
	Q_D(CartesianPlot);
	graphicsItem()->setFlag(QGraphicsItem::ItemIsSelectable, true);
	graphicsItem()->setFlag(QGraphicsItem::ItemIsMovable);
	graphicsItem()->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    graphicsItem()->setFlag(QGraphicsItem::ItemIsFocusable, true);

	m_coordinateSystem = new CartesianCoordinateSystem(this);
	m_plotArea = new PlotArea(name() + " plot area");
	addChild(m_plotArea);

	//Geometry, specify the plot rect in scene coordinates.
	//TODO: Use default settings for left, top, width, height and for min/max for the coordinate system
	float x = Worksheet::convertToSceneUnits(2, Worksheet::Centimeter);
	float y = Worksheet::convertToSceneUnits(2, Worksheet::Centimeter);
	float w = Worksheet::convertToSceneUnits(10, Worksheet::Centimeter);
	float h = Worksheet::convertToSceneUnits(10, Worksheet::Centimeter);

	//offset between the plot area and the area defining the coodinate system, in scene units.
	d->horizontalPadding = Worksheet::convertToSceneUnits(1.5, Worksheet::Centimeter);
	d->verticalPadding = Worksheet::convertToSceneUnits(1.5, Worksheet::Centimeter);

	//Axes
	Axis *axis = new Axis("x axis 1", Axis::AxisHorizontal);
	addChild(axis);
	axis->setStart(0);
	axis->setEnd(1);
	axis->setMajorTicksDirection(Axis::ticksIn);
	axis->setMajorTicksNumber(6);
	axis->setMinorTicksDirection(Axis::ticksIn);
	axis->setMinorTicksNumber(1);

	axis = new Axis("x axis 2", Axis::AxisHorizontal);
	addChild(axis);
	axis->setOffset(1);
	axis->setStart(0);
	axis->setEnd(1);
	axis->setMajorTicksDirection(Axis::ticksIn);
	axis->setMajorTicksNumber(6);
	axis->setMinorTicksDirection(Axis::ticksIn);
	axis->setMinorTicksNumber(1);
	axis->setLabelsPosition(Axis::NoLabels);
	axis->title()->setText("");

	axis = new Axis("y axis 1", Axis::AxisVertical);
	addChild(axis);
	axis->setStart(0);
	axis->setEnd(1);
	axis->setMajorTicksDirection(Axis::ticksIn);
	axis->setMajorTicksNumber(6);
	axis->setMinorTicksDirection(Axis::ticksIn);
	axis->setMinorTicksNumber(1);
	
	axis = new Axis("y axis 2", Axis::AxisVertical);
	addChild(axis);
	axis->setStart(0);
	axis->setEnd(1);
	axis->setOffset(1);
	axis->setMajorTicksDirection(Axis::ticksIn);
	axis->setMajorTicksNumber(6);
	axis->setMinorTicksDirection(Axis::ticksIn);
	axis->setMinorTicksNumber(1);
	axis->setLabelsPosition(Axis::NoLabels);
	axis->title()->setText("");
	
	//Plot title
 	m_title = new TextLabel(this->name());
	m_title->setText(this->name());
	addChild(m_title);
	m_title->setHidden(true);
	m_title->graphicsItem()->setParentItem(m_plotArea->graphicsItem()); //set the parent befor doing any positioning
	m_title->setHorizontalPosition(TextLabel::hPositionCenter);
	m_title->setVerticalPosition(TextLabel::vPositionTop);
	m_title->setHorizontalAlignment(TextLabel::hAlignCenter);
	m_title->setVerticalAlignment(TextLabel::vAlignBottom);
	
	//all plot children are initialized -> set the geometry of the plot in scene coordinates.
	d->setRect(QRectF(x,y,w,h));
		
	initActions();
	initMenus();
}

void CartesianPlot::initActions(){
#ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
	addCurveAction = new QAction(tr("xy-curve"), this);
	addHorizontalAxisAction = new QAction(tr("horizontal axis"), this);
	addVerticalAxisAction = new QAction(tr("vertical axis"), this);
#else
	addCurveAction = new KAction(i18n("xy-curve"), this);
	addHorizontalAxisAction = new KAction(KIcon("axis-horizontal"), i18n("horizontal axis"), this);
	addVerticalAxisAction = new KAction(KIcon("axis-vertical"), i18n("vertical axis"), this);
#endif
	connect(addCurveAction, SIGNAL(triggered()), SLOT(addCurve()));
}

void CartesianPlot::initMenus(){
	addNewMenu = new QMenu(tr("Add new"));
	addNewMenu->addAction(addCurveAction);
	addNewMenu->addSeparator();
	addNewMenu->addAction(addHorizontalAxisAction);
	addNewMenu->addAction(addVerticalAxisAction);
}

QMenu *CartesianPlot::createContextMenu(){
	QMenu *menu = AbstractWorksheetElement::createContextMenu();

	QAction* firstAction = menu->actions().first();
	menu->insertMenu(firstAction, addNewMenu);
	menu->insertSeparator(firstAction);

	return menu;
}

/*!
	Returns an icon to be used in the project explorer.
*/
QIcon CartesianPlot::icon() const{
	QIcon ico;
#ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
	ico.addPixmap(QPixmap(":/graph.xpm"));
#else
	ico = KIcon("office-chart-line");
#endif
	return ico;
}

/*!
	set the rectangular, defined in scene coordinates
 */
void CartesianPlot::setRect(const QRectF& r){
	Q_D(CartesianPlot);
	d->setRect(r);
}

//################################################################
//################### Slots ##########################
//################################################################
void CartesianPlot::addCurve(){
	this->addChild(new XYCurve("xy-curve"));
}


//#####################################################################
//################### Private implementation ##########################
//#####################################################################
CartesianPlotPrivate::CartesianPlotPrivate(CartesianPlot *owner) : AbstractPlotPrivate(owner){
}

QString CartesianPlotPrivate::name() const{
	return q->name();
}

/*!
	sets the rectangular of the plot in scene coordinates to \c r.
	The size of the plot corresponds to the size of the plot area, the area which is filled with the background color etc.
	and which can pose the parent item for several sub-items (like TextLabel).
	Note, the size of the area used to define the coordinate system doesn't need to be equal to this plot area.
	Also, the size (=bounding box) of CartesianPlot can be greater than the size of the plot area.
 */
void CartesianPlotPrivate::setRect(const QRectF& r){
	prepareGeometryChange();
	setPos( r.x()+r.width()/2, r.y()+r.height()/2);
	rect = r;
	this->retransform();
}

void CartesianPlotPrivate::retransform(){
	AbstractPlot* plot = dynamic_cast<AbstractPlot*>(q);
	CartesianCoordinateSystem *cSystem = dynamic_cast<CartesianCoordinateSystem *>(plot->coordinateSystem());
	QList<CartesianCoordinateSystem::Scale *> scales;
	
	//perform the mapping from the scene coordinates to the plot's coordinates here.
	QRectF itemRect= mapRectFromScene( rect );
	scales << CartesianCoordinateSystem::Scale::createLinearScale(Interval<double>(SCALE_MIN, SCALE_MAX),
																  itemRect.x()+horizontalPadding,
																  itemRect.x()+itemRect.width()-horizontalPadding,
																  0, 1);
	cSystem ->setXScales(scales);
	scales.clear();
	scales << CartesianCoordinateSystem::Scale::createLinearScale(Interval<double>(SCALE_MIN, SCALE_MAX),
																  itemRect.y()+itemRect.height()-verticalPadding,
																  itemRect.y()+verticalPadding, 
																  0, 1);
	cSystem ->setYScales(scales);

	//plotArea position is always (0, 0) in parent's coordinates, don't need to update here
	plot->plotArea()->setRect(rect);

	//call retransform() for the title:
	//when a predefined title position (Left, Centered etc.) is used, 
	//the actual title position needs to be updated on plot's geometry changes.
	plot->title()->retransform();

	q->retransform();
}

/*!
 * Reimplemented from QGraphicsItem.
 */
QVariant CartesianPlotPrivate::itemChange(GraphicsItemChange change, const QVariant &value){
	if (change == QGraphicsItem::ItemPositionChange) {
		QPointF itemPos = value.toPointF();//item's center point in parent's coordinates;
		float x = itemPos.x();
		float y = itemPos.y();
		
		//update rect
		float w = rect.width();
		float h = rect.height();
		rect.setX(x-w/2);
		rect.setY(y-h/2);
		rect.setWidth(w);
		rect.setHeight(h);
		emit dynamic_cast<CartesianPlot*>(q)->positionChanged();
     }
	return QGraphicsItem::itemChange(change, value);
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################

//! Save as XML
void CartesianPlot::save(QXmlStreamWriter* writer) const{
	Q_D(const CartesianPlot);

    writer->writeStartElement( "cartesianPlot" );
    writeBasicAttributes(writer);
    writeCommentElement(writer);

	//TODO
	
    //serialize all children
    QList<AbstractWorksheetElement *> childElements = children<AbstractWorksheetElement>(IncludeHidden);
    foreach(AbstractWorksheetElement *elem, childElements)
        elem->save(writer);

    writer->writeEndElement(); // close "cartesianPlot" section
}


//! Load from XML
bool CartesianPlot::load(XmlStreamReader* reader){
    if(!reader->isStartElement() || reader->name() != "cartesianPlot"){
        reader->raiseError(tr("no cartesianPlot element found"));
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
        if (reader->isEndElement() && reader->name() == "cartesianPlot")
            break;

        if (!reader->isStartElement())
            continue;

        if (reader->name() == "comment"){
            if (!readCommentElement(reader)) return false;
        }else{ // unknown element
            reader->raiseWarning(tr("unknown element '%1'").arg(reader->name().toString()));
            if (!reader->skipToEndElement()) return false;
        }
    }

    return true;
}
