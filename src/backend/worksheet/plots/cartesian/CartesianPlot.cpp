/***************************************************************************
    File                 : CartesianPlot.cpp
    Project              : LabPlot
    Description          : Cartesian plot
    --------------------------------------------------------------------
    Copyright            : (C) 2011-2018 by Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2016-2018 by Stefan Gerlach (stefan.gerlach@uni.kn)
    Copyright            : (C) 2017-2018 by Garvit Khatri (garvitdelhi@gmail.com)

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
#include "Histogram.h"
#include "XYEquationCurve.h"
#include "XYDataReductionCurve.h"
#include "XYDifferentiationCurve.h"
#include "XYIntegrationCurve.h"
#include "XYInterpolationCurve.h"
#include "XYSmoothCurve.h"
#include "XYFitCurve.h"
#include "XYFourierFilterCurve.h"
#include "XYFourierTransformCurve.h"
#include "XYConvolutionCurve.h"
#include "XYCorrelationCurve.h"
#include "backend/core/Project.h"
#include "backend/core/datatypes/DateTime2StringFilter.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/worksheet/plots/cartesian/CartesianPlotLegend.h"
#include "backend/worksheet/plots/cartesian/CustomPoint.h"
#include "backend/worksheet/plots/PlotArea.h"
#include "backend/worksheet/plots/AbstractPlotPrivate.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/Axis.h"
#include "backend/worksheet/TextLabel.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/macros.h"
#include "backend/lib/trace.h"
#include "kdefrontend/spreadsheet/PlotDataDialog.h" //for PlotDataDialog::AnalysisAction. TODO: find a better place for this enum.
#include "kdefrontend/ThemeHandler.h"
#include "kdefrontend/widgets/ThemesWidget.h"

#include <QDir>
#include <QDropEvent>
#include <QIcon>
#include <QMenu>
#include <QMimeData>
#include <QPainter>
#include <QWidgetAction>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>

/**
 * \class CartesianPlot
 * \brief A xy-plot.
 *
 *
 */
CartesianPlot::CartesianPlot(const QString &name)
	: AbstractPlot(name, new CartesianPlotPrivate(this), AspectType::CartesianPlot) {

	init();
}

CartesianPlot::CartesianPlot(const QString &name, CartesianPlotPrivate *dd)
	: AbstractPlot(name, dd, AspectType::CartesianPlot) {

	init();
}

CartesianPlot::~CartesianPlot() {
	if (m_menusInitialized) {
		delete addNewMenu;
		delete zoomMenu;
		delete themeMenu;
	}

	delete m_coordinateSystem;

	//don't need to delete objects added with addChild()

	//no need to delete the d-pointer here - it inherits from QGraphicsItem
	//and is deleted during the cleanup in QGraphicsScene
}

/*!
	initializes all member variables of \c CartesianPlot
*/
void CartesianPlot::init() {
	Q_D(CartesianPlot);

	d->cSystem = new CartesianCoordinateSystem(this);
	m_coordinateSystem = d->cSystem;

	d->rangeType = CartesianPlot::RangeFree;
	d->xRangeFormat = CartesianPlot::Numeric;
	d->yRangeFormat = CartesianPlot::Numeric;
	d->xRangeDateTimeFormat = "yyyy-MM-dd hh:mm:ss";
	d->yRangeDateTimeFormat = "yyyy-MM-dd hh:mm:ss";
	d->rangeFirstValues = 1000;
	d->rangeLastValues = 1000;
	d->autoScaleX = true;
	d->autoScaleY = true;
	d->xScale = ScaleLinear;
	d->yScale = ScaleLinear;
	d->xRangeBreakingEnabled = false;
	d->yRangeBreakingEnabled = false;

	//the following factor determines the size of the offset between the min/max points of the curves
	//and the coordinate system ranges, when doing auto scaling
	//Factor 0 corresponds to the exact match - min/max values of the curves correspond to the start/end values of the ranges.
	//TODO: make this factor optional.
	//Provide in the UI the possibility to choose between "exact" or 0% offset, 2%, 5% and 10% for the auto fit option
	d->autoScaleOffsetFactor = 0.0f;

	m_plotArea = new PlotArea(name() + " plot area");
	addChildFast(m_plotArea);

	//Plot title
	m_title = new TextLabel(this->name() + QLatin1String("- ") + i18n("Title"), TextLabel::PlotTitle);
	addChild(m_title);
	m_title->setHidden(true);
	m_title->setParentGraphicsItem(m_plotArea->graphicsItem());

	//offset between the plot area and the area defining the coordinate system, in scene units.
	d->horizontalPadding = Worksheet::convertToSceneUnits(1.5, Worksheet::Centimeter);
	d->verticalPadding = Worksheet::convertToSceneUnits(1.5, Worksheet::Centimeter);

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
void CartesianPlot::initDefault(Type type) {
	Q_D(CartesianPlot);

	switch (type) {
	case FourAxes: {
			d->xMin = 0.0;
			d->xMax = 1.0;
			d->yMin = 0.0;
			d->yMax = 1.0;

			//Axes
			Axis* axis = new Axis("x axis 1", Axis::AxisHorizontal);
			axis->setSuppressRetransform(true);
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
			axis->setSuppressRetransform(false);

			axis = new Axis("x axis 2", Axis::AxisHorizontal);
			axis->setSuppressRetransform(true);
			addChild(axis);
			axis->setPosition(Axis::AxisTop);
			axis->setStart(0);
			axis->setEnd(1);
			axis->setMajorTicksDirection(Axis::ticksIn);
			axis->setMajorTicksNumber(6);
			axis->setMinorTicksDirection(Axis::ticksIn);
			axis->setMinorTicksNumber(1);
			pen = axis->minorGridPen();
			pen.setStyle(Qt::NoPen);
			axis->setMajorGridPen(pen);
			pen = axis->minorGridPen();
			pen.setStyle(Qt::NoPen);
			axis->setMinorGridPen(pen);
			axis->setLabelsPosition(Axis::NoLabels);
			axis->title()->setText(QString());
			axis->setSuppressRetransform(false);

			axis = new Axis("y axis 1", Axis::AxisVertical);
			axis->setSuppressRetransform(true);
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
			axis->setSuppressRetransform(false);

			axis = new Axis("y axis 2", Axis::AxisVertical);
			axis->setSuppressRetransform(true);
			addChild(axis);
			axis->setPosition(Axis::AxisRight);
			axis->setStart(0);
			axis->setEnd(1);
			axis->setOffset(1);
			axis->setMajorTicksDirection(Axis::ticksIn);
			axis->setMajorTicksNumber(6);
			axis->setMinorTicksDirection(Axis::ticksIn);
			axis->setMinorTicksNumber(1);
			pen = axis->minorGridPen();
			pen.setStyle(Qt::NoPen);
			axis->setMajorGridPen(pen);
			pen = axis->minorGridPen();
			pen.setStyle(Qt::NoPen);
			axis->setLabelsPosition(Axis::NoLabels);
			axis->title()->setText(QString());
			axis->setSuppressRetransform(false);

			break;
		}
	case TwoAxes: {
			d->xMin = 0.0;
			d->xMax = 1.0;
			d->yMin = 0.0;
			d->yMax = 1.0;

			Axis* axis = new Axis("x axis 1", Axis::AxisHorizontal);
			axis->setSuppressRetransform(true);
			addChild(axis);
			axis->setPosition(Axis::AxisBottom);
			axis->setStart(0);
			axis->setEnd(1);
			axis->setMajorTicksDirection(Axis::ticksBoth);
			axis->setMajorTicksNumber(6);
			axis->setMinorTicksDirection(Axis::ticksBoth);
			axis->setMinorTicksNumber(1);
			axis->setArrowType(Axis::FilledArrowSmall);
			axis->setSuppressRetransform(false);

			axis = new Axis("y axis 1", Axis::AxisVertical);
			axis->setSuppressRetransform(true);
			addChild(axis);
			axis->setPosition(Axis::AxisLeft);
			axis->setStart(0);
			axis->setEnd(1);
			axis->setMajorTicksDirection(Axis::ticksBoth);
			axis->setMajorTicksNumber(6);
			axis->setMinorTicksDirection(Axis::ticksBoth);
			axis->setMinorTicksNumber(1);
			axis->setArrowType(Axis::FilledArrowSmall);
			axis->setSuppressRetransform(false);

			break;
		}
	case TwoAxesCentered: {
			d->xMin = -0.5;
			d->xMax = 0.5;
			d->yMin = -0.5;
			d->yMax = 0.5;

			d->horizontalPadding = Worksheet::convertToSceneUnits(1.0, Worksheet::Centimeter);
			d->verticalPadding = Worksheet::convertToSceneUnits(1.0, Worksheet::Centimeter);

			QPen pen = m_plotArea->borderPen();
			pen.setStyle(Qt::NoPen);
			m_plotArea->setBorderPen(pen);

			Axis* axis = new Axis("x axis 1", Axis::AxisHorizontal);
			axis->setSuppressRetransform(true);
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
			axis->setSuppressRetransform(false);

			axis = new Axis("y axis 1", Axis::AxisVertical);
			axis->setSuppressRetransform(true);
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
			axis->setSuppressRetransform(false);

			break;
		}
	case TwoAxesCenteredZero: {
			d->xMin = -0.5;
			d->xMax = 0.5;
			d->yMin = -0.5;
			d->yMax = 0.5;

			d->horizontalPadding = Worksheet::convertToSceneUnits(1.0, Worksheet::Centimeter);
			d->verticalPadding = Worksheet::convertToSceneUnits(1.0, Worksheet::Centimeter);

			QPen pen = m_plotArea->borderPen();
			pen.setStyle(Qt::NoPen);
			m_plotArea->setBorderPen(pen);

			Axis* axis = new Axis("x axis 1", Axis::AxisHorizontal);
			axis->setSuppressRetransform(true);
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
			axis->setSuppressRetransform(false);

			axis = new Axis("y axis 1", Axis::AxisVertical);
			axis->setSuppressRetransform(true);
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
			axis->setSuppressRetransform(false);

			break;
		}
	}

	d->xMinPrev = d->xMin;
	d->xMaxPrev = d->xMax;
	d->yMinPrev = d->yMin;
	d->yMaxPrev = d->yMax;

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
	addCurveAction = new QAction(QIcon::fromTheme("labplot-xy-curve"), i18n("xy-curve"), this);
	addHistogramAction = new QAction(QIcon::fromTheme("view-object-histogram-linear"), i18n("Histogram"), this);
	addEquationCurveAction = new QAction(QIcon::fromTheme("labplot-xy-equation-curve"), i18n("xy-curve from a mathematical Equation"), this);
// no icons yet
	addDataReductionCurveAction = new QAction(i18n("xy-curve from a Data Reduction"), this);
	addDifferentiationCurveAction = new QAction(i18n("xy-curve from a Differentiation"), this);
	addIntegrationCurveAction = new QAction(i18n("xy-curve from an Integration"), this);
	addInterpolationCurveAction = new QAction(i18n("xy-curve from an Interpolation"), this);
	addSmoothCurveAction = new QAction(i18n("xy-curve from a Smooth"), this);
	addFitCurveAction = new QAction(QIcon::fromTheme("labplot-xy-fit-curve"), i18n("xy-curve from a Fit to Data"), this);
	addFourierFilterCurveAction = new QAction(i18n("xy-curve from a Fourier Filter"), this);
	addFourierTransformCurveAction = new QAction(i18n("xy-curve from a Fourier Transform"), this);
	addConvolutionCurveAction = new QAction(i18n("xy-curve from a (De-)Convolution"), this);
	addCorrelationCurveAction = new QAction(i18n("xy-curve from a Auto-/Cross-Correlation"), this);
//	addInterpolationCurveAction = new QAction(QIcon::fromTheme("labplot-xy-interpolation-curve"), i18n("xy-curve from an interpolation"), this);
//	addSmoothCurveAction = new QAction(QIcon::fromTheme("labplot-xy-smooth-curve"), i18n("xy-curve from a smooth"), this);
//	addFourierFilterCurveAction = new QAction(QIcon::fromTheme("labplot-xy-fourier_filter-curve"), i18n("xy-curve from a Fourier filter"), this);
//	addFourierTransformCurveAction = new QAction(QIcon::fromTheme("labplot-xy-fourier_transform-curve"), i18n("xy-curve from a Fourier transform"), this);
//	addConvolutionCurveAction = new QAction(QIcon::fromTheme("labplot-xy-convolution-curve"), i18n("xy-curve from a (de-)convolution"), this);
//	addCorrelationCurveAction = new QAction(QIcon::fromTheme("labplot-xy-correlation-curve"), i18n("xy-curve from a auto-/cross-correlation"), this);
	addLegendAction = new QAction(QIcon::fromTheme("text-field"), i18n("Legend"), this);
	if (children<CartesianPlotLegend>().size()>0)
		addLegendAction->setEnabled(false);	//only one legend is allowed -> disable the action

	addHorizontalAxisAction = new QAction(QIcon::fromTheme("labplot-axis-horizontal"), i18n("Horizontal Axis"), this);
	addVerticalAxisAction = new QAction(QIcon::fromTheme("labplot-axis-vertical"), i18n("Vertical Axis"), this);
	addTextLabelAction = new QAction(QIcon::fromTheme("draw-text"), i18n("Text Label"), this);
	addCustomPointAction = new QAction(QIcon::fromTheme("draw-cross"), i18n("Custom Point"), this);

	connect(addCurveAction, SIGNAL(triggered()), SLOT(addCurve()));
	connect(addHistogramAction,SIGNAL(triggered()), SLOT(addHistogram()));
	connect(addEquationCurveAction, SIGNAL(triggered()), SLOT(addEquationCurve()));
	connect(addDataReductionCurveAction, SIGNAL(triggered()), SLOT(addDataReductionCurve()));
	connect(addDifferentiationCurveAction, SIGNAL(triggered()), SLOT(addDifferentiationCurve()));
	connect(addIntegrationCurveAction, SIGNAL(triggered()), SLOT(addIntegrationCurve()));
	connect(addInterpolationCurveAction, SIGNAL(triggered()), SLOT(addInterpolationCurve()));
	connect(addSmoothCurveAction, SIGNAL(triggered()), SLOT(addSmoothCurve()));
	connect(addFitCurveAction, SIGNAL(triggered()), SLOT(addFitCurve()));
	connect(addFourierFilterCurveAction, SIGNAL(triggered()), SLOT(addFourierFilterCurve()));
	connect(addFourierTransformCurveAction, SIGNAL(triggered()), SLOT(addFourierTransformCurve()));
	connect(addConvolutionCurveAction, SIGNAL(triggered()), SLOT(addConvolutionCurve()));
	connect(addCorrelationCurveAction, SIGNAL(triggered()), SLOT(addCorrelationCurve()));

	connect(addLegendAction, SIGNAL(triggered()), SLOT(addLegend()));
	connect(addHorizontalAxisAction, SIGNAL(triggered()), SLOT(addHorizontalAxis()));
	connect(addVerticalAxisAction, SIGNAL(triggered()), SLOT(addVerticalAxis()));
	connect(addTextLabelAction, SIGNAL(triggered()), SLOT(addTextLabel()));
	connect(addCustomPointAction, SIGNAL(triggered()), SLOT(addCustomPoint()));

	//Analysis menu actions
	addDataOperationAction = new QAction(i18n("Data Operation"), this);
	addDataReductionAction = new QAction(i18n("Reduce Data"), this);
	addDifferentiationAction = new QAction(i18n("Differentiate"), this);
	addIntegrationAction = new QAction(i18n("Integrate"), this);
	addInterpolationAction = new QAction(i18n("Interpolate"), this);
	addSmoothAction = new QAction(i18n("Smooth"), this);
	addConvolutionAction = new QAction(i18n("Convolute/Deconvolute"), this);
	addCorrelationAction = new QAction(i18n("Auto-/Cross-Correlation"), this);

	QAction* fitAction = new QAction(i18n("Linear"), this);
	fitAction->setData(PlotDataDialog::FitLinear);
	addFitAction.append(fitAction);

	fitAction = new QAction(i18n("Power"), this);
	fitAction->setData(PlotDataDialog::FitPower);
	addFitAction.append(fitAction);

	fitAction = new QAction(i18n("Exponential (degree 1)"), this);
	fitAction->setData(PlotDataDialog::FitExp1);
	addFitAction.append(fitAction);

	fitAction = new QAction(i18n("Exponential (degree 2)"), this);
	fitAction->setData(PlotDataDialog::FitExp2);
	addFitAction.append(fitAction);

	fitAction = new QAction(i18n("Inverse exponential"), this);
	fitAction->setData(PlotDataDialog::FitInvExp);
	addFitAction.append(fitAction);

	fitAction = new QAction(i18n("Gauss"), this);
	fitAction->setData(PlotDataDialog::FitGauss);
	addFitAction.append(fitAction);

	fitAction = new QAction(i18n("Cauchy-Lorentz"), this);
	fitAction->setData(PlotDataDialog::FitCauchyLorentz);
	addFitAction.append(fitAction);

	fitAction = new QAction(i18n("Arc Tangent"), this);
	fitAction->setData(PlotDataDialog::FitTan);
	addFitAction.append(fitAction);

	fitAction = new QAction(i18n("Hyperbolic Tangent"), this);
	fitAction->setData(PlotDataDialog::FitTanh);
	addFitAction.append(fitAction);

	fitAction = new QAction(i18n("Error Function"), this);
	fitAction->setData(PlotDataDialog::FitErrFunc);
	addFitAction.append(fitAction);

	fitAction = new QAction(i18n("Custom"), this);
	fitAction->setData(PlotDataDialog::FitCustom);
	addFitAction.append(fitAction);

	addFourierFilterAction = new QAction(i18n("Fourier Filter"), this);

	connect(addDataReductionAction, SIGNAL(triggered()), SLOT(addDataReductionCurve()));
	connect(addDifferentiationAction, SIGNAL(triggered()), SLOT(addDifferentiationCurve()));
	connect(addIntegrationAction, SIGNAL(triggered()), SLOT(addIntegrationCurve()));
	connect(addInterpolationAction, SIGNAL(triggered()), SLOT(addInterpolationCurve()));
	connect(addSmoothAction, SIGNAL(triggered()), SLOT(addSmoothCurve()));
	connect(addConvolutionAction, SIGNAL(triggered()), SLOT(addConvolutionCurve()));
	connect(addCorrelationAction, SIGNAL(triggered()), SLOT(addCorrelationCurve()));
	for (const auto& action : addFitAction)
		connect(action, SIGNAL(triggered()), SLOT(addFitCurve()));
	connect(addFourierFilterAction, SIGNAL(triggered()), SLOT(addFourierFilterCurve()));

	//zoom/navigate actions
	scaleAutoAction = new QAction(QIcon::fromTheme("labplot-auto-scale-all"), i18n("Auto Scale"), this);
	scaleAutoXAction = new QAction(QIcon::fromTheme("labplot-auto-scale-x"), i18n("Auto Scale X"), this);
	scaleAutoYAction = new QAction(QIcon::fromTheme("labplot-auto-scale-y"), i18n("Auto Scale Y"), this);
	zoomInAction = new QAction(QIcon::fromTheme("zoom-in"), i18n("Zoom In"), this);
	zoomOutAction = new QAction(QIcon::fromTheme("zoom-out"), i18n("Zoom Out"), this);
	zoomInXAction = new QAction(QIcon::fromTheme("labplot-zoom-in-x"), i18n("Zoom In X"), this);
	zoomOutXAction = new QAction(QIcon::fromTheme("labplot-zoom-out-x"), i18n("Zoom Out X"), this);
	zoomInYAction = new QAction(QIcon::fromTheme("labplot-zoom-in-y"), i18n("Zoom In Y"), this);
	zoomOutYAction = new QAction(QIcon::fromTheme("labplot-zoom-out-y"), i18n("Zoom Out Y"), this);
	shiftLeftXAction = new QAction(QIcon::fromTheme("labplot-shift-left-x"), i18n("Shift Left X"), this);
	shiftRightXAction = new QAction(QIcon::fromTheme("labplot-shift-right-x"), i18n("Shift Right X"), this);
	shiftUpYAction = new QAction(QIcon::fromTheme("labplot-shift-up-y"), i18n("Shift Up Y"), this);
	shiftDownYAction = new QAction(QIcon::fromTheme("labplot-shift-down-y"), i18n("Shift Down Y"), this);

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
	visibilityAction = new QAction(i18n("Visible"), this);
	visibilityAction->setCheckable(true);
	connect(visibilityAction, SIGNAL(triggered()), this, SLOT(visibilityChanged()));
}

void CartesianPlot::initMenus() {
	initActions();

	addNewMenu = new QMenu(i18n("Add New"));
	addNewMenu->addAction(addCurveAction);
	addNewMenu->addAction(addHistogramAction);
	addNewMenu->addAction(addEquationCurveAction);
	addNewMenu->addSeparator();
	addNewMenu->addAction(addDataReductionCurveAction);
	addNewMenu->addAction(addDifferentiationCurveAction);
	addNewMenu->addAction(addIntegrationCurveAction);
	addNewMenu->addAction(addInterpolationCurveAction);
	addNewMenu->addAction(addSmoothCurveAction);
	addNewMenu->addAction(addFitCurveAction);
	addNewMenu->addAction(addFourierFilterCurveAction);
	addNewMenu->addAction(addFourierTransformCurveAction);
	addNewMenu->addAction(addConvolutionCurveAction);
	addNewMenu->addAction(addCorrelationCurveAction);
	addNewMenu->addSeparator();
	addNewMenu->addAction(addLegendAction);
	addNewMenu->addSeparator();
	addNewMenu->addAction(addHorizontalAxisAction);
	addNewMenu->addAction(addVerticalAxisAction);
	addNewMenu->addSeparator();
	addNewMenu->addAction(addTextLabelAction);
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

	// Data manipulation menu
	QMenu* dataManipulationMenu = new QMenu(i18n("Data Manipulation"));
	dataManipulationMenu->setIcon(QIcon::fromTheme("zoom-draw"));
	dataManipulationMenu->addAction(addDataOperationAction);
	dataManipulationMenu->addAction(addDataReductionAction);

	// Data fit menu
	QMenu* dataFitMenu = new QMenu(i18n("Fit"));
	dataFitMenu->setIcon(QIcon::fromTheme("labplot-xy-fit-curve"));
	dataFitMenu->addAction(addFitAction.at(0));
	dataFitMenu->addAction(addFitAction.at(1));
	dataFitMenu->addAction(addFitAction.at(2));
	dataFitMenu->addAction(addFitAction.at(3));
	dataFitMenu->addAction(addFitAction.at(4));
	dataFitMenu->addSeparator();
	dataFitMenu->addAction(addFitAction.at(5));
	dataFitMenu->addAction(addFitAction.at(6));
	dataFitMenu->addSeparator();
	dataFitMenu->addAction(addFitAction.at(7));
	dataFitMenu->addAction(addFitAction.at(8));
	dataFitMenu->addAction(addFitAction.at(9));
	dataFitMenu->addSeparator();
	dataFitMenu->addAction(addFitAction.at(10));

	//analysis menu
	dataAnalysisMenu = new QMenu(i18n("Analysis"));
	dataAnalysisMenu->insertMenu(nullptr, dataManipulationMenu);
	dataAnalysisMenu->addSeparator();
	dataAnalysisMenu->addAction(addDifferentiationAction);
	dataAnalysisMenu->addAction(addIntegrationAction);
	dataAnalysisMenu->addSeparator();
	dataAnalysisMenu->addAction(addInterpolationAction);
	dataAnalysisMenu->addAction(addSmoothAction);
	dataAnalysisMenu->addAction(addFourierFilterAction);
	dataAnalysisMenu->addAction(addConvolutionAction);
	dataAnalysisMenu->addAction(addCorrelationAction);
	dataAnalysisMenu->addSeparator();
	dataAnalysisMenu->addMenu(dataFitMenu);

	//themes menu
	themeMenu = new QMenu(i18n("Apply Theme"));
	auto* themeWidget = new ThemesWidget(nullptr);
	connect(themeWidget, SIGNAL(themeSelected(QString)), this, SLOT(loadTheme(QString)));
	connect(themeWidget, SIGNAL(themeSelected(QString)), themeMenu, SLOT(close()));

	auto* widgetAction = new QWidgetAction(this);
	widgetAction->setDefaultWidget(themeWidget);
	themeMenu->addAction(widgetAction);

	m_menusInitialized = true;
}

QMenu* CartesianPlot::createContextMenu() {
	if (!m_menusInitialized)
		initMenus();

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

QMenu* CartesianPlot::analysisMenu() {
	if (!m_menusInitialized)
		initMenus();

	return dataAnalysisMenu;
}

/*!
	Returns an icon to be used in the project explorer.
*/
QIcon CartesianPlot::icon() const {
	return QIcon::fromTheme("office-chart-line");
}

QVector<AbstractAspect*> CartesianPlot::dependsOn() const {
	//aspects which the plotted data in the worksheet depends on (spreadsheets and later matrices)
	QVector<AbstractAspect*> aspects;

	for (const auto* curve : children<XYCurve>()) {
		if (curve->xColumn() && dynamic_cast<Spreadsheet*>(curve->xColumn()->parentAspect()) )
			aspects << curve->xColumn()->parentAspect();

		if (curve->yColumn() && dynamic_cast<Spreadsheet*>(curve->yColumn()->parentAspect()) )
			aspects << curve->yColumn()->parentAspect();
	}

	return aspects;
}

void CartesianPlot::navigate(CartesianPlot::NavigationOperation op) {
	if (op == ScaleAuto) scaleAuto();
	else if (op == ScaleAutoX) scaleAutoX();
	else if (op == ScaleAutoY) scaleAutoY();
	else if (op == ZoomIn) zoomIn();
	else if (op == ZoomOut) zoomOut();
	else if (op == ZoomInX) zoomInX();
	else if (op == ZoomOutX) zoomOutX();
	else if (op == ZoomInY) zoomInY();
	else if (op == ZoomOutY) zoomOutY();
	else if (op == ShiftLeftX) shiftLeftX();
	else if (op == ShiftRightX) shiftRightX();
	else if (op == ShiftUpY) shiftUpY();
	else if (op == ShiftDownY) shiftDownY();
}

void CartesianPlot::setSuppressDataChangedSignal(bool value) {
	Q_D(CartesianPlot);
	d->suppressRetransform = value;
}

void CartesianPlot::processDropEvent(QDropEvent* event) {
	PERFTRACE("CartesianPlot::processDropEvent");
	const QMimeData* mimeData = event->mimeData();
	if (!mimeData)
		return;

	//deserialize the mime data to the vector of aspect pointers
	QByteArray data = mimeData->data(QLatin1String("labplot-dnd"));
	QVector<quintptr> vec;
	QDataStream stream(&data, QIODevice::ReadOnly);
	stream >> vec;
	QVector<AbstractColumn*> columns;
	for (auto a : vec) {
		auto* aspect = (AbstractAspect*)a;
		auto* column = dynamic_cast<AbstractColumn*>(aspect);
		if (column)
			columns << column;
	}

	//return if there are no columns being dropped.
	//TODO: extend this later when we allow to drag&drop plots, etc.
	if (columns.isEmpty())
		return;

	//determine the first column with "x plot designation" as the x-data column for all curves to be created
	const AbstractColumn* xColumn = nullptr;
	for (const auto* column : columns) {
		if (column->plotDesignation() == AbstractColumn::X) {
			xColumn = column;
			break;
		}
	}

	//if no column with "x plot designation" is available, use the x-data column of the first curve in the plot,
	if (xColumn == nullptr) {
		QVector<XYCurve*> curves = children<XYCurve>();
		if (!curves.isEmpty())
			xColumn = curves.at(0)->xColumn();
	}

	//use the first dropped column if no column with "x plot designation" nor curves are available
	if (xColumn == nullptr)
		xColumn = columns.at(0);

	//create curves
	bool curvesAdded = false;
	for (const auto* column : columns) {
		if (column == xColumn)
			continue;

		XYCurve* curve = new XYCurve(column->name());
		curve->suppressRetransform(true); //suppress retransform, all curved will be recalculated at the end
		curve->setXColumn(xColumn);
		curve->setYColumn(column);
		addChild(curve);
		curve->suppressRetransform(false);
		curvesAdded = true;
	}

	if (curvesAdded)
		dataChanged();
}


bool CartesianPlot::isPanningActive() const {
	Q_D(const CartesianPlot);
	return d->panningStarted;
}

//##############################################################################
//################################  getter methods  ############################
//##############################################################################
BASIC_SHARED_D_READER_IMPL(CartesianPlot, CartesianPlot::RangeType, rangeType, rangeType)
BASIC_SHARED_D_READER_IMPL(CartesianPlot, CartesianPlot::RangeFormat, xRangeFormat, xRangeFormat)
BASIC_SHARED_D_READER_IMPL(CartesianPlot, CartesianPlot::RangeFormat, yRangeFormat, yRangeFormat)
BASIC_SHARED_D_READER_IMPL(CartesianPlot, int, rangeLastValues, rangeLastValues)
BASIC_SHARED_D_READER_IMPL(CartesianPlot, int, rangeFirstValues, rangeFirstValues)
BASIC_SHARED_D_READER_IMPL(CartesianPlot, bool, autoScaleX, autoScaleX)
BASIC_SHARED_D_READER_IMPL(CartesianPlot, double, xMin, xMin)
BASIC_SHARED_D_READER_IMPL(CartesianPlot, double, xMax, xMax)
BASIC_SHARED_D_READER_IMPL(CartesianPlot, CartesianPlot::Scale, xScale, xScale)
BASIC_SHARED_D_READER_IMPL(CartesianPlot, bool, xRangeBreakingEnabled, xRangeBreakingEnabled)
CLASS_SHARED_D_READER_IMPL(CartesianPlot, CartesianPlot::RangeBreaks, xRangeBreaks, xRangeBreaks)

BASIC_SHARED_D_READER_IMPL(CartesianPlot, bool, autoScaleY, autoScaleY)
BASIC_SHARED_D_READER_IMPL(CartesianPlot, double, yMin, yMin)
BASIC_SHARED_D_READER_IMPL(CartesianPlot, double, yMax, yMax)
BASIC_SHARED_D_READER_IMPL(CartesianPlot, CartesianPlot::Scale, yScale, yScale)
BASIC_SHARED_D_READER_IMPL(CartesianPlot, bool, yRangeBreakingEnabled, yRangeBreakingEnabled)
CLASS_SHARED_D_READER_IMPL(CartesianPlot, CartesianPlot::RangeBreaks, yRangeBreaks, yRangeBreaks)

CLASS_SHARED_D_READER_IMPL(CartesianPlot, QString, theme, theme)

/*!
	returns the actual bounding rectangular of the plot area showing data (plot's rectangular minus padding)
	in plot's coordinates
 */
QRectF CartesianPlot::dataRect() const {
	Q_D(const CartesianPlot);
	return d->dataRect;
}

CartesianPlot::MouseMode CartesianPlot::mouseMode() const {
	Q_D(const CartesianPlot);
	return d->mouseMode;
}

const QString& CartesianPlot::xRangeDateTimeFormat() const {
	Q_D(const CartesianPlot);
	return d->xRangeDateTimeFormat;
}

const QString& CartesianPlot::yRangeDateTimeFormat() const {
	Q_D(const CartesianPlot);
	return d->yRangeDateTimeFormat;
}

//##############################################################################
//######################  setter methods and undo commands  ####################
//##############################################################################
/*!
	set the rectangular, defined in scene coordinates
 */
class CartesianPlotSetRectCmd : public QUndoCommand {
public:
	CartesianPlotSetRectCmd(CartesianPlotPrivate* private_obj, QRectF rect) : m_private(private_obj), m_rect(rect) {
		setText(i18n("%1: change geometry rect", m_private->name()));
	};

	void redo() override {
		const double horizontalRatio = m_rect.width() / m_private->rect.width();
		const double verticalRatio = m_rect.height() / m_private->rect.height();

		qSwap(m_private->rect, m_rect);

		m_private->q->handleResize(horizontalRatio, verticalRatio, false);
		m_private->retransform();
		emit m_private->q->rectChanged(m_private->rect);
	};

	void undo() override {
		redo();
	}

private:
	CartesianPlotPrivate* m_private;
	QRectF m_rect;
};

void CartesianPlot::setRect(const QRectF& rect) {
	Q_D(CartesianPlot);
	if (rect != d->rect)
		exec(new CartesianPlotSetRectCmd(d, rect));
}

STD_SETTER_CMD_IMPL_F_S(CartesianPlot, SetRangeType, CartesianPlot::RangeType, rangeType, rangeChanged);
void CartesianPlot::setRangeType(RangeType type) {
	Q_D(CartesianPlot);
	if (type != d->rangeType)
		exec(new CartesianPlotSetRangeTypeCmd(d, type, ki18n("%1: set range type")));
}

STD_SETTER_CMD_IMPL_F_S(CartesianPlot, SetXRangeFormat, CartesianPlot::RangeFormat, xRangeFormat, xRangeFormatChanged);
void CartesianPlot::setXRangeFormat(RangeFormat format) {
	Q_D(CartesianPlot);
	if (format != d->xRangeFormat)
		exec(new CartesianPlotSetXRangeFormatCmd(d, format, ki18n("%1: set x-range format")));
}

STD_SETTER_CMD_IMPL_F_S(CartesianPlot, SetYRangeFormat, CartesianPlot::RangeFormat, yRangeFormat, yRangeFormatChanged);
void CartesianPlot::setYRangeFormat(RangeFormat format) {
	Q_D(CartesianPlot);
	if (format != d->yRangeFormat)
		exec(new CartesianPlotSetYRangeFormatCmd(d, format, ki18n("%1: set y-range format")));
}

STD_SETTER_CMD_IMPL_F_S(CartesianPlot, SetRangeLastValues, int, rangeLastValues, rangeChanged);
void CartesianPlot::setRangeLastValues(int values) {
	Q_D(CartesianPlot);
	if (values != d->rangeLastValues)
		exec(new CartesianPlotSetRangeLastValuesCmd(d, values, ki18n("%1: set range")));
}

STD_SETTER_CMD_IMPL_F_S(CartesianPlot, SetRangeFirstValues, int, rangeFirstValues, rangeChanged);
void CartesianPlot::setRangeFirstValues(int values) {
	Q_D(CartesianPlot);
	if (values != d->rangeFirstValues)
		exec(new CartesianPlotSetRangeFirstValuesCmd(d, values, ki18n("%1: set range")));
}


class CartesianPlotSetAutoScaleXCmd : public QUndoCommand {
public:
	CartesianPlotSetAutoScaleXCmd(CartesianPlotPrivate* private_obj, bool autoScale) :
		m_private(private_obj), m_autoScale(autoScale), m_autoScaleOld(false), m_minOld(0.0), m_maxOld(0.0) {
		setText(i18n("%1: change x-range auto scaling", m_private->name()));
	};

	void redo() override {
		m_autoScaleOld = m_private->autoScaleX;
		if (m_autoScale) {
			m_minOld = m_private->xMin;
			m_maxOld = m_private->xMax;
			m_private->q->scaleAutoX();
		}
		m_private->autoScaleX = m_autoScale;
		emit m_private->q->xAutoScaleChanged(m_autoScale);
	};

	void undo() override {
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
	double m_minOld;
	double m_maxOld;
};

void CartesianPlot::setAutoScaleX(bool autoScaleX) {
	Q_D(CartesianPlot);
	if (autoScaleX != d->autoScaleX)
		exec(new CartesianPlotSetAutoScaleXCmd(d, autoScaleX));
}

STD_SETTER_CMD_IMPL_F_S(CartesianPlot, SetXMin, double, xMin, retransformScales)
void CartesianPlot::setXMin(double xMin) {
	Q_D(CartesianPlot);
	if (xMin != d->xMin && xMin != -INFINITY && xMin != INFINITY)
		exec(new CartesianPlotSetXMinCmd(d, xMin, ki18n("%1: set min x")));
}

STD_SETTER_CMD_IMPL_F_S(CartesianPlot, SetXMax, double, xMax, retransformScales)
void CartesianPlot::setXMax(double xMax) {
	Q_D(CartesianPlot);
	if (xMax != d->xMax && xMax != -INFINITY && xMax != INFINITY)
		exec(new CartesianPlotSetXMaxCmd(d, xMax, ki18n("%1: set max x")));
}

STD_SETTER_CMD_IMPL_F_S(CartesianPlot, SetXScale, CartesianPlot::Scale, xScale, retransformScales)
void CartesianPlot::setXScale(Scale scale) {
	Q_D(CartesianPlot);
	if (scale != d->xScale)
		exec(new CartesianPlotSetXScaleCmd(d, scale, ki18n("%1: set x scale")));
}

STD_SETTER_CMD_IMPL_F_S(CartesianPlot, SetXRangeBreakingEnabled, bool, xRangeBreakingEnabled, retransformScales)
void CartesianPlot::setXRangeBreakingEnabled(bool enabled) {
	Q_D(CartesianPlot);
	if (enabled != d->xRangeBreakingEnabled)
		exec(new CartesianPlotSetXRangeBreakingEnabledCmd(d, enabled, ki18n("%1: x-range breaking enabled")));
}

STD_SETTER_CMD_IMPL_F_S(CartesianPlot, SetXRangeBreaks, CartesianPlot::RangeBreaks, xRangeBreaks, retransformScales)
void CartesianPlot::setXRangeBreaks(const RangeBreaks& breakings) {
	Q_D(CartesianPlot);
	exec(new CartesianPlotSetXRangeBreaksCmd(d, breakings, ki18n("%1: x-range breaks changed")));
}

class CartesianPlotSetAutoScaleYCmd : public QUndoCommand {
public:
	CartesianPlotSetAutoScaleYCmd(CartesianPlotPrivate* private_obj, bool autoScale) :
		m_private(private_obj), m_autoScale(autoScale), m_autoScaleOld(false), m_minOld(0.0), m_maxOld(0.0) {
		setText(i18n("%1: change y-range auto scaling", m_private->name()));
	};

	void redo() override {
		m_autoScaleOld = m_private->autoScaleY;
		if (m_autoScale) {
			m_minOld = m_private->yMin;
			m_maxOld = m_private->yMax;
			m_private->q->scaleAutoY();
		}
		m_private->autoScaleY = m_autoScale;
		emit m_private->q->yAutoScaleChanged(m_autoScale);
	};

	void undo() override {
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
	double m_minOld;
	double m_maxOld;
};

void CartesianPlot::setAutoScaleY(bool autoScaleY) {
	Q_D(CartesianPlot);
	if (autoScaleY != d->autoScaleY)
		exec(new CartesianPlotSetAutoScaleYCmd(d, autoScaleY));
}

STD_SETTER_CMD_IMPL_F_S(CartesianPlot, SetYMin, double, yMin, retransformScales)
void CartesianPlot::setYMin(double yMin) {
	Q_D(CartesianPlot);
	if (yMin != d->yMin)
		exec(new CartesianPlotSetYMinCmd(d, yMin, ki18n("%1: set min y")));
}

STD_SETTER_CMD_IMPL_F_S(CartesianPlot, SetYMax, double, yMax, retransformScales)
void CartesianPlot::setYMax(double yMax) {
	Q_D(CartesianPlot);
	if (yMax != d->yMax)
		exec(new CartesianPlotSetYMaxCmd(d, yMax, ki18n("%1: set max y")));
}

STD_SETTER_CMD_IMPL_F_S(CartesianPlot, SetYScale, CartesianPlot::Scale, yScale, retransformScales)
void CartesianPlot::setYScale(Scale scale) {
	Q_D(CartesianPlot);
	if (scale != d->yScale)
		exec(new CartesianPlotSetYScaleCmd(d, scale, ki18n("%1: set y scale")));
}

STD_SETTER_CMD_IMPL_F_S(CartesianPlot, SetYRangeBreakingEnabled, bool, yRangeBreakingEnabled, retransformScales)
void CartesianPlot::setYRangeBreakingEnabled(bool enabled) {
	Q_D(CartesianPlot);
	if (enabled != d->yRangeBreakingEnabled)
		exec(new CartesianPlotSetYRangeBreakingEnabledCmd(d, enabled, ki18n("%1: y-range breaking enabled")));
}

STD_SETTER_CMD_IMPL_F_S(CartesianPlot, SetYRangeBreaks, CartesianPlot::RangeBreaks, yRangeBreaks, retransformScales)
void CartesianPlot::setYRangeBreaks(const RangeBreaks& breaks) {
	Q_D(CartesianPlot);
	exec(new CartesianPlotSetYRangeBreaksCmd(d, breaks, ki18n("%1: y-range breaks changed")));
}

STD_SETTER_CMD_IMPL_S(CartesianPlot, SetTheme, QString, theme)
void CartesianPlot::setTheme(const QString& theme) {
	Q_D(CartesianPlot);
	if (theme != d->theme) {
		if (!theme.isEmpty()) {
			beginMacro( i18n("%1: load theme %2", name(), theme) );
			exec(new CartesianPlotSetThemeCmd(d, theme, ki18n("%1: set theme")));
			loadTheme(theme);
			endMacro();
		} else
			exec(new CartesianPlotSetThemeCmd(d, theme, ki18n("%1: disable theming")));
	}
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

void CartesianPlot::addCurve() {
	addChild(new XYCurve("xy-curve"));
}

void CartesianPlot::addEquationCurve() {
	addChild(new XYEquationCurve("f(x)"));
}

void CartesianPlot::addHistogram() {
	addChild(new Histogram("Histogram"));
}

/*!
 * returns the first selected XYCurve in the plot
 */
const XYCurve* CartesianPlot::currentCurve() const {
	for (const auto* curve : this->children<const XYCurve>()) {
		if (curve->graphicsItem()->isSelected())
			return curve;
	}

	return nullptr;
}

void CartesianPlot::addDataReductionCurve() {
	XYDataReductionCurve* curve = new XYDataReductionCurve("Data reduction");
	const XYCurve* curCurve = currentCurve();
	if (curCurve) {
		beginMacro( i18n("%1: reduce '%2'", name(), curCurve->name()) );
		curve->setName( i18n("Reduction of '%1'", curCurve->name()) );
		curve->setDataSourceType(XYAnalysisCurve::DataSourceCurve);
		curve->setDataSourceCurve(curCurve);
		this->addChild(curve);
		curve->recalculate();
		emit curve->dataReductionDataChanged(curve->dataReductionData());
	} else {
		beginMacro(i18n("%1: add data reduction curve", name()));
		this->addChild(curve);
	}

	endMacro();
}

void CartesianPlot::addDifferentiationCurve() {
	XYDifferentiationCurve* curve = new XYDifferentiationCurve("Differentiation");
	const XYCurve* curCurve = currentCurve();
	if (curCurve) {
		beginMacro( i18n("%1: differentiate '%2'", name(), curCurve->name()) );
		curve->setName( i18n("Derivative of '%1'", curCurve->name()) );
		curve->setDataSourceType(XYAnalysisCurve::DataSourceCurve);
		curve->setDataSourceCurve(curCurve);
		this->addChild(curve);
		curve->recalculate();
		emit curve->differentiationDataChanged(curve->differentiationData());
	} else {
		beginMacro(i18n("%1: add differentiation curve", name()));
		this->addChild(curve);
	}

	endMacro();
}

void CartesianPlot::addIntegrationCurve() {
	XYIntegrationCurve* curve = new XYIntegrationCurve("Integration");
	const XYCurve* curCurve = currentCurve();
	if (curCurve) {
		beginMacro( i18n("%1: integrate '%2'", name(), curCurve->name()) );
		curve->setName( i18n("Integral of '%1'", curCurve->name()) );
		curve->setDataSourceType(XYAnalysisCurve::DataSourceCurve);
		curve->setDataSourceCurve(curCurve);
		this->addChild(curve);
		curve->recalculate();
		emit curve->integrationDataChanged(curve->integrationData());
	} else {
		beginMacro(i18n("%1: add integration curve", name()));
		this->addChild(curve);
	}

	endMacro();
}

void CartesianPlot::addInterpolationCurve() {
	XYInterpolationCurve* curve = new XYInterpolationCurve("Interpolation");
	const XYCurve* curCurve = currentCurve();
	if (curCurve) {
		beginMacro( i18n("%1: interpolate '%2'", name(), curCurve->name()) );
		curve->setName( i18n("Interpolation of '%1'", curCurve->name()) );
		curve->setDataSourceType(XYAnalysisCurve::DataSourceCurve);
		curve->setDataSourceCurve(curCurve);
		curve->recalculate();
		this->addChild(curve);
		emit curve->interpolationDataChanged(curve->interpolationData());
	} else {
		beginMacro(i18n("%1: add interpolation curve", name()));
		this->addChild(curve);
	}

	endMacro();
}

void CartesianPlot::addSmoothCurve() {
	XYSmoothCurve* curve = new XYSmoothCurve("Smooth");
	const XYCurve* curCurve = currentCurve();
	if (curCurve) {
		beginMacro( i18n("%1: smooth '%2'", name(), curCurve->name()) );
		curve->setName( i18n("Smoothing of '%1'", curCurve->name()) );
		curve->setDataSourceType(XYAnalysisCurve::DataSourceCurve);
		curve->setDataSourceCurve(curCurve);
		this->addChild(curve);
		curve->recalculate();
		emit curve->smoothDataChanged(curve->smoothData());
	} else {
		beginMacro(i18n("%1: add smoothing curve", name()));
		this->addChild(curve);
	}

	endMacro();
}

void CartesianPlot::addFitCurve() {
	DEBUG("CartesianPlot::addFitCurve()");
	XYFitCurve* curve = new XYFitCurve("fit");
	const XYCurve* curCurve = currentCurve();
	if (curCurve) {
		beginMacro( i18n("%1: fit to '%2'", name(), curCurve->name()) );
		curve->setName( i18n("Fit to '%1'", curCurve->name()) );
		curve->setDataSourceType(XYAnalysisCurve::DataSourceCurve);
		curve->setDataSourceCurve(curCurve);


		//set the fit model category and type
		const auto* action = qobject_cast<const QAction*>(QObject::sender());
		PlotDataDialog::AnalysisAction type = (PlotDataDialog::AnalysisAction)action->data().toInt();
		curve->initFitData(type);
		curve->initStartValues(curCurve);

		//fit with weights for y if the curve has error bars for y
		if (curCurve->yErrorType() == XYCurve::SymmetricError && curCurve->yErrorPlusColumn()) {
			XYFitCurve::FitData fitData = curve->fitData();
			fitData.yWeightsType = nsl_fit_weight_instrumental;
			curve->setFitData(fitData);
			curve->setYErrorColumn(curCurve->yErrorPlusColumn());
		}

		this->addChild(curve);
		curve->recalculate();
		emit curve->fitDataChanged(curve->fitData());
	} else {
		beginMacro(i18n("%1: add fit curve", name()));
		this->addChild(curve);
	}

	endMacro();
}

void CartesianPlot::addFourierFilterCurve() {
	XYFourierFilterCurve* curve = new XYFourierFilterCurve("Fourier filter");
	const XYCurve* curCurve = currentCurve();
	if (curCurve) {
		beginMacro( i18n("%1: Fourier filtering of '%2'", name(), curCurve->name()) );
		curve->setName( i18n("Fourier filtering of '%1'", curCurve->name()) );
		curve->setDataSourceType(XYAnalysisCurve::DataSourceCurve);
		curve->setDataSourceCurve(curCurve);
		this->addChild(curve);
	} else {
		beginMacro(i18n("%1: add Fourier filter curve", name()));
		this->addChild(curve);
	}

	endMacro();
}

void CartesianPlot::addFourierTransformCurve() {
	XYFourierTransformCurve* curve = new XYFourierTransformCurve("Fourier transform");
	this->addChild(curve);
}

void CartesianPlot::addConvolutionCurve() {
	XYConvolutionCurve* curve = new XYConvolutionCurve("Convolution");
	this->addChild(curve);
}

void CartesianPlot::addCorrelationCurve() {
	XYCorrelationCurve* curve = new XYCorrelationCurve("Auto-/Cross-Correlation");
	this->addChild(curve);
}

/*!
 * public helper function to set a legend object created outside of CartesianPlot, e.g. in \c OriginProjectParser.
 */
void CartesianPlot::addLegend(CartesianPlotLegend* legend) {
	m_legend = legend;
	this->addChild(legend);
}

void CartesianPlot::addLegend() {
	//don't do anything if there's already a legend
	if (m_legend)
		return;

	m_legend = new CartesianPlotLegend(this, "legend");
	this->addChild(m_legend);
	m_legend->retransform();

	//only one legend is allowed -> disable the action
	if (m_menusInitialized)
		addLegendAction->setEnabled(false);
}

void CartesianPlot::addTextLabel() {
	TextLabel* label = new TextLabel("text label");
	this->addChild(label);
	label->setParentGraphicsItem(graphicsItem());
}

void CartesianPlot::addCustomPoint() {
	CustomPoint* point = new CustomPoint(this, "custom point");
	this->addChild(point);
}

void CartesianPlot::childAdded(const AbstractAspect* child) {
	Q_D(CartesianPlot);
	const auto* curve = qobject_cast<const XYCurve*>(child);
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

		//in case the first curve is added, check whether we start plotting datetime data
		if (children<XYCurve>().size() == 1) {
			const auto* col = dynamic_cast<const Column*>(curve->xColumn());
			if (col) {
				if (col->columnMode() == AbstractColumn::DateTime) {
					setUndoAware(false);
					setXRangeFormat(CartesianPlot::DateTime);
					setUndoAware(true);

					//set column's datetime format for all horizontal axis
					for (auto* axis : children<Axis>()) {
						if (axis->orientation() == Axis::AxisHorizontal) {
							auto* filter = static_cast<DateTime2StringFilter*>(col->outputFilter());
							d->xRangeDateTimeFormat = filter->format();
							axis->setUndoAware(false);
							axis->setLabelsDateTimeFormat(d->xRangeDateTimeFormat);
							axis->setUndoAware(true);
						}
					}
				}
			}

			col = dynamic_cast<const Column*>(curve->yColumn());
			if (col) {
				if (col->columnMode() == AbstractColumn::DateTime) {
					setUndoAware(false);
					setYRangeFormat(CartesianPlot::DateTime);
					setUndoAware(true);

					//set column's datetime format for all vertical axis
					for (auto* axis : children<Axis>()) {
						if (axis->orientation() == Axis::AxisVertical) {
							auto* filter = static_cast<DateTime2StringFilter*>(col->outputFilter());
							d->yRangeDateTimeFormat = filter->format();
							axis->setUndoAware(false);
							axis->setLabelsDateTimeFormat(d->yRangeDateTimeFormat);
							axis->setUndoAware(true);
						}
					}
				}
			}
		}
	} else {
		const auto* hist = qobject_cast<const Histogram*>(child);
		if (hist) {
			connect(hist, &Histogram::dataChanged, this, &CartesianPlot::dataChanged);
			connect(hist, &Histogram::visibilityChanged, this, &CartesianPlot::curveVisibilityChanged);

			updateLegend();
		}
	}

	if (!isLoading()) {
		//if a theme was selected, apply the theme settings for newly added children, too
		if (!d->theme.isEmpty()) {
			const auto* elem = dynamic_cast<const WorksheetElement*>(child);
			if (elem) {
				KConfig config(ThemeHandler::themeFilePath(d->theme), KConfig::SimpleConfig);
				const_cast<WorksheetElement*>(elem)->loadThemeConfig(config);
			}
		} else {
			//no theme is available, apply the default colors for curves only, s.a. XYCurve::loadThemeConfig()
			const auto* curve = dynamic_cast<const XYCurve*>(child);
			if (curve) {
				int index = indexOfChild<XYCurve>(curve);
				QColor themeColor;
				if (index < m_themeColorPalette.size())
					themeColor = m_themeColorPalette.at(index);
				else {
					if (m_themeColorPalette.size())
						themeColor = m_themeColorPalette.last();
				}

				auto* c = const_cast<XYCurve*>(curve);

				//Line
				QPen p = curve->linePen();
				p.setColor(themeColor);
				c->setLinePen(p);

				//Drop line
				p = curve->dropLinePen();
				p.setColor(themeColor);
				c->setDropLinePen(p);

				//Symbol
				QBrush brush = c->symbolsBrush();
				brush.setColor(themeColor);
				c->setSymbolsBrush(brush);
				p = c->symbolsPen();
				p.setColor(themeColor);
				c->setSymbolsPen(p);

				//Filling
				c->setFillingFirstColor(themeColor);

				//Error bars
				p.setColor(themeColor);
				c->setErrorBarsPen(p);
			}
		}
	}
}

void CartesianPlot::childRemoved(const AbstractAspect* parent, const AbstractAspect* before, const AbstractAspect* child) {
	Q_UNUSED(parent);
	Q_UNUSED(before);
	if (m_legend == child) {
		if (m_menusInitialized)
			addLegendAction->setEnabled(true);
		m_legend = nullptr;
	} else {
		const auto* curve = qobject_cast<const XYCurve*>(child);
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
	d->curvesXMinMaxIsDirty = true;
	d->curvesYMinMaxIsDirty = true;
	bool updated = false;
	if (d->autoScaleX && d->autoScaleY)
		updated = this->scaleAuto();
	else if (d->autoScaleX)
		updated = this->scaleAutoX();
	else if (d->autoScaleY)
		updated = this->scaleAutoY();

	if (!updated || !QObject::sender()) {
		//even if the plot ranges were not changed, either no auto scale active or the new data
		//is within the current ranges and no change of the ranges is required,
		//retransform the curve in order to show the changes
		auto* curve = dynamic_cast<XYCurve*>(QObject::sender());
		if (curve)
			curve->retransform();
		else {
			auto* hist = dynamic_cast<Histogram*>(QObject::sender());
			if (hist)
				hist->retransform();
			else {
				//no sender available, the function was called in CartesianPlot::dataChanged()
				//via plot->dataChaged() in the file filter (live data source got new data)
				//-> retransform all available curves since we don't know which curves are affected.
				//TODO: this logic can be very expensive
				for (auto* c : children<XYCurve>()) {
					c->recalcLogicalPoints();
					c->retransform();
				}
			}
		}
	}
}

/*!
	called when in one of the curves the x-data was changed.
	Autoscales the coordinate system and the x-axes, when "auto-scale" is active.
*/
void CartesianPlot::xDataChanged() {
	if (project() && project()->isLoading())
		return;

	Q_D(CartesianPlot);
	if (d->suppressRetransform)
		return;

	d->curvesXMinMaxIsDirty = true;
	bool updated = false;
	if (d->autoScaleX)
		updated = this->scaleAutoX();

	if (!updated) {
		//even if the plot ranges were not changed, either no auto scale active or the new data
		//is within the current ranges and no change of the ranges is required,
		//retransform the curve in order to show the changes
		auto* curve = dynamic_cast<XYCurve*>(QObject::sender());
		if (curve)
			curve->retransform();
		else {
			auto* hist = dynamic_cast<Histogram*>(QObject::sender());
			if (hist)
				hist->retransform();
		}
	}

	//in case there is only one curve and its column mode was changed, check whether we start plotting datetime data
	if (children<XYCurve>().size() == 1) {
		auto* curve = dynamic_cast<XYCurve*>(QObject::sender());
		const AbstractColumn* col = curve->xColumn();
		if (col->columnMode() == AbstractColumn::DateTime && d->xRangeFormat != CartesianPlot::DateTime) {
			setUndoAware(false);
			setXRangeFormat(CartesianPlot::DateTime);
			setUndoAware(true);
		}
	}
}

/*!
	called when in one of the curves the x-data was changed.
	Autoscales the coordinate system and the x-axes, when "auto-scale" is active.
*/
void CartesianPlot::yDataChanged() {
	if (project() && project()->isLoading())
		return;

	Q_D(CartesianPlot);
	if (d->suppressRetransform)
		return;

	d->curvesYMinMaxIsDirty = true;
	bool updated = false;
	if (d->autoScaleY)
		this->scaleAutoY();

	if (!updated) {
		//even if the plot ranges were not changed, either no auto scale active or the new data
		//is within the current ranges and no change of the ranges is required,
		//retransform the curve in order to show the changes
		auto* curve = dynamic_cast<XYCurve*>(QObject::sender());
		if (curve)
			curve->retransform();
		else {
			auto* hist = dynamic_cast<Histogram*>(QObject::sender());
			if (hist)
				hist->retransform();
		}
	}

	//in case there is only one curve and its column mode was changed, check whether we start plotting datetime data
	if (children<XYCurve>().size() == 1) {
		auto* curve = dynamic_cast<XYCurve*>(QObject::sender());
		const AbstractColumn* col = curve->yColumn();
		if (col->columnMode() == AbstractColumn::DateTime && d->xRangeFormat != CartesianPlot::DateTime) {
			setUndoAware(false);
			setYRangeFormat(CartesianPlot::DateTime);
			setUndoAware(true);
		}
	}
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
		for (auto* item : items)
			item->setFlag(QGraphicsItem::ItemStacksBehindParent, false);
	} else {
		for (auto* item : items)
			item->setFlag(QGraphicsItem::ItemStacksBehindParent, true);
	}

	//when doing zoom selection, prevent the graphics item from being movable
	//if it's currently movable (no worksheet layout available)
	const auto* worksheet = dynamic_cast<const Worksheet*>(parentAspect());
	if (worksheet) {
		if (mouseMode == CartesianPlot::SelectionMode) {
			if (worksheet->layout() != Worksheet::NoLayout)
				graphicsItem()->setFlag(QGraphicsItem::ItemIsMovable, false);
			else
				graphicsItem()->setFlag(QGraphicsItem::ItemIsMovable, true);
		} else   //zoom m_selection
			graphicsItem()->setFlag(QGraphicsItem::ItemIsMovable, false);
	}
}

void CartesianPlot::setLocked(bool locked) {
	Q_D(CartesianPlot);
	d->locked = locked;
}

bool CartesianPlot::scaleAutoX() {
	Q_D(CartesianPlot);
	if (d->curvesXMinMaxIsDirty) {
		int count = 0;
		switch (d->rangeType) {
		case CartesianPlot::RangeFree:
			count = 0;
			break;
		case CartesianPlot::RangeLast:
			count = -d->rangeLastValues;
			break;
		case CartesianPlot::RangeFirst:
			count = d->rangeFirstValues;
			break;
		}

		d->curvesXMin = INFINITY;
		d->curvesXMax = -INFINITY;

		//loop over all xy-curves and determine the maximum and minimum x-values
		for (const auto* curve : this->children<const XYCurve>()) {
			if (!curve->isVisible())
				continue;
			if (!curve->xColumn())
				continue;

			const double min = curve->xColumn()->minimum(count);
			if (min < d->curvesXMin)
				d->curvesXMin = min;

			const double max = curve->xColumn()->maximum(count);
			if (max > d->curvesXMax)
				d->curvesXMax = max;
		}

		//loop over all histograms and determine the maximum and minimum x-values
		for (const auto* curve : this->children<const Histogram>()) {
			if (!curve->isVisible())
				continue;
			if (!curve->dataColumn())
				continue;

			const double min = curve->getXMinimum();
			if (d->curvesXMin > min)
				d->curvesXMin = min;

			const double max = curve->getXMaximum();
			if (max > d->curvesXMax)
				d->curvesXMax = max;
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
			if (d->xMax != 0) {
				d->xMax = d->xMax*1.1;
				d->xMin = d->xMin*0.9;
			} else {
				d->xMax = 0.1;
				d->xMin = -0.1;
			}
		} else {
			double offset = (d->xMax - d->xMin)*d->autoScaleOffsetFactor;
			d->xMin -= offset;
			d->xMax += offset;
		}
		setAutoScaleX(true);
		d->retransformScales();
	}

	return update;
}

bool CartesianPlot::scaleAutoY() {
	Q_D(CartesianPlot);

	if (d->curvesYMinMaxIsDirty) {
		int count = 0;
		switch (d->rangeType) {
		case CartesianPlot::RangeFree:
			count = 0;
			break;
		case CartesianPlot::RangeLast:
			count = -d->rangeLastValues;
			break;
		case CartesianPlot::RangeFirst:
			count = d->rangeFirstValues;
			break;
		}

		d->curvesYMin = INFINITY;
		d->curvesYMax = -INFINITY;

		//loop over all xy-curves and determine the maximum and minimum y-values
		for (const auto* curve : this->children<const XYCurve>()) {
			if (!curve->isVisible())
				continue;
			if (!curve->yColumn())
				continue;

			const double min = curve->yColumn()->minimum(count);
			if (min < d->curvesYMin)
				d->curvesYMin = min;

			const double max = curve->yColumn()->maximum(count);
			if (max > d->curvesYMax)
				d->curvesYMax = max;
		}

		//loop over all histograms and determine the maximum y-value
		for (const auto* curve : this->children<const Histogram>()) {
			if (!curve->isVisible())
				continue;

			const double min = curve->getYMinimum();
			if (d->curvesYMin > min)
				d->curvesYMin = min;

			const double max = curve->getYMaximum();
			if (max > d->curvesYMax)
				d->curvesYMax = max;
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
			if (d->yMax != 0) {
				d->yMax = d->yMax*1.1;
				d->yMin = d->yMin*0.9;
			} else {
				d->yMax = 0.1;
				d->yMin = -0.1;
			}
		} else {
			double offset = (d->yMax - d->yMin)*d->autoScaleOffsetFactor;
			d->yMin -= offset;
			d->yMax += offset;
		}
		setAutoScaleY(true);
		d->retransformScales();
	}

	return update;
}

bool CartesianPlot::scaleAuto() {
	DEBUG("CartesianPlot::scaleAuto()");
	Q_D(CartesianPlot);

	int count = 0;
	switch (d->rangeType) {
	case CartesianPlot::RangeFree:
		count = 0;
		break;
	case CartesianPlot::RangeLast:
		count = -d->rangeLastValues;
		break;
	case CartesianPlot::RangeFirst:
		count = d->rangeFirstValues;
		break;
	}

	if (d->curvesXMinMaxIsDirty) {
		d->curvesXMin = INFINITY;
		d->curvesXMax = -INFINITY;

		//loop over all xy-curves and determine the maximum and minimum x-values
		for (const auto* curve : this->children<const XYCurve>()) {
			if (!curve->isVisible())
				continue;
			if (!curve->xColumn())
				continue;

			const double min = curve->xColumn()->minimum(count);
			if (min < d->curvesXMin)
				d->curvesXMin = min;

			double max = curve->xColumn()->maximum(count);
			if (max > d->curvesXMax)
				d->curvesXMax = max;
		}

		//loop over all histograms and determine the maximum and minimum x-values
		for (const auto* curve : this->children<const Histogram>()) {
			if (!curve->isVisible())
				continue;
			if (!curve->dataColumn())
				continue;

			const double min = curve->getXMinimum();
			if (d->curvesXMin > min)
				d->curvesXMin = min;

			const double max = curve->getXMaximum();
			if (max > d->curvesXMax)
				d->curvesXMax = max;

		}

		d->curvesXMinMaxIsDirty = false;
	}

	if (d->curvesYMinMaxIsDirty) {
		d->curvesYMin = INFINITY;
		d->curvesYMax = -INFINITY;

		//loop over all xy-curves and determine the maximum and minimum y-values
		for (const auto* curve : this->children<const XYCurve>()) {
			if (!curve->isVisible())
				continue;
			if (!curve->yColumn())
				continue;

			const double min = curve->yColumn()->minimum(count);
			if (min < d->curvesYMin)
				d->curvesYMin = min;

			const double max = curve->yColumn()->maximum(count);
			if (max > d->curvesYMax)
				d->curvesYMax = max;
		}

		//loop over all histograms and determine the maximum y-value
		for (const auto* curve : this->children<const Histogram>()) {
			if (!curve->isVisible())
				continue;

			const double min = curve->getYMinimum();
			if (d->curvesYMin > min)
				d->curvesYMin = min;

			const double max = curve->getYMaximum();
			if (max > d->curvesYMax)
				d->curvesYMax = max;
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
	DEBUG(" xmin/xmax = " << d->xMin << '/' << d->xMax << ", ymin/ymax = " << d->yMin << '/' << d->yMax);

	if (updateX || updateY) {
		if (updateX) {
			if (d->xMax == d->xMin) {
				//in case min and max are equal (e.g. if we plot a single point), subtract/add 10% of the value
				if (d->xMax != 0) {
					d->xMax = d->xMax*1.1;
					d->xMin = d->xMin*0.9;
				} else {
					d->xMax = 0.1;
					d->xMin = -0.1;
				}
			} else {
				double offset = (d->xMax - d->xMin)*d->autoScaleOffsetFactor;
				d->xMin -= offset;
				d->xMax += offset;
			}
			setAutoScaleX(true);
		}
		if (updateY) {
			if (d->yMax == d->yMin) {
				//in case min and max are equal (e.g. if we plot a single point), subtract/add 10% of the value
				if (d->yMax != 0) {
					d->yMax = d->yMax*1.1;
					d->yMin = d->yMin*0.9;
				} else {
					d->yMax = 0.1;
					d->yMin = -0.1;
				}
			} else {
				double offset = (d->yMax - d->yMin)*d->autoScaleOffsetFactor;
				d->yMin -= offset;
				d->yMax += offset;
			}
			setAutoScaleY(true);
		}
		d->retransformScales();
	}

	return (updateX || updateY);
}

void CartesianPlot::zoomIn() {
	Q_D(CartesianPlot);

	setUndoAware(false);
	setAutoScaleX(false);
	setAutoScaleY(false);
	setUndoAware(true);
	double oldRange = (d->xMax - d->xMin);
	double newRange = (d->xMax - d->xMin) / m_zoomFactor;
	d->xMax = d->xMax + (newRange - oldRange) / 2;
	d->xMin = d->xMin - (newRange - oldRange) / 2;

	oldRange = (d->yMax - d->yMin);
	newRange = (d->yMax - d->yMin) / m_zoomFactor;
	d->yMax = d->yMax + (newRange - oldRange) / 2;
	d->yMin = d->yMin - (newRange - oldRange) / 2;

	d->retransformScales();
}

void CartesianPlot::zoomOut() {
	Q_D(CartesianPlot);

	setUndoAware(false);
	setAutoScaleX(false);
	setAutoScaleY(false);
	setUndoAware(true);
	double oldRange = (d->xMax-d->xMin);
	double newRange = (d->xMax-d->xMin)*m_zoomFactor;
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

	setAutoScaleX(false);
	double oldRange = (d->xMax-d->xMin);
	double newRange = (d->xMax-d->xMin)/m_zoomFactor;
	d->xMax = d->xMax + (newRange-oldRange)/2;
	d->xMin = d->xMin - (newRange-oldRange)/2;
	d->retransformScales();
}

void CartesianPlot::zoomOutX() {
	Q_D(CartesianPlot);

	setUndoAware(false);
	setAutoScaleX(false);
	setUndoAware(true);
	double oldRange = (d->xMax-d->xMin);
	double newRange = (d->xMax-d->xMin)*m_zoomFactor;
	d->xMax = d->xMax + (newRange-oldRange)/2;
	d->xMin = d->xMin - (newRange-oldRange)/2;
	d->retransformScales();
}

void CartesianPlot::zoomInY() {
	Q_D(CartesianPlot);

	setUndoAware(false);
	setAutoScaleY(false);
	setUndoAware(true);
	double oldRange = (d->yMax-d->yMin);
	double newRange = (d->yMax-d->yMin)/m_zoomFactor;
	d->yMax = d->yMax + (newRange-oldRange)/2;
	d->yMin = d->yMin - (newRange-oldRange)/2;
	d->retransformScales();
}

void CartesianPlot::zoomOutY() {
	Q_D(CartesianPlot);

	setUndoAware(false);
	setAutoScaleY(false);
	setUndoAware(true);
	double oldRange = (d->yMax-d->yMin);
	double newRange = (d->yMax-d->yMin)*m_zoomFactor;
	d->yMax = d->yMax + (newRange-oldRange)/2;
	d->yMin = d->yMin - (newRange-oldRange)/2;
	d->retransformScales();
}

void CartesianPlot::shiftLeftX() {
	Q_D(CartesianPlot);

	setUndoAware(false);
	setAutoScaleX(false);
	setUndoAware(true);
	double offsetX = (d->xMax-d->xMin)*0.1;
	d->xMax -= offsetX;
	d->xMin -= offsetX;
	d->retransformScales();
}

void CartesianPlot::shiftRightX() {
	Q_D(CartesianPlot);

	setUndoAware(false);
	setAutoScaleX(false);
	setUndoAware(true);
	double offsetX = (d->xMax-d->xMin)*0.1;
	d->xMax += offsetX;
	d->xMin += offsetX;
	d->retransformScales();
}

void CartesianPlot::shiftUpY() {
	Q_D(CartesianPlot);

	setUndoAware(false);
	setAutoScaleY(false);
	setUndoAware(true);
	double offsetY = (d->yMax-d->yMin)*0.1;
	d->yMax += offsetY;
	d->yMin += offsetY;
	d->retransformScales();
}

void CartesianPlot::shiftDownY() {
	Q_D(CartesianPlot);

	setUndoAware(false);
	setAutoScaleY(false);
	setUndoAware(true);
	double offsetY = (d->yMax-d->yMin)*0.1;
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
CartesianPlotPrivate::CartesianPlotPrivate(CartesianPlot* plot) : AbstractPlotPrivate(plot), q(plot) {
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

	PERFTRACE("CartesianPlotPrivate::retransform()");
	prepareGeometryChange();
	setPos( rect.x()+rect.width()/2, rect.y()+rect.height()/2);

	updateDataRect();
	retransformScales();

	//plotArea position is always (0, 0) in parent's coordinates, don't need to update here
	q->plotArea()->setRect(rect);

	//call retransform() for the title and the legend (if available)
	//when a predefined position relative to the (Left, Centered etc.) is used,
	//the actual position needs to be updated on plot's geometry changes.
	if (q->title())
		q->title()->retransform();
	if (q->m_legend)
		q->m_legend->retransform();

	WorksheetElementContainerPrivate::recalcShapeAndBoundingRect();
}

void CartesianPlotPrivate::retransformScales() {
	DEBUG("CartesianPlotPrivate::retransformScales()");
	DEBUG(" xmin/xmax = " << xMin << '/'<< xMax << ", ymin/ymax = " << yMin << '/' << yMax);
	PERFTRACE("CartesianPlotPrivate::retransformScales()");

	auto* plot = dynamic_cast<CartesianPlot*>(q);
	QVector<CartesianScale*> scales;

	//perform the mapping from the scene coordinates to the plot's coordinates here.
	QRectF itemRect = mapRectFromScene(rect);

	//check ranges for log-scales
	if (xScale != CartesianPlot::ScaleLinear)
		checkXRange();

	//check whether we have x-range breaks - the first break, if available, should be valid
	bool hasValidBreak = (xRangeBreakingEnabled && !xRangeBreaks.list.isEmpty() && xRangeBreaks.list.first().isValid());

	static const int breakGap = 20;
	double sceneStart, sceneEnd, logicalStart, logicalEnd;

	//create x-scales
	int plotSceneStart = itemRect.x() + horizontalPadding;
	int plotSceneEnd = itemRect.x() + itemRect.width() - horizontalPadding;
	if (!hasValidBreak) {
		//no breaks available -> range goes from the plot beginning to the end of the plot
		sceneStart = plotSceneStart;
		sceneEnd = plotSceneEnd;
		logicalStart = xMin;
		logicalEnd = xMax;

		//TODO: how should we handle the case sceneStart == sceneEnd?
		//(to reproduce, create plots and adjust the spacing/pading to get zero size for the plots)
		if (sceneStart != sceneEnd)
			scales << this->createScale(xScale, sceneStart, sceneEnd, logicalStart, logicalEnd);
	} else {
		int sceneEndLast = plotSceneStart;
		int logicalEndLast = xMin;
		for (const auto& rb : xRangeBreaks.list) {
			if (!rb.isValid())
				break;

			//current range goes from the end of the previous one (or from the plot beginning) to curBreak.start
			sceneStart = sceneEndLast;
			if (&rb == &xRangeBreaks.list.first()) sceneStart += breakGap;
			sceneEnd = plotSceneStart + (plotSceneEnd-plotSceneStart) * rb.position;
			logicalStart = logicalEndLast;
			logicalEnd = rb.start;

			if (sceneStart != sceneEnd)
				scales << this->createScale(xScale, sceneStart, sceneEnd, logicalStart, logicalEnd);

			sceneEndLast = sceneEnd;
			logicalEndLast = rb.end;
		}

		//add the remaining range going from the last available range break to the end of the plot (=end of the x-data range)
		sceneStart = sceneEndLast+breakGap;
		sceneEnd = plotSceneEnd;
		logicalStart = logicalEndLast;
		logicalEnd = xMax;

		if (sceneStart != sceneEnd)
			scales << this->createScale(xScale, sceneStart, sceneEnd, logicalStart, logicalEnd);
	}

	cSystem->setXScales(scales);

	//check ranges for log-scales
	if (yScale != CartesianPlot::ScaleLinear)
		checkYRange();

	//check whether we have y-range breaks - the first break, if available, should be valid
	hasValidBreak = (yRangeBreakingEnabled && !yRangeBreaks.list.isEmpty() && yRangeBreaks.list.first().isValid());

	//create y-scales
	scales.clear();
	plotSceneStart = itemRect.y()+itemRect.height()-verticalPadding;
	plotSceneEnd = itemRect.y()+verticalPadding;
	if (!hasValidBreak) {
		//no breaks available -> range goes from the plot beginning to the end of the plot
		sceneStart = plotSceneStart;
		sceneEnd = plotSceneEnd;
		logicalStart = yMin;
		logicalEnd = yMax;

		if (sceneStart != sceneEnd)
			scales << this->createScale(yScale, sceneStart, sceneEnd, logicalStart, logicalEnd);
	} else {
		int sceneEndLast = plotSceneStart;
		int logicalEndLast = yMin;
		for (const auto& rb : yRangeBreaks.list) {
			if (!rb.isValid())
				break;

			//current range goes from the end of the previous one (or from the plot beginning) to curBreak.start
			sceneStart = sceneEndLast;
			if (&rb == &yRangeBreaks.list.first()) sceneStart -= breakGap;
			sceneEnd = plotSceneStart + (plotSceneEnd-plotSceneStart) * rb.position;
			logicalStart = logicalEndLast;
			logicalEnd = rb.start;

			if (sceneStart != sceneEnd)
				scales << this->createScale(yScale, sceneStart, sceneEnd, logicalStart, logicalEnd);

			sceneEndLast = sceneEnd;
			logicalEndLast = rb.end;
		}

		//add the remaining range going from the last available range break to the end of the plot (=end of the y-data range)
		sceneStart = sceneEndLast-breakGap;
		sceneEnd = plotSceneEnd;
		logicalStart = logicalEndLast;
		logicalEnd = yMax;

		if (sceneStart != sceneEnd)
			scales << this->createScale(yScale, sceneStart, sceneEnd, logicalStart, logicalEnd);
	}

	cSystem->setYScales(scales);

	//calculate the changes in x and y and save the current values for xMin, xMax, yMin, yMax
	double deltaXMin = 0;
	double deltaXMax = 0;
	double deltaYMin = 0;
	double deltaYMax = 0;

	if (xMin != xMinPrev) {
		deltaXMin = xMin - xMinPrev;
		emit plot->xMinChanged(xMin);
	}

	if (xMax != xMaxPrev) {
		deltaXMax = xMax - xMaxPrev;
		emit plot->xMaxChanged(xMax);
	}

	if (yMin != yMinPrev) {
		deltaYMin = yMin - yMinPrev;
		emit plot->yMinChanged(yMin);
	}

	if (yMax != yMaxPrev) {
		deltaYMax = yMax - yMaxPrev;
		emit plot->yMaxChanged(yMax);
	}

	xMinPrev = xMin;
	xMaxPrev = xMax;
	yMinPrev = yMin;
	yMaxPrev = yMax;
	//adjust auto-scale axes
	for (auto* axis : q->children<Axis>()) {
		if (!axis->autoScale())
			continue;

		if (axis->orientation() == Axis::AxisHorizontal) {
			if (deltaXMax != 0) {
				axis->setUndoAware(false);
				axis->setSuppressRetransform(true);
				axis->setEnd(xMax);
				axis->setUndoAware(true);
				axis->setSuppressRetransform(false);
			}
			if (deltaXMin != 0) {
				axis->setUndoAware(false);
				axis->setSuppressRetransform(true);
				axis->setStart(xMin);
				axis->setUndoAware(true);
				axis->setSuppressRetransform(false);
			}
			//TODO;
// 			if (axis->position() == Axis::AxisCustom && deltaYMin != 0) {
// 				axis->setOffset(axis->offset() + deltaYMin, false);
// 			}
		} else {
			if (deltaYMax != 0) {
				axis->setUndoAware(false);
				axis->setSuppressRetransform(true);
				axis->setEnd(yMax);
				axis->setUndoAware(true);
				axis->setSuppressRetransform(false);
			}
			if (deltaYMin != 0) {
				axis->setUndoAware(false);
				axis->setSuppressRetransform(true);
				axis->setStart(yMin);
				axis->setUndoAware(true);
				axis->setSuppressRetransform(false);
			}

			//TODO;
// 			if (axis->position() == Axis::AxisCustom && deltaXMin != 0) {
// 				axis->setOffset(axis->offset() + deltaXMin, false);
// 			}
		}
	}
	// call retransform() on the parent to trigger the update of all axes and curvesю
	//no need to do this on load since all plots are retransformed again after the project is loaded.
	if (!q->isLoading())
		q->retransform();
}

/*
 * calculates the rectangular of the are showing the actual data (plot's rect minus padding),
 * in plot's coordinates.
 */
void CartesianPlotPrivate::updateDataRect() {
	dataRect = mapRectFromScene(rect);
	dataRect.setX(dataRect.x() + horizontalPadding);
	dataRect.setY(dataRect.y() + verticalPadding);
	dataRect.setWidth(dataRect.width() - horizontalPadding);
	dataRect.setHeight(dataRect.height() - verticalPadding);
}

void CartesianPlotPrivate::rangeChanged() {
	curvesXMinMaxIsDirty = true;
	curvesYMinMaxIsDirty = true;
	if (autoScaleX && autoScaleY)
		q->scaleAuto();
	else if (autoScaleX)
		q->scaleAutoX();
	else if (autoScaleY)
		q->scaleAutoY();
}

void CartesianPlotPrivate::xRangeFormatChanged() {
	for (auto* axis : q->children<Axis>()) {
		if (axis->orientation() == Axis::AxisHorizontal)
			axis->retransformTickLabelStrings();
	}
}

void CartesianPlotPrivate::yRangeFormatChanged() {
	for (auto* axis : q->children<Axis>()) {
		if (axis->orientation() == Axis::AxisVertical)
			axis->retransformTickLabelStrings();
	}
}

/*!
 * don't allow any negative values for the x range when log or sqrt scalings are used
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
 * don't allow any negative values for the y range when log or sqrt scalings are used
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

CartesianScale* CartesianPlotPrivate::createScale(CartesianPlot::Scale type, double sceneStart, double sceneEnd, double logicalStart, double logicalEnd) {
	DEBUG("CartesianPlotPrivate::createScale() scene start/end = " << sceneStart << '/' << sceneEnd << ", logical start/end = " << logicalStart << '/' << logicalEnd);
// 	Interval<double> interval (logicalStart-0.01, logicalEnd+0.01); //TODO: move this to CartesianScale
	Interval<double> interval (std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max());
// 	Interval<double> interval (logicalStart, logicalEnd);
	if (type == CartesianPlot::ScaleLinear)
		return CartesianScale::createLinearScale(interval, sceneStart, sceneEnd, logicalStart, logicalEnd);
	else
		return CartesianScale::createLogScale(interval, sceneStart, sceneEnd, logicalStart, logicalEnd, type);
}

/*!
 * Reimplemented from QGraphicsItem.
 */
QVariant CartesianPlotPrivate::itemChange(GraphicsItemChange change, const QVariant &value) {
	if (change == QGraphicsItem::ItemPositionChange) {
		const QPointF& itemPos = value.toPointF();//item's center point in parent's coordinates;
		const qreal x = itemPos.x();
		const qreal y = itemPos.y();

		//calculate the new rect and forward the changes to the frontend
		QRectF newRect;
		const qreal w = rect.width();
		const qreal h = rect.height();
		newRect.setX(x-w/2);
		newRect.setY(y-h/2);
		newRect.setWidth(w);
		newRect.setHeight(h);
		emit q->rectChanged(newRect);
	}
	return QGraphicsItem::itemChange(change, value);
}

//##############################################################################
//##################################  Events  ##################################
//##############################################################################
void CartesianPlotPrivate::mousePressEvent(QGraphicsSceneMouseEvent *event) {
	if (mouseMode == CartesianPlot::ZoomSelectionMode || mouseMode == CartesianPlot::ZoomXSelectionMode || mouseMode == CartesianPlot::ZoomYSelectionMode) {

		if (mouseMode == CartesianPlot::ZoomSelectionMode)
			m_selectionStart = event->pos();
		else if (mouseMode == CartesianPlot::ZoomXSelectionMode) {
			m_selectionStart.setX(event->pos().x());
			m_selectionStart.setY(dataRect.height()/2);
		} else if (mouseMode == CartesianPlot::ZoomYSelectionMode) {
			m_selectionStart.setX(-dataRect.width()/2);
			m_selectionStart.setY(event->pos().y());
		}

		m_selectionEnd = m_selectionStart;
		m_selectionBandIsShown = true;
	} else {
		if (!locked && dataRect.contains(event->pos()) ){
			panningStarted = true;
			m_panningStart = event->pos();
			setCursor(Qt::ClosedHandCursor);
		}
		QGraphicsItem::mousePressEvent(event);
	}
}

void CartesianPlotPrivate::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
	if (mouseMode == CartesianPlot::SelectionMode) {
		if (panningStarted && dataRect.contains(event->pos()) ) {
			//don't retransform on small mouse movement deltas
			const int deltaXScene = (m_panningStart.x() - event->pos().x());
			const int deltaYScene = (m_panningStart.y() - event->pos().y());
			if (abs(deltaXScene) < 5 && abs(deltaYScene) < 5)
				return;

			const QPointF logicalEnd = cSystem->mapSceneToLogical(event->pos());
			const QPointF logicalStart = cSystem->mapSceneToLogical(m_panningStart);
			const float deltaX = (logicalStart.x() - logicalEnd.x());
			const float deltaY = (logicalStart.y() - logicalEnd.y());
			xMax += deltaX;
			xMin += deltaX;
			yMax += deltaY;
			yMin += deltaY;
			retransformScales();
			m_panningStart = event->pos();
		} else
			QGraphicsItem::mouseMoveEvent(event);
	} else if (mouseMode == CartesianPlot::ZoomSelectionMode || mouseMode == CartesianPlot::ZoomXSelectionMode || mouseMode == CartesianPlot::ZoomYSelectionMode) {
		QGraphicsItem::mouseMoveEvent(event);
		if ( !boundingRect().contains(event->pos()) ) {
			q->info(QString());
			return;
		}

		QString info;
		QPointF logicalStart = cSystem->mapSceneToLogical(m_selectionStart);
		if (mouseMode == CartesianPlot::ZoomSelectionMode) {
			m_selectionEnd = event->pos();
			QPointF logicalEnd = cSystem->mapSceneToLogical(m_selectionEnd);
			if (xRangeFormat == CartesianPlot::Numeric)
				 info = QString::fromUtf8("Δx=") + QString::number(logicalEnd.x()-logicalStart.x());
			else
				info = i18n("from x=%1 to x=%2", QDateTime::fromMSecsSinceEpoch(logicalStart.x()).toString(xRangeDateTimeFormat),
													QDateTime::fromMSecsSinceEpoch(logicalEnd.x()).toString(xRangeDateTimeFormat));

			info += QLatin1String(", ");
			if (yRangeFormat == CartesianPlot::Numeric)
				 info += QString::fromUtf8("Δy=") + QString::number(logicalEnd.y()-logicalStart.y());
			else
				info += i18n("from y=%1 to y=%2", QDateTime::fromMSecsSinceEpoch(logicalStart.y()).toString(xRangeDateTimeFormat),
													QDateTime::fromMSecsSinceEpoch(logicalEnd.y()).toString(xRangeDateTimeFormat));
		} else if (mouseMode == CartesianPlot::ZoomXSelectionMode) {
			m_selectionEnd.setX(event->pos().x());
			m_selectionEnd.setY(-dataRect.height()/2);
			QPointF logicalEnd = cSystem->mapSceneToLogical(m_selectionEnd);
			if (xRangeFormat == CartesianPlot::Numeric)
				 info = QString::fromUtf8("Δx=") + QString::number(logicalEnd.x()-logicalStart.x());
			else
				info = i18n("from x=%1 to x=%2", QDateTime::fromMSecsSinceEpoch(logicalStart.x()).toString(xRangeDateTimeFormat),
													QDateTime::fromMSecsSinceEpoch(logicalEnd.x()).toString(xRangeDateTimeFormat));
		} else if (mouseMode == CartesianPlot::ZoomYSelectionMode) {
			m_selectionEnd.setX(dataRect.width()/2);
			m_selectionEnd.setY(event->pos().y());
			QPointF logicalEnd = cSystem->mapSceneToLogical(m_selectionEnd);
			if (yRangeFormat == CartesianPlot::Numeric)
				 info = QString::fromUtf8("Δy=") + QString::number(logicalEnd.y()-logicalStart.y());
			else
				info = i18n("from y=%1 to y=%2", QDateTime::fromMSecsSinceEpoch(logicalStart.y()).toString(xRangeDateTimeFormat),
													QDateTime::fromMSecsSinceEpoch(logicalEnd.y()).toString(xRangeDateTimeFormat));
		}
		q->info(info);
		update();
	}
}

void CartesianPlotPrivate::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
	setCursor(Qt::ArrowCursor);
	if (mouseMode == CartesianPlot::SelectionMode) {
		panningStarted = false;

		//TODO: why do we do this all the time?!?!
		const QPointF& itemPos = pos();//item's center point in parent's coordinates;
		const qreal x = itemPos.x();
		const qreal y = itemPos.y();

		//calculate the new rect and set it
		QRectF newRect;
		const qreal w = rect.width();
		const qreal h = rect.height();
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
		if ( qAbs(m_selectionEnd.x()-m_selectionStart.x()) < 20 || qAbs(m_selectionEnd.y()-m_selectionStart.y()) < 20 ) {
			m_selectionBandIsShown = false;
			return;
		}

		//determine the new plot ranges
		QPointF logicalZoomStart = cSystem->mapSceneToLogical(m_selectionStart, AbstractCoordinateSystem::SuppressPageClipping);
		QPointF logicalZoomEnd = cSystem->mapSceneToLogical(m_selectionEnd, AbstractCoordinateSystem::SuppressPageClipping);
		if (m_selectionEnd.x() > m_selectionStart.x()) {
			xMin = logicalZoomStart.x();
			xMax = logicalZoomEnd.x();
		} else {
			xMin = logicalZoomEnd.x();
			xMax = logicalZoomStart.x();
		}

		if (m_selectionEnd.y() > m_selectionStart.y()) {
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
	if (locked)
		return;

	//determine first, which axes are selected and zoom only in the corresponding direction.
	//zoom the entire plot if no axes selected.
	bool zoomX = false;
	bool zoomY = false;
	for (auto* axis : q->children<Axis>()) {
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
	if (dataRect.contains(point)) {
		QPointF logicalPoint = cSystem->mapSceneToLogical(point);
		if (mouseMode == CartesianPlot::ZoomSelectionMode && !m_selectionBandIsShown) {
			info = "x=";
			if (xRangeFormat == CartesianPlot::Numeric)
				 info += QString::number(logicalPoint.x());
			else
				info += QDateTime::fromMSecsSinceEpoch(logicalPoint.x()).toString(xRangeDateTimeFormat);

			info += ", y=";
			if (yRangeFormat == CartesianPlot::Numeric)
				info += QString::number(logicalPoint.y());
			else
				info += QDateTime::fromMSecsSinceEpoch(logicalPoint.y()).toString(yRangeDateTimeFormat);
		} else if (mouseMode == CartesianPlot::ZoomXSelectionMode && !m_selectionBandIsShown) {
			QPointF p1(logicalPoint.x(), yMin);
			QPointF p2(logicalPoint.x(), yMax);
			m_selectionStartLine.setP1(cSystem->mapLogicalToScene(p1));
			m_selectionStartLine.setP2(cSystem->mapLogicalToScene(p2));
			info = "x=";
			if (xRangeFormat == CartesianPlot::Numeric)
				 info += QString::number(logicalPoint.x());
			else
				info += QDateTime::fromMSecsSinceEpoch(logicalPoint.x()).toString(xRangeDateTimeFormat);
			update();
		} else if (mouseMode == CartesianPlot::ZoomYSelectionMode && !m_selectionBandIsShown) {
			QPointF p1(xMin, logicalPoint.y());
			QPointF p2(xMax, logicalPoint.y());
			m_selectionStartLine.setP1(cSystem->mapLogicalToScene(p1));
			m_selectionStartLine.setP2(cSystem->mapLogicalToScene(p2));
			info = "y=";
			if (yRangeFormat == CartesianPlot::Numeric)
				info += QString::number(logicalPoint.y());
			else
				info += QDateTime::fromMSecsSinceEpoch(logicalPoint.y()).toString(yRangeDateTimeFormat);
			update();
        } else if (mouseMode == CartesianPlot::MouseMode::SelectionMode) {
			// hover the nearest curve to the mousepointer
			bool curve_hovered = false;
			QVector<XYCurve*> curves = q->children<XYCurve>();
			for (int i=curves.count() - 1; i >= 0; i--){ // because the last curve is above the other curves
				if(curve_hovered){ // if a curve is already hovered, disable hover for the rest
					curves[i]->setHover(false);
					continue;
				}
				if(curves[i]->activateCurve(event->pos())){
					curves[i]->setHover(true);
					curve_hovered = true;
					continue;
				}
				curves[i]->setHover(false);
			}
		}
	}
	q->info(info);

	QGraphicsItem::hoverMoveEvent(event);
}

void CartesianPlotPrivate::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
	Q_UNUSED(option)
	Q_UNUSED(widget)

	if (!isVisible())
		return;

	painter->setPen(QPen(Qt::black, 3));
	if ((mouseMode == CartesianPlot::ZoomXSelectionMode || mouseMode == CartesianPlot::ZoomYSelectionMode)
	        && (!m_selectionBandIsShown))
		painter->drawLine(m_selectionStartLine);

	if (m_selectionBandIsShown) {
		painter->save();
		painter->setPen(QPen(Qt::black, 5));
		painter->drawRect(QRectF(m_selectionStart, m_selectionEnd));
		painter->setBrush(Qt::blue);
		painter->setOpacity(0.2);
		painter->drawRect(QRectF(m_selectionStart, m_selectionEnd));
		painter->restore();
	}

	float penWidth = 6.;
	QRectF rect = q->m_plotArea->graphicsItem()->boundingRect();
	rect = QRectF(-rect.width()/2 - penWidth / 2, -rect.height()/2 - penWidth / 2,
				  rect.width() + penWidth, rect.height() + penWidth);

	if (m_hovered && !isSelected() && !m_printing) {
		painter->setPen(QPen(QApplication::palette().color(QPalette::Shadow), penWidth, Qt::SolidLine));
		painter->drawRect(rect);
	}

	if (isSelected() && !m_printing) {
		painter->setPen(QPen(QApplication::palette().color(QPalette::Highlight), penWidth, Qt::SolidLine));
		painter->drawRect(rect);
	}
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

	//applied theme
	if (!d->theme.isEmpty()) {
		writer->writeStartElement( "theme" );
		writer->writeAttribute("name", d->theme);
		writer->writeEndElement();
	}

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
	writer->writeAttribute( "xMin", QString::number(d->xMin, 'g', 16));
	writer->writeAttribute( "xMax", QString::number(d->xMax, 'g', 16) );
	writer->writeAttribute( "yMin", QString::number(d->yMin, 'g', 16) );
	writer->writeAttribute( "yMax", QString::number(d->yMax, 'g', 16) );
	writer->writeAttribute( "xScale", QString::number(d->xScale) );
	writer->writeAttribute( "yScale", QString::number(d->yScale) );
	writer->writeAttribute( "xRangeFormat", QString::number(d->xRangeFormat) );
	writer->writeAttribute( "yRangeFormat", QString::number(d->yRangeFormat) );
	writer->writeAttribute( "horizontalPadding", QString::number(d->horizontalPadding) );
	writer->writeAttribute( "verticalPadding", QString::number(d->verticalPadding) );
	writer->writeEndElement();

	//x-scale breaks
	if (d->xRangeBreakingEnabled || !d->xRangeBreaks.list.isEmpty()) {
		writer->writeStartElement("xRangeBreaks");
		writer->writeAttribute( "enabled", QString::number(d->xRangeBreakingEnabled) );
		for (const auto& rb : d->xRangeBreaks.list) {
			writer->writeStartElement("xRangeBreak");
			writer->writeAttribute("start", QString::number(rb.start));
			writer->writeAttribute("end", QString::number(rb.end));
			writer->writeAttribute("position", QString::number(rb.position));
			writer->writeAttribute("style", QString::number(rb.style));
			writer->writeEndElement();
		}
		writer->writeEndElement();
	}

	//y-scale breaks
	if (d->yRangeBreakingEnabled || !d->yRangeBreaks.list.isEmpty()) {
		writer->writeStartElement("yRangeBreaks");
		writer->writeAttribute( "enabled", QString::number(d->yRangeBreakingEnabled) );
		for (const auto& rb : d->yRangeBreaks.list) {
			writer->writeStartElement("yRangeBreak");
			writer->writeAttribute("start", QString::number(rb.start));
			writer->writeAttribute("end", QString::number(rb.end));
			writer->writeAttribute("position", QString::number(rb.position));
			writer->writeAttribute("style", QString::number(rb.style));
			writer->writeEndElement();
		}
		writer->writeEndElement();
	}

	//serialize all children (plot area, title text label, axes and curves)
	for (auto* elem : children<WorksheetElement>(IncludeHidden))
		elem->save(writer);

	writer->writeEndElement(); // close "cartesianPlot" section
}


//! Load from XML
bool CartesianPlot::load(XmlStreamReader* reader, bool preview) {
	Q_D(CartesianPlot);

	if (!readBasicAttributes(reader))
		return false;

	KLocalizedString attributeWarning = ki18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs;
	QString str;
	bool titleLabelRead = false;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == "cartesianPlot")
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == "comment") {
			if (!readCommentElement(reader))
				return false;
		} else if (!preview && reader->name() == "theme") {
			attribs = reader->attributes();
			d->theme = attribs.value("name").toString();
		} else if (!preview && reader->name() == "geometry") {
			attribs = reader->attributes();

			str = attribs.value("x").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("x").toString());
			else
				d->rect.setX( str.toDouble() );

			str = attribs.value("y").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("y").toString());
			else
				d->rect.setY( str.toDouble() );

			str = attribs.value("width").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("width").toString());
			else
				d->rect.setWidth( str.toDouble() );

			str = attribs.value("height").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("height").toString());
			else
				d->rect.setHeight( str.toDouble() );

			str = attribs.value("visible").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("visible").toString());
			else
				d->setVisible(str.toInt());
		} else if (!preview && reader->name() == "coordinateSystem") {
			attribs = reader->attributes();

			READ_INT_VALUE("autoScaleX", autoScaleX, bool);
			READ_INT_VALUE("autoScaleY", autoScaleY, bool);

			str = attribs.value("xMin").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("xMin").toString());
			else {
				d->xMin = str.toDouble();
				d->xMinPrev = d->xMin;
			}

			str = attribs.value("xMax").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("xMax").toString());
			else {
				d->xMax = str.toDouble();
				d->xMaxPrev = d->xMax;
			}

			str = attribs.value("yMin").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("yMin").toString());
			else {
				d->yMin = str.toDouble();
				d->yMinPrev = d->yMin;
			}

			str = attribs.value("yMax").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("yMax").toString());
			else {
				d->yMax = str.toDouble();
				d->yMaxPrev = d->yMax;
			}

			READ_INT_VALUE("xScale", xScale, CartesianPlot::Scale);
			READ_INT_VALUE("yScale", yScale, CartesianPlot::Scale);

			READ_INT_VALUE("xRangeFormat", xRangeFormat, CartesianPlot::RangeFormat);
			READ_INT_VALUE("yRangeFormat", yRangeFormat, CartesianPlot::RangeFormat);

			READ_DOUBLE_VALUE("horizontalPadding", horizontalPadding);
			READ_DOUBLE_VALUE("verticalPadding", verticalPadding);
		} else if (!preview && reader->name() == "xRangeBreaks") {
			//delete default rang break
			d->xRangeBreaks.list.clear();

			attribs = reader->attributes();
			READ_INT_VALUE("enabled", xRangeBreakingEnabled, bool);
		} else if (!preview && reader->name() == "xRangeBreak") {
			attribs = reader->attributes();

			RangeBreak b;
			str = attribs.value("start").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("start").toString());
			else
				b.start = str.toDouble();

			str = attribs.value("end").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("end").toString());
			else
				b.end = str.toDouble();

			str = attribs.value("position").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("position").toString());
			else
				b.position = str.toDouble();

			str = attribs.value("style").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("style").toString());
			else
				b.style = CartesianPlot::RangeBreakStyle(str.toInt());

			d->xRangeBreaks.list << b;
		} else if (!preview && reader->name() == "yRangeBreaks") {
			//delete default rang break
			d->yRangeBreaks.list.clear();

			attribs = reader->attributes();
			READ_INT_VALUE("enabled", yRangeBreakingEnabled, bool);
		} else if (!preview && reader->name() == "yRangeBreak") {
			attribs = reader->attributes();

			RangeBreak b;
			str = attribs.value("start").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("start").toString());
			else
				b.start = str.toDouble();

			str = attribs.value("end").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("end").toString());
			else
				b.end = str.toDouble();

			str = attribs.value("position").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("position").toString());
			else
				b.position = str.toDouble();

			str = attribs.value("style").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("style").toString());
			else
				b.style = CartesianPlot::RangeBreakStyle(str.toInt());

			d->yRangeBreaks.list << b;
		} else if (reader->name() == "textLabel") {
			if (!titleLabelRead) {
				//the first text label is always the title label
				m_title->load(reader, preview);
				titleLabelRead = true;

				//TODO: the name is read in m_title->load() but we overwrite it here
				//since the old projects don't have this " - Title" appendix yet that we add in init().
				//can be removed in couple of releases
				m_title->setName(name() + QLatin1String(" - ") + i18n("Title"));
			} else {
				TextLabel* label = new TextLabel("text label");
				if (label->load(reader, preview)) {
					addChildFast(label);
					label->setParentGraphicsItem(graphicsItem());
				} else {
					delete label;
					return false;
				}
			}
		} else if (reader->name() == "plotArea")
			m_plotArea->load(reader, preview);
		else if (reader->name() == "axis") {
			Axis* axis = new Axis(QString());
			if (axis->load(reader, preview))
				addChildFast(axis);
			else {
				delete axis;
				return false;
			}
		} else if (reader->name() == "xyCurve") {
			XYCurve* curve = new XYCurve(QString());
			if (curve->load(reader, preview))
				addChildFast(curve);
			else {
				removeChild(curve);
				return false;
			}
		} else if (reader->name() == "xyEquationCurve") {
			XYEquationCurve* curve = new XYEquationCurve(QString());
			if (curve->load(reader, preview))
				addChildFast(curve);
			else {
				removeChild(curve);
				return false;
			}
		} else if (reader->name() == "xyDataReductionCurve") {
			XYDataReductionCurve* curve = new XYDataReductionCurve(QString());
			if (curve->load(reader, preview))
				addChildFast(curve);
			else {
				removeChild(curve);
				return false;
			}
		} else if (reader->name() == "xyDifferentiationCurve") {
			XYDifferentiationCurve* curve = new XYDifferentiationCurve(QString());
			if (curve->load(reader, preview))
				addChildFast(curve);
			else {
				removeChild(curve);
				return false;
			}
		} else if (reader->name() == "xyIntegrationCurve") {
			XYIntegrationCurve* curve = new XYIntegrationCurve(QString());
			if (curve->load(reader, preview))
				addChildFast(curve);
			else {
				removeChild(curve);
				return false;
			}
		} else if (reader->name() == "xyInterpolationCurve") {
			XYInterpolationCurve* curve = new XYInterpolationCurve(QString());
			if (curve->load(reader, preview))
				addChildFast(curve);
			else {
				removeChild(curve);
				return false;
			}
		} else if (reader->name() == "xySmoothCurve") {
			XYSmoothCurve* curve = new XYSmoothCurve(QString());
			if (curve->load(reader, preview))
				addChildFast(curve);
			else {
				removeChild(curve);
				return false;
			}
		} else if (reader->name() == "xyFitCurve") {
			XYFitCurve* curve = new XYFitCurve(QString());
			if (curve->load(reader, preview))
				addChildFast(curve);
			else {
				removeChild(curve);
				return false;
			}
		} else if (reader->name() == "xyFourierFilterCurve") {
			XYFourierFilterCurve* curve = new XYFourierFilterCurve(QString());
			if (curve->load(reader, preview))
				addChildFast(curve);
			else {
				removeChild(curve);
				return false;
			}
		} else if (reader->name() == "xyFourierTransformCurve") {
			XYFourierTransformCurve* curve = new XYFourierTransformCurve(QString());
			if (curve->load(reader, preview))
				addChildFast(curve);
			else {
				removeChild(curve);
				return false;
			}
		} else if (reader->name() == "xyConvolutionCurve") {
			XYConvolutionCurve* curve = new XYConvolutionCurve(QString());
			if (curve->load(reader, preview))
				addChildFast(curve);
			else {
				removeChild(curve);
				return false;
			}
		} else if (reader->name() == "xyCorrelationCurve") {
			XYCorrelationCurve* curve = new XYCorrelationCurve(QString());
			if (curve->load(reader, preview))
				addChildFast(curve);
			else {
				removeChild(curve);
				return false;
			}
		} else if (reader->name() == "cartesianPlotLegend") {
			m_legend = new CartesianPlotLegend(this, QString());
			if (m_legend->load(reader, preview))
				addChildFast(m_legend);
			else {
				delete m_legend;
				return false;
			}
		} else if (reader->name() == "customPoint") {
			CustomPoint* point = new CustomPoint(this, QString());
			if (point->load(reader, preview))
				addChildFast(point);
			else {
				delete point;
				return false;
			}
		} else if (reader->name() == "Histogram") {
			Histogram* curve = new Histogram("Histogram");
			if (curve->load(reader, preview))
				addChildFast(curve);
			else {
				removeChild(curve);
				return false;
			}
		} else { // unknown element
			reader->raiseWarning(i18n("unknown cartesianPlot element '%1'", reader->name().toString()));
			if (!reader->skipToEndElement()) return false;
		}
	}

	if (preview)
		return true;

	d->retransform();

	//if a theme was used, initialize the color palette
	if (!d->theme.isEmpty()) {
		//TODO: check whether the theme config really exists
		KConfig config( ThemeHandler::themeFilePath(d->theme), KConfig::SimpleConfig );
		this->setColorPalette(config);
	} else {
		//initialize the color palette with default colors
		this->setColorPalette(KConfig());
	}

	return true;
}

//##############################################################################
//#########################  Theme management ##################################
//##############################################################################
void CartesianPlot::loadTheme(const QString& theme) {
	KConfig config(ThemeHandler::themeFilePath(theme), KConfig::SimpleConfig);
	loadThemeConfig(config);
}

void CartesianPlot::loadThemeConfig(const KConfig& config) {
	QString str = config.name();

	// theme path is saved with UNIX dir separator
	str = str.right(str.length() - str.lastIndexOf(QLatin1Char('/')) - 1);
	DEBUG("	set theme to " << str.toStdString());
	this->setTheme(str);

	//load the color palettes for the curves
	this->setColorPalette(config);

	//load the theme for all the children
	for (auto* child : children<WorksheetElement>(AbstractAspect::IncludeHidden))
		child->loadThemeConfig(config);

	Q_D(CartesianPlot);
	d->update(this->rect());
}

void CartesianPlot::saveTheme(KConfig &config) {
	const QVector<Axis*>& axisElements = children<Axis>(AbstractAspect::IncludeHidden);
	const QVector<PlotArea*>& plotAreaElements = children<PlotArea>(AbstractAspect::IncludeHidden);
	const QVector<TextLabel*>& textLabelElements = children<TextLabel>(AbstractAspect::IncludeHidden);

	axisElements.at(0)->saveThemeConfig(config);
	plotAreaElements.at(0)->saveThemeConfig(config);
	textLabelElements.at(0)->saveThemeConfig(config);

	for (auto *child : children<XYCurve>(AbstractAspect::IncludeHidden))
		child->saveThemeConfig(config);
}

//Generating colors from 5-color theme palette
void CartesianPlot::setColorPalette(const KConfig& config) {
	if (config.hasGroup(QLatin1String("Theme"))) {
		KConfigGroup group = config.group(QLatin1String("Theme"));

		//read the five colors defining the palette
		m_themeColorPalette.clear();
		m_themeColorPalette.append(group.readEntry("ThemePaletteColor1", QColor()));
		m_themeColorPalette.append(group.readEntry("ThemePaletteColor2", QColor()));
		m_themeColorPalette.append(group.readEntry("ThemePaletteColor3", QColor()));
		m_themeColorPalette.append(group.readEntry("ThemePaletteColor4", QColor()));
		m_themeColorPalette.append(group.readEntry("ThemePaletteColor5", QColor()));
	} else {
		//no theme is available, provide 5 "default colors"
		m_themeColorPalette.clear();
		m_themeColorPalette.append(QColor(25, 25, 25));
		m_themeColorPalette.append(QColor(0, 0, 127));
		m_themeColorPalette.append(QColor(127 ,0, 0));
		m_themeColorPalette.append(QColor(0, 127, 0));
		m_themeColorPalette.append(QColor(85, 0, 127));
	}

	//generate 30 additional shades if the color palette contains more than one color
	if (m_themeColorPalette.at(0) != m_themeColorPalette.at(1)) {
		QColor c;

		//3 factors to create shades from theme's palette
		float fac[3] = {0.25f,0.45f,0.65f};

		//Generate 15 lighter shades
		for (int i = 0; i < 5; i++) {
			for (int j = 1; j < 4; j++) {
				c.setRed( m_themeColorPalette.at(i).red()*(1-fac[j-1]) );
				c.setGreen( m_themeColorPalette.at(i).green()*(1-fac[j-1]) );
				c.setBlue( m_themeColorPalette.at(i).blue()*(1-fac[j-1]) );
				m_themeColorPalette.append(c);
			}
		}

		//Generate 15 darker shades
		for (int i = 0; i < 5; i++) {
			for (int j = 4; j < 7; j++) {
				c.setRed( m_themeColorPalette.at(i).red()+((255-m_themeColorPalette.at(i).red())*fac[j-4]) );
				c.setGreen( m_themeColorPalette.at(i).green()+((255-m_themeColorPalette.at(i).green())*fac[j-4]) );
				c.setBlue( m_themeColorPalette.at(i).blue()+((255-m_themeColorPalette.at(i).blue())*fac[j-4]) );
				m_themeColorPalette.append(c);
			}
		}
	}
}

const QList<QColor>& CartesianPlot::themeColorPalette() const {
	return m_themeColorPalette;
}
