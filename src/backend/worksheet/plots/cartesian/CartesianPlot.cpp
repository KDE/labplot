/***************************************************************************
    File                 : CartesianPlot.cpp
    Project              : LabPlot
    Description          : Cartesian plot
    --------------------------------------------------------------------
    Copyright            : (C) 2011-2016 by Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2016 by Stefan Gerlach (stefan.gerlach@uni.kn)

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
#include "XYInterpolationCurve.h"
#include "XYSmoothCurve.h"
#include "XYFitCurve.h"
#include "XYFourierFilterCurve.h"
#include "XYFourierTransformCurve.h"
#include "backend/core/Project.h"
#include "backend/worksheet/plots/cartesian/CartesianPlotLegend.h"
#include "backend/worksheet/plots/cartesian/CustomPoint.h"
#include "backend/worksheet/plots/PlotArea.h"
#include "backend/worksheet/plots/AbstractPlotPrivate.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/Axis.h"
#include "backend/worksheet/TextLabel.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"

#include <QDir>
#include <QMenu>
#include <QToolBar>
#include <QPainter>

#include <KConfigGroup>
#include <KIcon>
#include <KAction>
#include <KLocale>
#include "kdefrontend/ThemeHandler.h"
#include "kdefrontend/widgets/ThemesWidget.h"

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

CartesianPlot::~CartesianPlot() {
	delete m_coordinateSystem;
	delete addNewMenu;
	delete zoomMenu;
	delete themeMenu;
	//don't need to delete objects added with addChild()

	//no need to delete the d-pointer here - it inherits from QGraphicsItem
	//and is deleted during the cleanup in QGraphicsScene
}

/*!
	initializes all member variables of \c CartesianPlot
*/
void CartesianPlot::init() {
	Q_D(CartesianPlot);

	d->cSystem = new CartesianCoordinateSystem(this);;
	m_coordinateSystem = d->cSystem;

	d->autoScaleX = true;
	d->autoScaleY = true;
	d->xScale = ScaleLinear;
	d->yScale = ScaleLinear;
	d->xRangeBreakingEnabled = false;
	d->yRangeBreakingEnabled = false;

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
	connect(themeMenu, SIGNAL(triggered(QAction* action)), this, SLOT(loadTheme(QAction* action)));

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
void CartesianPlot::initDefault(Type type) {
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
		axis->setMajorTicksNumber(6);
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
		axis->setMajorTicksNumber(6);
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

void CartesianPlot::initActions() {
	//"add new" actions
	addCurveAction = new KAction(KIcon("labplot-xy-curve"), i18n("xy-curve"), this);
	addEquationCurveAction = new KAction(KIcon("labplot-xy-equation-curve"), i18n("xy-curve from a mathematical equation"), this);
	addFitCurveAction = new KAction(KIcon("labplot-xy-fit-curve"), i18n("xy-curve from a fit to data"), this);
	addFourierFilterCurveAction = new KAction(KIcon("labplot-xy-fourier_filter-curve"), i18n("xy-curve from a Fourier filter"), this);
	addFourierTransformCurveAction = new KAction(KIcon("labplot-xy-fourier_transform-curve"), i18n("xy-curve from a Fourier transform"), this);
	addInterpolationCurveAction = new KAction(KIcon("labplot-xy-interpolation-curve"), i18n("xy-curve from an interpolation"), this);
	addSmoothCurveAction = new KAction(KIcon("labplot-xy-smooth-curve"), i18n("xy-curve from a smooth"), this);
	addLegendAction = new KAction(KIcon("text-field"), i18n("legend"), this);
	addHorizontalAxisAction = new KAction(KIcon("labplot-axis-horizontal"), i18n("horizontal axis"), this);
	addVerticalAxisAction = new KAction(KIcon("labplot-axis-vertical"), i18n("vertical axis"), this);
	addCustomPointAction = new KAction(KIcon("draw-cross"), i18n("custom point"), this);

	connect(addCurveAction, SIGNAL(triggered()), SLOT(addCurve()));
	connect(addEquationCurveAction, SIGNAL(triggered()), SLOT(addEquationCurve()));
	connect(addFitCurveAction, SIGNAL(triggered()), SLOT(addFitCurve()));
	connect(addFourierFilterCurveAction, SIGNAL(triggered()), SLOT(addFourierFilterCurve()));
	connect(addFourierTransformCurveAction, SIGNAL(triggered()), SLOT(addFourierTransformCurve()));
	connect(addLegendAction, SIGNAL(triggered()), SLOT(addLegend()));
	connect(addHorizontalAxisAction, SIGNAL(triggered()), SLOT(addHorizontalAxis()));
	connect(addVerticalAxisAction, SIGNAL(triggered()), SLOT(addVerticalAxis()));
	connect(addCustomPointAction, SIGNAL(triggered()), SLOT(addCustomPoint()));

	//zoom/navigate actions
	scaleAutoAction = new KAction(KIcon("labplot-auto-scale-all"), i18n("auto scale"), this);
	scaleAutoXAction = new KAction(KIcon("labplot-auto-scale-x"), i18n("auto scale X"), this);
	scaleAutoYAction = new KAction(KIcon("labplot-auto-scale-y"), i18n("auto scale Y"), this);
	zoomInAction = new KAction(KIcon("zoom-in"), i18n("zoom in"), this);
	zoomOutAction = new KAction(KIcon("zoom-out"), i18n("zoom out"), this);
	zoomInXAction = new KAction(KIcon("labplot-zoom-in-x"), i18n("zoom in X"), this);
	zoomOutXAction = new KAction(KIcon("labplot-zoom-out-x"), i18n("zoom out X"), this);
	zoomInYAction = new KAction(KIcon("labplot-zoom-in-y"), i18n("zoom in Y"), this);
	zoomOutYAction = new KAction(KIcon("labplot-zoom-out-y"), i18n("zoom out Y"), this);
	shiftLeftXAction = new KAction(KIcon("labplot-shift-left-x"), i18n("shift left X"), this);
	shiftRightXAction = new KAction(KIcon("labplot-shift-right-x"), i18n("shift right X"), this);
	shiftUpYAction = new KAction(KIcon("labplot-shift-up-y"), i18n("shift up Y"), this);
	shiftDownYAction = new KAction(KIcon("labplot-shift-down-y"), i18n("shift down Y"), this);

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

	//visibility action
	visibilityAction = new QAction(i18n("visible"), this);
	visibilityAction->setCheckable(true);
	connect(visibilityAction, SIGNAL(triggered()), this, SLOT(visibilityChanged()));
}

void CartesianPlot::initMenus() {
	addNewMenu = new QMenu(i18n("Add new"));
	addNewMenu->addAction(addCurveAction);
	addNewMenu->addAction(addEquationCurveAction);
	addNewMenu->addAction(addFitCurveAction);
	addNewMenu->addAction(addFourierFilterCurveAction);
	addNewMenu->addAction(addFourierTransformCurveAction);
	addNewMenu->addAction(addLegendAction);
	addNewMenu->addSeparator();
	addNewMenu->addAction(addHorizontalAxisAction);
	addNewMenu->addAction(addVerticalAxisAction);
	addNewMenu->addSeparator();
	addNewMenu->addAction(addCustomPointAction);

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

	themeMenu = new QMenu(i18n("Apply Theme"));
	ThemesWidget* themeWidget = new ThemesWidget(0);
	connect(themeWidget, SIGNAL(themeSelected(QString)), this, SLOT(loadTheme(QString)));
	connect(themeWidget, SIGNAL(themeSelected(QString)), themeMenu, SLOT(close()));

	QWidgetAction* widgetAction = new QWidgetAction(this);
	widgetAction->setDefaultWidget(themeWidget);
	themeMenu->addAction(widgetAction);
}

QMenu* CartesianPlot::createContextMenu() {
	QMenu* menu = WorksheetElement::createContextMenu();
	QAction* firstAction = menu->actions().at(1);

	visibilityAction->setChecked(isVisible());
	menu->insertAction(firstAction, visibilityAction);

	menu->insertMenu(firstAction, addNewMenu);
	menu->insertMenu(firstAction, zoomMenu);
	menu->insertSeparator(firstAction);
	menu->insertMenu(firstAction, themeMenu);
	menu->insertSeparator(firstAction);

	return menu;
}

/*!
	Returns an icon to be used in the project explorer.
*/
QIcon CartesianPlot::icon() const {
	return KIcon("office-chart-line");
}

void CartesianPlot::navigate(CartesianPlot::NavigationOperation op) {
	if (op==ScaleAuto) scaleAuto();
	else if (op==ScaleAutoX) scaleAutoX();
	else if (op==ScaleAutoY) scaleAutoY();
	else if (op==ZoomIn) zoomIn();
	else if (op==ZoomOut) zoomOut();
	else if (op==ZoomInX) zoomInX();
	else if (op==ZoomOutX) zoomOutX();
	else if (op==ZoomInY) zoomInY();
	else if (op==ZoomOutY) zoomOutY();
	else if (op==ShiftLeftX) shiftLeftX();
	else if (op==ShiftRightX) shiftRightX();
	else if (op==ShiftUpY) shiftUpY();
	else if (op==ShiftDownY) shiftDownY();
}

//##############################################################################
//################################  getter methods  ############################
//##############################################################################
BASIC_SHARED_D_READER_IMPL(CartesianPlot, bool, autoScaleX, autoScaleX)
BASIC_SHARED_D_READER_IMPL(CartesianPlot, float, xMin, xMin)
BASIC_SHARED_D_READER_IMPL(CartesianPlot, float, xMax, xMax)
BASIC_SHARED_D_READER_IMPL(CartesianPlot, CartesianPlot::Scale, xScale, xScale)
BASIC_SHARED_D_READER_IMPL(CartesianPlot, bool, xRangeBreakingEnabled, xRangeBreakingEnabled)
CLASS_SHARED_D_READER_IMPL(CartesianPlot, CartesianPlot::RangeBreaks, xRangeBreaks, xRangeBreaks)

BASIC_SHARED_D_READER_IMPL(CartesianPlot, bool, autoScaleY, autoScaleY)
BASIC_SHARED_D_READER_IMPL(CartesianPlot, float, yMin, yMin)
BASIC_SHARED_D_READER_IMPL(CartesianPlot, float, yMax, yMax)
BASIC_SHARED_D_READER_IMPL(CartesianPlot, CartesianPlot::Scale, yScale, yScale)
BASIC_SHARED_D_READER_IMPL(CartesianPlot, bool, yRangeBreakingEnabled, yRangeBreakingEnabled)
CLASS_SHARED_D_READER_IMPL(CartesianPlot, CartesianPlot::RangeBreaks, yRangeBreaks, yRangeBreaks)

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
void CartesianPlot::setRect(const QRectF& rect) {
	Q_D(CartesianPlot);
	if (rect != d->rect)
		exec(new CartesianPlotSetRectCmd(d, rect, i18n("%1: set geometry rect")));
}

class CartesianPlotSetAutoScaleXCmd : public QUndoCommand {
public:
	CartesianPlotSetAutoScaleXCmd(CartesianPlotPrivate* private_obj, bool autoScale) {
		m_private = private_obj;
		m_autoScale = autoScale;
		setText(i18n("%1: change x-range auto scaling", m_private->name()));
	};

	virtual void redo() {
		m_autoScaleOld = m_private->autoScaleX;
		if (m_autoScale) {
			m_minOld = m_private->xMin;
			m_maxOld = m_private->xMax;
			m_private->q->scaleAutoX();
		}
		m_private->autoScaleX = m_autoScale;
		emit m_private->q->xAutoScaleChanged(m_autoScale);
	};

	virtual void undo() {
		if (!m_autoScaleOld) {
			m_private->xMin = m_minOld;
			m_private->xMax = m_maxOld;
			m_private->retransformScales();
		}
		m_private->autoScaleX = m_autoScaleOld;
		emit m_private->q->xAutoScaleChanged(m_autoScaleOld);
	}

private:
	CartesianPlotPrivate* m_private;
	bool m_autoScale;
	bool m_autoScaleOld;
	float m_minOld;
	float m_maxOld;
};

void CartesianPlot::setAutoScaleX(bool autoScaleX) {
	Q_D(CartesianPlot);
	if (autoScaleX != d->autoScaleX)
		exec(new CartesianPlotSetAutoScaleXCmd(d, autoScaleX));
}

STD_SETTER_CMD_IMPL_F_S(CartesianPlot, SetXMin, float, xMin, retransformScales)
void CartesianPlot::setXMin(float xMin) {
	Q_D(CartesianPlot);
	if (xMin != d->xMin)
		exec(new CartesianPlotSetXMinCmd(d, xMin, i18n("%1: set min x")));
}

STD_SETTER_CMD_IMPL_F_S(CartesianPlot, SetXMax, float, xMax, retransformScales);
void CartesianPlot::setXMax(float xMax) {
	Q_D(CartesianPlot);
	if (xMax != d->xMax)
		exec(new CartesianPlotSetXMaxCmd(d, xMax, i18n("%1: set max x")));
}

STD_SETTER_CMD_IMPL_F_S(CartesianPlot, SetXScale, CartesianPlot::Scale, xScale, retransformScales);
void CartesianPlot::setXScale(Scale scale) {
	Q_D(CartesianPlot);
	if (scale != d->xScale)
		exec(new CartesianPlotSetXScaleCmd(d, scale, i18n("%1: set x scale")));
}

STD_SETTER_CMD_IMPL_F_S(CartesianPlot, SetXRangeBreakingEnabled, bool, xRangeBreakingEnabled, retransformScales)
void CartesianPlot::setXRangeBreakingEnabled(bool enabled) {
	Q_D(CartesianPlot);
	if (enabled != d->xRangeBreakingEnabled)
		exec(new CartesianPlotSetXRangeBreakingEnabledCmd(d, enabled, i18n("%1: x-range breaking enabled")));
}

STD_SETTER_CMD_IMPL_F_S(CartesianPlot, SetXRangeBreaks, CartesianPlot::RangeBreaks, xRangeBreaks, retransformScales);
void CartesianPlot::setXRangeBreaks(const RangeBreaks& breakings) {
	Q_D(CartesianPlot);
	exec(new CartesianPlotSetXRangeBreaksCmd(d, breakings, i18n("%1: x-range breaks changed")));
}

class CartesianPlotSetAutoScaleYCmd : public QUndoCommand {
public:
	CartesianPlotSetAutoScaleYCmd(CartesianPlotPrivate* private_obj, bool autoScale) {
		m_private = private_obj;
		m_autoScale = autoScale;
		setText(i18n("%1: change y-range auto scaling", m_private->name()));
	};

	virtual void redo() {
		m_autoScaleOld = m_private->autoScaleY;
		if (m_autoScale) {
			m_minOld = m_private->yMin;
			m_maxOld = m_private->yMax;
			m_private->q->scaleAutoY();
		}
		m_private->autoScaleY = m_autoScale;
		emit m_private->q->yAutoScaleChanged(m_autoScale);
	};

	virtual void undo() {
		if (!m_autoScaleOld) {
			m_private->yMin = m_minOld;
			m_private->yMax = m_maxOld;
			m_private->retransformScales();
		}
		m_private->autoScaleY = m_autoScaleOld;
		emit m_private->q->yAutoScaleChanged(m_autoScaleOld);
	}

private:
	CartesianPlotPrivate* m_private;
	bool m_autoScale;
	bool m_autoScaleOld;
	float m_minOld;
	float m_maxOld;
};

void CartesianPlot::setAutoScaleY(bool autoScaleY) {
	Q_D(CartesianPlot);
	if (autoScaleY != d->autoScaleY)
		exec(new CartesianPlotSetAutoScaleYCmd(d, autoScaleY));
}

STD_SETTER_CMD_IMPL_F_S(CartesianPlot, SetYMin, float, yMin, retransformScales);
void CartesianPlot::setYMin(float yMin) {
	Q_D(CartesianPlot);
	if (yMin != d->yMin)
		exec(new CartesianPlotSetYMinCmd(d, yMin, i18n("%1: set min y")));
}

STD_SETTER_CMD_IMPL_F_S(CartesianPlot, SetYMax, float, yMax, retransformScales);
void CartesianPlot::setYMax(float yMax) {
	Q_D(CartesianPlot);
	if (yMax != d->yMax)
		exec(new CartesianPlotSetYMaxCmd(d, yMax, i18n("%1: set max y")));
}

STD_SETTER_CMD_IMPL_F_S(CartesianPlot, SetYScale, CartesianPlot::Scale, yScale, retransformScales);
void CartesianPlot::setYScale(Scale scale) {
	Q_D(CartesianPlot);
	if (scale != d->yScale)
		exec(new CartesianPlotSetYScaleCmd(d, scale, i18n("%1: set y scale")));
}

STD_SETTER_CMD_IMPL_F_S(CartesianPlot, SetYRangeBreakingEnabled, bool, yRangeBreakingEnabled, retransformScales)
void CartesianPlot::setYRangeBreakingEnabled(bool enabled) {
	Q_D(CartesianPlot);
	if (enabled != d->yRangeBreakingEnabled)
		exec(new CartesianPlotSetYRangeBreakingEnabledCmd(d, enabled, i18n("%1: y-range breaking enabled")));
}

STD_SETTER_CMD_IMPL_F_S(CartesianPlot, SetYRangeBreaks, CartesianPlot::RangeBreaks, yRangeBreaks, retransformScales);
void CartesianPlot::setYRangeBreaks(const RangeBreaks& breaks) {
	Q_D(CartesianPlot);
	exec(new CartesianPlotSetYRangeBreaksCmd(d, breaks, i18n("%1: y-range breaks changed")));
}

//################################################################
//########################## Slots ###############################
//################################################################
void CartesianPlot::addHorizontalAxis() {
	Axis* axis = new Axis("x-axis", Axis::AxisHorizontal);
	if (axis->autoScale()) {
		axis->setUndoAware(false);
		axis->setStart(xMin());
		axis->setEnd(xMax());
		axis->setUndoAware(true);
	}
	addChild(axis);
}

void CartesianPlot::addVerticalAxis() {
	Axis* axis = new Axis("y-axis", Axis::AxisVertical);
	if (axis->autoScale()) {
		axis->setUndoAware(false);
		axis->setStart(yMin());
		axis->setEnd(yMax());
		axis->setUndoAware(true);
	}
	addChild(axis);
}

XYCurve* CartesianPlot::addCurve() {
	XYCurve* curve = new XYCurve("xy-curve");
	this->addChild(curve);
	this->applyThemeOnNewCurve(curve);
	return curve;
}

XYEquationCurve* CartesianPlot::addEquationCurve() {
	XYEquationCurve* curve = new XYEquationCurve("f(x)");
	this->addChild(curve);
	this->applyThemeOnNewCurve(curve);
	return curve;
}

XYInterpolationCurve* CartesianPlot::addInterpolationCurve() {
	XYInterpolationCurve* curve = new XYInterpolationCurve("Interpolation");
	this->addChild(curve);
	this->applyThemeOnNewCurve(curve);
	return curve;
}

XYSmoothCurve* CartesianPlot::addSmoothCurve() {
	XYSmoothCurve* curve = new XYSmoothCurve("Smooth");
	this->addChild(curve);
	this->applyThemeOnNewCurve(curve);
	return curve;
}
XYFitCurve* CartesianPlot::addFitCurve() {
	XYFitCurve* curve = new XYFitCurve("fit");
	this->addChild(curve);
	this->applyThemeOnNewCurve(curve);
	return curve;
}

XYFourierFilterCurve* CartesianPlot::addFourierFilterCurve() {
	XYFourierFilterCurve* curve = new XYFourierFilterCurve("Fourier filter");
	this->addChild(curve);
	this->applyThemeOnNewCurve(curve);
	return curve;
}

XYFourierTransformCurve* CartesianPlot::addFourierTransformCurve() {
	XYFourierTransformCurve* curve = new XYFourierTransformCurve("Fourier transform");
	this->addChild(curve);
	this->applyThemeOnNewCurve(curve);
	return curve;
}

void CartesianPlot::addLegend() {
	//don't do anything if there's already a legend
	if (m_legend)
		return;

	m_legend = new CartesianPlotLegend(this, "legend");
	this->addChild(m_legend);
	m_legend->retransform();

	//only one legend is allowed -> disable the action
	addLegendAction->setEnabled(false);
}

void CartesianPlot::addCustomPoint() {
	CustomPoint* point= new CustomPoint(this, "custom point");
	this->addChild(point);
}

void CartesianPlot::childAdded(const AbstractAspect* child) {
	Q_D(CartesianPlot);
	const XYCurve* curve = qobject_cast<const XYCurve*>(child);
	if (curve) {
		connect(curve, SIGNAL(dataChanged()), this, SLOT(dataChanged()));
		connect(curve, SIGNAL(xDataChanged()), this, SLOT(xDataChanged()));
		connect(curve, SIGNAL(yDataChanged()), this, SLOT(yDataChanged()));
		connect(curve, SIGNAL(visibilityChanged(bool)), this, SLOT(curveVisibilityChanged()));

		//update the legend on changes of the name, line and symbol styles
		connect(curve, SIGNAL(aspectDescriptionChanged(const AbstractAspect*)), this, SLOT(updateLegend()));
		connect(curve, SIGNAL(lineTypeChanged(XYCurve::LineType)), this, SLOT(updateLegend()));
		connect(curve, SIGNAL(linePenChanged(QPen)), this, SLOT(updateLegend()));
		connect(curve, SIGNAL(lineOpacityChanged(qreal)), this, SLOT(updateLegend()));
		connect(curve, SIGNAL(symbolsStyleChanged(Symbol::Style)), this, SLOT(updateLegend()));
		connect(curve, SIGNAL(symbolsSizeChanged(qreal)), this, SLOT(updateLegend()));
		connect(curve, SIGNAL(symbolsRotationAngleChanged(qreal)), this, SLOT(updateLegend()));
		connect(curve, SIGNAL(symbolsOpacityChanged(qreal)), this, SLOT(updateLegend()));
		connect(curve, SIGNAL(symbolsBrushChanged(QBrush)), this, SLOT(updateLegend()));
		connect(curve, SIGNAL(symbolsPenChanged(QPen)), this, SLOT(updateLegend()));

		updateLegend();
		d->curvesXMinMaxIsDirty = true;
		d->curvesYMinMaxIsDirty = true;
	}
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
void CartesianPlot::dataChanged() {
	Q_D(CartesianPlot);
	XYCurve* curve = dynamic_cast<XYCurve*>(QObject::sender());
	Q_ASSERT(curve);
	d->curvesXMinMaxIsDirty = true;
	d->curvesYMinMaxIsDirty = true;
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
void CartesianPlot::xDataChanged() {
	if (project()->isLoading())
		return;

	Q_D(CartesianPlot);
	XYCurve* curve = dynamic_cast<XYCurve*>(QObject::sender());
	Q_ASSERT(curve);
	d->curvesXMinMaxIsDirty = true;
	if (d->autoScaleX)
		this->scaleAutoX();
	else
		curve->retransform();
}

/*!
	called when in one of the curves the x-data was changed.
	Autoscales the coordinate system and the x-axes, when "auto-scale" is active.
*/
void CartesianPlot::yDataChanged() {
	if (project()->isLoading())
		return;

	Q_D(CartesianPlot);
	XYCurve* curve = dynamic_cast<XYCurve*>(QObject::sender());
	Q_ASSERT(curve);
	d->curvesYMinMaxIsDirty = true;
	if (d->autoScaleY)
		this->scaleAutoY();
	else
		curve->retransform();
}

void CartesianPlot::curveVisibilityChanged() {
	Q_D(CartesianPlot);
	d->curvesXMinMaxIsDirty = true;
	d->curvesYMinMaxIsDirty = true;
	updateLegend();
	if (d->autoScaleX && d->autoScaleY)
		this->scaleAuto();
	else if (d->autoScaleX)
		this->scaleAutoX();
	else if (d->autoScaleY)
		this->scaleAutoY();
}

void CartesianPlot::setMouseMode(const MouseMode mouseMode) {
	Q_D(CartesianPlot);

	d->mouseMode = mouseMode;
	d->setHandlesChildEvents(mouseMode != CartesianPlot::SelectionMode);

	QList<QGraphicsItem*> items = d->childItems();
	if (d->mouseMode == CartesianPlot::SelectionMode) {
		foreach(QGraphicsItem* item, items)
			item->setFlag(QGraphicsItem::ItemStacksBehindParent, false);
	} else {
		foreach(QGraphicsItem* item, items)
			item->setFlag(QGraphicsItem::ItemStacksBehindParent, true);
	}

	//when doing zoom selection, prevent the graphics item from being movable
	//if it's currently movable (no worksheet layout available)
	const Worksheet* worksheet = dynamic_cast<const Worksheet*>(parentAspect());
	if (worksheet) {
		if (mouseMode == CartesianPlot::SelectionMode) {
			if (worksheet->layout() != Worksheet::NoLayout)
				graphicsItem()->setFlag(QGraphicsItem::ItemIsMovable, false);
			else
				graphicsItem()->setFlag(QGraphicsItem::ItemIsMovable, true);
		} else { //zoom m_selection
			graphicsItem()->setFlag(QGraphicsItem::ItemIsMovable, false);
		}
	}
}

void CartesianPlot::scaleAutoX() {
	Q_D(CartesianPlot);

	//loop over all xy-curves and determine the maximum x-value
	if (d->curvesXMinMaxIsDirty) {
		d->curvesXMin = INFINITY;
		d->curvesXMax = -INFINITY;
		QList<const XYCurve*> children = this->children<const XYCurve>();
		foreach(const XYCurve* curve, children) {
			if (!curve->isVisible())
				continue;
			if (!curve->xColumn())
				continue;

			if (curve->xColumn()->minimum() != INFINITY) {
				if (curve->xColumn()->minimum() < d->curvesXMin)
					d->curvesXMin = curve->xColumn()->minimum();
			}

			if (curve->xColumn()->maximum() != -INFINITY) {
				if (curve->xColumn()->maximum() > d->curvesXMax)
					d->curvesXMax = curve->xColumn()->maximum();
			}
		}

		d->curvesXMinMaxIsDirty = false;
	}

	bool update = false;
	if (d->curvesXMin != d->xMin && d->curvesXMin != INFINITY) {
		d->xMin = d->curvesXMin;
		update = true;
	}

	if (d->curvesXMax != d->xMax && d->curvesXMax != -INFINITY) {
		d->xMax = d->curvesXMax;
		update = true;
	}

	if (update) {
		if (d->xMax == d->xMin) {
			//in case min and max are equal (e.g. if we plot a single point), subtract/add 10% of the value
			if (d->xMax!=0) {
				d->xMax = d->xMax*1.1;
				d->xMin = d->xMin*0.9;
			} else {
				d->xMax = 0.1;
				d->xMin = -0.1;
			}
		} else {
			float offset = (d->xMax - d->xMin)*d->autoScaleOffsetFactor;
			d->xMin -= offset;
			d->xMax += offset;
		}
		d->retransformScales();
	}
}

void CartesianPlot::scaleAutoY() {
	Q_D(CartesianPlot);

	//loop over all xy-curves and determine the maximum y-value
	if (d->curvesYMinMaxIsDirty) {
		d->curvesYMin = INFINITY;
		d->curvesYMax = -INFINITY;
		QList<const XYCurve*> children = this->children<const XYCurve>();
		foreach(const XYCurve* curve, children) {
			if (!curve->isVisible())
				continue;
			if (!curve->yColumn())
				continue;

			if (curve->yColumn()->minimum() != INFINITY) {
				if (curve->yColumn()->minimum() < d->curvesYMin)
					d->curvesYMin = curve->yColumn()->minimum();
			}

			if (curve->yColumn()->maximum() != -INFINITY) {
				if (curve->yColumn()->maximum() > d->curvesYMax)
					d->curvesYMax = curve->yColumn()->maximum();
			}
		}

		d->curvesYMinMaxIsDirty = false;
	}

	bool update = false;
	if (d->curvesYMin != d->yMin && d->curvesYMin != INFINITY) {
		d->yMin = d->curvesYMin;
		update = true;
	}

	if (d->curvesYMax != d->yMax && d->curvesYMax != -INFINITY) {
		d->yMax = d->curvesYMax;
		update = true;
	}

	if (update) {
		if (d->yMax == d->yMin) {
			//in case min and max are equal (e.g. if we plot a single point), subtract/add 10% of the value
			if (d->yMax!=0) {
				d->yMax = d->yMax*1.1;
				d->yMin = d->yMin*0.9;
			} else {
				d->yMax = 0.1;
				d->yMin = -0.1;
			}
		} else {
			float offset = (d->yMax - d->yMin)*d->autoScaleOffsetFactor;
			d->yMin -= offset;
			d->yMax += offset;
		}
		d->retransformScales();
	}
}

void CartesianPlot::scaleAuto() {
	Q_D(CartesianPlot);

	//loop over all xy-curves and determine the maximum x-value
	QList<const XYCurve*> children = this->children<const XYCurve>();
	if (d->curvesXMinMaxIsDirty) {
		d->curvesXMin = INFINITY;
		d->curvesXMax = -INFINITY;
		foreach(const XYCurve* curve, children) {
			if (!curve->isVisible())
				continue;
			if (!curve->xColumn())
				continue;

			if (curve->xColumn()->minimum() != INFINITY) {
				if (curve->xColumn()->minimum() < d->curvesXMin)
					d->curvesXMin = curve->xColumn()->minimum();
			}

			if (curve->xColumn()->maximum() != -INFINITY) {
				if (curve->xColumn()->maximum() > d->curvesXMax)
					d->curvesXMax = curve->xColumn()->maximum();
			}

			d->curvesXMinMaxIsDirty = false;
		}
	}

	if (d->curvesYMinMaxIsDirty) {
		d->curvesYMin = INFINITY;
		d->curvesYMax = -INFINITY;
		foreach(const XYCurve* curve, children) {
			if (!curve->isVisible())
				continue;
			if (!curve->xColumn())
				continue;

			if (curve->yColumn()->minimum() != INFINITY) {
				if (curve->yColumn()->minimum() < d->curvesYMin)
					d->curvesYMin = curve->yColumn()->minimum();
			}

			if (curve->yColumn()->maximum() != -INFINITY) {
				if (curve->yColumn()->maximum() > d->curvesYMax)
					d->curvesYMax = curve->yColumn()->maximum();
			}
		}
	}

	bool updateX = false;
	bool updateY = false;
	if (d->curvesXMin != d->xMin && d->curvesXMin != INFINITY) {
		d->xMin = d->curvesXMin;
		updateX = true;
	}

	if (d->curvesXMax != d->xMax && d->curvesXMax != -INFINITY) {
		d->xMax = d->curvesXMax;
		updateX = true;
	}

	if (d->curvesYMin != d->yMin && d->curvesYMin != INFINITY) {
		d->yMin = d->curvesYMin;
		updateY = true;
	}

	if (d->curvesYMax != d->yMax && d->curvesYMax != -INFINITY) {
		d->yMax = d->curvesYMax;
		updateY = true;
	}

	if (updateX || updateY) {
		if (updateX) {
			if (d->xMax == d->xMin) {
				//in case min and max are equal (e.g. if we plot a single point), subtract/add 10% of the value
				if (d->xMax!=0) {
					d->xMax = d->xMax*1.1;
					d->xMin = d->xMin*0.9;
				} else {
					d->xMax = 0.1;
					d->xMin = -0.1;
				}
			} else {
				float offset = (d->xMax - d->xMin)*d->autoScaleOffsetFactor;
				d->xMin -= offset;
				d->xMax += offset;
			}
		}
		if (updateY) {
			if (d->yMax == d->yMin) {
				//in case min and max are equal (e.g. if we plot a single point), subtract/add 10% of the value
				if (d->yMax!=0) {
					d->yMax = d->yMax*1.1;
					d->yMin = d->yMin*0.9;
				} else {
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

void CartesianPlot::zoomIn() {
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

void CartesianPlot::zoomOut() {
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

void CartesianPlot::zoomInX() {
	Q_D(CartesianPlot);
	float oldRange = (d->xMax-d->xMin);
	float newRange = (d->xMax-d->xMin)/m_zoomFactor;
	d->xMax = d->xMax + (newRange-oldRange)/2;
	d->xMin = d->xMin - (newRange-oldRange)/2;
	d->retransformScales();
}

void CartesianPlot::zoomOutX() {
	Q_D(CartesianPlot);
	float oldRange = (d->xMax-d->xMin);
	float newRange = (d->xMax-d->xMin)*m_zoomFactor;
	d->xMax = d->xMax + (newRange-oldRange)/2;
	d->xMin = d->xMin - (newRange-oldRange)/2;
	d->retransformScales();
}

void CartesianPlot::zoomInY() {
	Q_D(CartesianPlot);
	float oldRange = (d->yMax-d->yMin);
	float newRange = (d->yMax-d->yMin)/m_zoomFactor;
	d->yMax = d->yMax + (newRange-oldRange)/2;
	d->yMin = d->yMin - (newRange-oldRange)/2;
	d->retransformScales();
}

void CartesianPlot::zoomOutY() {
	Q_D(CartesianPlot);
	float oldRange = (d->yMax-d->yMin);
	float newRange = (d->yMax-d->yMin)*m_zoomFactor;
	d->yMax = d->yMax + (newRange-oldRange)/2;
	d->yMin = d->yMin - (newRange-oldRange)/2;
	d->retransformScales();
}

void CartesianPlot::shiftLeftX() {
	Q_D(CartesianPlot);
	float offsetX = (d->xMax-d->xMin)*0.1;
	d->xMax -= offsetX;
	d->xMin -= offsetX;
	d->retransformScales();
}

void CartesianPlot::shiftRightX() {
	Q_D(CartesianPlot);
	float offsetX = (d->xMax-d->xMin)*0.1;
	d->xMax += offsetX;
	d->xMin += offsetX;
	d->retransformScales();
}

void CartesianPlot::shiftUpY() {
	Q_D(CartesianPlot);
	float offsetY = (d->yMax-d->yMin)*0.1;
	d->yMax += offsetY;
	d->yMin += offsetY;
	d->retransformScales();
}

void CartesianPlot::shiftDownY() {
	Q_D(CartesianPlot);
	float offsetY = (d->yMax-d->yMin)*0.1;
	d->yMax -= offsetY;
	d->yMin -= offsetY;
	d->retransformScales();
}

//##############################################################################
//######  SLOTs for changes triggered via QActions in the context menu  ########
//##############################################################################
void CartesianPlot::visibilityChanged() {
	Q_D(CartesianPlot);
	this->setVisible(!d->isVisible());
}

//#####################################################################
//################### Private implementation ##########################
//#####################################################################
CartesianPlotPrivate::CartesianPlotPrivate(CartesianPlot *owner)
	: AbstractPlotPrivate(owner), q(owner), curvesXMinMaxIsDirty(false), curvesYMinMaxIsDirty(false),
	  curvesXMin(INFINITY), curvesXMax(-INFINITY), curvesYMin(INFINITY), curvesYMax(-INFINITY),
	  suppressRetransform(false), m_printing(false), m_selectionBandIsShown(false), cSystem(0),
	  mouseMode(CartesianPlot::SelectionMode) {
	setData(0, WorksheetElement::NameCartesianPlot);
}

/*!
	updates the position of plot rectangular in scene coordinates to \c r and recalculates the scales.
	The size of the plot corresponds to the size of the plot area, the area which is filled with the background color etc.
	and which can pose the parent item for several sub-items (like TextLabel).
	Note, the size of the area used to define the coordinate system doesn't need to be equal to this plot area.
	Also, the size (=bounding box) of CartesianPlot can be greater than the size of the plot area.
 */
void CartesianPlotPrivate::retransform() {
	if (suppressRetransform)
		return;

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

	WorksheetElementContainerPrivate::recalcShapeAndBoundingRect();
}

void CartesianPlotPrivate::retransformScales() {
	CartesianPlot* plot = dynamic_cast<CartesianPlot*>(q);
	QList<CartesianCoordinateSystem::Scale*> scales;
	double sceneStart, sceneEnd, logicalStart, logicalEnd;

	//perform the mapping from the scene coordinates to the plot's coordinates here.
	QRectF itemRect = mapRectFromScene(rect);

	//check ranges for log-scales
	if (xScale != CartesianPlot::ScaleLinear)
		checkXRange();

	//check where we have x-range breaks
	bool hasValidBreak = false;
	if (xRangeBreakingEnabled && !xRangeBreaks.list.isEmpty()) {
		foreach(const CartesianPlot::RangeBreak& b, xRangeBreaks.list)
			hasValidBreak = (!std::isnan(b.start) && !std::isnan(b.end));
	}

	//create x-scales
	if (!hasValidBreak) {
		sceneStart = itemRect.x()+horizontalPadding;
		sceneEnd = itemRect.x()+itemRect.width()-horizontalPadding;
		logicalStart = xMin;
		logicalEnd = xMax;

		//TODO: how should we handle the case sceneStart=sceneEnd
		//(to reproduce, create plots and adjust the spacing/pading to get zero size for the plots)
		if (sceneStart!=sceneEnd) {
			Interval<double> interval(SCALE_MIN, SCALE_MAX);
			scales << this->createScale(xScale, interval, sceneStart, sceneEnd, logicalStart, logicalEnd);
		}
	} else {
		foreach(const CartesianPlot::RangeBreak& b, xRangeBreaks.list) {
			sceneStart = itemRect.x()+horizontalPadding;
			sceneEnd = itemRect.x()+itemRect.width()-horizontalPadding;
			sceneEnd = sceneStart+(sceneEnd-sceneStart)*b.position;
			logicalStart = xMin;
			logicalEnd = b.start;

			Interval<double> interval(SCALE_MIN, SCALE_MAX);
			scales << this->createScale(xScale, interval, sceneStart, sceneEnd, logicalStart, logicalEnd);

			sceneStart = sceneEnd+50;
			sceneEnd = itemRect.x()+itemRect.width()-horizontalPadding;
			logicalStart = b.end;
			logicalEnd = xMax;
			Interval<double> interval2(SCALE_MIN, SCALE_MAX);
			scales << this->createScale(xScale, interval2, sceneStart, sceneEnd, logicalStart, logicalEnd);
		}
	}

	cSystem ->setXScales(scales);

	//check ranges for log-scales
	if (yScale != CartesianPlot::ScaleLinear)
		checkYRange();

	//check where we have y-range breaks
	hasValidBreak = false;
	if (yRangeBreakingEnabled && !yRangeBreaks.list.isEmpty()) {
		foreach(const CartesianPlot::RangeBreak& b, yRangeBreaks.list)
			hasValidBreak = (!std::isnan(b.start) && !std::isnan(b.end));
	}

	//create y-scales
	scales.clear();
	if (!hasValidBreak) {
		sceneStart = itemRect.y()+itemRect.height()-verticalPadding;
		sceneEnd = itemRect.y()+verticalPadding;
		logicalStart = yMin;
		logicalEnd = yMax;
		Interval<double> interval (SCALE_MIN, SCALE_MAX);
		scales << this->createScale(yScale, interval, sceneStart, sceneEnd, logicalStart, logicalEnd);
	} else {
		foreach(const CartesianPlot::RangeBreak& b, yRangeBreaks.list) {
			sceneStart = itemRect.y()+itemRect.height()-verticalPadding;
			sceneEnd = itemRect.y()+verticalPadding;
			sceneEnd = sceneStart+(sceneEnd-sceneStart)*b.position;
			logicalStart = yMin;
			logicalEnd = b.start;

			Interval<double> interval(SCALE_MIN, SCALE_MAX);
			scales << this->createScale(yScale, interval, sceneStart, sceneEnd, logicalStart, logicalEnd);

			sceneStart = sceneEnd+50;
			sceneEnd = itemRect.y()+verticalPadding;
			logicalStart = b.end;
			logicalEnd = yMax;
			Interval<double> interval2(SCALE_MIN, SCALE_MAX);
			scales << this->createScale(yScale, interval2, sceneStart, sceneEnd, logicalStart, logicalEnd);
		}
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
	foreach(Axis* axis, childElements) {
		if (!axis->autoScale())
			continue;

		if (axis->orientation() == Axis::AxisHorizontal) {
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
			//TODO;
// 			if (axis->position() == Axis::AxisCustom && deltaYMin != 0) {
// 				axis->setOffset(axis->offset() + deltaYMin, false);
// 			}
		} else {
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

			//TODO;
// 			if (axis->position() == Axis::AxisCustom && deltaXMin != 0) {
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
	} else if (xMax <= 0.0) {
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
	} else if (yMax <= 0.0) {
		(-min > yMin*min) ? yMax = -min : yMax = yMin*min;
		emit q->yMaxChanged(yMax);
	}
}

CartesianCoordinateSystem::Scale* CartesianPlotPrivate::createScale(CartesianPlot::Scale type, Interval<double>& interval,
	double sceneStart, double sceneEnd,double logicalStart, double logicalEnd) {
	if (type == CartesianPlot::ScaleLinear) {
		return CartesianCoordinateSystem::Scale::createLinearScale(interval, sceneStart, sceneEnd, logicalStart, logicalEnd);
	} else {
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
QVariant CartesianPlotPrivate::itemChange(GraphicsItemChange change, const QVariant &value) {
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
			m_selectionStart.setX(event->pos().x());
			m_selectionStart.setY(q->plotRect().height()/2);
		} else if (mouseMode==CartesianPlot::ZoomYSelectionMode) {
			m_selectionStart.setX(-q->plotRect().width()/2);
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
		if ( !boundingRect().contains(event->pos()) ) {
			q->info("");
			return;
		}

		QString info;
		QPointF logicalStart = cSystem->mapSceneToLogical(m_selectionStart);
		if (mouseMode==CartesianPlot::ZoomSelectionMode) {
			m_selectionEnd = event->pos();
			QPointF logicalEnd = cSystem->mapSceneToLogical(m_selectionEnd);
			info = QString::fromUtf8("Δx=") + QString::number(logicalEnd.x()-logicalStart.x()) + QString::fromUtf8(", Δy=") + QString::number(logicalEnd.y()-logicalStart.y());
		} else if (mouseMode==CartesianPlot::ZoomXSelectionMode) {
			m_selectionEnd.setX(event->pos().x());
			m_selectionEnd.setY(-q->plotRect().height()/2);
			QPointF logicalEnd = cSystem->mapSceneToLogical(m_selectionEnd);
			info = QString::fromUtf8("Δx=") + QString::number(logicalEnd.x()-logicalStart.x());
		} else if (mouseMode==CartesianPlot::ZoomYSelectionMode) {
			m_selectionEnd.setX(q->plotRect().width()/2);
			m_selectionEnd.setY(event->pos().y());
			QPointF logicalEnd = cSystem->mapSceneToLogical(m_selectionEnd);
			info = QString::fromUtf8("Δy=") + QString::number(logicalEnd.y()-logicalStart.y());
		}
		q->info(info);
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
		//don't zoom if very small region was selected, avoid occasional/unwanted zooming
		if ( qAbs(m_selectionEnd.x()-m_selectionStart.x())<20 || qAbs(m_selectionEnd.y()-m_selectionStart.y())<20 ) {
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
	foreach(Axis* axis, axes) {
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

void CartesianPlotPrivate::hoverMoveEvent(QGraphicsSceneHoverEvent* event) {
	QPointF point = event->pos();
	QString info;
	if (q->plotRect().contains(point)) {
		QPointF logicalPoint = cSystem->mapSceneToLogical(point);
		if (mouseMode == CartesianPlot::ZoomSelectionMode && !m_selectionBandIsShown) {
			info = "x=" + QString::number(logicalPoint.x()) + ", y=" + QString::number(logicalPoint.y());
		} else if (mouseMode == CartesianPlot::ZoomXSelectionMode && !m_selectionBandIsShown) {
			QPointF p1(logicalPoint.x(), yMin);
			QPointF p2(logicalPoint.x(), yMax);
			m_selectionStartLine.setP1(cSystem->mapLogicalToScene(p1));
			m_selectionStartLine.setP2(cSystem->mapLogicalToScene(p2));
			info = "x=" + QString::number(logicalPoint.x());
			update();
		} else if (mouseMode == CartesianPlot::ZoomYSelectionMode && !m_selectionBandIsShown) {
			QPointF p1(xMin, logicalPoint.y());
			QPointF p2(xMax, logicalPoint.y());
			m_selectionStartLine.setP1(cSystem->mapLogicalToScene(p1));
			m_selectionStartLine.setP2(cSystem->mapLogicalToScene(p2));
			info = "y=" + QString::number(logicalPoint.y());
			update();
		}
	}
	q->info(info);

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

	if (m_selectionBandIsShown) {
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
void CartesianPlot::save(QXmlStreamWriter* writer) const {
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

	//x-scale breaks
	if (d->xRangeBreakingEnabled || !d->xRangeBreaks.list.isEmpty()) {
		writer->writeStartElement("xRangeBreaks");
		writer->writeAttribute( "enabled", QString::number(d->xRangeBreakingEnabled) );
		foreach(const RangeBreak& b, d->xRangeBreaks.list) {
			writer->writeStartElement("xRangeBreak");
			writer->writeAttribute("start", QString::number(b.start));
			writer->writeAttribute("end", QString::number(b.end));
			writer->writeAttribute("position", QString::number(b.position));
			writer->writeAttribute("style", QString::number(b.style));
			writer->writeEndElement();
		}
		writer->writeEndElement();
	}

	//y-scale breaks
	if (d->yRangeBreakingEnabled || !d->yRangeBreaks.list.isEmpty()) {
		writer->writeStartElement("yRangeBreaks");
		writer->writeAttribute( "enabled", QString::number(d->yRangeBreakingEnabled) );
		foreach(const RangeBreak& b, d->yRangeBreaks.list) {
			writer->writeStartElement("yRangeBreak");
			writer->writeAttribute("start", QString::number(b.start));
			writer->writeAttribute("end", QString::number(b.end));
			writer->writeAttribute("position", QString::number(b.position));
			writer->writeAttribute("style", QString::number(b.style));
			writer->writeEndElement();
		}
		writer->writeEndElement();
	}

	//serialize all children (plot area, title text label, axes and curves)
	QList<WorksheetElement*> childElements = children<WorksheetElement>(IncludeHidden);
	foreach(WorksheetElement *elem, childElements)
		elem->save(writer);

	writer->writeEndElement(); // close "cartesianPlot" section
}


//! Load from XML
bool CartesianPlot::load(XmlStreamReader* reader) {
	Q_D(CartesianPlot);

	if (!reader->isStartElement() || reader->name() != "cartesianPlot") {
		reader->raiseError(i18n("no cartesianPlot element found"));
		return false;
	}

	if (!readBasicAttributes(reader))
		return false;

	QString attributeWarning = i18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == "cartesianPlot")
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == "comment") {
			if (!readCommentElement(reader))
				return false;
		} else if (reader->name() == "geometry") {
			attribs = reader->attributes();

			str = attribs.value("x").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'x'"));
			else
				d->rect.setX( str.toDouble() );

			str = attribs.value("y").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'y'"));
			else
				d->rect.setY( str.toDouble() );

			str = attribs.value("width").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'width'"));
			else
				d->rect.setWidth( str.toDouble() );

			str = attribs.value("height").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'height'"));
			else
				d->rect.setHeight( str.toDouble() );

			str = attribs.value("visible").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'visible'"));
			else
				d->setVisible(str.toInt());
		} else if (reader->name() == "coordinateSystem") {
			attribs = reader->attributes();

			str = attribs.value("autoScaleX").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'autoScaleX'"));
			else
				d->autoScaleX = bool(str.toInt());

			str = attribs.value("autoScaleY").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'autoScaleY'"));
			else
				d->autoScaleY = bool(str.toInt());

			str = attribs.value("xMin").toString();
			if (str.isEmpty()) {
				reader->raiseWarning(attributeWarning.arg("'xMin'"));
			} else {
				d->xMin = str.toDouble();
				d->xMinPrev = d->xMin;
			}

			str = attribs.value("xMax").toString();
			if (str.isEmpty()) {
				reader->raiseWarning(attributeWarning.arg("'xMax'"));
			} else {
				d->xMax = str.toDouble();
				d->xMaxPrev = d->xMax;
			}

			str = attribs.value("yMin").toString();
			if (str.isEmpty()) {
				reader->raiseWarning(attributeWarning.arg("'yMin'"));
			} else {
				d->yMin = str.toDouble();
				d->yMinPrev = d->yMin;
			}

			str = attribs.value("yMax").toString();
			if (str.isEmpty()) {
				reader->raiseWarning(attributeWarning.arg("'yMax'"));
			} else {
				d->yMax = str.toDouble();
				d->yMaxPrev = d->yMax;
			}

			str = attribs.value("xScale").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'xScale'"));
			else
				d->xScale = CartesianPlot::Scale(str.toInt());

			str = attribs.value("yScale").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'yScale'"));
			else
				d->yScale = CartesianPlot::Scale(str.toInt());

			str = attribs.value("horizontalPadding").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'horizontalPadding'"));
			else
				d->horizontalPadding = str.toDouble();

			str = attribs.value("verticalPadding").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'verticalPadding'"));
			else
				d->verticalPadding = str.toDouble();
		} else if (reader->name() == "xRangeBreaks") {
			//delete default rang break
			d->xRangeBreaks.list.clear();

			attribs = reader->attributes();
			str = attribs.value("enabled").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'enabled'"));
			else
				d->xRangeBreakingEnabled = str.toInt();
		} else if (reader->name() == "xRangeBreak") {
			attribs = reader->attributes();

			RangeBreak b;
			str = attribs.value("start").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'start'"));
			else
				b.start = str.toDouble();

			str = attribs.value("end").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'end'"));
			else
				b.end = str.toDouble();

			str = attribs.value("position").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'position'"));
			else
				b.position = str.toDouble();

			str = attribs.value("style").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'style'"));
			else
				b.style = CartesianPlot::RangeBreakStyle(str.toInt());

			d->xRangeBreaks.list << b;
		} else if (reader->name() == "yRangeBreaks") {
			//delete default rang break
			d->yRangeBreaks.list.clear();

			attribs = reader->attributes();
			str = attribs.value("enabled").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'enabled'"));
			else
				d->yRangeBreakingEnabled = str.toInt();
		} else if (reader->name() == "yRangeBreak") {
			attribs = reader->attributes();

			RangeBreak b;
			str = attribs.value("start").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'start'"));
			else
				b.start = str.toDouble();

			str = attribs.value("end").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'end'"));
			else
				b.end = str.toDouble();

			str = attribs.value("position").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'position'"));
			else
				b.position = str.toDouble();

			str = attribs.value("style").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'style'"));
			else
				b.style = CartesianPlot::RangeBreakStyle(str.toInt());

			d->yRangeBreaks.list << b;
		} else if (reader->name() == "textLabel") {
			m_title = new TextLabel("");
			if (!m_title->load(reader)) {
				delete m_title;
				m_title=0;
				return false;
			} else {
				addChild(m_title);
			}
		} else if (reader->name() == "plotArea") {
			m_plotArea->load(reader);
		} else if (reader->name() == "axis") {
			Axis* axis = new Axis("");
			if (!axis->load(reader)) {
				delete axis;
				return false;
			} else {
				addChild(axis);
			}
		} else if (reader->name() == "xyCurve") {
			XYCurve* curve = addCurve();
			if (!curve->load(reader)) {
				removeChild(curve);
				return false;
			}
		} else if (reader->name() == "xyEquationCurve") {
			XYEquationCurve* curve = addEquationCurve();
			if (!curve->load(reader)) {
				removeChild(curve);
				return false;
			}
		} else if (reader->name() == "xyFitCurve") {
			XYFitCurve* curve = addFitCurve();
			if (!curve->load(reader)) {
				removeChild(curve);
				return false;
			}
		} else if (reader->name() == "xyFourierFilterCurve") {
			XYFourierFilterCurve* curve = addFourierFilterCurve();
			if (!curve->load(reader)) {
				removeChild(curve);
				return false;
			}
		} else if (reader->name() == "xyFourierTransformCurve") {
			XYFourierTransformCurve* curve = addFourierTransformCurve();
			if (!curve->load(reader)) {
				removeChild(curve);
				return false;
			}
		} else if (reader->name() == "xyInterpolationCurve") {
			XYInterpolationCurve* curve = addInterpolationCurve();
			if (!curve->load(reader)) {
				removeChild(curve);
				return false;
			}
		} else if (reader->name() == "xySmoothCurve") {
			XYSmoothCurve* curve = addSmoothCurve();
			if (!curve->load(reader)) {
				removeChild(curve);
			return false;
			}
		} else if (reader->name() == "cartesianPlotLegend") {
			m_legend = new CartesianPlotLegend(this, "");
			if (!m_legend->load(reader)) {
				delete m_legend;
				return false;
			} else {
				addChild(m_legend);
				addLegendAction->setEnabled(false);	//only one legend is allowed -> disable the action
			}
		} else if (reader->name() == "customPoint") {
			CustomPoint* point = new CustomPoint(this, "");
			if (!point->load(reader)) {
				delete point;
				return false;
			} else {
				addChild(point);
			}
		} else { // unknown element
			reader->raiseWarning(i18n("unknown cartesianPlot element '%1'", reader->name().toString()));
			if (!reader->skipToEndElement()) return false;
		}
	}

	d->retransform();//TODO: This is expensive. why do we need this on load?
	if (m_title) {
		m_title->setHidden(true);
		m_title->graphicsItem()->setParentItem(m_plotArea->graphicsItem());
	}

	return true;
}

void CartesianPlot::loadTheme(const QString& name) {
	KConfig config( ThemeHandler::themeConfigPath(name), KConfig::SimpleConfig );
	loadTheme(config);
}

void CartesianPlot::loadTheme(KConfig& config) {
	const QString str = config.name();
	QString themeName = str.right(str.length() - str.lastIndexOf(QDir::separator()) - 1);
	beginMacro( i18n("%1: Load theme %2.", AbstractAspect::name(), themeName) );

	const QList<WorksheetElement*>& childElements = children<WorksheetElement>(AbstractAspect::IncludeHidden);
	foreach(WorksheetElement *child, childElements)
		child->loadThemeConfig(config);

	const QList<XYCurve*>& childXYCurve = children<XYCurve>(AbstractAspect::IncludeHidden);
	m_themeColorPalette = childXYCurve.last()->getColorPalette();

	Q_D(CartesianPlot);
	d->update(this->rect());
	emit (themeLoaded());

	endMacro();
}

void CartesianPlot::saveTheme(KConfig &config) {
	const QList<Axis*>& axisElements = children<Axis>(AbstractAspect::IncludeHidden);
	const QList<PlotArea*>& plotAreaElements = children<PlotArea>(AbstractAspect::IncludeHidden);
	const QList<TextLabel*>& textLabelElements = children<TextLabel>(AbstractAspect::IncludeHidden);
	const QList<XYCurve*>& xyCurveElements = children<XYCurve>(AbstractAspect::IncludeHidden);

	axisElements.at(0)->saveThemeConfig(config);
	plotAreaElements.at(0)->saveThemeConfig(config);
	textLabelElements.at(0)->saveThemeConfig(config);

	foreach(XYCurve *child, xyCurveElements)
		child->saveThemeConfig(config);
}

void CartesianPlot::applyThemeOnNewCurve(XYCurve* curve) {
	curve->applyColorPalette(m_themeColorPalette);
}
