/***************************************************************************
    File                 : Axis.cpp
    Project              : LabPlot/SciDAVis
    Description          : Axis for cartesian coordinate systems.
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs*gmx.net)
    Copyright            : (C) 2011-2013 Alexander Semke (alexander.semke*web.de)
    Copyright            : (C) 2013 Stefan Gerlach  (stefan.gerlach*uni-konstanz.de)
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

#include "backend/worksheet/plots/cartesian/Axis.h"
#include "backend/worksheet/plots/cartesian/AxisPrivate.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/TextLabel.h"
#include "backend/worksheet/plots/AbstractCoordinateSystem.h"
#include "backend/worksheet/plots/cartesian/CartesianCoordinateSystem.h"
#include "CartesianPlot.h"
#include "backend/worksheet/plots/AbstractPlot.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/XmlStreamReader.h"

#include "kdefrontend/GuiTools.h"
#include <QBrush>
#include <QPen>
#include <QPainter>
#include <QFontMetricsF>
// #include <QtDebug>

#include <math.h>

#ifndef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
#include <KConfig>
#include <KConfigGroup>
#include <KIcon>
#include <KAction>
#include <KLocale>
#endif

/**
 * \class Axis
 * \brief Axis for cartesian coordinate systems.
 *
 *  
 */

// TODO: decide whether it makes sense to move some of the functionality into a class AbstractAxis

Axis::Axis(const QString &name, const AxisOrientation &orientation)
		: AbstractWorksheetElement(name), d_ptr(new AxisPrivate(this)) {
	d_ptr->orientation = orientation;
	init();
}

Axis::Axis(const QString &name, const AxisOrientation &orientation, AxisPrivate *dd)
		: AbstractWorksheetElement(name), d_ptr(dd) {
	d_ptr->orientation = orientation;
	init();
}

void Axis::init(){
	Q_D(Axis);

#ifndef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
	// only settings general to all axes
	KConfig config;
	KConfigGroup group = config.group( "Axis" );
	//TODO: add missing settings (see AxisDock::load())

	d->autoScale = true;
	d->position = Axis::AxisCustom;
	d->scale = (Axis::AxisScale) group.readEntry("Scale", (int) Axis::ScaleLinear);
	d->offset = group.readEntry("PositionOffset", 0);
	d->start = group.readEntry("Start", 0);
	d->end = group.readEntry("End", 10);
	d->scalingFactor = group.readEntry("ScalingFactor", 1.0);
	d->zeroOffset = group.readEntry("ZeroOffset", 0);
	
	d->linePen.setStyle( (Qt::PenStyle) group.readEntry("LineStyle", (int) Qt::SolidLine) );
	d->linePen.setWidthF( group.readEntry("LineWidth", Worksheet::convertToSceneUnits( 1.0, Worksheet::Point ) ) );
	d->lineOpacity = group.readEntry("LineOpacity", 1.0);

	// axis title
 	d->title = new TextLabel(this->name());
	connect( d->title, SIGNAL(changed()), this, SLOT(labelChanged()) );
	addChild(d->title);
	d->title->setHidden(true);
	d->title->graphicsItem()->setParentItem(graphicsItem());
	d->title->graphicsItem()->setFlag(QGraphicsItem::ItemIsMovable, false);
	// TODO: Crash (see bug  #3583420)
	d->title->setText(this->name());
	if ( d->orientation == AxisVertical )
		d->title->setRotationAngle(270);
	d->titleOffset = Worksheet::convertToSceneUnits(5, Worksheet::Point); //distance to the axis tick labels

	d->majorTicksDirection = (Axis::TicksDirection) group.readEntry("MajorTicksDirection", (int) Axis::ticksOut);
	d->majorTicksType = (Axis::TicksType) group.readEntry("MajorTicksType", (int) Axis::TicksTotalNumber);
	d->majorTicksNumber = group.readEntry("MajorTicksNumber", 11);
	d->majorTicksIncrement = group.readEntry("MajorTicksIncrement", 1.0);
	d->majorTicksLength = group.readEntry("MajorTicksLength", Worksheet::convertToSceneUnits(6.0, Worksheet::Point));
	d->majorTicksOpacity = group.readEntry("MajorTicksOpacity", 1.0);
	
	d->minorTicksDirection = (Axis::TicksDirection) group.readEntry("MinorTicksDirection", (int) Axis::ticksOut);
	d->minorTicksType = (Axis::TicksType) group.readEntry("MinorTicksType", (int) Axis::TicksTotalNumber);
	d->minorTicksNumber = group.readEntry("MinorTicksNumber", 1);
	d->minorTicksIncrement = group.readEntry("MinorTicksIncrement", 0.5);
	d->minorTicksLength = group.readEntry("MinorTicksLength", Worksheet::convertToSceneUnits(3.0, Worksheet::Point));
	d->minorTicksOpacity = group.readEntry("MinorTicksOpacity", 1.0);
	
	//Labels
	d->labelsFormat = (Axis::LabelsFormat) group.readEntry("LabelsFormat", (int)Axis::FormatDecimal);
	d->labelsAutoPrecision = group.readEntry("LabelsAutoPrecision", true);
	d->labelsPrecision = group.readEntry("LabelsPrecision", 1);
	d->labelsPosition = (Axis::LabelsPosition) group.readEntry("LabelsPosition", (int) Axis::LabelsOut);
	d->labelsOffset= group.readEntry("LabelsOffset",  Worksheet::convertToSceneUnits( 5.0, Worksheet::Point ));
	d->labelsColor = group.readEntry("LabelsFontColor", QColor(Qt::black));
	//TODO load font and font size from the config
	d->labelsFont.setPointSizeF( Worksheet::convertToSceneUnits( 8.0, Worksheet::Point ) );
	d->labelsRotationAngle = group.readEntry("LabelsRotation", 0);
	d->labelsOpacity = group.readEntry("LabelsOpacity", 1.0);

	//major grid
	d->majorGridPen.setStyle( (Qt::PenStyle) group.readEntry("MajorGridStyle", (int) Qt::NoPen) );
	d->majorGridPen.setColor(Qt::gray);//TODO load from config
	d->majorGridPen.setWidthF( group.readEntry("MajorGridWidth", Worksheet::convertToSceneUnits( 1.0, Worksheet::Point ) ) );
	d->majorGridOpacity = group.readEntry("MajorGridOpacity", 1.0);

	//minor grid
	d->minorGridPen.setStyle( (Qt::PenStyle) group.readEntry("MinorGridStyle", (int) Qt::NoPen) );
	d->minorGridPen.setColor(Qt::gray);//TODO load from config
	d->minorGridPen.setWidthF( group.readEntry("MinorGridWidth", Worksheet::convertToSceneUnits( 1.0, Worksheet::Point ) ) );
	d->minorGridOpacity = group.readEntry("MinorGridOpacity", 1.0);
#endif
	
	retransform();
	graphicsItem()->setFlag(QGraphicsItem::ItemIsSelectable, true);
	graphicsItem()->setFlag(QGraphicsItem::ItemIsFocusable, true);

	this->initActions();
	this->initMenus();
}

/*!
 * For the most frequently edited properties, create Actions and ActionGroups for the context menu.
 * For some ActionGroups the actual actions are created in \c GuiTool, 
 */
void Axis::initActions(){
	//Orientation 
	orientationActionGroup = new QActionGroup(this);
	orientationActionGroup->setExclusive(true);
	connect(orientationActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(orientationChanged(QAction*)));

	orientationHorizontalAction = new QAction(tr("horizontal"), orientationActionGroup);
	orientationHorizontalAction->setCheckable(true);
	
	orientationVerticalAction = new QAction(tr("vertical"), orientationActionGroup);
	orientationVerticalAction->setCheckable(true);
	
	//Line 
	lineStyleActionGroup = new QActionGroup(this);
	lineStyleActionGroup->setExclusive(true);
	connect(lineStyleActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(lineStyleChanged(QAction*)));
	
	lineColorActionGroup = new QActionGroup(this);
	lineColorActionGroup->setExclusive(true);
	connect(lineColorActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(lineColorChanged(QAction*)));
	
	//Ticks 
	//TODO
}

void Axis::initMenus(){
	//Orientation
	orientationMenu = new QMenu(tr("Orientation"));
	orientationMenu->addAction(orientationHorizontalAction);
	orientationMenu->addAction(orientationVerticalAction);
	
	//Line
	lineMenu = new QMenu(tr("Line"));
	lineStyleMenu = new QMenu(tr("style"), lineMenu);
	lineMenu->addMenu( lineStyleMenu );

	lineColorMenu = new QMenu(tr("color"), lineMenu);
	GuiTools::fillColorMenu( lineColorMenu, lineColorActionGroup );
	lineMenu->addMenu( lineColorMenu );
}

QMenu* Axis::createContextMenu(){
	Q_D(Axis);
	QMenu *menu = AbstractWorksheetElement::createContextMenu();
	QAction* firstAction = menu->actions().first();
	
	//Orientation
	if ( d->orientation == AxisHorizontal )
		orientationHorizontalAction->setChecked(true);
	else
		orientationVerticalAction->setChecked(true);
	
	menu->insertMenu(firstAction, orientationMenu);

	//Line styles
	GuiTools::updatePenStyles( lineStyleMenu, lineStyleActionGroup, d->linePen.color() );
	GuiTools::selectPenStyleAction(lineStyleActionGroup, d->linePen.style() );

	GuiTools::selectColorAction(lineColorActionGroup, d->linePen.color() );

	menu->insertMenu(firstAction, lineMenu);
	menu->insertSeparator(firstAction);

	return menu;
}

/*!
	Returns an icon to be used in the project explorer.
*/
QIcon Axis::icon() const{
	Q_D(const Axis);
	QIcon ico;
#ifndef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
	if (d->orientation == Axis::AxisHorizontal)
		ico = KIcon("axis-horizontal");
	else
		ico = KIcon("axis-vertical");
#endif
	return ico;
}

Axis::~Axis() {
	delete orientationMenu;
	delete lineMenu;
	
	//no need to delete d->title, since it was added with addChild in init();

	//no need to delete the d-pointer here - it inherits from QGraphicsItem 
	//and is deleted during the cleanup in QGraphicsScene	
}

QGraphicsItem *Axis::graphicsItem() const {
	return d_ptr;
}

void Axis::retransform() {
	Q_D(Axis);
	d->retransform();
}

void Axis::handlePageResize(double horizontalRatio, double verticalRatio) {
	Q_D(Axis);

	QPen pen = d->linePen;
	pen.setWidthF(pen.widthF() * (horizontalRatio + verticalRatio) / 2.0);
	setLinePen(pen);

	if (d->orientation & Axis::AxisHorizontal) {
		setMajorTicksLength(d->majorTicksLength * verticalRatio); // ticks are perpendicular to axis line -> verticalRatio relevant
		setMinorTicksLength(d->minorTicksLength * verticalRatio);
		//TODO setLabelsFontSize(d->labelsFontSize * verticalRatio);
	} else {
		setMajorTicksLength(d->majorTicksLength * horizontalRatio);
		setMinorTicksLength(d->minorTicksLength * horizontalRatio);
		//TODO setLabelsFontSize(d->labelsFontSize * verticalRatio); // this is not perfectly correct for rotated labels 
															// when the page aspect ratio changes, but should not matter
	}
	//TODO setLabelsOffset(QPointF(d->labelsOffset.x() * horizontalRatio, d->labelsOffset.y() * verticalRatio));

	retransform();
	BaseClass::handlePageResize(horizontalRatio, verticalRatio);
}

void Axis::labelChanged(){
	Q_D(Axis);
	d->recalcShapeAndBoundingRect();
}

/* ============================ accessor documentation ================= */
/**
   \fn Axis::CLASS_D_ACCESSOR_DECL(AxisOrientation, orientation, Orientation);
   \brief Get/set the axis orientation: left, right, bottom, or top (usually not changed after creation).

   The orientation has not much to do with the actual position of the axis, which
   is determined by Axis::Private::offset. It only determines whether the axis
   is horizontal or vertical and which direction means in/out for the ticks.
 */
/**
   \fn Axis::BASIC_D_ACCESSOR_DECL(qreal, offset, Offset);
   \brief Get/set the offset from zero in the directin perpendicular to the axis.
 */
/**
   \fn Axis::BASIC_D_ACCESSOR_DECL(qreal, start, Start);
   \brief Get/set the start coordinate of the axis line.
 */
/**
   \fn Axis::BASIC_D_ACCESSOR_DECL(qreal, end, End);
   \brief Get/set the end coordinate of the axis line.
 */
/**
   \fn Axis::BASIC_D_ACCESSOR_DECL(int, majorTicksNumber, MajorTicksNumber);
   \brief Get/set the number of major ticks.
 */
/**
   \fn Axis::BASIC_D_ACCESSOR_DECL(int, minorTicksNumber, MinorTicksNumber);
   \brief Get/set the number of minor ticks (between each two major ticks).
 */
/**
   \fn Axis::BASIC_D_ACCESSOR_DECL(qreal, majorTicksLength, MajorTicksLength);
   \brief Get/set the major tick length (in page units!).
 */
/**
   \fn Axis::BASIC_D_ACCESSOR_DECL(qreal, minorTicksLength, MinorTicksLength);
   \brief Get/set the minor tick length (in page units!).
 */
/**
   \fn Axis::CLASS_D_ACCESSOR_DECL(TicksDirection, majorTicksDirection, MajorTicksDirection);
   \brief Get/set the major ticks direction: inwards, outwards, both, or none.
 */
/**
   \fn Axis::CLASS_D_ACCESSOR_DECL(TicksDirection, minorTicksDirection, MinorTicksDirection);
   \brief Get/set the minor ticks direction: inwards, outwards, both, or none.
 */
/**
   \fn Axis::BASIC_D_ACCESSOR_DECL(qreal, labelsRotationAngle, LabelRotationAngle);
   \brief Get/set the rotation angle of the tick labels.
 */
/**
   \fn Axis::CLASS_D_ACCESSOR_DECL(QColor, labelsColor, LabelColor);
   \brief Get/set the color of the tick labels.
 */
/**
   \fn Axis::CLASS_D_ACCESSOR_DECL(QFont, labelsFont, LabelFont);
   \brief Get/set the font of the tick labels (size of the QFont will be ignored).
 */
/**
   \fn Axis::CLASS_D_ACCESSOR_DECL(float, labelsOffset, LabelOffset);
   \brief Get/set the position offset of the tick labels relative to the end of the tick line.
 */
/**
   \fn Axis::CLASS_D_ACCESSOR_DECL(QPen, pen, Pen);
   \brief Get/set the pen for the lines.
 */

/* ============================ getter methods ================= */
BASIC_SHARED_D_READER_IMPL(Axis, bool, autoScale, autoScale)
BASIC_SHARED_D_READER_IMPL(Axis, Axis::AxisOrientation, orientation, orientation)
BASIC_SHARED_D_READER_IMPL(Axis, Axis::AxisPosition, position, position)
BASIC_SHARED_D_READER_IMPL(Axis, Axis::AxisScale, scale, scale)
BASIC_SHARED_D_READER_IMPL(Axis, float, offset, offset)
BASIC_SHARED_D_READER_IMPL(Axis, float, start, start)
BASIC_SHARED_D_READER_IMPL(Axis, float, end, end)
BASIC_SHARED_D_READER_IMPL(Axis, qreal, scalingFactor, scalingFactor)
BASIC_SHARED_D_READER_IMPL(Axis, qreal, zeroOffset, zeroOffset)

BASIC_SHARED_D_READER_IMPL(Axis, TextLabel*, title, title)
BASIC_SHARED_D_READER_IMPL(Axis, float, titleOffset, titleOffset)

CLASS_SHARED_D_READER_IMPL(Axis, QPen, linePen, linePen)
BASIC_SHARED_D_READER_IMPL(Axis, qreal, lineOpacity, lineOpacity)

BASIC_SHARED_D_READER_IMPL(Axis, Axis::TicksDirection, majorTicksDirection, majorTicksDirection)
BASIC_SHARED_D_READER_IMPL(Axis, Axis::TicksType, majorTicksType, majorTicksType)
BASIC_SHARED_D_READER_IMPL(Axis, int, majorTicksNumber, majorTicksNumber)
BASIC_SHARED_D_READER_IMPL(Axis, qreal, majorTicksIncrement, majorTicksIncrement)
BASIC_SHARED_D_READER_IMPL(Axis, qreal, majorTicksLength, majorTicksLength)
CLASS_SHARED_D_READER_IMPL(Axis, QPen, majorTicksPen, majorTicksPen)
BASIC_SHARED_D_READER_IMPL(Axis, qreal, majorTicksOpacity, majorTicksOpacity)

BASIC_SHARED_D_READER_IMPL(Axis, Axis::TicksDirection, minorTicksDirection, minorTicksDirection)
BASIC_SHARED_D_READER_IMPL(Axis, Axis::TicksType, minorTicksType, minorTicksType)
BASIC_SHARED_D_READER_IMPL(Axis, int, minorTicksNumber, minorTicksNumber)
BASIC_SHARED_D_READER_IMPL(Axis, qreal, minorTicksIncrement, minorTicksIncrement)
BASIC_SHARED_D_READER_IMPL(Axis, qreal, minorTicksLength, minorTicksLength)
CLASS_SHARED_D_READER_IMPL(Axis, QPen, minorTicksPen, minorTicksPen)
BASIC_SHARED_D_READER_IMPL(Axis, qreal, minorTicksOpacity, minorTicksOpacity)

BASIC_SHARED_D_READER_IMPL(Axis, Axis::LabelsFormat, labelsFormat, labelsFormat);
BASIC_SHARED_D_READER_IMPL(Axis, bool, labelsAutoPrecision, labelsAutoPrecision);
BASIC_SHARED_D_READER_IMPL(Axis, int, labelsPrecision, labelsPrecision);
BASIC_SHARED_D_READER_IMPL(Axis, Axis::LabelsPosition, labelsPosition, labelsPosition);
BASIC_SHARED_D_READER_IMPL(Axis, float, labelsOffset, labelsOffset);
BASIC_SHARED_D_READER_IMPL(Axis, qreal, labelsRotationAngle, labelsRotationAngle);
CLASS_SHARED_D_READER_IMPL(Axis, QColor, labelsColor, labelsColor);
CLASS_SHARED_D_READER_IMPL(Axis, QFont, labelsFont, labelsFont);
CLASS_SHARED_D_READER_IMPL(Axis, QString, labelsPrefix, labelsPrefix);
CLASS_SHARED_D_READER_IMPL(Axis, QString, labelsSuffix, labelsSuffix);
BASIC_SHARED_D_READER_IMPL(Axis, qreal, labelsOpacity, labelsOpacity);

CLASS_SHARED_D_READER_IMPL(Axis, QPen, majorGridPen, majorGridPen)
BASIC_SHARED_D_READER_IMPL(Axis, qreal, majorGridOpacity, majorGridOpacity)
CLASS_SHARED_D_READER_IMPL(Axis, QPen, minorGridPen, minorGridPen)
BASIC_SHARED_D_READER_IMPL(Axis, qreal, minorGridOpacity, minorGridOpacity)

/* ============================ setter methods and undo commands ================= */
STD_SETTER_CMD_IMPL_F_S(Axis, SetAutoScale, bool, autoScale, retransform);
void Axis::setAutoScale(bool autoScale) {
	Q_D(Axis);
	if (autoScale != d->autoScale){
		exec(new AxisSetAutoScaleCmd(d, autoScale, tr("%1: set axis auto scaling")));
		
		if (autoScale){
			CartesianPlot *plot = qobject_cast<CartesianPlot*>(parentAspect());
			if (!plot)
				return;
			
			if (d->orientation == Axis::AxisHorizontal){
				d->end = plot->xMax();
				d->start = plot->xMin();
			}else{
				d->end = plot->yMax();
				d->start = plot->yMin();
			}
			retransform();
			emit endChanged(d->end);
			emit startChanged(d->start);
		}
	}
}

STD_SWAP_METHOD_SETTER_CMD_IMPL(Axis, SetVisible, bool, swapVisible);
void Axis::setVisible(bool on) {
	Q_D(Axis);
	exec(new AxisSetVisibleCmd(d, on, on ? tr("%1: set visible") : tr("%1: set invisible")));
}

bool Axis::isVisible() const {
	Q_D(const Axis);
	return d->isVisible();
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetOrientation, Axis::AxisOrientation, orientation, retransform);
void Axis::setOrientation( AxisOrientation orientation) {
	Q_D(Axis);
	if (orientation != d->orientation)
		exec(new AxisSetOrientationCmd(d, orientation, tr("%1: set axis orientation")));
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetPosition, Axis::AxisPosition, position, retransform);
void Axis::setPosition(AxisPosition position) {
	Q_D(Axis);
	if (position != d->position)
		exec(new AxisSetPositionCmd(d, position, tr("%1: set axis position")));
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetScaling, Axis::AxisScale, scale, retransform);
void Axis::setScale(AxisScale scale) {
	Q_D(Axis);
	if (scale != d->scale)
		exec(new AxisSetScalingCmd(d, scale, tr("%1: set axis scale")));
}

STD_SETTER_CMD_IMPL_F(Axis, SetOffset, float, offset, retransform);
void Axis::setOffset(float offset, bool undo) {
	Q_D(Axis);
	if (offset != d->offset){
		if (undo){
			exec(new AxisSetOffsetCmd(d, offset, tr("%1: set axis offset")));
		}else{
			d->offset = offset;
			//don't need to call retransform() afterward 
			//since the only usage of this call is in CartesianPlot, where retransform is called for all children anyway.
		}
		emit positionChanged(offset);
	}
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetStart, float, start, retransform);
void Axis::setStart(float start, bool undo) {
	Q_D(Axis);
	if (start != d->start){
		if (undo)
			exec(new AxisSetStartCmd(d, start, tr("%1: set axis start")));
		else
			d->start = start;
			
		emit startChanged(start);
	}
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetEnd, float, end, retransform);
void Axis::setEnd(float end, bool undo) {
	Q_D(Axis);
	if (end != d->end){
		if (undo)
			exec(new AxisSetEndCmd(d, end, tr("%1: set axis end")));
		else
			d->end = end;
		emit endChanged(end);
	}
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetZeroOffset, qreal, zeroOffset, retransform);
void Axis::setZeroOffset(qreal zeroOffset) {
	Q_D(Axis);
	if (zeroOffset != d->zeroOffset)
		exec(new AxisSetZeroOffsetCmd(d, zeroOffset, tr("%1: set axis zero offset")));
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetScalingFactor, qreal, scalingFactor, retransform);
void Axis::setScalingFactor(qreal scalingFactor) {
	Q_D(Axis);
	if (scalingFactor != d->scalingFactor)
		exec(new AxisSetScalingFactorCmd(d, scalingFactor, tr("%1: set axis scaling factor")));
}

//Title
STD_SETTER_CMD_IMPL_F_S(Axis, SetTitleOffset, float, titleOffset, retransform);
void Axis::setTitleOffset(float offset) {
	Q_D(Axis);
	if (offset != d->titleOffset)
		exec(new AxisSetTitleOffsetCmd(d, offset, tr("%1: set title offset")));
}

//Line
STD_SETTER_CMD_IMPL_F_S(Axis, SetLinePen, QPen, linePen, recalcShapeAndBoundingRect);
void Axis::setLinePen(const QPen &pen) {
	Q_D(Axis);
	if (pen != d->linePen)
		exec(new AxisSetLinePenCmd(d, pen, tr("%1: set line style")));
}

STD_SETTER_CMD_IMPL_F_S(Axis, SetLineOpacity, qreal, lineOpacity, update);
void Axis::setLineOpacity(qreal opacity){
	Q_D(Axis);
	if (opacity != d->lineOpacity)
		exec(new AxisSetLineOpacityCmd(d, opacity, tr("%1: set line opacity")));
}

//TODO undo-functions
//Major ticks
STD_SETTER_CMD_IMPL_F(Axis, SetMajorTicksDirection, Axis::TicksDirection, majorTicksDirection, retransformTicks);
void Axis::setMajorTicksDirection(const TicksDirection majorTicksDirection) {
	Q_D(Axis);
	if (majorTicksDirection != d->majorTicksDirection)
		exec(new AxisSetMajorTicksDirectionCmd(d, majorTicksDirection, tr("%1: set major ticks direction")));
}

STD_SETTER_CMD_IMPL_F(Axis, SetMajorTicksType, Axis::TicksType, majorTicksType, retransformTicks);
void Axis::setMajorTicksType(const TicksType majorTicksType) {
	Q_D(Axis);
	if (majorTicksType!= d->majorTicksType)
		exec(new AxisSetMajorTicksTypeCmd(d, majorTicksType, tr("%1: set major ticks type")));
}

STD_SETTER_CMD_IMPL_F(Axis, SetMajorTicksNumber, int, majorTicksNumber, retransformTicks);
void Axis::setMajorTicksNumber(int majorTicksNumber) {
	Q_D(Axis);
	if (majorTicksNumber != d->majorTicksNumber)
		exec(new AxisSetMajorTicksNumberCmd(d, majorTicksNumber, tr("%1: set the total number of the major ticks")));
}

STD_SETTER_CMD_IMPL_F(Axis, SetMajorTicksIncrement, qreal, majorTicksIncrement, retransformTicks);
void Axis::setMajorTicksIncrement(qreal majorTicksIncrement) {
	Q_D(Axis);
	if (majorTicksIncrement != d->majorTicksIncrement)
		exec(new AxisSetMajorTicksIncrementCmd(d, majorTicksIncrement, tr("%1: set the increment for the major ticks")));
}

STD_SETTER_CMD_IMPL_F(Axis, SetMajorTicksPen, QPen, majorTicksPen, recalcShapeAndBoundingRect);
void Axis::setMajorTicksPen(const QPen &pen) {
	Q_D(Axis);
	if (pen != d->majorTicksPen)
		exec(new AxisSetMajorTicksPenCmd(d, pen, tr("%1: set major ticks style")));
}

STD_SETTER_CMD_IMPL_F(Axis, SetMajorTicksLength, qreal, majorTicksLength, retransformTicks);
void Axis::setMajorTicksLength(qreal majorTicksLength) {
	Q_D(Axis);
	if (majorTicksLength != d->majorTicksLength)
		exec(new AxisSetMajorTicksLengthCmd(d, majorTicksLength, tr("%1: set major ticks length")));
}

STD_SETTER_CMD_IMPL_F(Axis, SetMajorTicksOpacity, qreal, majorTicksOpacity, update);
void Axis::setMajorTicksOpacity(qreal opacity){
	Q_D(Axis);
	if (opacity != d->majorTicksOpacity)
		exec(new AxisSetMajorTicksOpacityCmd(d, opacity, tr("%1: set major ticks opacity")));
}

//Minor ticks
STD_SETTER_CMD_IMPL_F(Axis, SetMinorTicksDirection, Axis::TicksDirection, minorTicksDirection, retransformTicks);
void Axis::setMinorTicksDirection(const TicksDirection minorTicksDirection) {
	Q_D(Axis);
	if (minorTicksDirection != d->minorTicksDirection)
		exec(new AxisSetMinorTicksDirectionCmd(d, minorTicksDirection, tr("%1: set minor ticks direction")));
}

STD_SETTER_CMD_IMPL_F(Axis, SetMinorTicksType, Axis::TicksType, minorTicksType, retransformTicks);
void Axis::setMinorTicksType(const TicksType minorTicksType) {
	Q_D(Axis);
	if (minorTicksType!= d->minorTicksType)
		exec(new AxisSetMinorTicksTypeCmd(d, minorTicksType, tr("%1: set minor ticks type")));
}

STD_SETTER_CMD_IMPL_F(Axis, SetMinorTicksNumber, int, minorTicksNumber, retransformTicks);
void Axis::setMinorTicksNumber(int minorTicksNumber) {
	Q_D(Axis);
	if (minorTicksNumber != d->minorTicksNumber)
		exec(new AxisSetMinorTicksNumberCmd(d, minorTicksNumber, tr("%1: set the total number of the minor ticks")));
}

STD_SETTER_CMD_IMPL_F(Axis, SetMinorTicksIncrement, qreal, minorTicksIncrement, retransformTicks);
void Axis::setMinorTicksIncrement(qreal minorTicksIncrement) {
	Q_D(Axis);
	if (minorTicksIncrement != d->minorTicksIncrement)
		exec(new AxisSetMinorTicksIncrementCmd(d, minorTicksIncrement, tr("%1: set the increment for the minor ticks")));
}

STD_SETTER_CMD_IMPL_F(Axis, SetMinorTicksPen, QPen, minorTicksPen, recalcShapeAndBoundingRect);
void Axis::setMinorTicksPen(const QPen &pen) {
	Q_D(Axis);
	if (pen != d->minorTicksPen)
		exec(new AxisSetMinorTicksPenCmd(d, pen, tr("%1: set minor ticks style")));
}

STD_SETTER_CMD_IMPL_F(Axis, SetMinorTicksLength, qreal, minorTicksLength, retransformTicks);
void Axis::setMinorTicksLength(qreal minorTicksLength) {
	Q_D(Axis);
	if (minorTicksLength != d->minorTicksLength)
		exec(new AxisSetMinorTicksLengthCmd(d, minorTicksLength, tr("%1: set minor ticks length")));
}

STD_SETTER_CMD_IMPL_F(Axis, SetMinorTicksOpacity, qreal, minorTicksOpacity, update);
void Axis::setMinorTicksOpacity(qreal opacity){
	Q_D(Axis);
	if (opacity != d->minorTicksOpacity)
		exec(new AxisSetMinorTicksOpacityCmd(d, opacity, tr("%1: set minor ticks opacity")));
}

//Labels
STD_SETTER_CMD_IMPL_F(Axis, SetLabelsFormat, Axis::LabelsFormat, labelsFormat, retransformTicks);
void Axis::setLabelsFormat(const LabelsFormat labelsFormat){
	Q_D(Axis);
	if (labelsFormat != d->labelsFormat)
		exec(new AxisSetLabelsFormatCmd(d, labelsFormat, tr("%1: set labels format")));
}

STD_SETTER_CMD_IMPL_F(Axis, SetLabelsAutoPrecision, bool, labelsAutoPrecision, retransformTickLabelStrings);
void Axis::setLabelsAutoPrecision(const bool labelsAutoPrecision){
	Q_D(Axis);
	if (labelsAutoPrecision != d->labelsAutoPrecision)
		exec(new AxisSetLabelsAutoPrecisionCmd(d, labelsAutoPrecision, tr("%1: set labels precision")));
}

STD_SETTER_CMD_IMPL_F(Axis, SetLabelsPrecision, int, labelsPrecision, retransformTickLabelStrings);
void Axis::setLabelsPrecision(const int labelsPrecision){
	Q_D(Axis);
	if (labelsPrecision != d->labelsPrecision)
		exec(new AxisSetLabelsPrecisionCmd(d, labelsPrecision, tr("%1: set labels precision")));
}

STD_SETTER_CMD_IMPL_F(Axis, SetLabelsPosition, Axis::LabelsPosition, labelsPosition, retransformTickLabels);
void Axis::setLabelsPosition(const LabelsPosition labelsPosition) {
	Q_D(Axis);
	if (labelsPosition != d->labelsPosition)
		exec(new AxisSetLabelsPositionCmd(d, labelsPosition, tr("%1: set labels position")));
}

STD_SETTER_CMD_IMPL_F(Axis, SetLabelsOffset, float, labelsOffset, retransformTickLabels);
void Axis::setLabelsOffset(float offset) {
	Q_D(Axis);
	if (offset != d->labelsOffset)
		exec(new AxisSetLabelsOffsetCmd(d, offset, tr("%1: set label offset")));
}

STD_SETTER_CMD_IMPL_F(Axis, SetLabelsRotationAngle, qreal, labelsRotationAngle, retransformTickLabels);
void Axis::setLabelsRotationAngle(qreal angle) {
	Q_D(Axis);
	if (angle != d->labelsRotationAngle)
		exec(new AxisSetLabelsRotationAngleCmd(d, angle, tr("%1: set label rotation angle")));
}

STD_SETTER_CMD_IMPL_F(Axis, SetLabelsColor, QColor, labelsColor, update);
void Axis::setLabelsColor(const QColor &color) {
	Q_D(Axis);
	if (color != d->labelsColor)
		exec(new AxisSetLabelsColorCmd(d, color, tr("%1: set label color")));
}

STD_SETTER_CMD_IMPL_F(Axis, SetLabelsFont, QFont, labelsFont, retransformTickLabels);
void Axis::setLabelsFont(const QFont &font) {
	Q_D(Axis);
	if (font != d->labelsFont)
		exec(new AxisSetLabelsFontCmd(d, font, tr("%1: set label font")));
}

STD_SETTER_CMD_IMPL_F(Axis, SetLabelsPrefix, QString, labelsPrefix, retransformTickLabels);
void Axis::setLabelsPrefix(const QString& prefix) {
	Q_D(Axis);
	if (prefix != d->labelsPrefix)
		exec(new AxisSetLabelsPrefixCmd(d, prefix, tr("%1: set label prefix")));
}

STD_SETTER_CMD_IMPL_F(Axis, SetLabelsSuffix, QString, labelsSuffix, retransformTickLabels);
void Axis::setLabelsSuffix(const QString& suffix) {
	Q_D(Axis);
	if (suffix != d->labelsSuffix)
		exec(new AxisSetLabelsSuffixCmd(d, suffix, tr("%1: set label suffix")));
}

STD_SETTER_CMD_IMPL_F(Axis, SetLabelsOpacity, qreal, labelsOpacity, update);
void Axis::setLabelsOpacity(qreal opacity){
	Q_D(Axis);
	if (opacity != d->labelsOpacity)
		exec(new AxisSetLabelsOpacityCmd(d, opacity, tr("%1: set labels opacity")));
}

//Major grid
STD_SETTER_CMD_IMPL_F(Axis, SetMajorGridPen, QPen, majorGridPen, retransformMajorGrid);
void Axis::setMajorGridPen(const QPen &pen) {
	Q_D(Axis);
	if (pen != d->majorGridPen)
		exec(new AxisSetMajorGridPenCmd(d, pen, tr("%1: set major grid style")));
}

STD_SETTER_CMD_IMPL_F(Axis, SetMajorGridOpacity, qreal, majorGridOpacity, update);
void Axis::setMajorGridOpacity(qreal opacity){
	Q_D(Axis);
	if (opacity != d->majorGridOpacity)
		exec(new AxisSetMajorGridOpacityCmd(d, opacity, tr("%1: set major grid opacity")));
}

//Minor grid
STD_SETTER_CMD_IMPL_F(Axis, SetMinorGridPen, QPen, minorGridPen, retransformMinorGrid);
void Axis::setMinorGridPen(const QPen &pen){
	Q_D(Axis);
	if (pen != d->minorGridPen)
		exec(new AxisSetMinorGridPenCmd(d, pen, tr("%1: set minor grid style")));
}

STD_SETTER_CMD_IMPL_F(Axis, SetMinorGridOpacity, qreal, minorGridOpacity, update);
void Axis::setMinorGridOpacity(qreal opacity){
	Q_D(Axis);
	if (opacity != d->minorGridOpacity)
		exec(new AxisSetMinorGridOpacityCmd(d, opacity, tr("%1: set minor grid opacity")));
}

//##############################################################################
//######  SLOTs for changes triggered via QActions in the context menu  ########
//##############################################################################
void Axis::orientationChanged(QAction* action){
	if (action == orientationHorizontalAction)
		this->setOrientation(AxisHorizontal);
	else
		this->setOrientation(AxisVertical);
}

void Axis::lineStyleChanged(QAction* action){
	Q_D(Axis);
	QPen pen = d->linePen;
	pen.setStyle(GuiTools::penStyleFromAction(lineStyleActionGroup, action));
	this->setLinePen(pen);
}

void Axis::lineColorChanged(QAction* action){
	Q_D(Axis);
	QPen pen = d->linePen;
	pen.setColor(GuiTools::colorFromAction(lineColorActionGroup, action));
	this->setLinePen(pen);
}

//#####################################################################
//################### Private implementation ##########################
//#####################################################################
AxisPrivate::AxisPrivate(Axis *owner) : m_plot(0), m_cSystem(0), q(owner){
}

QString AxisPrivate::name() const{
	return q->name();
}

bool AxisPrivate::swapVisible(bool on) {
	bool oldValue = isVisible();
	setVisible(on);
        emit q->visibleChanged(on);
	return oldValue;
}

QRectF AxisPrivate::boundingRect() const{
	return boundingRectangle;
}

/*!
  Returns the shape of the XYCurve as a QPainterPath in local coordinates
*/
QPainterPath AxisPrivate::shape() const{
	return axisShape;
}

/*!
	recalculates the position of the axis on the worksheet
 */
void AxisPrivate::retransform(){
	m_plot = qobject_cast<CartesianPlot*>(q->parentAspect());
	if (!m_plot)
		return;

	m_cSystem = dynamic_cast<const CartesianCoordinateSystem*>(m_plot->coordinateSystem());
	if (!m_cSystem)
		return;
	
	retransformLine();
	retransformTicks();
}

void AxisPrivate::retransformLine(){
	linePath = QPainterPath();

	QList<QLineF> lines;
	QPointF startPoint;
	QPointF endPoint;

	if (orientation == Axis::AxisHorizontal){
		if (position == Axis::AxisTop)
			offset = m_plot->yMax();
		else if (position == Axis::AxisBottom)
			offset = m_plot->yMin();

		startPoint.setX(start);
		startPoint.setY(offset);
		endPoint.setX(end);
		endPoint.setY(offset);
	} else { // vertical
		if (position == Axis::AxisLeft)
			offset = m_plot->xMin();
		else if (position == Axis::AxisRight)
			offset = m_plot->xMax();

		startPoint.setX(offset);
		startPoint.setY(start);
		endPoint.setY(end);
		endPoint.setX(offset);
	}

	lines.append(QLineF(startPoint, endPoint));
	lines = m_cSystem->mapLogicalToScene(lines, AbstractCoordinateSystem::MarkGaps);
	foreach (QLineF line, lines) {
		linePath.moveTo(line.p1());
		linePath.lineTo(line.p2());
	}
}

//! helper function for retransformTicks(const AbstractCoordinateSystem *cSystem)
//TODO refactor this function, coordinate system should provide a function to map a single point
bool AxisPrivate::transformAnchor(QPointF *anchorPoint) {
	QList<QPointF> points;
	points.append(*anchorPoint);
	points = m_cSystem->mapLogicalToScene(points, AbstractCoordinateSystem::SuppressPageClipping);
	if (points.count() != 1){ // point is not mappable or in a coordinate gap
		return false;
	}else{
		*anchorPoint = points.at(0);
		return true;
	}
}

/*!
	recalculates the position of the axis ticks.
 */ 
void AxisPrivate::retransformTicks(){
	//TODO: check that start and end are > 0 for log and >=0 for sqrt, etc.

	majorTicksPath = QPainterPath();
	minorTicksPath = QPainterPath();
	majorTickPoints.clear();
	minorTickPoints.clear();
	tickLabelValues.clear();
  
  if (majorTicksNumber<1 || (majorTicksDirection == Axis::noTicks && minorTicksDirection == Axis::noTicks) ){
	retransformTickLabels(); //this calls recalcShapeAndBoundingRect()
	return;
  }
  
  //determine the spacing for the major ticks
  double majorTicksSpacing;
  int tmpMajorTicksNumber;
  if (majorTicksType == Axis::TicksTotalNumber){
	//the total number of the major ticks is given - > determine the spacing
	  switch (scale){
		case Axis::ScaleLinear:
		  majorTicksSpacing = (end - start)/ (majorTicksNumber - 1);
		  break;
		case Axis::ScaleLog10:
		  majorTicksSpacing = (log10(end) - log10(start))/(majorTicksNumber - 1);
		  break;
		case Axis::ScaleLog2:
		  majorTicksSpacing = (log(end) - log(start))/log(2)/(majorTicksNumber - 1);
		  break;
		case Axis::ScaleLn:
		  majorTicksSpacing = (log(end) - log(start))/(majorTicksNumber - 1);
		  break;		
		case Axis::ScaleSqrt:
		  majorTicksSpacing = (sqrt(end) - sqrt(start))/(majorTicksNumber - 1);
		  break;					
		case Axis::ScaleX2:
		  majorTicksSpacing = (pow(end,2) - pow(start,2))/(majorTicksNumber - 1);
		  break;			
		default://Linear
		  majorTicksSpacing = (end - start)/(majorTicksNumber - 1);
	  }
	  tmpMajorTicksNumber = majorTicksNumber;
  }else{
	//the spacing (increment ) of the major ticks is given - > determine the number
	majorTicksSpacing = majorTicksIncrement;
	switch (scale){
	  case Axis::ScaleLinear:
		tmpMajorTicksNumber = (end-start)/majorTicksSpacing + 1;
		break;
	  case Axis::ScaleLog10:
		tmpMajorTicksNumber = (log10(end)-log10(start))/majorTicksSpacing + 1;
		break;
	  case Axis::ScaleLog2:
		tmpMajorTicksNumber = (log(end)-log(start))/log(2)/majorTicksSpacing + 1;
		break;
	  case Axis::ScaleLn:
		tmpMajorTicksNumber = (log(end)-log(start))/majorTicksSpacing + 1;
		break;		
	  case Axis::ScaleSqrt:
		tmpMajorTicksNumber = (sqrt(end)-sqrt(start))/majorTicksSpacing + 1;
		break;					
	  case Axis::ScaleX2:
		tmpMajorTicksNumber = (pow(end,2)-pow(start,2))/majorTicksSpacing + 1;
		break;			
	  default://Linear
		tmpMajorTicksNumber = (end-start)/majorTicksSpacing + 1;
	}
  }
  
  int tmpMinorTicksNumber;
	if (minorTicksType == Axis::TicksTotalNumber){
	  tmpMinorTicksNumber = minorTicksNumber;
  }else{
	  tmpMinorTicksNumber = (end - start)/ (majorTicksNumber - 1)/minorTicksIncrement - 1;
  }
  
	QPointF anchorPoint;
	QPointF startPoint;
	QPointF endPoint;
	qreal majorTickPos;
	qreal minorTickPos;
	qreal nextMajorTickPos;
	int xDirection = m_cSystem->xDirection();
	int yDirection = m_cSystem->yDirection();
	float middleX = (m_plot->xMax() - m_plot->xMin())/2;
	float middleY = (m_plot->yMax() - m_plot->yMin())/2;

	for (int iMajor = 0; iMajor < tmpMajorTicksNumber; iMajor++){
	  switch (scale){
		case Axis::ScaleLinear:
		  majorTickPos = start + majorTicksSpacing * (qreal)iMajor;
		  break;
		case Axis::ScaleLog10:
		  majorTickPos = pow(10, log10(start) + majorTicksSpacing * (qreal)iMajor);
		  break;
		case Axis::ScaleLog2:
		  majorTickPos = pow(2, log(start)/log(2) + majorTicksSpacing * (qreal)iMajor);
		  break;
		case Axis::ScaleLn:
		  majorTickPos = exp(log(start) + majorTicksSpacing * (qreal)iMajor);
		  break;
		case Axis::ScaleSqrt:
		  majorTickPos = pow(sqrt(start) + majorTicksSpacing * (qreal)iMajor, 2);
		  break;
		case Axis::ScaleX2:
		  majorTickPos = sqrt(sqrt(start) + majorTicksSpacing * (qreal)iMajor);
		  break;		  
		default://Linear
		  majorTickPos = start + majorTicksSpacing * (qreal)iMajor; 
	  }
  
	  if (majorTicksDirection != Axis::noTicks ){
		  if (orientation == Axis::AxisHorizontal){
			  anchorPoint.setX(majorTickPos);
			  anchorPoint.setY(offset);
			  
			  if (transformAnchor(&anchorPoint)){
				if(offset < middleY) {
					startPoint = anchorPoint + QPointF(0, (majorTicksDirection & Axis::ticksIn)  ? yDirection * majorTicksLength  : 0);
					endPoint   = anchorPoint + QPointF(0, (majorTicksDirection & Axis::ticksOut) ? -yDirection * majorTicksLength : 0);
				}
				else {
					startPoint = anchorPoint + QPointF(0, (majorTicksDirection & Axis::ticksOut)  ? yDirection * majorTicksLength  : 0);
					endPoint   = anchorPoint + QPointF(0, (majorTicksDirection & Axis::ticksIn) ? -yDirection * majorTicksLength : 0);
				}
			}
		  }else{ // vertical
			  anchorPoint.setY(majorTickPos);
			  anchorPoint.setX(offset);

			  if (transformAnchor(&anchorPoint)){
				if(offset < middleX) {
					startPoint = anchorPoint + QPointF((majorTicksDirection & Axis::ticksIn)  ? xDirection * majorTicksLength  : 0, 0);
					endPoint = anchorPoint + QPointF((majorTicksDirection & Axis::ticksOut) ? -xDirection * majorTicksLength : 0, 0);
				  }
				  else {
					startPoint = anchorPoint + QPointF((majorTicksDirection & Axis::ticksOut) ? xDirection * majorTicksLength : 0, 0);
					endPoint = anchorPoint + QPointF((majorTicksDirection & Axis::ticksIn)  ? -xDirection *  majorTicksLength  : 0, 0);
				  }
			  }
		  }
		  
		majorTicksPath.moveTo(startPoint);
		majorTicksPath.lineTo(endPoint);
		majorTickPoints << anchorPoint;
		
		//Tick-labels
		tickLabelValues<< scalingFactor*majorTickPos+zeroOffset;
	  }

	  //minor ticks
	  if ((Axis::noTicks != minorTicksDirection) && (tmpMajorTicksNumber > 1) && (tmpMinorTicksNumber > 0) && (iMajor < tmpMajorTicksNumber - 1)) {
		  for (int iMinor = 0; iMinor < tmpMinorTicksNumber; iMinor++) {
			switch (scale){
			  case Axis::ScaleLinear:
				minorTickPos = majorTickPos + (qreal)(iMinor + 1) * majorTicksSpacing / (qreal)(tmpMinorTicksNumber + 1);
				break;
			  case Axis::ScaleLog10:
				nextMajorTickPos = start + pow(10, majorTicksSpacing * (qreal)(iMajor + 1));
				minorTickPos = majorTickPos + (qreal)(iMinor + 1) * (nextMajorTickPos - majorTickPos) / (qreal)(tmpMinorTicksNumber + 1);
				break;
			  case Axis::ScaleLog2:
				nextMajorTickPos = start + pow(2, majorTicksSpacing * (qreal)(iMajor + 1));
				minorTickPos = majorTickPos + (qreal)(iMinor + 1) * (nextMajorTickPos - majorTickPos) / (qreal)(tmpMinorTicksNumber + 1);
				break;
			  case Axis::ScaleLn:
				nextMajorTickPos = start + exp(majorTicksSpacing * (qreal)(iMajor + 1));
				minorTickPos = majorTickPos + (qreal)(iMinor + 1) * (nextMajorTickPos - majorTickPos) / (qreal)(tmpMinorTicksNumber + 1);
				break;
			  case Axis::ScaleSqrt:
				nextMajorTickPos = start + pow(majorTicksSpacing * (qreal)(iMajor + 1), 2);
				minorTickPos = majorTickPos + (qreal)(iMinor + 1) * (nextMajorTickPos - majorTickPos) / (qreal)(tmpMinorTicksNumber + 1);
				break;
			  case Axis::ScaleX2:
				nextMajorTickPos = start + sqrt(majorTicksSpacing * (qreal)(iMajor + 1));
				minorTickPos = majorTickPos + (qreal)(iMinor + 1) * (nextMajorTickPos - majorTickPos) / (qreal)(tmpMinorTicksNumber + 1);
				break;
			  default://Linear
				minorTickPos = majorTickPos + (qreal)(iMinor + 1) * majorTicksSpacing / (qreal)(tmpMinorTicksNumber + 1);
			}
			  
			if (orientation == Axis::AxisHorizontal){
				anchorPoint.setX(minorTickPos);
				anchorPoint.setY(offset);

				if (transformAnchor(&anchorPoint)){
					if(offset < middleY) {
						startPoint = anchorPoint + QPointF(0, (minorTicksDirection & Axis::ticksIn)  ? yDirection * minorTicksLength  : 0);
						endPoint   = anchorPoint + QPointF(0, (minorTicksDirection & Axis::ticksOut) ? -yDirection * minorTicksLength : 0);
					}
					else {
						startPoint = anchorPoint + QPointF(0, (minorTicksDirection & Axis::ticksOut)  ? yDirection * minorTicksLength  : 0);
						endPoint   = anchorPoint + QPointF(0, (minorTicksDirection & Axis::ticksIn) ? -yDirection * minorTicksLength : 0);
					}
				}
			}else{ // vertical
				anchorPoint.setY(minorTickPos);
				anchorPoint.setX(offset);

				if (transformAnchor(&anchorPoint)){
				if(offset < middleX) {
					startPoint = anchorPoint + QPointF((minorTicksDirection & Axis::ticksIn)  ? xDirection * minorTicksLength  : 0, 0);
				  	endPoint   = anchorPoint + QPointF((minorTicksDirection & Axis::ticksOut) ? -xDirection * minorTicksLength : 0, 0);
				}
				else {
					startPoint = anchorPoint + QPointF((minorTicksDirection & Axis::ticksOut)  ? xDirection * minorTicksLength  : 0, 0);
				  	endPoint   = anchorPoint + QPointF((minorTicksDirection & Axis::ticksIn) ? -xDirection * minorTicksLength : 0, 0);
				}
				}
			}
			
			minorTicksPath.moveTo(startPoint);
			minorTicksPath.lineTo(endPoint);
			minorTickPoints << anchorPoint;
		  }
	  }
	}

	//tick positions where changed -> update the position of the tick labels and grid lines
	retransformTickLabelStrings();
	retransformMajorGrid();
	retransformMinorGrid();
}

/*!
	creates the tick label strings starting with the most optimal
	(=the smallest possible number of float digits) precision for the floats
*/
void AxisPrivate::retransformTickLabelStrings(){
	if (labelsAutoPrecision){
		//check, whether we need to increase the current precision
		int newPrecision = upperLabelsPrecision(labelsPrecision);
		if (newPrecision!= labelsPrecision){
			labelsPrecision = newPrecision;
			emit q->labelsPrecisionChanged(labelsPrecision);
		}else{
			//check, whether we can reduce the current precision
			newPrecision = lowerLabelsPrecision(labelsPrecision);
			if (newPrecision!= labelsPrecision){
				labelsPrecision = newPrecision;
				emit q->labelsPrecisionChanged(labelsPrecision);
			}
		}
	}

	char format;
	if (labelsFormat == Axis::FormatDecimal)
		format = 'f';
	else
		format = 'e';

	tickLabelStrings.clear();
	foreach(float value, tickLabelValues)
		tickLabelStrings << QString::number(value, format, labelsPrecision);	
	
	retransformTickLabels();
}

/*!
	returns the smalles upper limit for the precision
	where no duplicates for the tick label float occur.
 */
int AxisPrivate::upperLabelsPrecision(int precision){
	//round float to the current precision and look for duplicates.
	//if there are duplicates, increase the precision.
	QList<float> tempValues;
	for (int i=0; i<tickLabelValues.size(); ++i){
		tempValues.append( round(tickLabelValues[i], precision) );
	}
	
	for (int i=0; i<tempValues.size(); ++i){
		if (tempValues.count(tempValues[i]) > 1){
			//duplicate for the current precision found, increase the precision and check again
			return upperLabelsPrecision(precision+1);
		}
	}
	
	//no duplicates for the current precision found, return the current value
	return precision;
}

/*!
	returns highest lower limit for the precision
	where no duplicates for the tick label float occur.
*/
int AxisPrivate::lowerLabelsPrecision(int precision){
	if (precision == 0)
		return 0;

	//round float to the current precision and look for duplicates.
	//if there are duplicates, decrease the precision.
	QList<float> tempValues;
	for (int i=0; i<tickLabelValues.size(); ++i){
		tempValues.append( round(tickLabelValues[i], precision-1) );
	}
	
	for (int i=0; i<tempValues.size(); ++i){
		if (tempValues.count(tempValues[i]) > 1){
			//duplicate found for the reduced precision
			//-> current precision cannot be reduced, return the current value
			return precision;
		}
	}
	
	//no duplicates found, reduce further, and check again
	return lowerLabelsPrecision(precision-1);
}

float AxisPrivate::round(float value, int precision){
	return int(value*pow(10, precision) + (value<0 ? -0.5 : 0.5))/pow(10, precision);
}

/*!
	recalculates the postion of the tick labels.
	Called when the geometry related properties (position, offset, font size, suffix, prefix) of the labels are changed.
 */
//TODO optimize
void AxisPrivate::retransformTickLabels(){
	tickLabelPoints.clear();
	if (majorTicksDirection == Axis::noTicks || labelsPosition == Axis::NoLabels){
		recalcShapeAndBoundingRect();
		return;
	}

	QFontMetrics fm(labelsFont);
	float width = 0;
	float height = fm.ascent();
	QString label;
	QPointF pos;

	int xDirection = m_cSystem->xDirection();
	int yDirection = m_cSystem->yDirection();

	QPointF startPoint, endPoint, anchorPoint;

	//TODO optimize this loop
	for ( int i=0; i<majorTickPoints.size(); i++ ){
		label = labelsPrefix + tickLabelStrings.at(i) + labelsSuffix;
		width = fm.width( label );
		anchorPoint = majorTickPoints.at(i);
		  
		//center align all labels with respect to the end point of the tick line
		if (orientation == Axis::AxisHorizontal){
			if(offset < 0.5) {
				startPoint = anchorPoint + QPointF(0, (majorTicksDirection & Axis::ticksIn)  ? yDirection * majorTicksLength  : 0);
				endPoint   = anchorPoint + QPointF(0, (majorTicksDirection & Axis::ticksOut) ? -yDirection * majorTicksLength : 0);
			}
			else {
				startPoint = anchorPoint + QPointF(0, (majorTicksDirection & Axis::ticksOut)  ? yDirection * majorTicksLength  : 0);
				endPoint   = anchorPoint + QPointF(0, (majorTicksDirection & Axis::ticksIn) ? -yDirection * majorTicksLength : 0);
			}
			if (labelsPosition == Axis::LabelsOut){
				pos.setX( endPoint.x() - width/2);
				pos.setY( endPoint.y() + height + labelsOffset );
			}else{
				pos.setX( startPoint.x() - width/2);
				pos.setY( startPoint.y() - labelsOffset );
			}
		}else{// vertical
			if(offset < 0.5) {
				startPoint = anchorPoint + QPointF((majorTicksDirection & Axis::ticksIn)  ? xDirection * majorTicksLength  : 0, 0);
				endPoint = anchorPoint + QPointF((majorTicksDirection & Axis::ticksOut) ? -xDirection * majorTicksLength : 0, 0);
			} else {
				startPoint = anchorPoint + QPointF((majorTicksDirection & Axis::ticksOut) ? xDirection * majorTicksLength : 0, 0);
				endPoint = anchorPoint + QPointF((majorTicksDirection & Axis::ticksIn)  ? -xDirection *  majorTicksLength  : 0, 0);
			}
			if (labelsPosition == Axis::LabelsOut){
				pos.setX( endPoint.x() - width - labelsOffset );
				pos.setY( endPoint.y() + height/2 );
			}else{
				pos.setX( startPoint.x() + labelsOffset );
				pos.setY( startPoint.y() + height/2 );
			}
		}
		tickLabelPoints<<pos;
	}

	recalcShapeAndBoundingRect();
}

void AxisPrivate::retransformMajorGrid(){
	majorGridPath = QPainterPath();
	if (majorGridPen.style() == Qt::NoPen){
		update();
		return;
	}

	//major tick points are already in scene coordinates, convert them back to logical...
	QList<QPointF> logicalMajorTickPoints = m_cSystem->mapSceneToLogical(majorTickPoints, AbstractCoordinateSystem::SuppressPageClipping);

	QList<QLineF> lines;
	if (orientation == Axis::AxisHorizontal){ //horizontal axis
		float yMin = m_plot->yMin();
		float yMax = m_plot->yMax();

		//skip the first and the last points, since we don't want to paint any grid lines at the plot boundaries
		for (int i=1; i<logicalMajorTickPoints.size()-1; ++i){
			const QPointF& point = logicalMajorTickPoints.at(i);
			lines.append( QLineF(point.x(), yMin, point.x(), yMax) );
		}
	}else{ //vertical axis
		float xMin = m_plot->xMin();
		float xMax = m_plot->xMax();

		//skip the first and the last points, since we don't want to paint any grid lines at the plot boundaries
		for (int i=1; i<logicalMajorTickPoints.size()-1; ++i){
			const QPointF& point = logicalMajorTickPoints.at(i);
			lines.append( QLineF(xMin, point.y(), xMax, point.y()) );
		}
	}
	
	lines = m_cSystem->mapLogicalToScene(lines, AbstractCoordinateSystem::MarkGaps);
	foreach (QLineF line, lines) {
		majorGridPath.moveTo(line.p1());
		majorGridPath.lineTo(line.p2());
	}

	update();
}

void AxisPrivate::retransformMinorGrid(){
	minorGridPath = QPainterPath();
	if (minorGridPen.style() == Qt::NoPen){
		update();
		return;
	}

	//minor tick points are already in scene coordinates, convert them back to logical...
	QList<QPointF> logicalMinorTickPoints = m_cSystem->mapSceneToLogical(minorTickPoints, AbstractCoordinateSystem::SuppressPageClipping);

	QList<QLineF> lines;
	if (orientation == Axis::AxisHorizontal){ //horizontal axis
		float yMin = m_plot->yMin();
		float yMax = m_plot->yMax();

		for (int i=0; i<logicalMinorTickPoints.size(); ++i){
			const QPointF& point = logicalMinorTickPoints.at(i);
			lines.append( QLineF(point.x(), yMin, point.x(), yMax) );
		}
	}else{ //vertical axis
		float xMin = m_plot->xMin();
		float xMax = m_plot->xMax();

		for (int i=0; i<logicalMinorTickPoints.size(); ++i){
			const QPointF& point = logicalMinorTickPoints.at(i);
			lines.append( QLineF(xMin, point.y(), xMax, point.y()) );
		}
	}

	lines = m_cSystem->mapLogicalToScene(lines, AbstractCoordinateSystem::MarkGaps);
	foreach (QLineF line, lines) {
		minorGridPath.moveTo(line.p1());
		minorGridPath.lineTo(line.p2());
	}

	update();
}

void AxisPrivate::recalcShapeAndBoundingRect() {
	prepareGeometryChange();

	float w = linePath.boundingRect().width();
	float h = linePath.boundingRect().height();
	float penWidth = linePen.width();
	boundingRectangle = QRectF(-w/2 - penWidth/2, -h/2 - penWidth/2, w + penWidth, h + penWidth);
	
	w = majorTicksPath.boundingRect().width();
	h = majorTicksPath.boundingRect().height();
	penWidth = majorTicksPen.width();
	boundingRectangle |=  QRectF(-w/2 - penWidth/2, -h/2 - penWidth/2, w + penWidth, h + penWidth);
	
	w = minorTicksPath.boundingRect().width();
	h = minorTicksPath.boundingRect().height();
	penWidth = minorTicksPen.width();
	boundingRectangle |= QRectF(-w/2 - penWidth/2, -h/2 - penWidth/2, w + penWidth, h + penWidth);

	axisShape = AbstractWorksheetElement::shapeFromPath(linePath, linePen);
	axisShape.addPath(AbstractWorksheetElement::shapeFromPath(majorTicksPath, majorTicksPen));
	axisShape.addPath(AbstractWorksheetElement::shapeFromPath(minorTicksPath, minorTicksPen));
	
	QPainterPath  tickLabelsPath = QPainterPath();
	if (labelsPosition != Axis::NoLabels){
		QTransform trafo;
		QPainterPath tempPath;
	  	for (int i=0; i<tickLabelPoints.size(); i++){
			tempPath = QPainterPath();
			tempPath.addText( QPoint(0,0), labelsFont, tickLabelStrings.at(i) );

			trafo.reset();
			trafo.translate( tickLabelPoints.at(i).x(), tickLabelPoints.at(i).y() );
			trafo.rotate( -labelsRotationAngle );	
			tempPath = trafo.map(tempPath);

			tickLabelsPath.addPath(AbstractWorksheetElement::shapeFromPath(tempPath, linePen));
		}
		axisShape.addPath(AbstractWorksheetElement::shapeFromPath(tickLabelsPath, QPen()));
		boundingRectangle = boundingRectangle.united(tickLabelsPath.boundingRect());
	}
	
	//add title label, if available
	if ( title->isVisible() && title->text().text!="" ){
		//determine the new position of the title label:
		//we calculate the new position here and not in retransform(),
		//since it depends on the size and position of the tick labels, tickLabelsPath, available here.
		QRectF rect=linePath.boundingRect();
		float offset = titleOffset + labelsOffset; //the distance to the axis line
		if (orientation == Axis::AxisHorizontal){
			offset += title->graphicsItem()->boundingRect().height()/2 + tickLabelsPath.boundingRect().height();
			title->setPosition( QPointF( (rect.topLeft().x() + rect.topRight().x())/2, rect.bottomLeft().y() + offset ) );
		}else{
			offset += title->graphicsItem()->boundingRect().width()/2 + tickLabelsPath.boundingRect().width();
			title->setPosition( QPointF( rect.topLeft().x() - offset, (rect.topLeft().y() + rect.bottomLeft().y())/2 ) );
		}
		
		boundingRectangle |=mapRectFromItem( title->graphicsItem(), title->graphicsItem()->boundingRect() );
		axisShape.addPath(AbstractWorksheetElement::shapeFromPath(title->graphicsItem()->mapToParent(title->graphicsItem()->shape()), linePen));
	}
	
// 	boundingRectangle = boundingRectangle.normalized();
}

/*!
	paints the content of the axis. Reimplemented from \c QGraphicsItem.
	\sa QGraphicsItem::paint()
 */
void AxisPrivate::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget * widget) {
	Q_UNUSED(option)
	Q_UNUSED(widget)

  if (!isVisible())
	return;
  
  //draw the axis title
	//TODO

  //draw the line
  if (linePen.style() != Qt::NoPen){
	painter->setOpacity(lineOpacity);
	painter->setPen(linePen);
	painter->setBrush(Qt::NoBrush);
	painter->drawPath(linePath);
  }

  //draw the major ticks
  if (majorTicksDirection != Axis::noTicks){
	painter->setOpacity(majorTicksOpacity);
	painter->setPen(majorTicksPen);
	painter->setBrush(Qt::NoBrush);
	painter->drawPath(majorTicksPath);
  }
  
  //draw the minor ticks
  if (minorTicksDirection != Axis::noTicks){
	painter->setOpacity(minorTicksOpacity);
	painter->setPen(minorTicksPen);
	painter->setBrush(Qt::NoBrush);
	painter->drawPath(minorTicksPath);
  }

  	//draw major grid
	if (majorGridPen.style() != Qt::NoPen){
		painter->setOpacity(majorGridOpacity);
		painter->setPen(majorGridPen);
		painter->setBrush(Qt::NoBrush);
		painter->drawPath(majorGridPath);
	}

	//draw minor grid
	if (minorGridPen.style() != Qt::NoPen){
		painter->setOpacity(minorGridOpacity);
		painter->setPen(minorGridPen);
		painter->setBrush(Qt::NoBrush);
		painter->drawPath(minorGridPath);
	}

  // draw tick labels
  if (labelsPosition != Axis::NoLabels){
	painter->setOpacity(labelsOpacity);
	painter->setPen(QPen(labelsColor));
	painter->setFont(labelsFont);
	for (int i=0; i<tickLabelPoints.size(); i++){
	  painter->translate(tickLabelPoints.at(i));
	  painter->save();
	  painter->rotate(-labelsRotationAngle);
 	  painter->drawText(QPoint(0,0), tickLabelStrings.at(i) );
 	  painter->restore();
	  painter->translate(-tickLabelPoints.at(i));
	}
  }
  
   if (isSelected()){
	painter->setPen(QPen(Qt::blue, 0, Qt::DashLine));
	painter->drawPath(shape());
  }
}

void AxisPrivate::contextMenuEvent(QGraphicsSceneContextMenuEvent* event){
    q->createContextMenu()->exec(event->screenPos());
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
//! Save as XML
void Axis::save(QXmlStreamWriter* writer) const{
	Q_D(const Axis);

    writer->writeStartElement( "axis" );
    writeBasicAttributes( writer );
    writeCommentElement( writer );

	//general
    writer->writeStartElement( "general" );
	writer->writeAttribute( "autoScale", QString::number(d->autoScale) );
	writer->writeAttribute( "orientation", QString::number(d->orientation) );
	writer->writeAttribute( "position", QString::number(d->position) );
	writer->writeAttribute( "scale", QString::number(d->scale) );
	writer->writeAttribute( "offset", QString::number(d->offset) );
	writer->writeAttribute( "start", QString::number(d->start) );
	writer->writeAttribute( "end", QString::number(d->end) );
	writer->writeAttribute( "scalingFactor", QString::number(d->scalingFactor) );
	writer->writeAttribute( "zeroOffset", QString::number(d->zeroOffset) );
	writer->writeAttribute( "titleOffset", QString::number(d->titleOffset) );
	writer->writeAttribute( "visible", QString::number(d->isVisible()) );
	writer->writeEndElement();
	
	//label
	d->title->save( writer );
	
	//line
	writer->writeStartElement( "line" );
	WRITE_QPEN(d->linePen);
	writer->writeAttribute( "opacity", QString::number(d->lineOpacity) );
	writer->writeEndElement();
	
	//major ticks
	writer->writeStartElement( "majorTicks" );
	writer->writeAttribute( "direction", QString::number(d->majorTicksDirection) );
	writer->writeAttribute( "type", QString::number(d->majorTicksType) );
	writer->writeAttribute( "number", QString::number(d->majorTicksNumber) );
	writer->writeAttribute( "length", QString::number(d->majorTicksLength) );
	WRITE_QPEN(d->majorTicksPen);
	writer->writeAttribute( "opacity", QString::number(d->majorTicksOpacity) );
	writer->writeEndElement();

	//minor ticks
	writer->writeStartElement( "minorTicks" );
	writer->writeAttribute( "direction", QString::number(d->minorTicksDirection) );
	writer->writeAttribute( "type", QString::number(d->minorTicksType) );
	writer->writeAttribute( "number", QString::number(d->minorTicksNumber) );
	writer->writeAttribute( "length", QString::number(d->minorTicksLength) );
	WRITE_QPEN(d->minorTicksPen);
	writer->writeAttribute( "opacity", QString::number(d->minorTicksOpacity) );
	writer->writeEndElement();
	
	//extra ticks

	//labels
	writer->writeStartElement( "labels" );
	writer->writeAttribute( "position", QString::number(d->labelsPosition) );
	writer->writeAttribute( "offset", QString::number(d->labelsOffset) );
	writer->writeAttribute( "rotation", QString::number(d->labelsRotationAngle) );
	WRITE_QCOLOR(d->labelsColor);
	WRITE_QFONT(d->labelsFont);
	writer->writeAttribute( "prefix", d->labelsPrefix );
	writer->writeAttribute( "suffix", d->labelsSuffix );
	writer->writeAttribute( "opacity", QString::number(d->labelsOpacity) );
	writer->writeEndElement();
	
	//grid
	writer->writeStartElement( "majorGrid" );
	WRITE_QPEN(d->majorGridPen);
	writer->writeAttribute( "opacity", QString::number(d->majorGridOpacity) );
	writer->writeEndElement();

	writer->writeStartElement( "minorGrid" );
	WRITE_QPEN(d->minorGridPen);
	writer->writeAttribute( "opacity", QString::number(d->minorGridOpacity) );
	writer->writeEndElement();

    writer->writeEndElement(); // close "axis" section
}

//! Load from XML
bool Axis::load(XmlStreamReader* reader){
	Q_D(Axis);

    if(!reader->isStartElement() || reader->name() != "axis"){
        reader->raiseError(tr("no axis element found"));
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
        if (reader->isEndElement() && reader->name() == "axis")
            break;

        if (!reader->isStartElement())
            continue;

        if (reader->name() == "comment"){
            if (!readCommentElement(reader)) return false;
		}else if (reader->name() == "general"){
            attribs = reader->attributes();

			str = attribs.value("autoScale").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'autoScale'"));
            else
                d->autoScale = (bool)str.toInt();

            str = attribs.value("orientation").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'orientation'"));
            else
                d->orientation = (Axis::AxisOrientation)str.toInt();

            str = attribs.value("position").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'position'"));
            else
                d->position = (Axis::AxisPosition)str.toInt();

            str = attribs.value("scale").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'scale'"));
            else
                d->scale = (Axis::AxisScale)str.toInt();

            str = attribs.value("offset").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'offset'"));
            else
                d->offset = str.toDouble();

            str = attribs.value("start").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'start'"));
            else
                d->start = str.toDouble();

			str = attribs.value("end").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'end'"));
            else
                d->end = str.toDouble();
			
            str = attribs.value("scalingFactor").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'scalingFactor'"));
            else
                d->scalingFactor = str.toDouble();
			
			str = attribs.value("zeroOffset").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'zeroOffset'"));
            else
                d->zeroOffset = str.toDouble();

			str = attribs.value("titleOffset").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'titleOffset'"));
            else
                d->titleOffset = str.toDouble();

			str = attribs.value("visible").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'visible'"));
            else
                d->setVisible(str.toInt());
		}else if (reader->name() == "textLabel"){
			d->title->load(reader);
		}else if (reader->name() == "line"){
			attribs = reader->attributes();

			READ_QPEN(d->linePen);

			str = attribs.value("opacity").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'opacity'"));
            else
                d->lineOpacity = str.toInt();
		}else if (reader->name() == "majorTicks"){
			attribs = reader->attributes();
	
			str = attribs.value("direction").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'direction'"));
            else
                d->majorTicksDirection = (Axis::TicksDirection)str.toInt();
			
			str = attribs.value("type").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'type'"));
            else
                d->majorTicksType = (Axis::TicksType)str.toInt();
			
			str = attribs.value("number").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'number'"));
            else
                d->majorTicksNumber = str.toInt();

			str = attribs.value("length").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'length'"));
            else
                d->majorTicksLength = str.toDouble();

			READ_QPEN(d->majorTicksPen);

			str = attribs.value("opacity").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'opacity'"));
            else
                d->majorTicksOpacity = str.toInt();
		}else if (reader->name() == "minorTicks"){
			attribs = reader->attributes();
	
			str = attribs.value("direction").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'direction'"));
            else
                d->minorTicksDirection = (Axis::TicksDirection)str.toInt();
			
			str = attribs.value("type").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'type'"));
            else
                d->minorTicksType = (Axis::TicksType)str.toInt();
			
			str = attribs.value("number").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'number'"));
            else
                d->minorTicksNumber = str.toInt();

			str = attribs.value("length").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'length'"));
            else
                d->minorTicksLength = str.toDouble();
			
			READ_QPEN(d->minorTicksPen);
			
			str = attribs.value("opacity").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'opacity'"));
            else
                d->minorTicksOpacity = str.toInt();
		}else if (reader->name() == "labels"){
			attribs = reader->attributes();

			str = attribs.value("position").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'position'"));
            else
                d->labelsPosition = (Axis::LabelsPosition)str.toInt();
			
			str = attribs.value("offset").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'offset'"));
            else
                d->labelsOffset = str.toDouble();
			
			str = attribs.value("rotation").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'rotation'"));
            else
                d->labelsRotationAngle = str.toDouble();
			
			READ_QCOLOR(d->labelsColor);
			READ_QFONT(d->labelsFont);

			//don't produce any warning if no prefix or suffix is set (empty string is allowd here in xml)
			d->labelsPrefix = attribs.value("prefix").toString();
			d->labelsSuffix = attribs.value("suffix").toString();

			str = attribs.value("opacity").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'opacity'"));
            else
                d->labelsOpacity = str.toInt();
		}else if (reader->name() == "majorGrid"){
			attribs = reader->attributes();

			READ_QPEN(d->majorGridPen);

			str = attribs.value("opacity").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'opacity'"));
            else
                d->majorGridOpacity = str.toInt();
		}else if (reader->name() == "minorGrid"){
			attribs = reader->attributes();

			READ_QPEN(d->minorGridPen);

			str = attribs.value("opacity").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'opacity'"));
            else
                d->minorGridOpacity = str.toInt();
        }else{ // unknown element
            reader->raiseWarning(tr("unknown element '%1'").arg(reader->name().toString()));
            if (!reader->skipToEndElement()) return false;
        }
    }

    return true;
}
