/***************************************************************************
    File                 : CartesianPlot.cpp
    Project              : LabPlot
    Description          : A plot containing decoration elements.
    --------------------------------------------------------------------
    Copyright            : (C) 2011-2014 by Alexander Semke (alexander.semke*web.de)
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
#include "CartesianPlotPrivate.h"
#include "Axis.h"
#include "XYCurve.h"
#include "XYEquationCurve.h"
#include "XYFitCurve.h"
#include "backend/worksheet/plots/cartesian/CartesianPlotLegend.h"
#include "backend/worksheet/plots/PlotArea.h"
#include "backend/worksheet/plots/AbstractPlotPrivate.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/Axis.h"
#include "backend/worksheet/TextLabel.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
#include <math.h>

#include <QDebug>
#include <QMenu>
#include <QToolBar>
#include <QGraphicsView>
#include <QGraphicsColorizeEffect>

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
CartesianPlot::CartesianPlot(const QString &name):AbstractPlot(name, new CartesianPlotPrivate(this)),
	m_legend(0), m_zoomFactor(1.2) {
	init();
}

CartesianPlot::CartesianPlot(const QString &name, CartesianPlotPrivate *dd):AbstractPlot(name, dd),
	m_legend(0), m_zoomFactor(1.2) {
	init();
}

CartesianPlot::~CartesianPlot(){
	delete m_coordinateSystem;
	delete addNewMenu;

	//don't need to delete objects added with addChild()

	//no need to delete the d-pointer here - it inherits from QGraphicsItem
	//and is deleted during the cleanup in QGraphicsScene
}

/*!
	initializes all member variables of \c CartesianPlot
*/
void CartesianPlot::init(){
	Q_D(CartesianPlot);

	d->cSystem = new CartesianCoordinateSystem(this);;
	m_coordinateSystem = d->cSystem;

	d->autoScaleX = true;
	d->autoScaleY = true;
	d->xScale = ScaleLinear;
	d->yScale = ScaleLinear;

	//the following factor determines the size of the offset between the min/max points of the curves
	//and the coordinate system ranges, when doing auto scaling
	//Factor 1 corresponds to the exact match - min/max values of the curves correspond to the start/end values of the ranges.
	d->autoScaleOffsetFactor = 0.05;

	//TODO: make this factor optional.
	//Provide in the UI the possibility to choose between "exact" or 0% offset, 2%, 5% and 10% for the auto fit option

	m_plotArea = new PlotArea(name() + " plot area");
	addChild(m_plotArea);

	//offset between the plot area and the area defining the coordinate system, in scene units.
	d->horizontalPadding = Worksheet::convertToSceneUnits(1.5, Worksheet::Centimeter);
	d->verticalPadding = Worksheet::convertToSceneUnits(1.5, Worksheet::Centimeter);

	initActions();
	initMenus();

	connect(this, SIGNAL(aspectAdded(const AbstractAspect*)), this, SLOT(childAdded(const AbstractAspect*)));
	connect(this, SIGNAL(aspectRemoved(const AbstractAspect*,const AbstractAspect*,const AbstractAspect*)),
			this, SLOT(childRemoved(const AbstractAspect*,const AbstractAspect*,const AbstractAspect*)));
	graphicsItem()->setFlag(QGraphicsItem::ItemIsMovable, true);
	graphicsItem()->setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
	graphicsItem()->setFlag(QGraphicsItem::ItemIsSelectable, true);
	graphicsItem()->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    graphicsItem()->setFlag(QGraphicsItem::ItemIsFocusable, true);
}

/*!
	initializes all children of \c CartesianPlot and
	setups a default plot of type \c type with a plot title.
*/
void CartesianPlot::initDefault(Type type){
	Q_D(CartesianPlot);

	switch (type) {
		case FourAxes:
		{
			d->xMin = 0;
			d->xMax = 1;
			d->yMin = 0;
			d->yMax = 1;

			//Axes
			Axis *axis = new Axis("x axis 1", Axis::AxisHorizontal);
			addChild(axis);
			axis->setPosition(Axis::AxisBottom);
			axis->setStart(0);
			axis->setEnd(1);
			axis->setMajorTicksDirection(Axis::ticksIn);
			axis->setMajorTicksNumber(6);
			axis->setMinorTicksDirection(Axis::ticksIn);
			axis->setMinorTicksNumber(1);
			QPen pen = axis->majorGridPen();
			pen.setStyle(Qt::SolidLine);
			axis->setMajorGridPen(pen);
			pen = axis->minorGridPen();
			pen.setStyle(Qt::DotLine);
			axis->setMinorGridPen(pen);

			axis = new Axis("x axis 2", Axis::AxisHorizontal);
			addChild(axis);
			axis->setPosition(Axis::AxisTop);
			axis->setStart(0);
			axis->setEnd(1);
			axis->setMajorTicksDirection(Axis::ticksIn);
			axis->setMajorTicksNumber(6);
			axis->setMinorTicksDirection(Axis::ticksIn);
			axis->setMinorTicksNumber(1);
			axis->setLabelsPosition(Axis::NoLabels);
			axis->title()->setText(QString());

			axis = new Axis("y axis 1", Axis::AxisVertical);
			addChild(axis);
			axis->setPosition(Axis::AxisLeft);
			axis->setStart(0);
			axis->setEnd(1);
			axis->setMajorTicksDirection(Axis::ticksIn);
			axis->setMajorTicksNumber(6);
			axis->setMinorTicksDirection(Axis::ticksIn);
			axis->setMinorTicksNumber(1);
			pen = axis->majorGridPen();
			pen.setStyle(Qt::SolidLine);
			axis->setMajorGridPen(pen);
			pen = axis->minorGridPen();
			pen.setStyle(Qt::DotLine);
			axis->setMinorGridPen(pen);

			axis = new Axis("y axis 2", Axis::AxisVertical);
			addChild(axis);
			axis->setPosition(Axis::AxisRight);
			axis->setStart(0);
			axis->setEnd(1);
			axis->setOffset(1);
			axis->setMajorTicksDirection(Axis::ticksIn);
			axis->setMajorTicksNumber(6);
			axis->setMinorTicksDirection(Axis::ticksIn);
			axis->setMinorTicksNumber(1);
			axis->setLabelsPosition(Axis::NoLabels);
			axis->title()->setText(QString());

			break;
		}
		case TwoAxes:
		{
			d->xMin = 0;
			d->xMax = 1;
			d->yMin = 0;
			d->yMax = 1;

			Axis *axis = new Axis("x axis 1", Axis::AxisHorizontal);
			addChild(axis);
			axis->setPosition(Axis::AxisBottom);
			axis->setStart(0);
			axis->setEnd(1);
			axis->setMajorTicksDirection(Axis::ticksBoth);
			axis->setMajorTicksNumber(6);
			axis->setMinorTicksDirection(Axis::ticksBoth);
			axis->setMinorTicksNumber(1);
			axis->setArrowType(Axis::FilledArrowSmall);

			axis = new Axis("y axis 1", Axis::AxisVertical);
			addChild(axis);
			axis->setPosition(Axis::AxisLeft);
			axis->setStart(0);
			axis->setEnd(1);
			axis->setMajorTicksDirection(Axis::ticksBoth);
			axis->setMajorTicksNumber(6);
			axis->setMinorTicksDirection(Axis::ticksBoth);
			axis->setMinorTicksNumber(1);
			axis->setArrowType(Axis::FilledArrowSmall);

			break;
		}
		case TwoAxesCentered:
		{
			d->xMin = -0.5;
			d->xMax = 0.5;
			d->yMin = -0.5;
			d->yMax = 0.5;

			d->horizontalPadding = Worksheet::convertToSceneUnits(1.0, Worksheet::Centimeter);
			d->verticalPadding = Worksheet::convertToSceneUnits(1.0, Worksheet::Centimeter);

			QPen pen = m_plotArea->borderPen();
			pen.setStyle(Qt::NoPen);
			m_plotArea->setBorderPen(pen);

			Axis *axis = new Axis("x axis 1", Axis::AxisHorizontal);
			addChild(axis);
			axis->setPosition(Axis::AxisCentered);
			axis->setStart(-0.5);
			axis->setEnd(0.5);
			axis->setMajorTicksDirection(Axis::ticksBoth);
			axis->setMajorTicksNumber(10);
			axis->setMinorTicksDirection(Axis::ticksBoth);
			axis->setMinorTicksNumber(1);
			axis->setArrowType(Axis::FilledArrowSmall);
			axis->title()->setText(QString());

			axis = new Axis("y axis 1", Axis::AxisVertical);
			addChild(axis);
			axis->setPosition(Axis::AxisCentered);
			axis->setStart(-0.5);
			axis->setEnd(0.5);
			axis->setMajorTicksDirection(Axis::ticksBoth);
			axis->setMajorTicksNumber(10);
			axis->setMinorTicksDirection(Axis::ticksBoth);
			axis->setMinorTicksNumber(1);
			axis->setArrowType(Axis::FilledArrowSmall);
			axis->title()->setText(QString());

			break;
		}
		case TwoAxesCenteredZero:
		{
			d->xMin = -0.5;
			d->xMax = 0.5;
			d->yMin = -0.5;
			d->yMax = 0.5;

			d->horizontalPadding = Worksheet::convertToSceneUnits(1.0, Worksheet::Centimeter);
			d->verticalPadding = Worksheet::convertToSceneUnits(1.0, Worksheet::Centimeter);

			QPen pen = m_plotArea->borderPen();
			pen.setStyle(Qt::NoPen);
			m_plotArea->setBorderPen(pen);

			Axis *axis = new Axis("x axis 1", Axis::AxisHorizontal);
			addChild(axis);
			axis->setPosition(Axis::AxisCustom);
			axis->setOffset(0);
			axis->setStart(-0.5);
			axis->setEnd(0.5);
			axis->setMajorTicksDirection(Axis::ticksBoth);
			axis->setMajorTicksNumber(6);
			axis->setMinorTicksDirection(Axis::ticksBoth);
			axis->setMinorTicksNumber(1);
			axis->setArrowType(Axis::FilledArrowSmall);
			axis->title()->setText(QString());

			axis = new Axis("y axis 1", Axis::AxisVertical);
			addChild(axis);
			axis->setPosition(Axis::AxisCustom);
			axis->setOffset(0);
			axis->setStart(-0.5);
			axis->setEnd(0.5);
			axis->setMajorTicksDirection(Axis::ticksBoth);
			axis->setMajorTicksNumber(6);
			axis->setMinorTicksDirection(Axis::ticksBoth);
			axis->setMinorTicksNumber(1);
			axis->setArrowType(Axis::FilledArrowSmall);
			axis->title()->setText(QString());

			break;
		}
	}

	d->xMinPrev = d->xMin;
	d->xMaxPrev = d->xMax;
	d->yMinPrev = d->yMin;
	d->yMaxPrev = d->yMax;

	//Plot title
 	m_title = new TextLabel(this->name(), TextLabel::PlotTitle);
	addChild(m_title);
	m_title->setHidden(true);
	m_title->setParentGraphicsItem(m_plotArea->graphicsItem());

	//Geometry, specify the plot rect in scene coordinates.
	//TODO: Use default settings for left, top, width, height and for min/max for the coordinate system
	float x = Worksheet::convertToSceneUnits(2, Worksheet::Centimeter);
	float y = Worksheet::convertToSceneUnits(2, Worksheet::Centimeter);
	float w = Worksheet::convertToSceneUnits(10, Worksheet::Centimeter);
	float h = Worksheet::convertToSceneUnits(10, Worksheet::Centimeter);

	//all plot children are initialized -> set the geometry of the plot in scene coordinates.
	d->rect = QRectF(x,y,w,h);
	d->retransform();
}

void CartesianPlot::initActions(){
	QActionGroup* mouseModeActionGroup = new QActionGroup(this);
	mouseModeActionGroup->setExclusive(true);
	selectionModeAction = new KAction(KIcon("cursor-arrow"), i18n("Select and edit"), mouseModeActionGroup);
	selectionModeAction->setCheckable(true);
	selectionModeAction->setChecked(true);

	zoomSelectionModeAction = new KAction(KIcon("zoom-select"), i18n("Select region and zoom in"), mouseModeActionGroup);
	zoomSelectionModeAction->setCheckable(true);

	zoomXSelectionModeAction = new KAction(KIcon("zoom-select-x"), i18n("Select x-region and zoom in"), mouseModeActionGroup);
	zoomXSelectionModeAction->setCheckable(true);

	zoomYSelectionModeAction = new KAction(KIcon("zoom-select-y"), i18n("Select y-region and zoom in"), mouseModeActionGroup);
	zoomYSelectionModeAction->setCheckable(true);

	connect(mouseModeActionGroup, SIGNAL(triggered(QAction*)), SLOT(mouseModeChanged(QAction*)));

	addCurveAction = new KAction(KIcon("xy-curve"), i18n("xy-curve"), this);
	addEquationCurveAction = new KAction(KIcon("xy-equation-curve"), i18n("xy-curve from a mathematical equation"), this);
	addFitCurveAction = new KAction(KIcon("xy-fit-curve"), i18n("xy-curve from a fit to data"), this);
	addLegendAction = new KAction(KIcon("text-field"), i18n("legend"), this);
	addHorizontalAxisAction = new KAction(KIcon("axis-horizontal"), i18n("horizontal axis"), this);
	addVerticalAxisAction = new KAction(KIcon("axis-vertical"), i18n("vertical axis"), this);

	scaleAutoAction = new KAction(KIcon("auto-scale-all"), i18n("auto scale"), this);
	scaleAutoXAction = new KAction(KIcon("auto-scale-x"), i18n("auto scale X"), this);
	scaleAutoYAction = new KAction(KIcon("auto-scale-y"), i18n("auto scale Y"), this);
	zoomInAction = new KAction(KIcon("zoom-in"), i18n("zoom in"), this);
	zoomOutAction = new KAction(KIcon("zoom-out"), i18n("zoom out"), this);
	zoomInXAction = new KAction(KIcon("x-zoom-in"), i18n("zoom in X"), this);
	zoomOutXAction = new KAction(KIcon("x-zoom-out"), i18n("zoom out X"), this);
	zoomInYAction = new KAction(KIcon("y-zoom-in"), i18n("zoom in Y"), this);
	zoomOutYAction = new KAction(KIcon("y-zoom-out"), i18n("zoom out Y"), this);
    shiftLeftXAction = new KAction(KIcon("shift-left-x"), i18n("shift left X"), this);
	shiftRightXAction = new KAction(KIcon("shift-right-x"), i18n("shift right X"), this);
	shiftUpYAction = new KAction(KIcon("shift-up-y"), i18n("shift up Y"), this);
	shiftDownYAction = new KAction(KIcon("shift-down-y"), i18n("shift down Y"), this);

	connect(addCurveAction, SIGNAL(triggered()), SLOT(addCurve()));
	connect(addEquationCurveAction, SIGNAL(triggered()), SLOT(addEquationCurve()));
	connect(addFitCurveAction, SIGNAL(triggered()), SLOT(addFitCurve()));
	connect(addLegendAction, SIGNAL(triggered()), SLOT(addLegend()));
	connect(addHorizontalAxisAction, SIGNAL(triggered()), SLOT(addAxis()));
	connect(addVerticalAxisAction, SIGNAL(triggered()), SLOT(addAxis()));

	//zoom actions
	connect(scaleAutoAction, SIGNAL(triggered()), SLOT(scaleAuto()));
	connect(scaleAutoXAction, SIGNAL(triggered()), SLOT(scaleAutoX()));
	connect(scaleAutoYAction, SIGNAL(triggered()), SLOT(scaleAutoY()));
	connect(zoomInAction, SIGNAL(triggered()), SLOT(zoomIn()));
	connect(zoomOutAction, SIGNAL(triggered()), SLOT(zoomOut()));
	connect(zoomInXAction, SIGNAL(triggered()), SLOT(zoomInX()));
	connect(zoomOutXAction, SIGNAL(triggered()), SLOT(zoomOutX()));
	connect(zoomInYAction, SIGNAL(triggered()), SLOT(zoomInY()));
	connect(zoomOutYAction, SIGNAL(triggered()), SLOT(zoomOutY()));
	connect(shiftLeftXAction, SIGNAL(triggered()), SLOT(shiftLeftX()));
	connect(shiftRightXAction, SIGNAL(triggered()), SLOT(shiftRightX()));
	connect(shiftUpYAction, SIGNAL(triggered()), SLOT(shiftUpY()));
	connect(shiftDownYAction, SIGNAL(triggered()), SLOT(shiftDownY()));

	visibilityAction = new QAction(i18n("visible"), this);
	visibilityAction->setCheckable(true);
	connect(visibilityAction, SIGNAL(triggered()), this, SLOT(visibilityChanged()));
}

void CartesianPlot::initMenus(){
	addNewMenu = new QMenu(i18n("Add new"));
	addNewMenu->addAction(addCurveAction);
	addNewMenu->addAction(addEquationCurveAction);
	addNewMenu->addAction(addFitCurveAction);
	addNewMenu->addAction(addLegendAction);
	addNewMenu->addSeparator();
	addNewMenu->addAction(addHorizontalAxisAction);
	addNewMenu->addAction(addVerticalAxisAction);

	zoomMenu = new QMenu(i18n("Zoom"));
	zoomMenu->addAction(scaleAutoAction);
	zoomMenu->addAction(scaleAutoXAction);
	zoomMenu->addAction(scaleAutoYAction);
	zoomMenu->addSeparator();
	zoomMenu->addAction(zoomInAction);
	zoomMenu->addAction(zoomOutAction);
	zoomMenu->addSeparator();
	zoomMenu->addAction(zoomInXAction);
	zoomMenu->addAction(zoomOutXAction);
	zoomMenu->addSeparator();
	zoomMenu->addAction(zoomInYAction);
	zoomMenu->addAction(zoomOutYAction);
	zoomMenu->addSeparator();
	zoomMenu->addAction(shiftLeftXAction);
	zoomMenu->addAction(shiftRightXAction);
	zoomMenu->addSeparator();
	zoomMenu->addAction(shiftUpYAction);
	zoomMenu->addAction(shiftDownYAction);
}

QMenu* CartesianPlot::createContextMenu(){
	QMenu* menu = AbstractWorksheetElement::createContextMenu();

#ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
	QAction* firstAction = menu->actions().first();
#else
	QAction* firstAction = menu->actions().at(1);
#endif

	visibilityAction->setChecked(isVisible());
	menu->insertAction(firstAction, visibilityAction);

	menu->insertMenu(firstAction, addNewMenu);
	menu->insertMenu(firstAction, zoomMenu);
	menu->insertSeparator(firstAction);

	return menu;
}

void CartesianPlot::fillToolBar(QToolBar* toolBar) const{
	toolBar->addAction(selectionModeAction);
	toolBar->addAction(zoomSelectionModeAction);
	toolBar->addAction(zoomXSelectionModeAction);
	toolBar->addAction(zoomYSelectionModeAction);
	toolBar->addSeparator();
	toolBar->addAction(addCurveAction);
	toolBar->addAction(addEquationCurveAction);
	toolBar->addAction(addFitCurveAction);
	toolBar->addAction(addLegendAction);
	toolBar->addSeparator();
	toolBar->addAction(addHorizontalAxisAction);
	toolBar->addAction(addVerticalAxisAction);
	toolBar->addSeparator();
	toolBar->addAction(scaleAutoAction);
	toolBar->addAction(scaleAutoXAction);
	toolBar->addAction(scaleAutoYAction);
	toolBar->addAction(zoomInAction);
	toolBar->addAction(zoomOutAction);
	toolBar->addAction(zoomInXAction);
	toolBar->addAction(zoomOutXAction);
	toolBar->addAction(zoomInYAction);
	toolBar->addAction(zoomOutYAction);
	toolBar->addAction(shiftLeftXAction);
	toolBar->addAction(shiftRightXAction);
	toolBar->addAction(shiftUpYAction);
	toolBar->addAction(shiftDownYAction);
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

//##############################################################################
//################################  getter methods  ############################
//##############################################################################
BASIC_SHARED_D_READER_IMPL(CartesianPlot, bool, autoScaleX, autoScaleX)
BASIC_SHARED_D_READER_IMPL(CartesianPlot, float, xMin, xMin)
BASIC_SHARED_D_READER_IMPL(CartesianPlot, float, xMax, xMax)
BASIC_SHARED_D_READER_IMPL(CartesianPlot, CartesianPlot::Scale, xScale, xScale)
CLASS_SHARED_D_READER_IMPL(CartesianPlot, CartesianPlot::ScaleBreakings, xScaleBreakings, xScaleBreakings)

BASIC_SHARED_D_READER_IMPL(CartesianPlot, bool, autoScaleY, autoScaleY)
BASIC_SHARED_D_READER_IMPL(CartesianPlot, float, yMin, yMin)
BASIC_SHARED_D_READER_IMPL(CartesianPlot, float, yMax, yMax)
BASIC_SHARED_D_READER_IMPL(CartesianPlot, CartesianPlot::Scale, yScale, yScale)
CLASS_SHARED_D_READER_IMPL(CartesianPlot, CartesianPlot::ScaleBreakings, yScaleBreakings, yScaleBreakings)

/*!
	return the actual bounding rectangular of the plot (plot's rectangular minus padding)
	in plot's coordinates
 */
//TODO: return here a private variable only, update this variable on rect and padding changes.
QRectF CartesianPlot::plotRect() {
	Q_D(const CartesianPlot);
	QRectF rect = d->mapRectFromScene(d->rect);
	rect.setX(rect.x() + d->horizontalPadding);
	rect.setY(rect.y() + d->verticalPadding);
	rect.setWidth(rect.width() - d->horizontalPadding);
	rect.setHeight(rect.height()-d->verticalPadding);
	return rect;
}

CartesianPlot::MouseMode CartesianPlot::mouseMode() const {
	Q_D(const CartesianPlot);
	return d->mouseMode;
}

//##############################################################################
//######################  setter methods and undo commands  ####################
//##############################################################################
/*!
	set the rectangular, defined in scene coordinates
 */
STD_SETTER_CMD_IMPL_F_S(CartesianPlot, SetRect, QRectF, rect, retransform)
void CartesianPlot::setRect(const QRectF& rect){
	Q_D(CartesianPlot);
	if (rect != d->rect)
		exec(new CartesianPlotSetRectCmd(d, rect, i18n("%1: set geometry rect")));
}

//TODO: provide an undo-aware version
// STD_SETTER_CMD_IMPL_F(CartesianPlot, SetAutoScaleX, bool, autoScaleX, retransformScales)
void CartesianPlot::setAutoScaleX(bool autoScaleX){
	Q_D(CartesianPlot);
	if (autoScaleX != d->autoScaleX){
// 		exec(new CartesianPlotSetAutoScaleXCmd(d, autoScaleX, i18n("%1: change auto scale x")));
		d->autoScaleX = autoScaleX;
		if (autoScaleX)
			this->scaleAutoX();
	}
}

STD_SETTER_CMD_IMPL_F_S(CartesianPlot, SetXMin, float, xMin, retransformScales)
void CartesianPlot::setXMin(float xMin){
	Q_D(CartesianPlot);
	if (xMin != d->xMin)
		exec(new CartesianPlotSetXMinCmd(d, xMin, i18n("%1: set min x")));
}

STD_SETTER_CMD_IMPL_F_S(CartesianPlot, SetXMax, float, xMax, retransformScales);
void CartesianPlot::setXMax(float xMax){
	Q_D(CartesianPlot);
	if (xMax != d->xMax)
		exec(new CartesianPlotSetXMaxCmd(d, xMax, i18n("%1: set max x")));
}

STD_SETTER_CMD_IMPL_F_S(CartesianPlot, SetXScale, CartesianPlot::Scale, xScale, retransformScales);
void CartesianPlot::setXScale(Scale scale){
	Q_D(CartesianPlot);
	if (scale != d->xScale)
		exec(new CartesianPlotSetXScaleCmd(d, scale, i18n("%1: set x scale")));
}

STD_SETTER_CMD_IMPL_F_S(CartesianPlot, SetXScaleBreakings, CartesianPlot::ScaleBreakings, xScaleBreakings, retransformScales);
void CartesianPlot::setXScaleBreakings(const ScaleBreakings& breakings) {
	Q_D(CartesianPlot);
	d->xScaleBreakings = breakings;
	exec(new CartesianPlotSetXScaleBreakingsCmd(d, breakings, i18n("%1: set x-scale breakings")));
}


//TODO: provide an undo-aware version
// STD_SETTER_CMD_IMPL_F(CartesianPlot, SetAutoScaleY, bool, autoScaleY, retransformScales)
void CartesianPlot::setAutoScaleY(bool autoScaleY){
	Q_D(CartesianPlot);
	if (autoScaleY != d->autoScaleY){
// 		exec(new CartesianPlotSetAutoScaleYCmd(d, autoScaleY, i18n("%1: change auto scale y")));
		d->autoScaleY = autoScaleY;
		if (autoScaleY)
			this->scaleAutoY();
	}
}

STD_SETTER_CMD_IMPL_F_S(CartesianPlot, SetYMin, float, yMin, retransformScales);
void CartesianPlot::setYMin(float yMin){
	Q_D(CartesianPlot);
	if (yMin != d->yMin)
		exec(new CartesianPlotSetYMinCmd(d, yMin, i18n("%1: set min y")));
}

STD_SETTER_CMD_IMPL_F_S(CartesianPlot, SetYMax, float, yMax, retransformScales);
void CartesianPlot::setYMax(float yMax){
	Q_D(CartesianPlot);
	if (yMax != d->yMax)
		exec(new CartesianPlotSetYMaxCmd(d, yMax, i18n("%1: set max y")));
}

STD_SETTER_CMD_IMPL_F_S(CartesianPlot, SetYScale, CartesianPlot::Scale, yScale, retransformScales);
void CartesianPlot::setYScale(Scale scale){
	Q_D(CartesianPlot);
	if (scale != d->yScale)
		exec(new CartesianPlotSetYScaleCmd(d, scale, i18n("%1: set y scale")));
}

STD_SETTER_CMD_IMPL_F_S(CartesianPlot, SetYScaleBreakings, CartesianPlot::ScaleBreakings, yScaleBreakings, retransformScales);
void CartesianPlot::setYScaleBreakings(const ScaleBreakings& breakings) {
	Q_D(CartesianPlot);
	d->yScaleBreakings = breakings;
	exec(new CartesianPlotSetYScaleBreakingsCmd(d, breakings, i18n("%1: set y-scale breakings")));
}

//################################################################
//########################## Slots ###############################
//################################################################
void CartesianPlot::addAxis(){
	if (QObject::sender() == addHorizontalAxisAction) {
		Axis* axis = new Axis("x-axis", Axis::AxisHorizontal);
		if (axis->autoScale()) {
			axis->setUndoAware(false);
			axis->setStart(xMin());
			axis->setEnd(xMax());
			axis->setUndoAware(true);
		}
		addChild(axis);
	} else {
		Axis* axis = new Axis("y-axis", Axis::AxisVertical);
		if (axis->autoScale()) {
			axis->setUndoAware(false);
			axis->setStart(yMin());
			axis->setEnd(yMax());
			axis->setUndoAware(true);
		}
		addChild(axis);
	}
}

XYCurve* CartesianPlot::addCurve(){
	XYCurve* curve = new XYCurve("xy-curve");
	this->addChild(curve);
	connect(curve, SIGNAL(dataChanged()), this, SLOT(dataChanged()));
	connect(curve, SIGNAL(xDataChanged()), this, SLOT(xDataChanged()));
	connect(curve, SIGNAL(yDataChanged()), this, SLOT(yDataChanged()));

	//update the legend on changes of the name, line and symbol styles
	connect(curve, SIGNAL(aspectDescriptionChanged(const AbstractAspect*)), this, SLOT(updateLegend()));
	connect(curve, SIGNAL(lineTypeChanged(XYCurve::LineType)), this, SLOT(updateLegend()));
	connect(curve, SIGNAL(linePenChanged(QPen)), this, SLOT(updateLegend()));
	connect(curve, SIGNAL(symbolsTypeIdChanged(QString)), this, SLOT(updateLegend()));
	connect(curve, SIGNAL(symbolsPenChanged(QPen)), this, SLOT(updateLegend()));

	return curve;
}

XYEquationCurve* CartesianPlot::addEquationCurve(){
	XYEquationCurve* curve = new XYEquationCurve("f(x)");
	this->addChild(curve);
	connect(curve, SIGNAL(dataChanged()), this, SLOT(dataChanged()));
	connect(curve, SIGNAL(xDataChanged()), this, SLOT(xDataChanged()));
	connect(curve, SIGNAL(yDataChanged()), this, SLOT(yDataChanged()));

	//update the legend on changes of the name, line and symbol styles
	connect(curve, SIGNAL(aspectDescriptionChanged(const AbstractAspect*)), this, SLOT(updateLegend()));
	connect(curve, SIGNAL(lineTypeChanged(XYCurve::LineType)), this, SLOT(updateLegend()));
	connect(curve, SIGNAL(linePenChanged(QPen)), this, SLOT(updateLegend()));
	connect(curve, SIGNAL(symbolsTypeIdChanged(QString)), this, SLOT(updateLegend()));
	connect(curve, SIGNAL(symbolsPenChanged(QPen)), this, SLOT(updateLegend()));

	return curve;
}

XYFitCurve* CartesianPlot::addFitCurve(){
	XYFitCurve* curve = new XYFitCurve("fit");
	this->addChild(curve);
	connect(curve, SIGNAL(dataChanged()), this, SLOT(dataChanged()));
	connect(curve, SIGNAL(xDataChanged()), this, SLOT(xDataChanged()));
	connect(curve, SIGNAL(yDataChanged()), this, SLOT(yDataChanged()));

	//update the legend on changes of the name, line and symbol styles
	connect(curve, SIGNAL(aspectDescriptionChanged(const AbstractAspect*)), this, SLOT(updateLegend()));
	connect(curve, SIGNAL(lineTypeChanged(XYCurve::LineType)), this, SLOT(updateLegend()));
	connect(curve, SIGNAL(linePenChanged(QPen)), this, SLOT(updateLegend()));
	connect(curve, SIGNAL(symbolsTypeIdChanged(QString)), this, SLOT(updateLegend()));
	connect(curve, SIGNAL(symbolsPenChanged(QPen)), this, SLOT(updateLegend()));

	return curve;
}

void CartesianPlot::addLegend(){
	m_legend = new CartesianPlotLegend(this, "legend");
	this->addChild(m_legend);

	//only one legend is allowed -> disable the action
	addLegendAction->setEnabled(false);
}

void CartesianPlot::childAdded(const AbstractAspect* child) {
	const XYCurve* curve = qobject_cast<const XYCurve*>(child);
	if (curve)
		updateLegend();
}

void CartesianPlot::childRemoved(const AbstractAspect* parent, const AbstractAspect* before, const AbstractAspect* child) {
	Q_UNUSED(parent);
	Q_UNUSED(before);
	if (m_legend == child) {
		addLegendAction->setEnabled(true);
		m_legend = 0;
	} else {
		const XYCurve* curve = qobject_cast<const XYCurve*>(child);
		if (curve)
			updateLegend();
	}
}

void CartesianPlot::updateLegend() {
	if (m_legend)
		m_legend->retransform();
}

/*!
	called when in one of the curves the data was changed.
	Autoscales the coordinate system and the x-axes, when "auto-scale" is active.
*/
void CartesianPlot::dataChanged(){
	qDebug()<<"CartesianPlot::dataChanged()";
	Q_D(CartesianPlot);
	XYCurve* curve = dynamic_cast<XYCurve*>(QObject::sender());
	Q_ASSERT(curve);
	if (d->autoScaleX && d->autoScaleY)
		this->scaleAuto();
	else if (d->autoScaleX)
		this->scaleAutoX();
	else if (d->autoScaleY)
		this->scaleAutoY();
	else
		curve->retransform();
}

/*!
	called when in one of the curves the x-data was changed.
	Autoscales the coordinate system and the x-axes, when "auto-scale" is active.
*/
void CartesianPlot::xDataChanged(){
	qDebug()<<"CartesianPlot::xDataChanged()";
	Q_D(CartesianPlot);
	XYCurve* curve = dynamic_cast<XYCurve*>(QObject::sender());
	Q_ASSERT(curve);
	if (d->autoScaleX)
		this->scaleAutoX();
	else
		curve->retransform();
}

/*!
	called when in one of the curves the x-data was changed.
	Autoscales the coordinate system and the x-axes, when "auto-scale" is active.
*/
void CartesianPlot::yDataChanged(){
	qDebug()<<"CartesianPlot::yDataChanged()";
	Q_D(CartesianPlot);
	XYCurve* curve = dynamic_cast<XYCurve*>(QObject::sender());
	Q_ASSERT(curve);
	if (d->autoScaleY)
		this->scaleAutoY();
	else
		curve->retransform();
}

void CartesianPlot::mouseModeChanged(QAction* action) {
	Q_D(CartesianPlot);
	if (action==selectionModeAction) {
		d->scene()->views().first()->setCursor(Qt::ArrowCursor);
		d->mouseMode = CartesianPlot::SelectionMode;
		d->setHandlesChildEvents(false);
	} else if (action==zoomSelectionModeAction) {
		d->scene()->views().first()->setCursor(Qt::CrossCursor);
		d->mouseMode = CartesianPlot::ZoomSelectionMode;
		d->setHandlesChildEvents(true);
	} else if (action==zoomXSelectionModeAction) {
		d->scene()->views().first()->setCursor(Qt::SizeHorCursor);
		d->mouseMode = CartesianPlot::ZoomXSelectionMode;
		d->setHandlesChildEvents(true);
	} else if (action==zoomYSelectionModeAction) {
		d->scene()->views().first()->setCursor(Qt::SizeVerCursor);
		d->mouseMode = CartesianPlot::ZoomYSelectionMode;
		d->setHandlesChildEvents(true);
	}

	QList<QGraphicsItem*> items = d->childItems();
	if (d->mouseMode == CartesianPlot::SelectionMode) {
		foreach(QGraphicsItem* item, items)
			item->setFlag(QGraphicsItem::ItemStacksBehindParent, false);
	} else {
		foreach(QGraphicsItem* item, items)
			item->setFlag(QGraphicsItem::ItemStacksBehindParent, true);
	}
	d->update();
}

void CartesianPlot::scaleAutoX(){
	Q_D(CartesianPlot);

	//loop over all xy-curves and determine the maximum x-value
	double min = INFINITY;
	double max = -INFINITY;
	QList<XYCurve*> children = this->children<XYCurve>();
	foreach(XYCurve* curve, children) {
		if (!curve->xColumn())
			continue;

		if (curve->xColumn()->minimum() != INFINITY){
			if (curve->xColumn()->minimum() < min)
				min = curve->xColumn()->minimum();
		}

		if (curve->xColumn()->maximum() != -INFINITY){
			if (curve->xColumn()->maximum() > max)
				max = curve->xColumn()->maximum();
		}
	}

	bool update = false;
	if (min != d->xMin && min != INFINITY){
		d->xMin = min;
		update = true;
	}

	if (max != d->xMax && max != -INFINITY){
		d->xMax = max;
		update = true;
	}

	if(update){
		if (d->xMax == d->xMin){
			//in case min and max are equal (e.g. if we plot a single point), subtract/add 10% of the value
			if (d->xMax!=0){
				d->xMax = d->xMax*1.1;
				d->xMin = d->xMin*0.9;
			}else{
				d->xMax = 0.1;
				d->xMin = -0.1;
			}
		}else{
			float offset = (d->xMax - d->xMin)*d->autoScaleOffsetFactor;
			d->xMin -= offset;
			d->xMax += offset;
		}
		d->retransformScales();
	}
}

void CartesianPlot::scaleAutoY(){
	qDebug()<<"CartesianPlot::scaleAutoY()";
	Q_D(CartesianPlot);

	//loop over all xy-curves and determine the maximum y-value
	double min = INFINITY;
	double max = -INFINITY;
	QList<XYCurve*> children = this->children<XYCurve>();
	foreach(XYCurve* curve, children) {
		if (!curve->yColumn())
			continue;

		if (curve->yColumn()->minimum() != INFINITY){
			if (curve->yColumn()->minimum() < min)
				min = curve->yColumn()->minimum();
		}

		if (curve->yColumn()->maximum() != -INFINITY){
			if (curve->yColumn()->maximum() > max)
				max = curve->yColumn()->maximum();
		}
	}

	bool update = false;
	if (min != d->yMin && min != INFINITY){
		d->yMin = min;
		update = true;
	}

	if (max != d->yMax && max != -INFINITY){
		d->yMax = max;
		update = true;
	}

	if(update){
		if (d->yMax == d->yMin){
			//in case min and max are equal (e.g. if we plot a single point), subtract/add 10% of the value
			if (d->yMax!=0){
				d->yMax = d->yMax*1.1;
				d->yMin = d->yMin*0.9;
			}else{
				d->yMax = 0.1;
				d->yMin = -0.1;
			}
		}else{
			float offset = (d->yMax - d->yMin)*d->autoScaleOffsetFactor;
			d->yMin -= offset;
			d->yMax += offset;
		}
		d->retransformScales();
	}
}

void CartesianPlot::scaleAuto(){
	Q_D(CartesianPlot);

	//loop over all xy-curves and determine the maximum x-value
	double xMin = INFINITY;
	double xMax = -INFINITY;
	double yMin = INFINITY;
	double yMax = -INFINITY;
	QList<XYCurve*> children = this->children<XYCurve>();
	foreach(XYCurve* curve, children) {
		if (!curve->xColumn())
			continue;

		if (curve->xColumn()->minimum() != INFINITY){
			if (curve->xColumn()->minimum() < xMin)
				xMin = curve->xColumn()->minimum();
		}

		if (curve->xColumn()->maximum() != -INFINITY){
			if (curve->xColumn()->maximum() > xMax)
				xMax = curve->xColumn()->maximum();
		}

		if (!curve->yColumn())
			continue;

		if (curve->yColumn()->minimum() != INFINITY){
			if (curve->yColumn()->minimum() < yMin)
				yMin = curve->yColumn()->minimum();
		}

		if (curve->yColumn()->maximum() != -INFINITY){
			if (curve->yColumn()->maximum() > yMax)
				yMax = curve->yColumn()->maximum();
		}
	}

	bool updateX = false;
	bool updateY = false;
	if (xMin != d->xMin && xMin != INFINITY){
		d->xMin = xMin;
		updateX = true;
	}

	if (xMax != d->xMax && xMax != -INFINITY){
		d->xMax = xMax;
		updateX = true;
	}

	if (yMin != d->yMin && yMin != INFINITY){
		d->yMin = yMin;
		updateY = true;
	}

	if (yMax != d->yMax && yMax != -INFINITY){
		d->yMax = yMax;
		updateY = true;
	}

	if(updateX || updateY){
		if (updateX){
			if (d->xMax == d->xMin) {
				//in case min and max are equal (e.g. if we plot a single point), subtract/add 10% of the value
				if (d->xMax!=0){
					d->xMax = d->xMax*1.1;
					d->xMin = d->xMin*0.9;
				}else{
					d->xMax = 0.1;
					d->xMin = -0.1;
				}
			} else {
				float offset = (d->xMax - d->xMin)*d->autoScaleOffsetFactor;
				d->xMin -= offset;
				d->xMax += offset;
			}
		}
		if (updateY){
			if (d->yMax == d->yMin) {
				//in case min and max are equal (e.g. if we plot a single point), subtract/add 10% of the value
				if (d->yMax!=0){
					d->yMax = d->yMax*1.1;
					d->yMin = d->yMin*0.9;
				}else{
					d->yMax = 0.1;
					d->yMin = -0.1;
				}
			} else {
				float offset = (d->yMax - d->yMin)*d->autoScaleOffsetFactor;
				d->yMin -= offset;
				d->yMax += offset;
			}
		}
		d->retransformScales();
	}
}

void CartesianPlot::zoomIn(){
	Q_D(CartesianPlot);
	float oldRange = (d->xMax-d->xMin);
	float newRange = (d->xMax-d->xMin)/m_zoomFactor;
	d->xMax = d->xMax + (newRange-oldRange)/2;
	d->xMin = d->xMin - (newRange-oldRange)/2;

	oldRange = (d->yMax-d->yMin);
	newRange = (d->yMax-d->yMin)/m_zoomFactor;
	d->yMax = d->yMax + (newRange-oldRange)/2;
	d->yMin = d->yMin - (newRange-oldRange)/2;

	d->retransformScales();
}

void CartesianPlot::zoomOut(){
	Q_D(CartesianPlot);
	float oldRange = (d->xMax-d->xMin);
	float newRange = (d->xMax-d->xMin)*m_zoomFactor;
	d->xMax = d->xMax + (newRange-oldRange)/2;
	d->xMin = d->xMin - (newRange-oldRange)/2;

	oldRange = (d->yMax-d->yMin);
	newRange = (d->yMax-d->yMin)*m_zoomFactor;
	d->yMax = d->yMax + (newRange-oldRange)/2;
	d->yMin = d->yMin - (newRange-oldRange)/2;

	d->retransformScales();
}

void CartesianPlot::zoomInX(){
	Q_D(CartesianPlot);
	float oldRange = (d->xMax-d->xMin);
	float newRange = (d->xMax-d->xMin)/m_zoomFactor;
	d->xMax = d->xMax + (newRange-oldRange)/2;
	d->xMin = d->xMin - (newRange-oldRange)/2;
	d->retransformScales();
}

void CartesianPlot::zoomOutX(){
	Q_D(CartesianPlot);
	float oldRange = (d->xMax-d->xMin);
	float newRange = (d->xMax-d->xMin)*m_zoomFactor;
	d->xMax = d->xMax + (newRange-oldRange)/2;
	d->xMin = d->xMin - (newRange-oldRange)/2;
	d->retransformScales();
}

void CartesianPlot::zoomInY(){
	Q_D(CartesianPlot);
	float oldRange = (d->yMax-d->yMin);
	float newRange = (d->yMax-d->yMin)/m_zoomFactor;
	d->yMax = d->yMax + (newRange-oldRange)/2;
	d->yMin = d->yMin - (newRange-oldRange)/2;
	d->retransformScales();
}

void CartesianPlot::zoomOutY(){
	Q_D(CartesianPlot);
	float oldRange = (d->yMax-d->yMin);
	float newRange = (d->yMax-d->yMin)*m_zoomFactor;
	d->yMax = d->yMax + (newRange-oldRange)/2;
	d->yMin = d->yMin - (newRange-oldRange)/2;
	d->retransformScales();
}

void CartesianPlot::shiftLeftX(){
	Q_D(CartesianPlot);
	float offsetX = (d->xMax-d->xMin)*0.1;
	d->xMax -= offsetX;
	d->xMin -= offsetX;
	d->retransformScales();
}

void CartesianPlot::shiftRightX(){
	Q_D(CartesianPlot);
	float offsetX = (d->xMax-d->xMin)*0.1;
	d->xMax += offsetX;
	d->xMin += offsetX;
	d->retransformScales();
}

void CartesianPlot::shiftUpY(){
	Q_D(CartesianPlot);
	float offsetY = (d->yMax-d->yMin)*0.1;
	d->yMax += offsetY;
	d->yMin += offsetY;
	d->retransformScales();
}

void CartesianPlot::shiftDownY(){
	Q_D(CartesianPlot);
	float offsetY = (d->yMax-d->yMin)*0.1;
	d->yMax -= offsetY;
	d->yMin -= offsetY;
	d->retransformScales();
}

//##############################################################################
//######  SLOTs for changes triggered via QActions in the context menu  ########
//##############################################################################
void CartesianPlot::visibilityChanged(){
	Q_D(CartesianPlot);
	this->setVisible(!d->isVisible());
}

//#####################################################################
//################### Private implementation ##########################
//#####################################################################
CartesianPlotPrivate::CartesianPlotPrivate(CartesianPlot *owner)
	: AbstractPlotPrivate(owner), q(owner),suppressRetransform(false),
	m_printing(false), m_selectionBandIsShown(false), cSystem(0),
	mouseMode(CartesianPlot::SelectionMode){
}

/*!
	updates the position of plot rectangular in scene coordinates to \c r and recalculates the scales.
	The size of the plot corresponds to the size of the plot area, the area which is filled with the background color etc.
	and which can pose the parent item for several sub-items (like TextLabel).
	Note, the size of the area used to define the coordinate system doesn't need to be equal to this plot area.
	Also, the size (=bounding box) of CartesianPlot can be greater than the size of the plot area.
 */
void CartesianPlotPrivate::retransform(){
	if (suppressRetransform)
		return;

	qDebug()<<"CartesianPlotPrivate::retransform";
	prepareGeometryChange();
	setPos( rect.x()+rect.width()/2, rect.y()+rect.height()/2);

	retransformScales();

	//plotArea position is always (0, 0) in parent's coordinates, don't need to update here
	q->plotArea()->setRect(rect);

	//call retransform() for the title and the legend (if available)
	//when a predefined position relative to the (Left, Centered etc.) is used,
	//the actual position needs to be updated on plot's geometry changes.
	q->title()->retransform();
	if (q->m_legend)
		q->m_legend->retransform();

	q->retransform();
}

void CartesianPlotPrivate::retransformScales(){
	qDebug()<<"CartesianPlotPrivate::retransformScales";
	CartesianPlot* plot = dynamic_cast<CartesianPlot*>(q);
	QList<CartesianCoordinateSystem::Scale*> scales;
	double sceneStart, sceneEnd, logicalStart, logicalEnd;

	//perform the mapping from the scene coordinates to the plot's coordinates here.
	QRectF itemRect = mapRectFromScene(rect);

	//check ranges for log-scales
	if (xScale != CartesianPlot::ScaleLinear)
		checkXRange();

	//create x-scales
	if (xScaleBreakings.list.size()==0) {
		sceneStart = itemRect.x()+horizontalPadding;
		sceneEnd = itemRect.x()+itemRect.width()-horizontalPadding;
		logicalStart = xMin;
		logicalEnd = xMax;
		Interval<double> interval (SCALE_MIN, SCALE_MAX);
		scales << this->createScale(xScale, interval, sceneStart, sceneEnd, logicalStart, logicalEnd);
	} else {
		//TODO:
// 		foreach(CartesianPlot::ScaleBreaking* breaking, xScaleBreakings) {
// 		}
	}

	cSystem ->setXScales(scales);

	//check ranges for log-scales
	if (yScale != CartesianPlot::ScaleLinear)
		checkYRange();

	//create y-scales
	scales.clear();
	if (yScaleBreakings.list.size()==0) {
		sceneStart = itemRect.y()+itemRect.height()-verticalPadding;
		sceneEnd = itemRect.y()+verticalPadding;
		logicalStart = yMin;
		logicalEnd = yMax;
		Interval<double> interval (SCALE_MIN, SCALE_MAX);
		scales << this->createScale(yScale, interval, sceneStart, sceneEnd, logicalStart, logicalEnd);
	} else {
		//TODO:
// 		foreach(CartesianPlot::ScaleBreaking* breaking, xScaleBreakings) {
// 		}
	}

	cSystem ->setYScales(scales);

	//calculate the changes in x and y and save the current values for xMin, xMax, yMin, yMax
	float deltaXMin = 0;
	float deltaXMax = 0;
	float deltaYMin = 0;
	float deltaYMax = 0;

	if (xMin!=xMinPrev) {
		deltaXMin = xMin - xMinPrev;
		emit plot->xMinChanged(xMin);
	}

	if (xMax!=xMaxPrev) {
		deltaXMax = xMax - xMaxPrev;
		emit plot->xMaxChanged(xMax);
	}

	if (yMin!=yMinPrev) {
		deltaYMin = yMin - yMinPrev;
		emit plot->yMinChanged(yMin);
	}

	if (yMax!=yMaxPrev) {
		deltaYMax = yMax - yMaxPrev;
		emit plot->yMaxChanged(yMax);
	}

	xMinPrev = xMin;
	xMaxPrev = xMax;
	yMinPrev = yMin;
	yMaxPrev = yMax;

	//adjust auto-scale axes
	QList<Axis*> childElements = q->children<Axis>();
	foreach(Axis* axis, childElements){
		if (!axis->autoScale())
			continue;

		if (axis->orientation() == Axis::AxisHorizontal){
			if (deltaXMax!=0) {
				axis->setUndoAware(false);
				axis->setEnd(xMax);
				axis->setUndoAware(true);
			}
			if (deltaXMin!=0) {
				axis->setUndoAware(false);
				axis->setStart(xMin);
				axis->setUndoAware(true);
			}

// 			if (axis->position() == Axis::AxisCustom && deltaYMin != 0){
// 				axis->setOffset(axis->offset() + deltaYMin, false);
// 			}
		}else{
			if (deltaYMax!=0) {
				axis->setUndoAware(false);
				axis->setEnd(yMax);
				axis->setUndoAware(true);
			}
			if (deltaYMin!=0) {
				axis->setUndoAware(false);
				axis->setStart(yMin);
				axis->setUndoAware(true);
			}

// 			if (axis->position() == Axis::AxisCustom && deltaXMin != 0){
// 				axis->setOffset(axis->offset() + deltaXMin, false);
// 			}
		}
	}

	// call retransform() on the parent to trigger the update of all axes and curves
	q->retransform();
}

/*!
 * don't allow any negative values on for the x range when log or sqrt scalings are used
 */
void CartesianPlotPrivate::checkXRange() {
	double min = 0.01;

	if (xMin <= 0.0) {
		(min < xMax*min) ? xMin = min : xMin = xMax*min;
		emit q->xMinChanged(xMin);
	}else if (xMax <= 0.0) {
		(-min > xMin*min) ? xMax = -min : xMax = xMin*min;
		emit q->xMaxChanged(xMax);
	}
}

/*!
 * don't allow any negative values on for the y range when log or sqrt scalings are used
 */
void CartesianPlotPrivate::checkYRange() {
	double min = 0.01;

	if (yMin <= 0.0) {
		(min < yMax*min) ? yMin = min : yMin = yMax*min;
		emit q->yMinChanged(yMin);
	}else if (yMax <= 0.0) {
		(-min > yMin*min) ? yMax = -min : yMax = yMin*min;
		emit q->yMaxChanged(yMax);
	}
}

float CartesianPlotPrivate::round(float value, int precision){
	return int(value*pow(10, precision) + (value<0 ? -0.5 : 0.5))/pow(10, precision);
}


CartesianCoordinateSystem::Scale* CartesianPlotPrivate::createScale(CartesianPlot::Scale type, Interval<double>& interval,
																	double sceneStart, double sceneEnd,
																	double logicalStart, double logicalEnd) {
	if (type == CartesianPlot::ScaleLinear){
		return CartesianCoordinateSystem::Scale::createLinearScale(interval, sceneStart, sceneEnd, logicalStart, logicalEnd);
	}else {
		float base;
		if (type == CartesianPlot::ScaleLog10)
			base = 10.0;
		else if (type == CartesianPlot::ScaleLog2)
			base = 2.0;
		else
			base = 2.71828;

		return CartesianCoordinateSystem::Scale::createLogScale(interval, sceneStart, sceneEnd, logicalStart, logicalEnd, base);
	}
}

/*!
 * Reimplemented from QGraphicsItem.
 */
QVariant CartesianPlotPrivate::itemChange(GraphicsItemChange change, const QVariant &value){
	if (change == QGraphicsItem::ItemPositionChange) {
		const QPointF& itemPos = value.toPointF();//item's center point in parent's coordinates;
		float x = itemPos.x();
		float y = itemPos.y();

		//calculate the new rect and forward the changes to the frontend
		QRectF newRect;
		float w = rect.width();
		float h = rect.height();
		newRect.setX(x-w/2);
		newRect.setY(y-h/2);
		newRect.setWidth(w);
		newRect.setHeight(h);
		emit q->rectChanged(newRect);
     }
	return QGraphicsItem::itemChange(change, value);
}

void CartesianPlotPrivate::mousePressEvent(QGraphicsSceneMouseEvent *event) {
	if (mouseMode == CartesianPlot::ZoomSelectionMode || mouseMode == CartesianPlot::ZoomXSelectionMode || mouseMode == CartesianPlot::ZoomYSelectionMode) {

		if (mouseMode==CartesianPlot::ZoomSelectionMode) {
			m_selectionStart = event->pos();
		} else if (mouseMode==CartesianPlot::ZoomXSelectionMode) {
			//determine the start point of the selection band in item's coordinates first (set y to the current maximal value),
			//and map the start point to global coordinates (=m_rubberBandStart)
			float ymax = cSystem->mapLogicalToScene(QPointF(0, yMax), AbstractCoordinateSystem::SuppressPageClipping).y();
			m_selectionStart.setX(event->pos().x());
			m_selectionStart.setY(ymax);
		} else if (mouseMode==CartesianPlot::ZoomYSelectionMode) {
			//determine the start point of the selection band in item's coordinates first (set x to the current minimal value),
			//and map the start point to global coordinates (=m_rubberBandStart)
			float xmin = cSystem->mapLogicalToScene(QPointF(xMin, 0), AbstractCoordinateSystem::SuppressPageClipping).x();
			m_selectionStart.setX(xmin);
			m_selectionStart.setY(event->pos().y());
		}

		m_selectionEnd = m_selectionStart;
		m_selectionBandIsShown = true;
	} else {
		QGraphicsItem::mousePressEvent(event);
	}
}

void CartesianPlotPrivate::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
	if (mouseMode == CartesianPlot::SelectionMode) {
		QGraphicsItem::mouseMoveEvent(event);
	} else if (mouseMode == CartesianPlot::ZoomSelectionMode || mouseMode == CartesianPlot::ZoomXSelectionMode || mouseMode == CartesianPlot::ZoomYSelectionMode) {
		QGraphicsItem::mouseMoveEvent(event);
		if ( !boundingRect().contains(event->pos()) )
			return;

		if (mouseMode==CartesianPlot::ZoomSelectionMode) {
			m_selectionEnd = event->pos();
		} else if (mouseMode==CartesianPlot::ZoomXSelectionMode) {
			//determine the end point of the selection band in item's coordinates first (set y to the current minimal value),
			//and map the end point to global coordinates (=m_rubberBandStart)
			float ymin = cSystem->mapLogicalToScene(QPointF(0, yMin), AbstractCoordinateSystem::SuppressPageClipping).y();
			m_selectionEnd.setX(event->pos().x());
			m_selectionEnd.setY(ymin);
		} else if (mouseMode==CartesianPlot::ZoomYSelectionMode) {
			//determine the end point of the selection band in item's coordinates first (set x to the current maximal value),
			//and map the end point to global coordinates (=m_rubberBandStart)
			float xmax = cSystem->mapLogicalToScene(QPointF(xMax, 0), AbstractCoordinateSystem::SuppressPageClipping).x();
			m_selectionEnd.setX(xmax);
			m_selectionEnd.setY(event->pos().y());
		}
		update();
	}


	//TODO: implement the navigation in plot on mouse move events,
	//calculate the position changes and call shift*()-functions
}

void CartesianPlotPrivate::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
	if (mouseMode == CartesianPlot::SelectionMode) {
		const QPointF& itemPos = pos();//item's center point in parent's coordinates;
		float x = itemPos.x();
		float y = itemPos.y();

		//calculate the new rect and set it
		QRectF newRect;
		float w = rect.width();
		float h = rect.height();
		newRect.setX(x-w/2);
		newRect.setY(y-h/2);
		newRect.setWidth(w);
		newRect.setHeight(h);

		suppressRetransform = true;
		q->setRect(newRect);
		suppressRetransform = false;

		QGraphicsItem::mouseReleaseEvent(event);
	} else if (mouseMode == CartesianPlot::ZoomSelectionMode || mouseMode == CartesianPlot::ZoomXSelectionMode || mouseMode == CartesianPlot::ZoomYSelectionMode) {
		if (mouseMode == CartesianPlot::ZoomSelectionMode) {
			m_selectionEnd = event->pos();
		} else if (mouseMode == CartesianPlot::ZoomXSelectionMode ) {
			m_selectionEnd.setX(event->pos().x());
			float ymin = cSystem->mapLogicalToScene(QPointF(0, yMin), AbstractCoordinateSystem::SuppressPageClipping).y();
			m_selectionEnd.setY(ymin);
		} else if (mouseMode == CartesianPlot::ZoomYSelectionMode ) {
			float xmax = cSystem->mapLogicalToScene(QPointF(xMax, 0), AbstractCoordinateSystem::SuppressPageClipping).x();
			m_selectionEnd.setX(xmax);
			m_selectionEnd.setY(event->pos().y());
		}

		//don't zoom if very small region was selected, avoid occasional/unwanted zooming
		if ( abs(m_selectionEnd.x()-m_selectionStart.x())<20 || abs(m_selectionEnd.y()-m_selectionStart.y())<20 ) {
			m_selectionBandIsShown = false;
			return;
		}

		//determine the new plot ranges
		QPointF logicalZoomStart = cSystem->mapSceneToLogical(m_selectionStart, AbstractCoordinateSystem::SuppressPageClipping);
		QPointF logicalZoomEnd = cSystem->mapSceneToLogical(m_selectionEnd, AbstractCoordinateSystem::SuppressPageClipping);
		if (m_selectionEnd.x()>m_selectionStart.x()) {
			xMin = logicalZoomStart.x();
			xMax = logicalZoomEnd.x();
		} else {
			xMin = logicalZoomEnd.x();
			xMax = logicalZoomStart.x();
		}

		if (m_selectionEnd.y()>m_selectionStart.y()) {
			yMin = logicalZoomEnd.y();
			yMax = logicalZoomStart.y();
		} else {
			yMin = logicalZoomStart.y();
			yMax = logicalZoomEnd.y();
		}

		m_selectionBandIsShown = false;
		retransformScales();
	}
}

void CartesianPlotPrivate::wheelEvent(QGraphicsSceneWheelEvent* event) {
	//determine first, which axes are selected and zoom only in the corresponding direction.
	//zoom the entire plot if no axes selected.
	bool zoomX = false;
	bool zoomY = false;
	QList<Axis*> axes = q->children<Axis>();
	foreach(Axis* axis, axes){
		if (!axis->graphicsItem()->isSelected())
			continue;

		if (axis->orientation() == Axis::AxisHorizontal)
			zoomX  = true;
		else
			zoomY = true;
	}

	if (event->delta() > 0) {
		if (!zoomX && !zoomY) {
			//no special axis selected -> zoom in everything
			q->zoomIn();
		} else {
			if (zoomX) q->zoomInX();
			if (zoomY) q->zoomInY();
		}
	} else {
		if (!zoomX && !zoomY) {
			//no special axis selected -> zoom in everything
			q->zoomOut();
		} else {
			if (zoomX) q->zoomOutX();
			if (zoomY) q->zoomOutY();
		}
	}
}

void CartesianPlotPrivate::hoverMoveEvent (QGraphicsSceneHoverEvent* event){
	QPointF point = event->pos();
	if (q->plotRect().contains(point)){
		QPointF logicalPoint = cSystem->mapSceneToLogical(point);
		QString info = "x=" + QString::number(logicalPoint.x()) + ", y=" + QString::number(logicalPoint.y());
		q->info(info);
		if (mouseMode == CartesianPlot::ZoomXSelectionMode && !m_selectionBandIsShown) {
			QPointF p1(logicalPoint.x(), yMin);
			QPointF p2(logicalPoint.x(), yMax);
			m_selectionStartLine.setP1(cSystem->mapLogicalToScene(p1));
			m_selectionStartLine.setP2(cSystem->mapLogicalToScene(p2));
			update();
		} else if (mouseMode == CartesianPlot::ZoomYSelectionMode && !m_selectionBandIsShown) {
			QPointF p1(xMin, logicalPoint.y());
			QPointF p2(xMax, logicalPoint.y());
			m_selectionStartLine.setP1(cSystem->mapLogicalToScene(p1));
			m_selectionStartLine.setP2(cSystem->mapLogicalToScene(p2));
			update();
		}
	} else {
		q->info("");
	}

	QGraphicsItem::hoverMoveEvent(event);
}

void CartesianPlotPrivate::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget * widget) {
	if (!isVisible())
		return;

	painter->setPen(QPen(Qt::black, 3));
	if ( (mouseMode == CartesianPlot::ZoomXSelectionMode || mouseMode == CartesianPlot::ZoomYSelectionMode)
		&& (!m_selectionBandIsShown)) {
		painter->drawLine(m_selectionStartLine);
	}

	if (m_selectionBandIsShown){
		painter->save();
		painter->setPen(QPen(Qt::black, 5));
		painter->drawRect(QRectF(m_selectionStart, m_selectionEnd));
		painter->setBrush(Qt::blue);
		painter->setOpacity(0.2);
		painter->drawRect(QRectF(m_selectionStart, m_selectionEnd));
		painter->restore();
	}

	WorksheetElementContainerPrivate::paint(painter, option, widget);
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

	//geometry
	writer->writeStartElement( "geometry" );
	writer->writeAttribute( "x", QString::number(d->rect.x()) );
	writer->writeAttribute( "y", QString::number(d->rect.y()) );
	writer->writeAttribute( "width", QString::number(d->rect.width()) );
	writer->writeAttribute( "height", QString::number(d->rect.height()) );
	writer->writeAttribute( "visible", QString::number(d->isVisible()) );
	writer->writeEndElement();

	//coordinate system and padding
	writer->writeStartElement( "coordinateSystem" );
	writer->writeAttribute( "autoScaleX", QString::number(d->autoScaleX) );
	writer->writeAttribute( "autoScaleY", QString::number(d->autoScaleY) );
	writer->writeAttribute( "xMin", QString::number(d->xMin) );
	writer->writeAttribute( "xMax", QString::number(d->xMax) );
	writer->writeAttribute( "yMin", QString::number(d->yMin) );
	writer->writeAttribute( "yMax", QString::number(d->yMax) );
	writer->writeAttribute( "xScale", QString::number(d->xScale) );
	writer->writeAttribute( "yScale", QString::number(d->yScale) );
	writer->writeAttribute( "horizontalPadding", QString::number(d->horizontalPadding) );
	writer->writeAttribute( "verticalPadding", QString::number(d->verticalPadding) );
	writer->writeEndElement();

	//x-scale breakings
	if (d->xScaleBreakings.list.size()) {
		writer->writeStartElement("xScaleBreakings");
		foreach(const ScaleBreaking& breaking, d->xScaleBreakings.list) {
			writer->writeStartElement("item");
			writer->writeAttribute("start", QString::number(breaking.start));
			writer->writeAttribute("end", QString::number(breaking.end));
			writer->writeAttribute("position", QString::number(breaking.position));
			writer->writeAttribute("style", QString::number(breaking.style));
			writer->writeAttribute("isValid", QString::number(breaking.isValid));
			writer->writeEndElement();
		}
		writer->writeEndElement();
	}

	//y-scale breakings
	if (d->yScaleBreakings.list.size()) {
		writer->writeStartElement("yScaleBreakings");
		foreach(const ScaleBreaking& breaking, d->yScaleBreakings.list) {
			writer->writeStartElement("item");
			writer->writeAttribute("start", QString::number(breaking.start));
			writer->writeAttribute("end", QString::number(breaking.end));
			writer->writeAttribute("position", QString::number(breaking.position));
			writer->writeAttribute("style", QString::number(breaking.style));
			writer->writeAttribute("isValid", QString::number(breaking.isValid));
			writer->writeEndElement();
		}
		writer->writeEndElement();
	}

    //serialize all children (plot area, title text label, axes and curves)
    QList<AbstractWorksheetElement *> childElements = children<AbstractWorksheetElement>(IncludeHidden);
    foreach(AbstractWorksheetElement *elem, childElements)
        elem->save(writer);

    writer->writeEndElement(); // close "cartesianPlot" section
}


//! Load from XML
bool CartesianPlot::load(XmlStreamReader* reader){
	Q_D(CartesianPlot);

    if(!reader->isStartElement() || reader->name() != "cartesianPlot"){
        reader->raiseError(i18n("no cartesianPlot element found"));
        return false;
    }

    if (!readBasicAttributes(reader))
        return false;

    QString attributeWarning = i18n("Attribute '%1' missing or empty, default value is used");
    QXmlStreamAttributes attribs;
    QString str;

    while (!reader->atEnd()){
        reader->readNext();
        if (reader->isEndElement() && reader->name() == "cartesianPlot")
            break;

        if (!reader->isStartElement())
            continue;

        if (reader->name() == "comment"){
            if (!readCommentElement(reader))
				return false;
		}else if(reader->name() == "geometry"){
            attribs = reader->attributes();

            str = attribs.value("x").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'x'"));
            else
                d->rect.setX( str.toDouble() );

			str = attribs.value("y").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'y'"));
            else
                d->rect.setY( str.toDouble() );

			str = attribs.value("width").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'width'"));
            else
                d->rect.setWidth( str.toDouble() );

			str = attribs.value("height").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'height'"));
            else
                d->rect.setHeight( str.toDouble() );

			str = attribs.value("visible").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'visible'"));
            else
                d->setVisible(str.toInt());
		}else if(reader->name() == "coordinateSystem"){
            attribs = reader->attributes();

			str = attribs.value("autoScaleX").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'autoScaleX'"));
            else
                d->autoScaleX = bool(str.toInt());

			str = attribs.value("autoScaleY").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'autoScaleY'"));
            else
                d->autoScaleY = bool(str.toInt());

            str = attribs.value("xMin").toString();
            if(str.isEmpty()) {
                reader->raiseWarning(attributeWarning.arg("'xMin'"));
			} else {
                d->xMin = str.toDouble();
				d->xMinPrev = d->xMin;
			}

            str = attribs.value("xMax").toString();
            if(str.isEmpty()) {
                reader->raiseWarning(attributeWarning.arg("'xMax'"));
			} else {
                d->xMax = str.toDouble();
				d->xMaxPrev = d->xMax;
			}

            str = attribs.value("yMin").toString();
            if(str.isEmpty()) {
                reader->raiseWarning(attributeWarning.arg("'yMin'"));
			} else {
                d->yMin = str.toDouble();
				d->yMinPrev = d->yMin;
			}

            str = attribs.value("yMax").toString();
            if(str.isEmpty()) {
                reader->raiseWarning(attributeWarning.arg("'yMax'"));
			} else {
                d->yMax = str.toDouble();
				d->yMaxPrev = d->yMax;
			}

			str = attribs.value("xScale").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'xScale'"));
            else
                d->xScale = CartesianPlot::Scale(str.toInt());

			str = attribs.value("yScale").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'yScale'"));
            else
                d->yScale = CartesianPlot::Scale(str.toInt());

            str = attribs.value("horizontalPadding").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'horizontalPadding'"));
            else
                d->horizontalPadding = str.toDouble();

            str = attribs.value("verticalPadding").toString();
            if(str.isEmpty())
                reader->raiseWarning(attributeWarning.arg("'verticalPadding'"));
            else
                d->verticalPadding = str.toDouble();
        }else if(reader->name() == "textLabel"){
            m_title = new TextLabel("");
            if (!m_title->load(reader)){
                delete m_title;
				m_title=0;
                return false;
            }else{
                addChild(m_title);
            }
		}else if(reader->name() == "plotArea"){
			m_plotArea->load(reader);
		}else if(reader->name() == "axis"){
            Axis* axis = new Axis("");
            if (!axis->load(reader)){
                delete axis;
                return false;
            }else{
                addChild(axis);
            }
		}else if(reader->name() == "xyCurve"){
            XYCurve* curve = addCurve();
            if (!curve->load(reader)){
                removeChild(curve);
                return false;
            }
		}else if(reader->name() == "xyEquationCurve"){
			XYEquationCurve* curve = addEquationCurve();
            if (!curve->load(reader)){
				removeChild(curve);
                return false;
            }
		}else if(reader->name() == "xyFitCurve"){
			XYFitCurve* curve = addFitCurve();
            if (!curve->load(reader)){
				removeChild(curve);
                return false;
            }
		}else if(reader->name() == "cartesianPlotLegend"){
            m_legend = new CartesianPlotLegend(this, "");
            if (!m_legend->load(reader)){
                delete m_legend;
                return false;
            }else{
                addChild(m_legend);
				addLegendAction->setEnabled(false);	//only one legend is allowed -> disable the action
            }
        }else{ // unknown element
            reader->raiseWarning(i18n("unknown cartesianPlot element '%1'", reader->name().toString()));
            if (!reader->skipToEndElement()) return false;
        }
    }

	d->retransform();
	if (m_title) {
		m_title->setHidden(true);
		m_title->graphicsItem()->setParentItem(m_plotArea->graphicsItem());
	}

    return true;
}
