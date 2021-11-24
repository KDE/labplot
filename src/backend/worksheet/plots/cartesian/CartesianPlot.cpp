/*
    File                 : CartesianPlot.cpp
    Project              : LabPlot
    Description          : Cartesian plot
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2011-2021 Alexander Semke <alexander.semke@web.de>
    SPDX-FileCopyrightText: 2016-2021 Stefan Gerlach <stefan.gerlach@uni.kn>
    SPDX-FileCopyrightText: 2017-2018 Garvit Khatri <garvitdelhi@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#include "CartesianPlot.h"
#include "CartesianPlotPrivate.h"
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
#include "XYHilbertTransformCurve.h"
#include "XYConvolutionCurve.h"
#include "XYCorrelationCurve.h"
#include "backend/core/column/Column.h"
#include "backend/core/Project.h"
#include "backend/core/datatypes/DateTime2StringFilter.h"
#include "backend/core/AbstractAspect.h"
#include "backend/worksheet/plots/cartesian/BoxPlot.h"
#include "backend/worksheet/plots/cartesian/CartesianPlotLegend.h"
#include "backend/worksheet/plots/cartesian/CustomPoint.h"
#include "backend/worksheet/plots/cartesian/ReferenceLine.h"
#include "backend/worksheet/plots/cartesian/Symbol.h"
#include "backend/worksheet/plots/PlotArea.h"
#include "backend/worksheet/plots/AbstractPlotPrivate.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/Axis.h"
#include "backend/worksheet/Image.h"
#include "backend/worksheet/TextLabel.h"
#include "backend/worksheet/InfoElement.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/macros.h"
#include "backend/lib/trace.h"
#include "kdefrontend/ThemeHandler.h"
#include "kdefrontend/widgets/ThemesWidget.h"

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>

#include <QDir>
#include <QDropEvent>
#include <QIcon>
#include <QMenu>
#include <QMimeData>
#include <QPainter>
#include <QWidgetAction>

#include <array>
#include <cmath>

/**
 * \class CartesianPlot
 * \brief A xy-plot.
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

	while (!m_coordinateSystems.isEmpty())
		delete m_coordinateSystems.takeFirst();

	//no need to delete objects added with addChild()

	//no need to delete the d-pointer here - it inherits from QGraphicsItem
	//and is deleted during the cleanup in QGraphicsScene
}

/*!
	initializes all member variables of \c CartesianPlot
*/
void CartesianPlot::init() {
	Q_D(CartesianPlot);

	m_coordinateSystems.append( new CartesianCoordinateSystem(this) );
	// TEST: second cSystem for testing
	//d->coordinateSystems.append( new CartesianCoordinateSystem(this) );
	//m_coordinateSystems.append( d->coordinateSystems.at(1) );
	// TEST: set x range to second x range
	//d->coordinateSystems[1]->setXIndex(1);
	// TEST: second x/y range
	//d->xRanges.append(Range<double>(0, 2));
	//d->yRanges.append(Range<double>(0, 2));

	m_plotArea = new PlotArea(name() + " plot area", this);
	addChildFast(m_plotArea);

	//Plot title
	m_title = new TextLabel(this->name() + QLatin1String("- ") + i18n("Title"), TextLabel::Type::PlotTitle);
	addChild(m_title);
	m_title->setHidden(true);
	m_title->setParentGraphicsItem(m_plotArea->graphicsItem());

	//offset between the plot area and the area defining the coordinate system, in scene units.
	d->horizontalPadding = Worksheet::convertToSceneUnits(1.5, Worksheet::Unit::Centimeter);
	d->verticalPadding = Worksheet::convertToSceneUnits(1.5, Worksheet::Unit::Centimeter);
	d->rightPadding = Worksheet::convertToSceneUnits(1.5, Worksheet::Unit::Centimeter);
	d->bottomPadding = Worksheet::convertToSceneUnits(1.5, Worksheet::Unit::Centimeter);
	d->symmetricPadding = true;

	connect(this, &AbstractAspect::aspectAdded, this, &CartesianPlot::childAdded);
	connect(this, &AbstractAspect::aspectRemoved, this, &CartesianPlot::childRemoved);

	graphicsItem()->setFlag(QGraphicsItem::ItemIsMovable, true);
	graphicsItem()->setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
	graphicsItem()->setFlag(QGraphicsItem::ItemIsSelectable, true);
	graphicsItem()->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
	graphicsItem()->setFlag(QGraphicsItem::ItemIsFocusable, true);

	//theme is not set at this point, initialize the color palette with default colors
	this->setColorPalette(KConfig());
}

/*!
	initializes all children of \c CartesianPlot and
	setups a default plot of type \c type with a plot title.
*/
void CartesianPlot::setType(Type type) {
	Q_D(CartesianPlot);

	d->type = type;

	switch (type) {
	case Type::FourAxes: {
			//Axes
			Axis* axis = new Axis(QLatin1String("x"), Axis::Orientation::Horizontal);
			axis->setDefault(true);
			axis->setSuppressRetransform(true);
			addChild(axis);
			axis->setPosition(Axis::Position::Bottom);
			axis->setRange(0., 1.);
			axis->setMajorTicksDirection(Axis::ticksIn);
			axis->setMinorTicksDirection(Axis::ticksIn);
			axis->setMinorTicksNumber(1);
			axis->setSuppressRetransform(false);

			axis = new Axis(QLatin1String("x2"), Axis::Orientation::Horizontal);
			//TEST: use second cSystem of plot
			//axis->setCoordinateSystemIndex(1);
			axis->setDefault(true);
			axis->setSuppressRetransform(true);

			addChild(axis);
			axis->setPosition(Axis::Position::Top);
			axis->setRange(0., 1.);
			axis->setMajorTicksDirection(Axis::ticksIn);
			axis->setMinorTicksDirection(Axis::ticksIn);
			axis->setMinorTicksNumber(1);
			QPen pen = axis->minorGridPen();
			pen.setStyle(Qt::NoPen);
			axis->setMajorGridPen(pen);
			pen = axis->minorGridPen();
			pen.setStyle(Qt::NoPen);
			axis->setMinorGridPen(pen);
			axis->setLabelsPosition(Axis::LabelsPosition::NoLabels);
			axis->title()->setText(QString());
			axis->setSuppressRetransform(false);

			axis = new Axis(QLatin1String("y"), Axis::Orientation::Vertical);
			axis->setDefault(true);
			axis->setSuppressRetransform(true);
			addChild(axis);
			axis->setPosition(Axis::Position::Left);
			axis->setRange(0., 1.);
			axis->setMajorTicksDirection(Axis::ticksIn);
			axis->setMinorTicksDirection(Axis::ticksIn);
			axis->setMinorTicksNumber(1);
			axis->setSuppressRetransform(false);

			axis = new Axis(QLatin1String("y2"), Axis::Orientation::Vertical);
			axis->setDefault(true);
			axis->setSuppressRetransform(true);
			addChild(axis);
			axis->setPosition(Axis::Position::Right);
			axis->setRange(0., 1.);
			axis->setMajorTicksDirection(Axis::ticksIn);
			axis->setMinorTicksDirection(Axis::ticksIn);
			axis->setMinorTicksNumber(1);
			pen = axis->minorGridPen();
			pen.setStyle(Qt::NoPen);
			axis->setMajorGridPen(pen);
			pen = axis->minorGridPen();
			pen.setStyle(Qt::NoPen);
			axis->setMinorGridPen(pen);
			axis->setLabelsPosition(Axis::LabelsPosition::NoLabels);
			axis->title()->setText(QString());
			axis->setSuppressRetransform(false);

			break;
		}
	case Type::TwoAxes: {
			Axis* axis = new Axis(QLatin1String("x"), Axis::Orientation::Horizontal);
			axis->setDefault(true);
			axis->setSuppressRetransform(true);
			addChild(axis);
			axis->setPosition(Axis::Position::Bottom);
			axis->setRange(0., 1.);
			axis->setMajorTicksDirection(Axis::ticksBoth);
			axis->setMinorTicksDirection(Axis::ticksBoth);
			axis->setMinorTicksNumber(1);
			axis->setArrowType(Axis::ArrowType::FilledSmall);
			axis->setSuppressRetransform(false);

			axis = new Axis(QLatin1String("y"), Axis::Orientation::Vertical);
			axis->setDefault(true);
			axis->setSuppressRetransform(true);
			addChild(axis);
			axis->setPosition(Axis::Position::Left);
			axis->setRange(0., 1.);
			axis->setMajorTicksDirection(Axis::ticksBoth);
			axis->setMinorTicksDirection(Axis::ticksBoth);
			axis->setMinorTicksNumber(1);
			axis->setArrowType(Axis::ArrowType::FilledSmall);
			axis->setSuppressRetransform(false);

			break;
		}
	case Type::TwoAxesCentered: {
			d->xRanges[0].range.setRange(-0.5, 0.5);
			d->yRanges[0].range.setRange(-0.5, 0.5);

			d->horizontalPadding = Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Centimeter);
			d->verticalPadding = Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Centimeter);

			QPen pen = m_plotArea->borderPen();
			pen.setStyle(Qt::NoPen);
			m_plotArea->setBorderPen(pen);

			Axis* axis = new Axis(QLatin1String("x"), Axis::Orientation::Horizontal);
			axis->setDefault(true);
			axis->setSuppressRetransform(true);
			addChild(axis);
			axis->setPosition(Axis::Position::Centered);
			axis->setRange(-0.5, 0.5);
			axis->setMajorTicksDirection(Axis::ticksBoth);
			axis->setMinorTicksDirection(Axis::ticksBoth);
			axis->setMinorTicksNumber(1);
			axis->setArrowType(Axis::ArrowType::FilledSmall);
			axis->title()->setText(QString());
			axis->setSuppressRetransform(false);

			axis = new Axis(QLatin1String("y"), Axis::Orientation::Vertical);
			axis->setDefault(true);
			axis->setSuppressRetransform(true);
			addChild(axis);
			axis->setPosition(Axis::Position::Centered);
			axis->setRange(-0.5, 0.5);
			axis->setMajorTicksDirection(Axis::ticksBoth);
			axis->setMinorTicksDirection(Axis::ticksBoth);
			axis->setMinorTicksNumber(1);
			axis->setArrowType(Axis::ArrowType::FilledSmall);
			axis->title()->setText(QString());
			axis->setSuppressRetransform(false);

			break;
		}
	case Type::TwoAxesCenteredZero: {
			d->xRanges[0].range.setRange(-0.5, 0.5);
			d->yRanges[0].range.setRange(-0.5, 0.5);

			d->horizontalPadding = Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Centimeter);
			d->verticalPadding = Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Centimeter);

			QPen pen = m_plotArea->borderPen();
			pen.setStyle(Qt::NoPen);
			m_plotArea->setBorderPen(pen);

			Axis* axis = new Axis(QLatin1String("x"), Axis::Orientation::Horizontal);
			axis->setDefault(true);
			axis->setSuppressRetransform(true);
			addChild(axis);
			axis->setPosition(Axis::Position::Logical);
			axis->setOffset(0);
			axis->setLogicalPosition(0);
			axis->setRange(-0.5, 0.5);
			axis->setMajorTicksDirection(Axis::ticksBoth);
			axis->setMinorTicksDirection(Axis::ticksBoth);
			axis->setMinorTicksNumber(1);
			axis->setArrowType(Axis::ArrowType::FilledSmall);
			axis->title()->setText(QString());
			axis->setSuppressRetransform(false);

			axis = new Axis(QLatin1String("y"), Axis::Orientation::Vertical);
			axis->setDefault(true);
			axis->setSuppressRetransform(true);
			addChild(axis);
			axis->setPosition(Axis::Position::Logical);
			axis->setOffset(0);
			axis->setLogicalPosition(0);
			axis->setRange(-0.5, 0.5);
			axis->setMajorTicksDirection(Axis::ticksBoth);
			axis->setMinorTicksDirection(Axis::ticksBoth);
			axis->setMinorTicksNumber(1);
			axis->setArrowType(Axis::ArrowType::FilledSmall);
			axis->title()->setText(QString());
			axis->setSuppressRetransform(false);

			break;
		}
	}

	d->xRanges[0].prev = xRange();
	d->yRanges[0].prev = yRange();

	//Geometry, specify the plot rect in scene coordinates.
	//TODO: Use default settings for left, top, width, height and for min/max for the coordinate system
	double x = Worksheet::convertToSceneUnits(2, Worksheet::Unit::Centimeter);
	double y = Worksheet::convertToSceneUnits(2, Worksheet::Unit::Centimeter);
	double w = Worksheet::convertToSceneUnits(10, Worksheet::Unit::Centimeter);
	double h = Worksheet::convertToSceneUnits(10, Worksheet::Unit::Centimeter);

	//all plot children are initialized -> set the geometry of the plot in scene coordinates.
	d->rect = QRectF(x, y, w, h);
	d->retransform();
}

CartesianPlot::Type CartesianPlot::type() const {
	Q_D(const CartesianPlot);
	return d->type;
}

void CartesianPlot::initActions() {
	//"add new" actions
	addCurveAction = new QAction(QIcon::fromTheme("labplot-xy-curve"), i18n("xy-curve"), this);
	addHistogramAction = new QAction(QIcon::fromTheme("view-object-histogram-linear"), i18n("Histogram"), this);
	addBoxPlotAction = new QAction(QIcon::fromTheme("view-object-histogram-linear"), i18n("Box Plot"), this);
	addEquationCurveAction = new QAction(QIcon::fromTheme("labplot-xy-equation-curve"), i18n("xy-curve from a mathematical Equation"), this);
// no icons yet
	addDataReductionCurveAction = new QAction(QIcon::fromTheme("labplot-xy-curve"), i18n("Data Reduction"), this);
	addDifferentiationCurveAction = new QAction(QIcon::fromTheme("labplot-xy-curve"), i18n("Differentiation"), this);
	addIntegrationCurveAction = new QAction(QIcon::fromTheme("labplot-xy-curve"), i18n("Integration"), this);
	addInterpolationCurveAction = new QAction(QIcon::fromTheme("labplot-xy-interpolation-curve"), i18n("Interpolation"), this);
	addSmoothCurveAction = new QAction(QIcon::fromTheme("labplot-xy-smoothing-curve"), i18n("Smooth"), this);
	addFitCurveAction = new QAction(QIcon::fromTheme("labplot-xy-fit-curve"), i18n("Fit"), this);
	addFourierFilterCurveAction = new QAction(QIcon::fromTheme("labplot-xy-fourier-filter-curve"), i18n("Fourier Filter"), this);
	addFourierTransformCurveAction = new QAction(QIcon::fromTheme("labplot-xy-fourier-transform-curve"), i18n("Fourier Transform"), this);
	addHilbertTransformCurveAction = new QAction(QIcon::fromTheme("labplot-xy-curve"), i18n("Hilbert Transform"), this);
	addConvolutionCurveAction = new QAction(QIcon::fromTheme("labplot-xy-curve"),i18n("(De-)Convolution"), this);
	addCorrelationCurveAction = new QAction(QIcon::fromTheme("labplot-xy-curve"),i18n("Auto-/Cross-Correlation"), this);

	addLegendAction = new QAction(QIcon::fromTheme("text-field"), i18n("Legend"), this);
	if (children<CartesianPlotLegend>().size() > 0)
		addLegendAction->setEnabled(false);	//only one legend is allowed -> disable the action

	addHorizontalAxisAction = new QAction(QIcon::fromTheme("labplot-axis-horizontal"), i18n("Horizontal Axis"), this);
	addVerticalAxisAction = new QAction(QIcon::fromTheme("labplot-axis-vertical"), i18n("Vertical Axis"), this);
	addTextLabelAction = new QAction(QIcon::fromTheme("draw-text"), i18n("Text"), this);
	addImageAction = new QAction(QIcon::fromTheme("viewimage"), i18n("Image"), this);
	addInfoElementAction = new QAction(QIcon::fromTheme("draw-text"), i18n("Info Element"), this);
	addCustomPointAction = new QAction(QIcon::fromTheme("draw-cross"), i18n("Custom Point"), this);
	addReferenceLineAction = new QAction(QIcon::fromTheme("draw-line"), i18n("Reference Line"), this);

	connect(addCurveAction, &QAction::triggered, this, &CartesianPlot::addCurve);
	connect(addHistogramAction, &QAction::triggered, this, &CartesianPlot::addHistogram);
	connect(addBoxPlotAction, &QAction::triggered, this, &CartesianPlot::addBoxPlot);
	connect(addEquationCurveAction, &QAction::triggered, this, &CartesianPlot::addEquationCurve);
	connect(addDataReductionCurveAction, &QAction::triggered, this, &CartesianPlot::addDataReductionCurve);
	connect(addDifferentiationCurveAction, &QAction::triggered, this, &CartesianPlot::addDifferentiationCurve);
	connect(addIntegrationCurveAction, &QAction::triggered, this, &CartesianPlot::addIntegrationCurve);
	connect(addInterpolationCurveAction, &QAction::triggered, this, &CartesianPlot::addInterpolationCurve);
	connect(addSmoothCurveAction, &QAction::triggered, this, &CartesianPlot::addSmoothCurve);
	connect(addFitCurveAction, &QAction::triggered, this, &CartesianPlot::addFitCurve);
	connect(addFourierFilterCurveAction, &QAction::triggered, this, &CartesianPlot::addFourierFilterCurve);
	connect(addFourierTransformCurveAction, &QAction::triggered, this, &CartesianPlot::addFourierTransformCurve);
	connect(addHilbertTransformCurveAction, &QAction::triggered, this, &CartesianPlot::addHilbertTransformCurve);
	connect(addConvolutionCurveAction, &QAction::triggered, this, &CartesianPlot::addConvolutionCurve);
	connect(addCorrelationCurveAction, &QAction::triggered, this, &CartesianPlot::addCorrelationCurve);

	connect(addLegendAction, &QAction::triggered, this, static_cast<void (CartesianPlot::*)()>(&CartesianPlot::addLegend));
	connect(addHorizontalAxisAction, &QAction::triggered, this, &CartesianPlot::addHorizontalAxis);
	connect(addVerticalAxisAction, &QAction::triggered, this, &CartesianPlot::addVerticalAxis);
	connect(addTextLabelAction, &QAction::triggered, this, &CartesianPlot::addTextLabel);
	connect(addImageAction, &QAction::triggered, this, &CartesianPlot::addImage);
	connect(addInfoElementAction, &QAction::triggered, this, &CartesianPlot::addInfoElement);
	connect(addCustomPointAction, &QAction::triggered, this, &CartesianPlot::addCustomPoint);
	connect(addReferenceLineAction, &QAction::triggered, this, &CartesianPlot::addReferenceLine);

	//Analysis menu actions
// 	addDataOperationAction = new QAction(i18n("Data Operation"), this);
	addDataReductionAction = new QAction(QIcon::fromTheme("labplot-xy-curve"), i18n("Data Reduction"), this);
	addDifferentiationAction = new QAction(QIcon::fromTheme("labplot-xy-curve"), i18n("Differentiate"), this);
	addIntegrationAction = new QAction(QIcon::fromTheme("labplot-xy-curve"), i18n("Integrate"), this);
	addInterpolationAction = new QAction(QIcon::fromTheme("labplot-xy-interpolation-curve"), i18n("Interpolate"), this);
	addSmoothAction = new QAction(QIcon::fromTheme("labplot-xy-smoothing-curve"), i18n("Smooth"), this);
	addConvolutionAction = new QAction(QIcon::fromTheme("labplot-xy-curve"), i18n("Convolute/Deconvolute"), this);
	addCorrelationAction = new QAction(QIcon::fromTheme("labplot-xy-curve"), i18n("Auto-/Cross-Correlation"), this);

	QAction* fitAction = new QAction(i18n("Linear"), this);
	fitAction->setData(static_cast<int>(XYAnalysisCurve::AnalysisAction::FitLinear));
	addFitActions.append(fitAction);

	fitAction = new QAction(i18n("Power"), this);
	fitAction->setData(static_cast<int>(XYAnalysisCurve::AnalysisAction::FitPower));
	addFitActions.append(fitAction);

	fitAction = new QAction(i18n("Exponential (degree 1)"), this);
	fitAction->setData(static_cast<int>(XYAnalysisCurve::AnalysisAction::FitExp1));
	addFitActions.append(fitAction);

	fitAction = new QAction(i18n("Exponential (degree 2)"), this);
	fitAction->setData(static_cast<int>(XYAnalysisCurve::AnalysisAction::FitExp2));
	addFitActions.append(fitAction);

	fitAction = new QAction(i18n("Inverse exponential"), this);
	fitAction->setData(static_cast<int>(XYAnalysisCurve::AnalysisAction::FitInvExp));
	addFitActions.append(fitAction);

	fitAction = new QAction(i18n("Gauss"), this);
	fitAction->setData(static_cast<int>(XYAnalysisCurve::AnalysisAction::FitGauss));
	addFitActions.append(fitAction);

	fitAction = new QAction(i18n("Cauchy-Lorentz"), this);
	fitAction->setData(static_cast<int>(XYAnalysisCurve::AnalysisAction::FitCauchyLorentz));
	addFitActions.append(fitAction);

	fitAction = new QAction(i18n("Arc Tangent"), this);
	fitAction->setData(static_cast<int>(XYAnalysisCurve::AnalysisAction::FitTan));
	addFitActions.append(fitAction);

	fitAction = new QAction(i18n("Hyperbolic Tangent"), this);
	fitAction->setData(static_cast<int>(XYAnalysisCurve::AnalysisAction::FitTanh));
	addFitActions.append(fitAction);

	fitAction = new QAction(i18n("Error Function"), this);
	fitAction->setData(static_cast<int>(XYAnalysisCurve::AnalysisAction::FitErrFunc));
	addFitActions.append(fitAction);

	fitAction = new QAction(i18n("Custom"), this);
	fitAction->setData(static_cast<int>(XYAnalysisCurve::AnalysisAction::FitCustom));
	addFitActions.append(fitAction);

	addFourierFilterAction = new QAction(QIcon::fromTheme("labplot-xy-fourier-filter-curve"), i18n("Fourier Filter"), this);
	addFourierTransformAction = new QAction(QIcon::fromTheme("labplot-xy-fourier-transform-curve"), i18n("Fourier Transform"), this);
	addHilbertTransformAction = new QAction(QIcon::fromTheme("labplot-xy-curve"), i18n("Hilbert Transform"), this);

	connect(addDataReductionAction, &QAction::triggered, this, &CartesianPlot::addDataReductionCurve);
	connect(addDifferentiationAction, &QAction::triggered, this, &CartesianPlot::addDifferentiationCurve);
	connect(addIntegrationAction, &QAction::triggered, this, &CartesianPlot::addIntegrationCurve);
	connect(addInterpolationAction, &QAction::triggered, this, &CartesianPlot::addInterpolationCurve);
	connect(addSmoothAction, &QAction::triggered, this, &CartesianPlot::addSmoothCurve);
	connect(addConvolutionAction, &QAction::triggered, this, &CartesianPlot::addConvolutionCurve);
	connect(addCorrelationAction, &QAction::triggered, this, &CartesianPlot::addCorrelationCurve);
	for (const auto& action : addFitActions)
		connect(action, &QAction::triggered, this, &CartesianPlot::addFitCurve);
	connect(addFourierFilterAction, &QAction::triggered, this, &CartesianPlot::addFourierFilterCurve);
	connect(addFourierTransformAction, &QAction::triggered, this, &CartesianPlot::addFourierTransformCurve);
	connect(addHilbertTransformAction, &QAction::triggered, this, &CartesianPlot::addHilbertTransformCurve);

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

	connect(scaleAutoAction, &QAction::triggered, this, &CartesianPlot::scaleAutoTriggered);
	connect(scaleAutoXAction, &QAction::triggered, this, &CartesianPlot::scaleAutoTriggered);
	connect(scaleAutoYAction, &QAction::triggered, this, &CartesianPlot::scaleAutoTriggered);
	connect(zoomInAction, &QAction::triggered, [this] {zoomIn();});
	connect(zoomOutAction, &QAction::triggered, [this] {zoomOut();});
	connect(zoomInXAction, &QAction::triggered, [this] {zoomInX();});
	connect(zoomOutXAction, &QAction::triggered, [this] {zoomOutX();});
	connect(zoomInYAction, &QAction::triggered, [this] {zoomInY();});
	connect(zoomOutYAction, &QAction::triggered, [this] {zoomOutY();});
	connect(shiftLeftXAction, &QAction::triggered, [this] {shiftLeftX();});
	connect(shiftRightXAction, &QAction::triggered, [this] {shiftRightX();});
	connect(shiftUpYAction, &QAction::triggered, [this] {shiftUpY();});
	connect(shiftDownYAction, &QAction::triggered, [this] {shiftDownY();});

	//visibility action
	visibilityAction = new QAction(QIcon::fromTheme("view-visible"), i18n("Visible"), this);
	visibilityAction->setCheckable(true);
	connect(visibilityAction, &QAction::triggered, this, &CartesianPlot::visibilityChanged);
}

void CartesianPlot::initMenus() {
	initActions();

	addNewMenu = new QMenu(i18n("Add New"));
	addNewMenu->setIcon(QIcon::fromTheme("list-add"));
	addNewMenu->addAction(addCurveAction);
	addNewMenu->addAction(addHistogramAction);
	addNewMenu->addAction(addBoxPlotAction);
	addNewMenu->addAction(addEquationCurveAction);
	addNewMenu->addSeparator();

	addNewAnalysisMenu = new QMenu(i18n("Analysis Curve"));
	addNewAnalysisMenu->addAction(addFitCurveAction);
	addNewAnalysisMenu->addSeparator();
	addNewAnalysisMenu->addAction(addDifferentiationCurveAction);
	addNewAnalysisMenu->addAction(addIntegrationCurveAction);
	addNewAnalysisMenu->addSeparator();
	addNewAnalysisMenu->addAction(addInterpolationCurveAction);
	addNewAnalysisMenu->addAction(addSmoothCurveAction);
	addNewAnalysisMenu->addSeparator();
	addNewAnalysisMenu->addAction(addFourierFilterCurveAction);
	addNewAnalysisMenu->addAction(addFourierTransformCurveAction);
	addNewAnalysisMenu->addAction(addHilbertTransformCurveAction);
	addNewAnalysisMenu->addSeparator();
	addNewAnalysisMenu->addAction(addConvolutionCurveAction);
	addNewAnalysisMenu->addAction(addCorrelationCurveAction);
	addNewAnalysisMenu->addSeparator();
	addNewAnalysisMenu->addAction(addDataReductionCurveAction);
	addNewMenu->addMenu(addNewAnalysisMenu);

	addNewMenu->addSeparator();
	addNewMenu->addAction(addLegendAction);
	addNewMenu->addSeparator();
	addNewMenu->addAction(addHorizontalAxisAction);
	addNewMenu->addAction(addVerticalAxisAction);
	addNewMenu->addSeparator();
	addNewMenu->addAction(addTextLabelAction);
	addNewMenu->addAction(addImageAction);
	addNewMenu->addAction(addInfoElementAction);
	addNewMenu->addSeparator();
	addNewMenu->addAction(addCustomPointAction);
	addNewMenu->addAction(addReferenceLineAction);

	zoomMenu = new QMenu(i18n("Zoom/Navigate"));
	zoomMenu->setIcon(QIcon::fromTheme("zoom-draw"));
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
// 	QMenu* dataManipulationMenu = new QMenu(i18n("Data Manipulation"));
// 	dataManipulationMenu->setIcon(QIcon::fromTheme("zoom-draw"));
// 	dataManipulationMenu->addAction(addDataOperationAction);
// 	dataManipulationMenu->addAction(addDataReductionAction);

	// Data fit menu
	QMenu* dataFitMenu = new QMenu(i18n("Fit"));
	dataFitMenu->setIcon(QIcon::fromTheme("labplot-xy-fit-curve"));
	dataFitMenu->addAction(addFitActions.at(0));
	dataFitMenu->addAction(addFitActions.at(1));
	dataFitMenu->addAction(addFitActions.at(2));
	dataFitMenu->addAction(addFitActions.at(3));
	dataFitMenu->addAction(addFitActions.at(4));
	dataFitMenu->addSeparator();
	dataFitMenu->addAction(addFitActions.at(5));
	dataFitMenu->addAction(addFitActions.at(6));
	dataFitMenu->addSeparator();
	dataFitMenu->addAction(addFitActions.at(7));
	dataFitMenu->addAction(addFitActions.at(8));
	dataFitMenu->addAction(addFitActions.at(9));
	dataFitMenu->addSeparator();
	dataFitMenu->addAction(addFitActions.at(10));

	//analysis menu
	dataAnalysisMenu = new QMenu(i18n("Analysis"));
	dataAnalysisMenu->addMenu(dataFitMenu);
	dataAnalysisMenu->addSeparator();
	dataAnalysisMenu->addAction(addDifferentiationAction);
	dataAnalysisMenu->addAction(addIntegrationAction);
	dataAnalysisMenu->addSeparator();
	dataAnalysisMenu->addAction(addInterpolationAction);
	dataAnalysisMenu->addAction(addSmoothAction);
	dataAnalysisMenu->addSeparator();
	dataAnalysisMenu->addAction(addFourierFilterAction);
	dataAnalysisMenu->addAction(addFourierTransformAction);
	dataAnalysisMenu->addAction(addHilbertTransformAction);
	dataAnalysisMenu->addSeparator();
	dataAnalysisMenu->addAction(addConvolutionAction);
	dataAnalysisMenu->addAction(addCorrelationAction);
	dataAnalysisMenu->addSeparator();
// 	dataAnalysisMenu->insertMenu(nullptr, dataManipulationMenu);
	dataAnalysisMenu->addAction(addDataReductionAction);

	//theme menu
	themeMenu = new QMenu(i18n("Apply Theme"));
	themeMenu->setIcon(QIcon::fromTheme("color-management"));
#ifndef SDK
	auto* themeWidget = new ThemesWidget(nullptr);
	themeWidget->setFixedMode();
	connect(themeWidget, &ThemesWidget::themeSelected, this, &CartesianPlot::loadTheme);
	connect(themeWidget, &ThemesWidget::themeSelected, themeMenu, &QMenu::close);

	auto* widgetAction = new QWidgetAction(this);
	widgetAction->setDefaultWidget(themeWidget);
	themeMenu->addAction(widgetAction);
#endif

	m_menusInitialized = true;
}

QMenu* CartesianPlot::createContextMenu() {
	if (!m_menusInitialized)
		initMenus();

	QMenu* menu = WorksheetElement::createContextMenu();
	// seems to be a bug, because the tooltips are not shown
	menu->setToolTipsVisible(true);
	QAction* firstAction = menu->actions().at(1);

	menu->insertMenu(firstAction, addNewMenu);
	menu->insertSeparator(firstAction);
	menu->insertMenu(firstAction, zoomMenu);
	menu->insertSeparator(firstAction);
	menu->insertMenu(firstAction, themeMenu);
	menu->insertSeparator(firstAction);

	visibilityAction->setChecked(isVisible());
	menu->insertAction(firstAction, visibilityAction);
	menu->insertSeparator(firstAction);

	if (children<XYCurve>().isEmpty()) {
		addInfoElementAction->setEnabled(false);
		addInfoElementAction->setToolTip("No curve inside plot.");
	} else {
		addInfoElementAction->setEnabled(true);
		addInfoElementAction->setToolTip("");
	}

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
		if (curve->xColumn() && curve->xColumn()->parentAspect()->type() == AspectType::Spreadsheet)
			aspects << curve->xColumn()->parentAspect();

		if (curve->yColumn() && curve->yColumn()->parentAspect()->type() == AspectType::Spreadsheet)
			aspects << curve->yColumn()->parentAspect();
	}

	return aspects;
}

QVector<AspectType> CartesianPlot::pasteTypes() const {
	QVector<AspectType> types{
		AspectType::XYCurve, AspectType::Histogram, AspectType::BoxPlot,
		AspectType::Axis, AspectType::XYEquationCurve,
		AspectType::XYConvolutionCurve, AspectType::XYCorrelationCurve,
		AspectType::XYDataReductionCurve, AspectType::XYDifferentiationCurve,
		AspectType::XYFitCurve, AspectType::XYFourierFilterCurve,
		AspectType::XYFourierTransformCurve, AspectType::XYIntegrationCurve,
		AspectType::XYInterpolationCurve, AspectType::XYSmoothCurve,
		AspectType::TextLabel, AspectType::Image,
		AspectType::InfoElement, AspectType::CustomPoint,
		AspectType::ReferenceLine
	};

	//only allow to paste a legend if there is no legend available yet in the plot
	if (!m_legend)
		types << AspectType::CartesianPlotLegend;

	return types;
}

void CartesianPlot::navigate(int cSystemIndex, NavigationOperation op) {
	auto cSystem = coordinateSystem(cSystemIndex);
	int xIndex = -1, yIndex = -1;
	if (cSystem) {
		xIndex = cSystem->xIndex();
		yIndex = cSystem->yIndex();
	}

	if (op == NavigationOperation::ScaleAuto) {
		if (cSystemIndex == -1) {
			// TODO: needed? because the indices can also be -1 so only the else part is needed?
			for (int i = 0; i < coordinateSystemCount(); i++) {
				auto cSystem = coordinateSystem(i);
				auto xDirty = xRangeDirty(cSystem->xIndex());
				auto yDirty = yRangeDirty(cSystem->yIndex());

				if (xDirty || yDirty || !autoScaleX(cSystem->xIndex()) || !autoScaleY(cSystem->yIndex())) {
					setXRangeDirty(cSystem->xIndex(), true);
					setYRangeDirty(cSystem->yIndex(), true);
				}
				scaleAuto(cSystem->xIndex(), cSystem->yIndex());
			}
		} else {
			auto xDirty = xRangeDirty(xIndex);
			auto yDirty = yRangeDirty(yIndex);

			if (xDirty || yDirty || !autoScaleX(xIndex) || !autoScaleY(yIndex)) {
				setXRangeDirty(xIndex, true);
				setYRangeDirty(yIndex, true);
			}
			scaleAuto(xIndex, yIndex);
		}
	} else if (op == NavigationOperation::ScaleAutoX) {
		if (scaleAutoX(xIndex, true)) {
			for (int i=0; i < m_coordinateSystems.count(); i++) {
				auto cs = coordinateSystem(i);
				if ((cSystemIndex == -1 || xIndex == cs->xIndex()) && autoScaleY(cs->yIndex()))
					scaleAutoY(cs->yIndex());
			}
		}
	} else if (op == NavigationOperation::ScaleAutoY) {
		if (scaleAutoY(yIndex, true)) {
			for (int i=0; i < m_coordinateSystems.count(); i++) {
				auto cs = coordinateSystem(i);
				if ((cSystemIndex == -1 || yIndex == cs->yIndex()) && autoScaleX(cs->xIndex()))
					scaleAutoX(cs->xIndex());
			}
		}
	} else if (op == NavigationOperation::ZoomIn) zoomIn(xIndex, yIndex);
	else if (op == NavigationOperation::ZoomOut) zoomOut(xIndex, yIndex);
	else if (op == NavigationOperation::ZoomInX) zoomInX(xIndex);
	else if (op == NavigationOperation::ZoomOutX) zoomOutX(xIndex);
	else if (op == NavigationOperation::ZoomInY) zoomInY(yIndex);
	else if (op == NavigationOperation::ZoomOutY) zoomOutY(yIndex);
	else if (op == NavigationOperation::ShiftLeftX) shiftLeftX(xIndex);
	else if (op == NavigationOperation::ShiftRightX) shiftRightX(xIndex);
	else if (op == NavigationOperation::ShiftUpY) shiftUpY(yIndex);
	else if (op == NavigationOperation::ShiftDownY) shiftDownY(yIndex);
}

void CartesianPlot::setSuppressDataChangedSignal(bool value) {
	Q_D(CartesianPlot);
	d->suppressRetransform = value;
}

void CartesianPlot::processDropEvent(const QVector<quintptr>& vec) {
	PERFTRACE("CartesianPlot::processDropEvent");

	QVector<AbstractColumn*> columns;
	for (auto a : vec) {
		auto* aspect = (AbstractAspect*)a;
		auto* column = qobject_cast<AbstractColumn*>(aspect);
		if (column)
			columns << column;
	}

	//return if there are no columns being dropped.
	//TODO: extend this later when we allow to drag&drop plots, etc.
	if (columns.isEmpty())
		return;

	//determine the first column with "x plot designation" as the x-data column for all curves to be created
	const AbstractColumn* xColumn = nullptr;
	for (const auto* column : qAsConst(columns)) {
		if (column->plotDesignation() == AbstractColumn::PlotDesignation::X) {
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
	for (const auto* column : qAsConst(columns)) {
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
		dataChanged(-1, -1); // TODO: check if all ranges must be updated
}

bool CartesianPlot::isPanningActive() const {
	Q_D(const CartesianPlot);
	return d->panningStarted;
}

bool CartesianPlot::isHovered() const {
	Q_D(const CartesianPlot);
	return d->m_hovered;
}
bool CartesianPlot::isPrinted() const {
	Q_D(const CartesianPlot);
	return d->m_printing;
}

bool CartesianPlot::isSelected() const {
	Q_D(const CartesianPlot);
	return d->isSelected();
}

//##############################################################################
//################################  getter methods  ############################
//##############################################################################
BASIC_SHARED_D_READER_IMPL(CartesianPlot, CartesianPlot::RangeType, rangeType, rangeType)
BASIC_SHARED_D_READER_IMPL(CartesianPlot, int, rangeLastValues, rangeLastValues)
BASIC_SHARED_D_READER_IMPL(CartesianPlot, int, rangeFirstValues, rangeFirstValues)

BASIC_SHARED_D_READER_IMPL(CartesianPlot, bool, xRangeBreakingEnabled, xRangeBreakingEnabled)
BASIC_SHARED_D_READER_IMPL(CartesianPlot, CartesianPlot::RangeBreaks, xRangeBreaks, xRangeBreaks)
BASIC_SHARED_D_READER_IMPL(CartesianPlot, bool, yRangeBreakingEnabled, yRangeBreakingEnabled)
BASIC_SHARED_D_READER_IMPL(CartesianPlot, CartesianPlot::RangeBreaks, yRangeBreaks, yRangeBreaks)

BASIC_SHARED_D_READER_IMPL(CartesianPlot, QPen, cursorPen, cursorPen);
BASIC_SHARED_D_READER_IMPL(CartesianPlot, bool, cursor0Enable, cursor0Enable);
BASIC_SHARED_D_READER_IMPL(CartesianPlot, bool, cursor1Enable, cursor1Enable);
BASIC_SHARED_D_READER_IMPL(CartesianPlot, QString, theme, theme)

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

const QString CartesianPlot::xRangeDateTimeFormat() const {
	const int index{ defaultCoordinateSystem()->xIndex() };
	return xRangeDateTimeFormat(index);
}
const QString CartesianPlot::yRangeDateTimeFormat() const {
	const int index{ defaultCoordinateSystem()->yIndex() };
	return yRangeDateTimeFormat(index);
}
const QString CartesianPlot::xRangeDateTimeFormat(const int index) const {
	Q_D(const CartesianPlot);
	return d->xRanges.at(index).range.dateTimeFormat();
}
const QString CartesianPlot::yRangeDateTimeFormat(const int index) const {
	Q_D(const CartesianPlot);
	return d->yRanges.at(index).range.dateTimeFormat();
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
	}

	void redo() override {
// 		const double horizontalRatio = m_rect.width() / m_private->rect.width();
// 		const double verticalRatio = m_rect.height() / m_private->rect.height();

		qSwap(m_private->rect, m_rect);

// 		m_private->q->handleResize(horizontalRatio, verticalRatio, false);
		m_private->retransform();
		emit m_private->q->rectChanged(m_private->rect);
	}

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

// x/y ranges

class CartesianPlotSetXRangeFormatIndexCmd : public QUndoCommand {
public:
	CartesianPlotSetXRangeFormatIndexCmd(CartesianPlotPrivate* private_obj, RangeT::Format format, int index) :
		m_private(private_obj), m_format(format), m_index(index) {
		setText(i18n("%1: change x-range %2 format", m_private->name(), index + 1));
	}

	void redo() override {
		m_formatOld = m_private->xRanges.at(m_index).range.format();
		m_private->xRanges[m_index].range.setFormat(m_format);
		emit m_private->q->xRangeFormatChanged(m_index, m_format);
	}

	void undo() override {
		m_private->xRanges[m_index].range.setFormat(m_formatOld);
		emit m_private->q->xRangeFormatChanged(m_index, m_formatOld);
	}

private:
	CartesianPlotPrivate* m_private;
	RangeT::Format m_format;
	int m_index;
	RangeT::Format m_formatOld;
};
class CartesianPlotSetYRangeFormatIndexCmd : public QUndoCommand {
public:
	CartesianPlotSetYRangeFormatIndexCmd(CartesianPlotPrivate* private_obj, RangeT::Format format, int index) :
		m_private(private_obj), m_format(format), m_index(index) {
		setText(i18n("%1: change y-range %2 format", m_private->name(), index + 1));
	}

	void redo() override {
		m_formatOld = m_private->yRanges.at(m_index).range.format();
		m_private->yRanges[m_index].range.setFormat(m_format);
		emit m_private->q->yRangeFormatChanged(m_index, m_format);
	}

	void undo() override {
		m_private->yRanges[m_index].range.setFormat(m_formatOld);
		emit m_private->q->yRangeFormatChanged(m_index, m_formatOld);
	}

private:
	CartesianPlotPrivate* m_private;
	RangeT::Format m_format;
	int m_index;
	RangeT::Format m_formatOld;
};

RangeT::Format CartesianPlot::xRangeFormat() const {
	return xRangeFormat(defaultCoordinateSystem()->xIndex());
}
RangeT::Format CartesianPlot::yRangeFormat() const {
	return yRangeFormat(defaultCoordinateSystem()->yIndex());
}
RangeT::Format CartesianPlot::xRangeFormat(const int index) const {
	Q_D(const CartesianPlot);
	if (index < 0 || index > xRangeCount()) {
		DEBUG(Q_FUNC_INFO << ", index " << index << " out of range")
		return RangeT::Format::Numeric;
	}
	return d->xRanges.at(index).range.format();
}
RangeT::Format CartesianPlot::yRangeFormat(const int index) const {
	Q_D(const CartesianPlot);
	if (index < 0 || index > yRangeCount()) {
		DEBUG(Q_FUNC_INFO << ", index " << index << " out of range")
		return RangeT::Format::Numeric;
	}
	return d->yRanges.at(index).range.format();
}
void CartesianPlot::setXRangeFormat(const RangeT::Format format) {
	setXRangeFormat(defaultCoordinateSystem()->xIndex(), format);
}
void CartesianPlot::setYRangeFormat(const RangeT::Format format) {
	setYRangeFormat(defaultCoordinateSystem()->yIndex(), format);
}
void CartesianPlot::setXRangeFormat(const int index, const RangeT::Format format) {
	Q_D(CartesianPlot);
	if (index < 0 || index > xRangeCount()) {
		DEBUG(Q_FUNC_INFO << ", index " << index << " out of range")
		return;
	}
	if (format != xRangeFormat(index)) {
//		d->xRanges[index].setFormat(format);
		exec(new CartesianPlotSetXRangeFormatIndexCmd(d, format, index));
		emit d->xRangeFormatChanged();
		if (project())
			project()->setChanged(true);
	}
}
void CartesianPlot::setYRangeFormat(const int index, const RangeT::Format format) {
	Q_D(CartesianPlot);
	if (index < 0 || index > yRangeCount()) {
		DEBUG(Q_FUNC_INFO << ", index " << index << " out of range")
		return;
	}
	if (format != yRangeFormat(index)) {
//		d->yRanges[index].setFormat(format);
		exec(new CartesianPlotSetYRangeFormatIndexCmd(d, format, index));
		emit d->yRangeFormatChanged();
		if (project())
			project()->setChanged(true);
	}
}

// auto scale

bool CartesianPlot::autoScaleX(int index) const {
	if (index == -1) {
		for (int i = 0; i < xRangeCount(); i++) {
			if (!xRange(i).autoScale())
				return false;
		}
		return true;
	}
	return xRange(index).autoScale();
}
bool CartesianPlot::autoScaleY(int index) const {
	if (index == -1) {
		for (int i = 0; i < yRangeCount(); i++) {
			if (!yRange(i).autoScale())
				return false;
		}
		return true;
	}
	return yRange(index).autoScale();
}

class CartesianPlotSetAutoScaleXIndexCmd : public QUndoCommand {
public:
	CartesianPlotSetAutoScaleXIndexCmd(CartesianPlotPrivate* private_obj, bool autoScale, int index) :
		m_private(private_obj), m_autoScale(autoScale), m_index(index), m_autoScaleOld(false), m_oldRange(0.0, 0.0) {
		setText(i18n("%1: change x-range %2 auto scaling", m_private->name(), m_index + 1));
	}

	void redo() override {
		m_autoScaleOld = m_private->autoScaleX(m_index);
		if (m_autoScale) {
			m_oldRange = m_private->xRange(m_index);
			m_private->q->scaleAutoX(m_index);
		}
		m_private->setAutoScaleX(m_index, m_autoScale);
		emit m_private->q->xAutoScaleChanged(m_index, m_autoScale);
	}

	void undo() override {
		if (!m_autoScaleOld) {
			m_private->xRange(m_index) = m_oldRange;
			m_private->retransformScales(m_index, -1); // TODO: correct?
		}
		m_private->setAutoScaleX(m_index, m_autoScaleOld);
		emit m_private->q->xAutoScaleChanged(m_index, m_autoScaleOld);
	}

private:
	CartesianPlotPrivate* m_private;
	bool m_autoScale;
	int m_index;
	bool m_autoScaleOld;
	Range<double> m_oldRange;
};
class CartesianPlotSetAutoScaleYIndexCmd : public QUndoCommand {
public:
	CartesianPlotSetAutoScaleYIndexCmd(CartesianPlotPrivate* private_obj, bool autoScale, int index) :
		m_private(private_obj), m_autoScale(autoScale), m_index(index), m_autoScaleOld(false), m_oldRange(0.0, 0.0) {
		setText(i18n("%1: change y-range %2 auto scaling", m_private->name(), m_index + 1));
	}

	void redo() override {
		m_autoScaleOld = m_private->autoScaleY(m_index);
		if (m_autoScale) {
			m_oldRange = m_private->yRange(m_index);
			m_private->q->scaleAutoY(m_index);
		}
		m_private->setAutoScaleY(m_index, m_autoScale);
		emit m_private->q->yAutoScaleChanged(m_index, m_autoScale);
	}

	void undo() override {
		if (!m_autoScaleOld) {
			m_private->yRange(m_index) = m_oldRange;
			m_private->retransformScales(-1, m_index);
		}
		m_private->setAutoScaleY(m_index, m_autoScaleOld);
		emit m_private->q->yAutoScaleChanged(m_index, m_autoScaleOld);
	}

private:
	CartesianPlotPrivate* m_private;
	bool m_autoScale;
	int m_index;
	bool m_autoScaleOld;
	Range<double> m_oldRange;
};

// auto scales x range index
void CartesianPlot::setAutoScaleX(int index, const bool autoScaleX) {
	Q_D(CartesianPlot);
	if (index == -1) {
		for (int i = 0; i < xRangeCount(); i++)
			setAutoScaleX(i, autoScaleX);
		return;
	}
	if (autoScaleX != xRange(index).autoScale()) {
		//d->xRanges[index].setAutoScale(autoScaleX);
		// TODO: maybe using the first and then adding the first one as parent to the next undo command
		exec(new CartesianPlotSetAutoScaleXIndexCmd(d, autoScaleX, index));
		if (project())
			project()->setChanged(true);
	}

}
void CartesianPlot::setAutoScaleY(int index, const bool autoScaleY) {
	Q_D(CartesianPlot);
	if (index == -1) {
		for (int i = 0; i < yRangeCount(); i++)
			setAutoScaleY(i, autoScaleY);
		return;
	}
	if (autoScaleY != yRange(index).autoScale()) {
		//d->yRanges[index].setAutoScale(autoScaleY);
		exec(new CartesianPlotSetAutoScaleYIndexCmd(d, autoScaleY, index));
		if (project())
			project()->setChanged(true);
	}
}

int CartesianPlot::xRangeCount() const {
	Q_D(const CartesianPlot);
	return ( d ? d->xRanges.size() : 0 );
}
int CartesianPlot::yRangeCount() const {
	Q_D(const CartesianPlot);
	return ( d ? d->yRanges.size() : 0 );
}
const Range<double>& CartesianPlot::xRange(int index) const {
	if (index == -1)
		index = defaultCoordinateSystem()->xIndex();
	Q_D(const CartesianPlot);
	return d->xRanges.at(index).range;
}
const Range<double>& CartesianPlot::yRange(int index) const {
	if (index == -1)
		index = defaultCoordinateSystem()->yIndex();
	Q_D(const CartesianPlot);
	return d->yRanges.at(index).range;
}
// sets x range of default plot range
void CartesianPlot::setXRange(const Range<double> range) {
	const int index{ defaultCoordinateSystem()->xIndex() };
	setXRange(index, range);
}
// sets y range of default plot range
void CartesianPlot::setYRange(const Range<double> range) {
	const int index{ defaultCoordinateSystem()->yIndex()};
	setYRange(index, range);
}
// set x/y range command with index
//class CartesianPlotSetXRangeIndexCmd: public StandardQVectorSetterCmd<CartesianPlot::Private, Range<double>> {
//public:
//	CartesianPlotSetXRangeIndexCmd(CartesianPlot::Private *target, Range<double> newValue, int index, const KLocalizedString &description)
//		: StandardQVectorSetterCmd<CartesianPlot::Private, Range<double>>(target, &CartesianPlot::Private::xRanges, index, newValue, description) {}
//	virtual void finalize() override { m_target->retransformScales(); emit m_target->q->xRangeChanged((*m_target.*m_field).at(m_index)); }
//};
//class CartesianPlotSetYRangeIndexCmd: public StandardQVectorSetterCmd<CartesianPlot::Private, Range<double>> {
//public:
//	CartesianPlotSetYRangeIndexCmd(CartesianPlot::Private *target, Range<double> newValue, int index, const KLocalizedString &description)
//		: StandardQVectorSetterCmd<CartesianPlot::Private, Range<double>>(target, &CartesianPlot::Private::CoordinateSystemProperties, index, newValue, description) {}
//	virtual void finalize() override { m_target->retransformScales(); emit m_target->q->yRangeChanged((*m_target.*m_field).at(m_index)); }
//};

#define CartesianPlotSetRangeIndexCmd(element) \
class CartesianPlotSet ## element ## IndexCmd: public QUndoCommand \
{ \
public: \
	CartesianPlotSet ## element ## IndexCmd(CartesianPlot::Private *target, Range<double> newValue, int index) \
		: QUndoCommand(), m_target(target), m_otherValue(newValue), m_index(index) {} \
	void redo() override { \
		auto tmp = m_target->element ## s.at(m_index).range; \
		m_target->element ## s[m_index].range = m_otherValue; \
		m_otherValue = tmp; \
		finalize(); \
	} \
	void undo() override { redo(); } \
	virtual void finalize() { m_target->retransformScales(-1, -1); emit m_target->q->element ## Changed(m_index, m_target->element ## s.at(m_index).range); } \
private:\
	CartesianPlot::Private* m_target; \
	Range<double> m_otherValue; \
	int m_index; \
};

CartesianPlotSetRangeIndexCmd(xRange);
void CartesianPlot::setXRange(const int index, const Range<double>& range) {
	Q_D(CartesianPlot);
	if (index >= 0 && index < d->xRanges.count() && range.finite() && range != d->xRanges.at(index).range) {
		d->yRanges[index].dirty = true;
		exec(new CartesianPlotSetxRangeIndexCmd(d, range, index));
		int scaled = 0;
		for (int i = 0; i < coordinateSystemCount(); i++) {
			auto cs = coordinateSystem(i);
			if (cs->xIndex() == index && autoScaleY(cs->yIndex()))
				scaled += scaleAutoY(cs->yIndex(), false, true);
		}
		if (scaled)
			d->retransformScales(index, -1);
	}
}
CartesianPlotSetRangeIndexCmd(yRange)
void CartesianPlot::setYRange(const int index, const Range<double>& range) {
	Q_D(CartesianPlot);
	if (index >= 0 && index < d->yRanges.count() && range.finite() && range != d->yRanges.at(index).range) {
		exec(new CartesianPlotSetyRangeIndexCmd(d, range, index));
		int scaled = 0;
		for (int i = 0; i < coordinateSystemCount(); i++) {
			auto cs = coordinateSystem(i);
			if (cs->yIndex() == index && autoScaleX(cs->xIndex()))
				scaled += scaleAutoX(cs->xIndex(), false, true);
		}
		if (scaled)
			d->retransformScales(-1, index); // TODO: when only one cSystem uses this range not all must be retransformed
	}
}

const Range<double>& CartesianPlot::dataXRange(int index) {
	if (index == -1)
		index = defaultCoordinateSystem()->xIndex();

	if (xRangeDirty(index))
		calculateDataXRange(index, true);

	Q_D(CartesianPlot);
	return d->dataXRange(index);
}
const Range<double>& CartesianPlot::dataYRange(int index) {
	if (index == -1)
		index = defaultCoordinateSystem()->yIndex();

	if (yRangeDirty(index))
		calculateDataYRange(index, true);

	Q_D(CartesianPlot);
	return d->dataYRange(index);
}

bool CartesianPlot::xRangeDirty(int index) {
	Q_D(CartesianPlot);
	return d->xRanges.at(index).dirty;
}
bool CartesianPlot::yRangeDirty(int index) {
	Q_D(CartesianPlot);
	return d->yRanges.at(index).dirty;
}

void CartesianPlot::setXRangeDirty(int index, bool dirty) {
	Q_D(CartesianPlot);
	if (index >= 0 && index < xRangeCount())
		d->xRanges[index].dirty = dirty;
	else {
		for (auto r : d->xRanges)
			r.dirty = dirty;
	}
}
void CartesianPlot::setYRangeDirty(int index, bool dirty) {
	Q_D(CartesianPlot);
	if (index >= 0 && index < yRangeCount())
		d->yRanges[index].dirty = dirty;
	else {
		for (auto r : d->yRanges)
			r.dirty = dirty;
	}
}

void CartesianPlot::addXRange() {
	Q_D(CartesianPlot);
	d->xRanges.append(CartesianPlot::Private::RichRange());
	if (project())
		project()->setChanged(true);
}
void CartesianPlot::addYRange() {
	Q_D(CartesianPlot);
	d->yRanges.append(CartesianPlot::Private::RichRange());
	if (project())
		project()->setChanged(true);
}
void CartesianPlot::addXRange(const Range<double>& range) {
	Q_D(CartesianPlot);
	d->xRanges.append(CartesianPlot::Private::RichRange(range));
	if (project())
		project()->setChanged(true);
}
void CartesianPlot::addYRange(const Range<double>& range) {
	Q_D(CartesianPlot);
	d->yRanges.append(CartesianPlot::Private::RichRange(range));
	if (project())
		project()->setChanged(true);
}

void CartesianPlot::removeXRange(int index) {
	Q_D(CartesianPlot);
	if (index < 0 || index > xRangeCount()) {
		DEBUG(Q_FUNC_INFO << ", index " << index << " out of range")
		return;
	}
	d->xRanges.remove(index);
	if (project())
		project()->setChanged(true);
}
void CartesianPlot::removeYRange(int index) {
	Q_D(CartesianPlot);
	if (index < 0 || index > yRangeCount()) {
		DEBUG(Q_FUNC_INFO << ", index " << index << " out of range")
		return;
	}
	d->yRanges.remove(index);
	if (project())
		project()->setChanged(true);
}
void CartesianPlot::setXMin(const int index, const double value) {
	DEBUG(Q_FUNC_INFO << ", value = " << value)
	Range<double> range{ xRange(index) };
	range.setStart(value);
	DEBUG(Q_FUNC_INFO << ", new range = " << range.toStdString())

	setXRange(index, range);
}
void CartesianPlot::setXMax(const int index, const double value) {
	DEBUG(Q_FUNC_INFO)
	Range<double> range{ xRange(index) };
	range.setEnd(value);

	setXRange(index, range);
}
void CartesianPlot::setYMin(const int index, const double value) {
	DEBUG(Q_FUNC_INFO)
	Range<double> range{ yRange(index) };
	range.setStart(value);

	setYRange(index, range);
}
void CartesianPlot::setYMax(const int index, const double value) {
	Range<double> range{ yRange(index) };
	range.setEnd(value);

	setYRange(index, range);
}

// x/y scale

class CartesianPlotSetXScaleIndexCmd : public QUndoCommand {
public:
	CartesianPlotSetXScaleIndexCmd(CartesianPlotPrivate* private_obj, RangeT::Scale scale, int index) :
		m_private(private_obj), m_scale(scale), m_index(index) {
		setText(i18n("%1: change x-range %2 scale", m_private->name(), index + 1));
	}

	void redo() override {
		m_scaleOld = m_private->xRanges.at(m_index).range.scale();
		m_private->xRanges[m_index].range.setScale(m_scale);
		emit m_private->q->xScaleChanged(m_index, m_scale);
	}

	void undo() override {
		m_private->xRanges[m_index].range.setScale(m_scaleOld);
		emit m_private->q->xScaleChanged(m_index, m_scaleOld);
	}

private:
	CartesianPlotPrivate* m_private;
	RangeT::Scale m_scale;
	int m_index;
	RangeT::Scale m_scaleOld;
};
class CartesianPlotSetYScaleIndexCmd : public QUndoCommand {
public:
	CartesianPlotSetYScaleIndexCmd(CartesianPlotPrivate* private_obj, RangeT::Scale scale, int index) :
		m_private(private_obj), m_scale(scale), m_index(index) {
		setText(i18n("%1: change x-range %2 scale", m_private->name(), index + 1));
	}

	void redo() override {
		m_scaleOld = m_private->yRanges.at(m_index).range.scale();
		m_private->yRanges[m_index].range.setScale(m_scale);
		emit m_private->q->yScaleChanged(m_index, m_scale);
	}

	void undo() override {
		m_private->yRanges[m_index].range.setScale(m_scaleOld);
		emit m_private->q->yScaleChanged(m_index, m_scaleOld);
	}

private:
	CartesianPlotPrivate* m_private;
	RangeT::Scale m_scale;
	int m_index;
	RangeT::Scale m_scaleOld;
};

RangeT::Scale CartesianPlot::xRangeScale() const {
	return xRangeScale(defaultCoordinateSystem()->xIndex());
}
RangeT::Scale CartesianPlot::yRangeScale() const {
	return yRangeScale(defaultCoordinateSystem()->yIndex());
}
RangeT::Scale CartesianPlot::xRangeScale(const int index) const {
	if (index < 0 || index > xRangeCount()) {
		DEBUG(Q_FUNC_INFO << ", index " << index << " out of range")
		return RangeT::Scale::Linear;
	}
	return xRange(index).scale();
}
RangeT::Scale CartesianPlot::yRangeScale(const int index) const {
	if (index < 0 || index > yRangeCount()) {
		DEBUG(Q_FUNC_INFO << ", index " << index << " out of range")
		return RangeT::Scale::Linear;
	}
	return yRange(index).scale();
}
void CartesianPlot::setXRangeScale(const RangeT::Scale scale) {
	setXRangeScale(defaultCoordinateSystem()->xIndex(), scale);
}

void CartesianPlot::setYRangeScale(const RangeT::Scale scale) {
	setYRangeScale(defaultCoordinateSystem()->yIndex(), scale);
}

void CartesianPlot::setXRangeScale(const int index, const RangeT::Scale scale) {
	Q_D(CartesianPlot);
	if (index < 0 || index > xRangeCount()) {
		DEBUG(Q_FUNC_INFO << ", index " << index << " out of range")
		return;
	}
//	d->xRanges[index].setScale(scale);
	exec(new CartesianPlotSetXScaleIndexCmd(d, scale, index));
	d->retransformScales(index, -1); // TODO: check which scales must be updated
	if (project())
		project()->setChanged(true);
}
void CartesianPlot::setYRangeScale(const int index, const RangeT::Scale scale) {
	Q_D(CartesianPlot);
	if (index < 0 || index > yRangeCount()) {
		DEBUG(Q_FUNC_INFO << ", index " << index << " out of range")
		return;
	}
//	d->yRanges[index].setScale(scale);
	exec(new CartesianPlotSetYScaleIndexCmd(d, scale, index));
	d->retransformScales(-1, index); // TODO: check which indices must be retransformed
	if (project())
		project()->setChanged(true);
}

// coordinate systems

int CartesianPlot::coordinateSystemCount() const {
	return m_coordinateSystems.size();
}

CartesianCoordinateSystem* CartesianPlot::coordinateSystem(int index) const {
	//DEBUG(Q_FUNC_INFO << ", nr of cSystems = " << coordinateSystemCount() << ", index = " << index)
	if (index >= coordinateSystemCount() || index < 0)
		return nullptr;

	return dynamic_cast<CartesianCoordinateSystem*>(m_coordinateSystems.at(index));
}

void CartesianPlot::addCoordinateSystem() {
	addCoordinateSystem(new CartesianCoordinateSystem(this));
}
void CartesianPlot::addCoordinateSystem(CartesianCoordinateSystem* s) {
	m_coordinateSystems.append(s);
	if (project())
		project()->setChanged(true);
}
void CartesianPlot::removeCoordinateSystem(int index) {
	if (index < 0 || index > coordinateSystemCount()) {
		DEBUG(Q_FUNC_INFO << ", index " << index << " out of range")
		return;
	}

	// TODO: deleting cSystem?
	m_coordinateSystems.remove(index);
	if (project())
		project()->setChanged(true);
}

STD_SETTER_CMD_IMPL_F_S_Arguments(CartesianPlot, SetDefaultCoordinateSystemIndex, int, defaultCoordinateSystemIndex, retransformScales(-1, -1))
int CartesianPlot::defaultCoordinateSystemIndex() const {
	Q_D(const CartesianPlot);
	return d->defaultCoordinateSystemIndex;
}
void CartesianPlot::setDefaultCoordinateSystemIndex(int index) {
        Q_D(CartesianPlot);
	if (index != d->defaultCoordinateSystemIndex) {
		exec(new CartesianPlotSetDefaultCoordinateSystemIndexCmd(d, index, ki18n("%1: set default plot range")));
		d->retransform(); //TODO: check
	}
}
CartesianCoordinateSystem* CartesianPlot::defaultCoordinateSystem() const {
	Q_D(const CartesianPlot);
	return d->defaultCoordinateSystem();
}

// range breaks

STD_SETTER_CMD_IMPL_F_S_Arguments(CartesianPlot, SetXRangeBreakingEnabled, bool, xRangeBreakingEnabled, retransformScales(-1, -1))
void CartesianPlot::setXRangeBreakingEnabled(bool enabled) {
	Q_D(CartesianPlot);
	if (enabled != d->xRangeBreakingEnabled)
		exec(new CartesianPlotSetXRangeBreakingEnabledCmd(d, enabled, ki18n("%1: x-range breaking enabled")));
}

STD_SETTER_CMD_IMPL_F_S_Arguments(CartesianPlot, SetXRangeBreaks, CartesianPlot::RangeBreaks, xRangeBreaks, retransformScales(-1, -1))
void CartesianPlot::setXRangeBreaks(const RangeBreaks& breakings) {
	Q_D(CartesianPlot);
	exec(new CartesianPlotSetXRangeBreaksCmd(d, breakings, ki18n("%1: x-range breaks changed")));
}

STD_SETTER_CMD_IMPL_F_S_Arguments(CartesianPlot, SetYRangeBreakingEnabled, bool, yRangeBreakingEnabled, retransformScales(-1, -1))
void CartesianPlot::setYRangeBreakingEnabled(bool enabled) {
	Q_D(CartesianPlot);
	if (enabled != d->yRangeBreakingEnabled)
		exec(new CartesianPlotSetYRangeBreakingEnabledCmd(d, enabled, ki18n("%1: y-range breaking enabled")));
}

STD_SETTER_CMD_IMPL_F_S_Arguments(CartesianPlot, SetYRangeBreaks, CartesianPlot::RangeBreaks, yRangeBreaks, retransformScales(-1, -1))
void CartesianPlot::setYRangeBreaks(const RangeBreaks& breaks) {
	Q_D(CartesianPlot);
	exec(new CartesianPlotSetYRangeBreaksCmd(d, breaks, ki18n("%1: y-range breaks changed")));
}

// cursor

STD_SETTER_CMD_IMPL_F_S(CartesianPlot, SetCursorPen, QPen, cursorPen, update)
void CartesianPlot::setCursorPen(const QPen &pen) {
	Q_D(CartesianPlot);
	if (pen != d->cursorPen)
		exec(new CartesianPlotSetCursorPenCmd(d, pen, ki18n("%1: y-range breaks changed")));
}

STD_SETTER_CMD_IMPL_F_S(CartesianPlot, SetCursor0Enable, bool, cursor0Enable, updateCursor)
void CartesianPlot::setCursor0Enable(const bool &enable) {
	Q_D(CartesianPlot);
	if (enable != d->cursor0Enable) {
		if (std::isnan(d->cursor0Pos.x())) { // if never set, set initial position
			d->cursor0Pos.setX(defaultCoordinateSystem()->mapSceneToLogical(QPointF(0, 0)).x());
			mousePressCursorModeSignal(0, d->cursor0Pos); // simulate mousePress to update values in the cursor dock
		}
		exec(new CartesianPlotSetCursor0EnableCmd(d, enable, ki18n("%1: Cursor0 enable")));
	}
}

STD_SETTER_CMD_IMPL_F_S(CartesianPlot, SetCursor1Enable, bool, cursor1Enable, updateCursor)
void CartesianPlot::setCursor1Enable(const bool &enable) {
	Q_D(CartesianPlot);
	if (enable != d->cursor1Enable) {
		if (std::isnan(d->cursor1Pos.x())) { // if never set, set initial position
			d->cursor1Pos.setX(defaultCoordinateSystem()->mapSceneToLogical(QPointF(0, 0)).x());
			mousePressCursorModeSignal(1, d->cursor1Pos); // simulate mousePress to update values in the cursor dock
		}
		exec(new CartesianPlotSetCursor1EnableCmd(d, enable, ki18n("%1: Cursor1 enable")));
	}
}

// theme

STD_SETTER_CMD_IMPL_S(CartesianPlot, SetTheme, QString, theme)
void CartesianPlot::setTheme(const QString& theme) {
	Q_D(CartesianPlot);
	if (theme != d->theme) {
		QString info;
		if (!theme.isEmpty())
			info = i18n("%1: load theme %2", name(), theme);
		else
			info = i18n("%1: load default theme", name());
		beginMacro(info);
		exec(new CartesianPlotSetThemeCmd(d, theme, ki18n("%1: set theme")));
		loadTheme(theme);
		endMacro();
	}
}

//################################################################
//########################## Slots ###############################
//################################################################
void CartesianPlot::addHorizontalAxis() {
	DEBUG(Q_FUNC_INFO)
	Axis* axis = new Axis("x-axis", Axis::Orientation::Horizontal);
	axis->setSuppressRetransform(true);	// retransformTicks() needs plot
	axis->setCoordinateSystemIndex(defaultCoordinateSystemIndex());
	if (axis->rangeType() == Axis::RangeType::Auto) {
		axis->setUndoAware(false);
		// use x range of default plot range
		axis->setRange( xRange() );
		axis->setMajorTicksNumber( xRange().autoTickCount() );
		axis->setUndoAware(true);
	}
	addChild(axis);
	axis->setSuppressRetransform(false);
	axis->retransform();
}

void CartesianPlot::addVerticalAxis() {
	Axis* axis = new Axis("y-axis", Axis::Orientation::Vertical);
	axis->setSuppressRetransform(true);	// retransformTicks() needs plot
	axis->setCoordinateSystemIndex(defaultCoordinateSystemIndex());
	if (axis->rangeType() == Axis::RangeType::Auto) {
		axis->setUndoAware(false);
		// use y range of default plot range
		axis->setRange( yRange() );
		axis->setMajorTicksNumber( yRange().autoTickCount() );
		axis->setUndoAware(true);
	}
	addChild(axis);
	axis->setSuppressRetransform(false);
	axis->retransform();
}

void CartesianPlot::addCurve() {
	DEBUG(Q_FUNC_INFO)
	auto* curve{ new XYCurve("xy-curve") };
	curve->setCoordinateSystemIndex(defaultCoordinateSystemIndex());
	addChild(curve);
}

void CartesianPlot::addEquationCurve() {
	DEBUG(Q_FUNC_INFO << ", to default coordinate system " << defaultCoordinateSystemIndex())
	auto* curve{ new XYEquationCurve("f(x)") };
	curve->setCoordinateSystemIndex(defaultCoordinateSystemIndex());
	addChild(curve);
}

void CartesianPlot::addHistogram() {
	DEBUG(Q_FUNC_INFO << ", to default coordinate system " << defaultCoordinateSystemIndex())
	auto* hist{ new Histogram("Histogram") };
	hist->setCoordinateSystemIndex(defaultCoordinateSystemIndex());
	addChild(hist);
}

void CartesianPlot::addHistogramFit(Histogram* hist, nsl_sf_stats_distribution type) {
	if(!hist)
		return;

	beginMacro( i18n("%1: distribution fit to '%2'", name(), hist->name()) );
	auto* curve = new XYFitCurve(i18n("Distribution Fit to '%1'", hist->name()));
	curve->setCoordinateSystemIndex(defaultCoordinateSystemIndex());
	curve->setDataSourceType(XYAnalysisCurve::DataSourceType::Histogram);
	curve->setDataSourceHistogram(hist);

	// set fit model category and type and initialize fit
	XYFitCurve::FitData fitData = curve->fitData();
	fitData.modelCategory = nsl_fit_model_distribution;
	fitData.modelType = (int)type;
	XYFitCurve::initFitData(fitData);
	curve->setFitData(fitData);

	curve->recalculate();

	//add the child after the fit was calculated so the dock widgets gets the fit results
	//and call retransform() after this to calculate and to paint the data points of the fit-curve
	this->addChild(curve);
	curve->retransform();

	endMacro();
}

void CartesianPlot::addBoxPlot() {
	addChild(new BoxPlot("Box Plot"));
}

/*!
 * returns the first selected XYCurve in the plot
 */
const XYCurve* CartesianPlot::currentCurve() const {
	for (const auto* curve : children<const XYCurve>()) {
		if (curve->graphicsItem()->isSelected())
			return curve;
	}

	return nullptr;
}

void CartesianPlot::addDataReductionCurve() {
	auto* curve = new XYDataReductionCurve("Data reduction");
	const XYCurve* curCurve = currentCurve();
	curve->setCoordinateSystemIndex(defaultCoordinateSystemIndex());
	if (curCurve) {
		beginMacro( i18n("%1: reduce '%2'", name(), curCurve->name()) );
		curve->setName( i18n("Reduction of '%1'", curCurve->name()) );
		curve->setDataSourceType(XYAnalysisCurve::DataSourceType::Curve);
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
	auto* curve = new XYDifferentiationCurve("Differentiation");
	const XYCurve* curCurve = currentCurve();
	curve->setCoordinateSystemIndex(defaultCoordinateSystemIndex());
	if (curCurve) {
		beginMacro( i18n("%1: differentiate '%2'", name(), curCurve->name()) );
		curve->setName( i18n("Derivative of '%1'", curCurve->name()) );
		curve->setDataSourceType(XYAnalysisCurve::DataSourceType::Curve);
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
	auto* curve = new XYIntegrationCurve("Integration");
	const XYCurve* curCurve = currentCurve();
	curve->setCoordinateSystemIndex(defaultCoordinateSystemIndex());
	if (curCurve) {
		beginMacro( i18n("%1: integrate '%2'", name(), curCurve->name()) );
		curve->setName( i18n("Integral of '%1'", curCurve->name()) );
		curve->setDataSourceType(XYAnalysisCurve::DataSourceType::Curve);
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
	auto* curve = new XYInterpolationCurve("Interpolation");
	const XYCurve* curCurve = currentCurve();
	curve->setCoordinateSystemIndex(defaultCoordinateSystemIndex());
	if (curCurve) {
		beginMacro( i18n("%1: interpolate '%2'", name(), curCurve->name()) );
		curve->setName( i18n("Interpolation of '%1'", curCurve->name()) );
		curve->setDataSourceType(XYAnalysisCurve::DataSourceType::Curve);
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
	auto* curve = new XYSmoothCurve("Smooth");
	const XYCurve* curCurve = currentCurve();
	curve->setCoordinateSystemIndex(defaultCoordinateSystemIndex());
	if (curCurve) {
		beginMacro( i18n("%1: smooth '%2'", name(), curCurve->name()) );
		curve->setName( i18n("Smoothing of '%1'", curCurve->name()) );
		curve->setDataSourceType(XYAnalysisCurve::DataSourceType::Curve);
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
	auto* curve = new XYFitCurve("fit");
	const auto* curCurve = currentCurve();
	curve->setCoordinateSystemIndex(defaultCoordinateSystemIndex());
	if (curCurve) {
		beginMacro( i18n("%1: fit to '%2'", name(), curCurve->name()) );
		curve->setName( i18n("Fit to '%1'", curCurve->name()) );
		curve->setDataSourceType(XYAnalysisCurve::DataSourceType::Curve);
		curve->setDataSourceCurve(curCurve);

		//set the fit model category and type
		const auto* action = qobject_cast<const QAction*>(QObject::sender());
		if (action) {
			auto type = static_cast<XYAnalysisCurve::AnalysisAction>(action->data().toInt());
			curve->initFitData(type);
		} else {
			DEBUG(Q_FUNC_INFO << "WARNING: no action found!")
		}
		curve->initStartValues(curCurve);

		//fit with weights for y if the curve has error bars for y
		if (curCurve->yErrorType() == XYCurve::ErrorType::Symmetric && curCurve->yErrorPlusColumn()) {
			auto fitData = curve->fitData();
			fitData.yWeightsType = nsl_fit_weight_instrumental;
			curve->setFitData(fitData);
			curve->setYErrorColumn(curCurve->yErrorPlusColumn());
		}

		curve->recalculate();

		//add the child after the fit was calculated so the dock widgets gets the fit results
		//and call retransform() after this to calculate and to paint the data points of the fit-curve
		this->addChild(curve);
		curve->retransform();
	} else {
		beginMacro(i18n("%1: add fit curve", name()));
		this->addChild(curve);
	}

	endMacro();
}

void CartesianPlot::addFourierFilterCurve() {
	auto* curve = new XYFourierFilterCurve("Fourier filter");
	const XYCurve* curCurve = currentCurve();
	if (curCurve) {
		beginMacro( i18n("%1: Fourier filtering of '%2'", name(), curCurve->name()) );
		curve->setName( i18n("Fourier filtering of '%1'", curCurve->name()) );
		curve->setDataSourceType(XYAnalysisCurve::DataSourceType::Curve);
		curve->setDataSourceCurve(curCurve);
	} else {
		beginMacro(i18n("%1: add Fourier filter curve", name()));
	}
	curve->setCoordinateSystemIndex(defaultCoordinateSystemIndex());
	this->addChild(curve);

	endMacro();
}

void CartesianPlot::addFourierTransformCurve() {
	auto* curve = new XYFourierTransformCurve("Fourier transform");
	curve->setCoordinateSystemIndex(defaultCoordinateSystemIndex());
	this->addChild(curve);
}

void CartesianPlot::addHilbertTransformCurve() {
	auto* curve = new XYHilbertTransformCurve("Hilbert transform");
	curve->setCoordinateSystemIndex(defaultCoordinateSystemIndex());
	this->addChild(curve);
}

void CartesianPlot::addConvolutionCurve() {
	auto* curve = new XYConvolutionCurve("Convolution");
	curve->setCoordinateSystemIndex(defaultCoordinateSystemIndex());
	this->addChild(curve);
}

void CartesianPlot::addCorrelationCurve() {
	auto* curve = new XYCorrelationCurve("Auto-/Cross-Correlation");
	curve->setCoordinateSystemIndex(defaultCoordinateSystemIndex());
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

	m_legend = new CartesianPlotLegend("legend");
	this->addChild(m_legend);
	m_legend->retransform();

	//only one legend is allowed -> disable the action
	if (m_menusInitialized)
		addLegendAction->setEnabled(false);
}

void CartesianPlot::addInfoElement() {
	XYCurve* curve = nullptr;
	auto curves = children<XYCurve>();
	if (curves.count())
		curve = curves.first();

	double pos;
	Q_D(CartesianPlot);
	if (d->calledFromContextMenu) {
		pos = d->logicalPos.x();
		d->calledFromContextMenu = false;
	} else
		pos = xRange().center();

	auto* element = new InfoElement("Info Element", this, curve, pos);
	this->addChild(element);
	element->setParentGraphicsItem(graphicsItem());
	element->retransform(); // must be done, because the element must be retransformed (see https://invent.kde.org/marmsoler/labplot/issues/9)
}

void CartesianPlot::addTextLabel() {
	auto* label = new TextLabel("text label", this);

	Q_D(CartesianPlot);
	if (d->calledFromContextMenu)  {
		auto position = label->position();
		position.point = label->parentPosToRelativePos(d->scenePos,
							d->boundingRect(),
							label->graphicsItem()->boundingRect(),
							position,
							label->horizontalAlignment(),
							label->verticalAlignment()
		);
		label->setPosition(position);
		d->calledFromContextMenu = false;
	}

	this->addChild(label);
	label->setParentGraphicsItem(graphicsItem());
}

void CartesianPlot::addImage() {
	auto* image = new Image("image");

	Q_D(CartesianPlot);
	if (d->calledFromContextMenu)  {
		auto position = image->position();
		position.point = image->parentPosToRelativePos(d->scenePos,
							d->boundingRect(),
							image->graphicsItem()->boundingRect(),
							position,
							image->horizontalAlignment(),
							image->verticalAlignment()
		);
		image->setPosition(position);
		d->calledFromContextMenu = false;
	}

	//make the new image somewhat smaller so it's completely visible also on smaller plots
	image->setWidth((int)Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Centimeter));

	this->addChild(image);
	image->retransform();
}

void CartesianPlot::addCustomPoint() {
	Q_D(CartesianPlot);
	auto* point = new CustomPoint(this, "custom point");
	point->setCoordinateSystemIndex(defaultCoordinateSystemIndex());

	if (d->calledFromContextMenu) {
		point->setPosition(d->logicalPos);
		d->calledFromContextMenu = false;
	}

	this->addChild(point);
	point->retransform();
}

void CartesianPlot::addReferenceLine() {
	Q_D(CartesianPlot);
	auto* line = new ReferenceLine(this, "reference line");
	line->setCoordinateSystemIndex(defaultCoordinateSystemIndex());

	if (d->calledFromContextMenu)  {
		if (line->orientation() == WorksheetElement::Orientation::Vertical)
			line->setPosition(d->logicalPos.x());
		else
			line->setPosition(d->logicalPos.y());
		d->calledFromContextMenu = false;
	}

	this->addChild(line);
	line->retransform();
}

int CartesianPlot::curveCount() {
	return children<XYCurve>().size();
}

int CartesianPlot::curveTotalCount() const {
	int count = children<XYCurve>().size();
	count += children<Histogram>().size();
	count += children<BoxPlot>().size();
	return count;
}

const XYCurve* CartesianPlot::getCurve(int index) {
	return children<XYCurve>().at(index);
}

double CartesianPlot::cursorPos(int cursorNumber) {
	Q_D(const CartesianPlot);
	return ( cursorNumber == 0 ? d->cursor0Pos.x() : d->cursor1Pos.x() );
}

/*!
 * returns the index of the child \c curve in the list of all "curve-like"
 * children (xy-curve, histogram, boxplot, etc.).
 * This function is used when applying the theme color to the newly added "curve".:
 */
int CartesianPlot::curveChildIndex(const WorksheetElement* curve) const {
	int index = 0;
	const auto& children = this->children<WorksheetElement>();
	for (auto* child : children) {
		if (child == curve)
			break;

		if (child->inherits(AspectType::XYCurve)
			|| child->type() == AspectType::Histogram
			|| child->type() == AspectType::BoxPlot)
		++index;
	}

	return index;
}

void CartesianPlot::childAdded(const AbstractAspect* child) {
	Q_D(CartesianPlot);

	const auto* curve = qobject_cast<const XYCurve*>(child);
	int cSystemIndex = -1;
	if (curve) {
		DEBUG(Q_FUNC_INFO << ", CURVE")
		//x and y data
		connect(curve, &XYCurve::dataChanged, this, [this, curve]() {
			auto cs = coordinateSystem(curve->coordinateSystemIndex());
			this->dataChanged(cs->xIndex(), cs->yIndex(), const_cast<XYCurve*>(curve));
		});

		//x data
		connect(curve, &XYCurve::xColumnChanged, this, [this](const AbstractColumn* column) {
			if (curveTotalCount() == 1) //first curve addded
				checkAxisFormat(column, Axis::Orientation::Horizontal);
		});
		connect(curve, &XYCurve::xDataChanged, [this, curve] () {this->xDataChanged(const_cast<XYCurve*>(curve));});
		connect(curve, &XYCurve::xErrorTypeChanged, [this, curve] () {this->xDataChanged(const_cast<XYCurve*>(curve));});
		connect(curve, &XYCurve::xErrorPlusColumnChanged, [this, curve] () {this->xDataChanged(const_cast<XYCurve*>(curve));});
		connect(curve, &XYCurve::xErrorMinusColumnChanged, [this, curve] () {this->xDataChanged(const_cast<XYCurve*>(curve));});

		//y data
		connect(curve, &XYCurve::yColumnChanged, this, [this](const AbstractColumn* column) {
			if (curveTotalCount() == 1)
				checkAxisFormat(column, Axis::Orientation::Vertical);
		});
		connect(curve, &XYCurve::yDataChanged, [this, curve] () {this->yDataChanged(const_cast<XYCurve*>(curve));});
		connect(curve, &XYCurve::yErrorTypeChanged, [this, curve] () {this->yDataChanged(const_cast<XYCurve*>(curve));});
		connect(curve, &XYCurve::yErrorPlusColumnChanged, [this, curve] () {this->yDataChanged(const_cast<XYCurve*>(curve));});
		connect(curve, &XYCurve::yErrorMinusColumnChanged, [this, curve] () {this->yDataChanged(const_cast<XYCurve*>(curve));});

		//visibility
		connect(curve, QOverload<bool>::of(&XYCurve::visibilityChanged),this, &CartesianPlot::curveVisibilityChanged);

		//update the legend on changes of the name, line and symbol styles
		connect(curve, &XYCurve::aspectDescriptionChanged, this, &CartesianPlot::updateLegend);
		connect(curve, &XYCurve::aspectDescriptionChanged, this, &CartesianPlot::curveNameChanged);
		connect(curve, &XYCurve::legendVisibleChanged, this, &CartesianPlot::updateLegend);
		connect(curve, &XYCurve::lineTypeChanged, this, &CartesianPlot::updateLegend);
		connect(curve, &XYCurve::linePenChanged, this, &CartesianPlot::updateLegend);
		connect(curve, &XYCurve::linePenChanged, this, QOverload<QPen>::of(&CartesianPlot::curveLinePenChanged)); // forward to Worksheet to update CursorDock
		connect(curve, &XYCurve::lineOpacityChanged, this, &CartesianPlot::updateLegend);
		connect(curve->symbol(), &Symbol::updateRequested, this, &CartesianPlot::updateLegend);

		updateLegend();

		//in case the first curve is added, check whether we start plotting datetime data
		if (curveTotalCount() == 1) {
			checkAxisFormat(curve->xColumn(), Axis::Orientation::Horizontal);
			checkAxisFormat(curve->yColumn(), Axis::Orientation::Vertical);
		}

		cSystemIndex = curve->coordinateSystemIndex();
		emit curveAdded(curve);

	} else {
		const auto* hist = qobject_cast<const Histogram*>(child);
		if (hist) {
			DEBUG(Q_FUNC_INFO << ", HISTOGRAM")
			// TODO: check if all ranges must be updated
			connect(hist, &Histogram::dataChanged, [this, hist] {this->dataChanged(-1, -1, const_cast<Histogram*>(hist));});
			connect(hist, &Histogram::visibilityChanged, this, &CartesianPlot::curveVisibilityChanged);
			connect(hist, &BoxPlot::aspectDescriptionChanged, this, &CartesianPlot::updateLegend);

			updateLegend();
			cSystemIndex = hist->coordinateSystemIndex();

			if (curveTotalCount() == 1)
				checkAxisFormat(hist->dataColumn(), Axis::Orientation::Horizontal);
		}

		const auto* boxPlot = qobject_cast<const BoxPlot*>(child);
		if (boxPlot) {
			DEBUG(Q_FUNC_INFO << ", BOX PLOT")
			// TODO: check if all ranges must be updated
			connect(boxPlot, &BoxPlot::dataChanged, [this, boxPlot] {this->dataChanged(-1, -1, const_cast<BoxPlot*>(boxPlot));});
			connect(boxPlot, &BoxPlot::visibilityChanged, this, &CartesianPlot::curveVisibilityChanged);
			connect(boxPlot, &BoxPlot::aspectDescriptionChanged, this, &CartesianPlot::updateLegend);

			updateLegend();
			cSystemIndex = boxPlot->coordinateSystemIndex();

			if (curveTotalCount() == 1) {
				connect(boxPlot, &BoxPlot::orientationChanged, this, &CartesianPlot::boxPlotOrientationChanged);
				boxPlotOrientationChanged(boxPlot->orientation());
			}
		}

		const auto* infoElement = qobject_cast<const InfoElement*>(child);
		if (infoElement)
			connect(this, &CartesianPlot::curveRemoved, infoElement, &InfoElement::removeCurve);

		// if an element is hovered, the curves which are handled manually in this class
		// must be unhovered
		const auto* wsElement = qobject_cast<const WorksheetElement*>(child);
		if (wsElement)
			connect(wsElement, &WorksheetElement::hovered, this, &CartesianPlot::childHovered);
	}

	if (INRANGE(cSystemIndex, 0, m_coordinateSystems.count())) {
		setXRangeDirty(cSystemIndex, true);
		setYRangeDirty(cSystemIndex, true);

		autoScale(-1, -1);	// update all plot ranges
	}

	if (!isLoading() && !this->pasted() && !child->pasted() && !child->isMoved()) {
		//if a theme was selected, apply the theme settings for newly added children,
		//load default theme settings otherwise.
		const auto* elem = qobject_cast<const WorksheetElement*>(child);
		if (elem) {
//TODO			const_cast<WorksheetElement*>(elem)->setCoordinateSystemIndex(defaultCoordinateSystemIndex());
			if (!d->theme.isEmpty()) {
				KConfig config(ThemeHandler::themeFilePath(d->theme), KConfig::SimpleConfig);
				const_cast<WorksheetElement*>(elem)->loadThemeConfig(config);
			} else {
				KConfig config;
				const_cast<WorksheetElement*>(elem)->loadThemeConfig(config);
			}
		}
	}
}

// set format of axis from data column
void CartesianPlot::checkAxisFormat(const AbstractColumn* column, Axis::Orientation orientation) {
	const auto* col = qobject_cast<const Column*>(column);
	if (!col)
		return;

	Q_D(CartesianPlot);
	if (col->columnMode() == AbstractColumn::ColumnMode::DateTime) {
		setUndoAware(false);
		if (orientation == Axis::Orientation::Horizontal)
			setXRangeFormat(RangeT::Format::DateTime);
		else
			setYRangeFormat(RangeT::Format::DateTime);
		setUndoAware(true);

		//set column's datetime format for all horizontal axis
		for (auto* axis : children<Axis>()) {
			const auto* cSystem{ coordinateSystem(axis->coordinateSystemIndex()) };
			if (axis->orientation() == orientation) {
				const auto* filter = static_cast<DateTime2StringFilter*>(col->outputFilter());
				d->xRanges[cSystem->xIndex()].range.setDateTimeFormat(filter->format());
				axis->setUndoAware(false);
				axis->setLabelsDateTimeFormat(xRangeDateTimeFormat());
				axis->setUndoAware(true);
			}
		}
	} else {
		setUndoAware(false);
		if (orientation == Axis::Orientation::Horizontal)
			setXRangeFormat(RangeT::Format::Numeric);
		else
			setYRangeFormat(RangeT::Format::Numeric);

		setUndoAware(true);
	}
}

void CartesianPlot::boxPlotOrientationChanged(BoxPlot::Orientation orientation) {
	const auto& axes = children<Axis>();

	//don't show any labels for the first axis orthogonal to the orientation of the boxplot
	for (auto* axis : axes) {
		if (axis->orientation() != orientation) {
			if (axis->labelsTextType() != Axis::LabelsTextType::CustomValues) {
				axis->setUndoAware(false);
				axis->setLabelsPosition(Axis::LabelsPosition::NoLabels);
				axis->setUndoAware(true);
			}
			break;
		}
	}

	//don't show any labels for the first axis parallel to the orientation of the boxplot
	for (auto* axis : axes) {
		if (axis->orientation() == orientation) {
			if (axis->labelsTextType() != Axis::LabelsTextType::CustomValues) {
				axis->setUndoAware(false);
				axis->setLabelsPosition(Axis::LabelsPosition::Out);
				axis->setUndoAware(true);
			}
			break;
		}
	}

}

void CartesianPlot::childRemoved(const AbstractAspect* /*parent*/, const AbstractAspect* /*before*/, const AbstractAspect* child) {
	if (m_legend == child) {
		DEBUG(Q_FUNC_INFO << ", a legend")
		if (m_menusInitialized)
			addLegendAction->setEnabled(true);
		m_legend = nullptr;
	} else {
		const auto* curve = qobject_cast<const XYCurve*>(child);
		if (curve) {
			DEBUG(Q_FUNC_INFO << ", a curve")
			updateLegend();
			emit curveRemoved(curve);
			auto cs = coordinateSystem(curve->coordinateSystemIndex());
			autoScale(cs->xIndex(), cs->yIndex());	// update all plot ranges
		}
	}
}

/*!
 * \brief CartesianPlot::childHovered
 * Unhover all curves, when another child is hovered. The hover handling for the curves is done in their parent (CartesianPlot),
 * because the hover should set when the curve is hovered and not just the bounding rect (for more see hoverMoveEvent)
 */
void CartesianPlot::childHovered() {
	Q_D(CartesianPlot);
	bool curveSender = qobject_cast<XYCurve*>(QObject::sender()) != nullptr;
	if (!d->isSelected()) {
		if (d->m_hovered)
			d->m_hovered = false;
		d->update();
	}
	if (!curveSender) {
		for (auto curve: children<XYCurve>())
			curve->setHover(false);
	}
}

void CartesianPlot::updateLegend() {
	if (m_legend)
		m_legend->retransform();
}

bool CartesianPlot::autoScale(int xIndex, int yIndex, bool fullRange) {
	DEBUG(Q_FUNC_INFO << ", auto scale x/y = " << autoScaleX(xIndex) << "/" << autoScaleY(yIndex))

	bool updated{ false };
	if (autoScaleX(xIndex) && autoScaleY(yIndex))
		updated = scaleAuto(xIndex, yIndex, fullRange);
	else if (autoScaleX(xIndex))
		updated = scaleAutoX(xIndex, fullRange);
	else if (autoScaleY(yIndex))
		updated = scaleAutoY(yIndex, fullRange);

	return updated;
}

/*!
	called when in one of the curves the data was changed.
	Autoscales the coordinate system and the x-axes, when "auto-scale" is active.
*/
void CartesianPlot::dataChanged(int xIndex, int yIndex, WorksheetElement* sender) {
	DEBUG(Q_FUNC_INFO << ", x/y index = " << xIndex << "/" << yIndex)
	if (project() && project()->isLoading())
		return;

	Q_D(CartesianPlot);
	if (xIndex == -1) {
		for (int i = 0; i < xRangeCount(); i++)
			d->xRanges[i].dirty = true;
	} else
		d->xRanges[xIndex].dirty = true;

	if (yIndex == -1) {
		for (int i = 0; i < yRangeCount(); i++)
			d->yRanges[i].dirty = true;
	} else
		d->yRanges[yIndex].dirty = true;

	const bool updated{ autoScale(xIndex, yIndex) };
	if (!updated || !sender) {
		//even if the plot ranges were not changed, either no auto scale active or the new data
		//is within the current ranges and no change of the ranges is required,
		//retransform the curve in order to show the changes
		if (sender)
			sender->retransform();
		else {
			//no sender available, the function was called directly in the file filter (live data source got new data)
			//or in Project::load() -> retransform all available curves since we don't know which curves are affected.
			//TODO: this logic can be very expensive
			for (auto* child : children<XYCurve>()) {
				child->recalcLogicalPoints();
				child->retransform();
			}
		}
	}
}

/*!
	called when in one of the curves the x-data was changed.
	Autoscales the coordinate system and the x-axes, when "auto-scale" is active.
*/
void CartesianPlot::xDataChanged(XYCurve* curve) {
	DEBUG(Q_FUNC_INFO)
	if (project() && project()->isLoading())
		return;

	Q_D(CartesianPlot);
	if (d->suppressRetransform)
		return;

	if (!curve)
		return;

	int cSystemIndex = curve->coordinateSystemIndex();
	int index = coordinateSystem(cSystemIndex)->xIndex();
	if (index == -1) {
		for (int i = 0; i < xRangeCount(); i++)
			d->xRanges[coordinateSystem(i)->xIndex()].dirty = true;
		// don't return!
	} else
		d->xRanges[index].dirty = true;

	bool updated = false;
	if (autoScaleX(index))
		updated = this->scaleAutoX(index);

	if (!updated) {
		//even if the plot ranges were not changed, either no auto scale active or the new data
		//is within the current ranges and no change of the ranges is required,
		//retransform the curve in order to show the changes
		curve->retransform();
	}

	//in case there is only one curve and its column mode was changed, check whether we start plotting datetime data
	if (children<XYCurve>().size() == 1) {
		const AbstractColumn* col = curve->xColumn();
		const auto xRangeFormat{ xRange().format() };
		if (col->columnMode() == AbstractColumn::ColumnMode::DateTime && xRangeFormat != RangeT::Format::DateTime) {
			setUndoAware(false);
			setXRangeFormat(RangeT::Format::DateTime);
			setUndoAware(true);
		}
	}
	emit curveDataChanged(curve);
}

/*!
	called when in one of the curves the x-data was changed.
	Autoscales the coordinate system and the x-axes, when "auto-scale" is active.
*/
void CartesianPlot::yDataChanged(XYCurve* curve) {
	if (project() && project()->isLoading())
		return;

	Q_D(CartesianPlot);
	if (d->suppressRetransform)
		return;

	if (!curve)
		return;

	int cSystemIndex = curve->coordinateSystemIndex();
	int index = coordinateSystem(cSystemIndex)->yIndex();
	if (index == -1) {
		for (int i = 0; i < yRangeCount(); i++)
			d->yRanges[coordinateSystem(i)->yIndex()].dirty = true;
		// don't return!
	} else
		d->yRanges[index].dirty = true;

	bool updated = false;
	if (autoScaleY(index))
		updated = this->scaleAutoY(index);

	if (!updated) {
		//even if the plot ranges were not changed, either no auto scale active or the new data
		//is within the current ranges and no change of the ranges is required,
		//retransform the curve in order to show the changes
		curve->retransform();

	}

	//in case there is only one curve and its column mode was changed, check whether we start plotting datetime data
	if (children<XYCurve>().size() == 1) {
		const AbstractColumn* col = curve->yColumn();
		const auto yRangeFormat{ yRange().format() };
		if (col && col->columnMode() == AbstractColumn::ColumnMode::DateTime && yRangeFormat != RangeT::Format::DateTime) {
			setUndoAware(false);
			setYRangeFormat(RangeT::Format::DateTime);
			setUndoAware(true);
		}
	}
	emit curveDataChanged(curve);
}

void CartesianPlot::curveVisibilityChanged() {
	int index = static_cast<WorksheetElement*>(QObject::sender())->coordinateSystemIndex();
	int xIndex = coordinateSystem(index)->xIndex();
	int yIndex = coordinateSystem(index)->yIndex();
	setXRangeDirty(xIndex, true);
	setYRangeDirty(yIndex, true);
	updateLegend();
	if (autoScaleX(xIndex) && autoScaleY(yIndex))
		this->scaleAuto(xIndex, yIndex);
	else if (autoScaleX(xIndex))
		this->scaleAutoX(xIndex);
	else if (autoScaleY(yIndex))
		this->scaleAutoY(yIndex);

	emit curveVisibilityChangedSignal();
}

void CartesianPlot::curveLinePenChanged(QPen pen) {
	const auto* curve = qobject_cast<const XYCurve*>(QObject::sender());
	emit curveLinePenChanged(pen, curve->name());
}

void CartesianPlot::setMouseMode(MouseMode mouseMode) {
	Q_D(CartesianPlot);

	d->mouseMode = mouseMode;
	d->setHandlesChildEvents(mouseMode != MouseMode::Selection);

	QList<QGraphicsItem*> items = d->childItems();
	if (mouseMode == MouseMode::Selection) {
		d->setZoomSelectionBandShow(false);
		d->setCursor(Qt::ArrowCursor);
		for (auto* item : items)
			item->setFlag(QGraphicsItem::ItemStacksBehindParent, false);
	} else {
		if (mouseMode == MouseMode::ZoomSelection || mouseMode == MouseMode::Crosshair)
			d->setCursor(Qt::CrossCursor);
		else if (mouseMode == MouseMode::ZoomXSelection)
			d->setCursor(Qt::SizeHorCursor);
		else if (mouseMode == MouseMode::ZoomYSelection)
			d->setCursor(Qt::SizeVerCursor);

		for (auto* item : items)
			item->setFlag(QGraphicsItem::ItemStacksBehindParent, true);
	}

	//when doing zoom selection, prevent the graphics item from being movable
	//if it's currently movable (no worksheet layout available)
	const auto* worksheet = qobject_cast<const Worksheet*>(parentAspect());
	if (worksheet) {
		if (mouseMode == MouseMode::Selection) {
			if (worksheet->layout() != Worksheet::Layout::NoLayout)
				graphicsItem()->setFlag(QGraphicsItem::ItemIsMovable, false);
			else
				graphicsItem()->setFlag(QGraphicsItem::ItemIsMovable, true);
		} else   //zoom m_selection
			graphicsItem()->setFlag(QGraphicsItem::ItemIsMovable, false);
	}

	emit mouseModeChanged(mouseMode);
}

BASIC_SHARED_D_ACCESSOR_IMPL(CartesianPlot, bool, isLocked, Locked, locked)

// auto scale

void CartesianPlot::scaleAutoTriggered() {
	QAction* action = qobject_cast<QAction*>(QObject::sender());
	if (!action)
		return;

	if (action == scaleAutoAction)
		scaleAuto(-1, -1);
	else if (action == scaleAutoXAction)
		setAutoScaleX();
	else if (action == scaleAutoYAction)
		setAutoScaleY();
}

bool CartesianPlot::scaleAutoX(int index, bool fullRange, bool suppressRetransform) {
	Q_D(CartesianPlot);
	if (index == -1) {	// all ranges
		bool updated = false;
		for (int i = 0; i < xRangeCount(); i++) {
			if (scaleAutoX(i, fullRange, true)) {
				updated = true; // at least one was updated
			}
		}
		if (!suppressRetransform)
			d->retransformScales(index, -1);
		return updated;
	}

	DEBUG(Q_FUNC_INFO << ", csystem index = " << index << " full range = " << fullRange)
	if (xRangeDirty(index)) {
		calculateDataXRange(index, fullRange);
		setXRangeDirty(index, false);

		for (int i = 0; i < m_coordinateSystems.count(); i++) {
			auto cs = coordinateSystem(i);
			if (cs->xIndex() == index)
				setYRangeDirty(cs->yIndex(), true);
		}
	}

	auto& xRange{ d->xRange(index) };
	auto dataRange = d->dataXRange(index);
	if (dataRange.finite())
		dataRange.niceExtend();	// auto scale to nice data range

	// if no curve: do not reset to [0, 1]

	DEBUG(Q_FUNC_INFO << ", x range = " << xRange.toStdString() << "., curves x range = " << d->dataXRange(index).toStdString())
	bool update = false;
	if (!qFuzzyCompare(dataRange.start(), xRange.start()) && !qIsInf(dataRange.start())) {
		xRange.start() = dataRange.start();
		update = true;
	}

	if (!qFuzzyCompare(dataRange.end(), xRange.end()) && !qIsInf(dataRange.end()) ) {
		xRange.end() = dataRange.end();
		update = true;
	}

	if (update) {
		DEBUG(Q_FUNC_INFO << ", set new x range = " << xRange.toStdString())
		//in case min and max are equal (e.g. if we plot a single point), subtract/add 10% of the value
		if (xRange.isZero()) {
			const double value{ xRange.start() };
			if (!qFuzzyIsNull(value))
				xRange.setRange(value * 0.9, value * 1.1);
			else
				xRange.setRange(-0.1, 0.1);
		} else {
			xRange.extend( xRange.size() * d->autoScaleOffsetFactor );
		}
		// extend to nice values
//		xRange.niceExtend();

		if (!suppressRetransform)
			d->retransformScales(index, -1); // TODO: all y?
	}

	return update;
}

// TODO: copy paste code?
bool CartesianPlot::scaleAutoY(int index, bool fullRange, bool suppressRetransform) {
	Q_D(CartesianPlot);
	if (index == -1) {
		bool updated = false;
		for (int i = 0; i < yRangeCount(); i++) {
			if (scaleAutoY(i, fullRange, true)) {
				updated = true; // at least one was updated
			}
		}
		if (!suppressRetransform)
			d->retransformScales(-1, index);
		return updated;
	}

	DEBUG(Q_FUNC_INFO << ", index = " << index << " full range = " << fullRange)

	if (yRangeDirty(index)) {
		calculateDataYRange(index, fullRange);
		setYRangeDirty(index, false);

		for (int i = 0; i < m_coordinateSystems.count(); i++) {
			// All x ranges with this yIndex must be dirty
			auto cs = coordinateSystem(i);
			if (cs->yIndex() == index)
				setXRangeDirty(cs->xIndex(), true);
		}
	}

	auto& yRange{ d->yRange(index) };
	auto dataRange = d->dataYRange(index);
	if (dataRange.finite())
		dataRange.niceExtend();	// auto scale to nice data range

	bool update = false;
	DEBUG(Q_FUNC_INFO << ", y range = " << yRange.toStdString() << ", curves y range = " << d->dataYRange(index).toStdString())
	if (!qFuzzyCompare(dataRange.start(), yRange.start()) && !qIsInf(dataRange.start()) ) {
		yRange.start() = dataRange.start();
		update = true;
	}

	if (!qFuzzyCompare(dataRange.end(), yRange.end()) && !qIsInf(dataRange.end()) ) {
		yRange.end() = dataRange.end();
		update = true;
	}
	if (update) {
		DEBUG(Q_FUNC_INFO << ", set new y range = " << yRange.toStdString())
		//in case min and max are equal (e.g. if we plot a single point), subtract/add 10% of the value
		if (yRange.isZero()) {
			const double value{ yRange.start() };
			if (!qFuzzyIsNull(value))
				yRange.setRange(value * 0.9, value * 1.1);
			else
				yRange.setRange(-0.1, 0.1);
		} else {
			yRange.extend( yRange.size() * d->autoScaleOffsetFactor );
		}

		if (!suppressRetransform)
			d->retransformScales(-1, index); // TODO: all x?
	}

	return update;
}

bool CartesianPlot::scaleAuto(int xIndex, int yIndex, bool fullRange) {
	bool updateX = scaleAutoX(xIndex, fullRange, true);
	bool updateY = scaleAutoY(yIndex, fullRange, true);

	// x range is dirty, because scaleAutoY sets it to dirty.
	if (xIndex < 0) {
		for (int i = 0; i < m_coordinateSystems.count(); i++) {
			setXRangeDirty(coordinateSystem(i)->xIndex(), false);
			//setYRangeDirty(coordinateSystem(i)->yIndex(), false);
		}
	} else {
		setXRangeDirty(xIndex, false);
		//setYRangeDirty(coordinateSystem(cSystemIndex)->yIndex(), false);
	}

	Q_D(CartesianPlot);
	if (updateX || updateY) {
		if (updateX)
			setAutoScaleX(xIndex);
		if (updateY)
			setAutoScaleY(yIndex);

		d->retransformScales(xIndex, yIndex);
	}

	return (updateX || updateY);
}

/*!
 * Calculates and saves data x range.
 * The range of the y axis is not considered.
 */
void CartesianPlot::calculateDataXRange(const int index, bool completeRange) {
	DEBUG(Q_FUNC_INFO << ", complete range = " << completeRange)
	Q_D(CartesianPlot);

	d->dataXRange(index).setRange(qInf(), -qInf());

	//loop over all xy-curves and determine the maximum and minimum x-values
	for (const auto* curve : this->children<const XYCurve>()) {
		//only curves with correct xIndex
		if (coordinateSystem(curve->coordinateSystemIndex())->xIndex() != index)
			continue;
		if (!curve->isVisible())
			continue;

		auto* xColumn = curve->xColumn();
		if (!xColumn)
			continue;

		// range of indices
		Range<int> indexRange{0, 0};
		if (!completeRange && d->rangeType == RangeType::Free && curve->yColumn()) { // only data within y range
			const int yIndex = coordinateSystem(curve->coordinateSystemIndex())->yIndex();
			DEBUG(Q_FUNC_INFO << ", free incomplete range with y column. y range = " << yRange(yIndex).toStdString())
			curve->yColumn()->indicesMinMax(yRange(yIndex).start(), yRange(yIndex).end(), indexRange.start(), indexRange.end());
		} else {	// all data
			DEBUG(Q_FUNC_INFO << ", else. range type = " << (int)d->rangeType)
			switch (d->rangeType) {
			case RangeType::Free:
				indexRange.setRange(0, xColumn->rowCount());
				break;
			case RangeType::Last:
				indexRange.setRange(xColumn->rowCount() - d->rangeLastValues, xColumn->rowCount());
				break;
			case RangeType::First:
				indexRange.setRange(0, d->rangeFirstValues);
				break;
			}
		}
		DEBUG(Q_FUNC_INFO << ", index range = " << indexRange.toStdString())

		auto range{d->xRange(index)};	// value does not matter, will be overwritten
		curve->minMaxX(indexRange, range, true);

		if (range.start() < d->dataXRange(index).start())
			d->dataXRange(index).start() = range.start();

		if (range.end() > d->dataXRange(index).end())
			d->dataXRange(index).end() = range.end();
		DEBUG(Q_FUNC_INFO << ", curves x range i = " << d->dataXRange(index).toStdString())
	}

	//loop over all histograms and determine the maximum and minimum x-value
	for (const auto* curve : this->children<const Histogram>()) {
		if (!curve->isVisible())
			continue;
		if (!curve->dataColumn())
			continue;

		const double min = curve->xMinimum();
		if (d->dataXRange(index).start() > min)
			d->dataXRange(index).start() = min;

		const double max = curve->xMaximum();
		if (max > d->dataXRange(index).end())
			d->dataXRange(index).end() = max;
	}

	//loop over all box plots and determine the maximum and minimum x-values
	for (const auto* curve : this->children<const BoxPlot>()) {
		if (!curve->isVisible())
			continue;
		if (curve->dataColumns().isEmpty())
			continue;

		const double min = curve->xMinimum();
		if (d->dataXRange(index).start() > min)
			d->dataXRange(index).start() = min;

		const double max = curve->xMaximum();
		if (max > d->dataXRange(index).end())
			d->dataXRange(index).end() = max;
	}

	DEBUG(Q_FUNC_INFO << ", data x range = " << d->dataXRange(index).toStdString())
}

void CartesianPlot::retransformScales() {
	Q_D(CartesianPlot);
	d->retransformScales(-1, -1);
}

/*!
 * Calculates and sets data y range.
 * The range of the x axis is not considered.
 */
void CartesianPlot::calculateDataYRange(const int index, bool completeRange) {
	DEBUG(Q_FUNC_INFO << ", index = " << index << ", complete range = " << completeRange)
	Q_D(CartesianPlot);

	d->dataYRange(index).setRange(qInf(), -qInf());
	Range<double> range{d->dataYRange(index)};

	//loop over all xy-curves and determine the maximum and minimum y-values
	for (const auto* curve : this->children<const XYCurve>()) {
		DEBUG( Q_FUNC_INFO << ", curve " << STDSTRING(curve->name()) << ", range type = "
			<< ENUM_TO_STRING(CartesianPlot, RangeType, d->rangeType) )
		if (coordinateSystem(curve->coordinateSystemIndex())->yIndex() != index)
			continue;
		if (!curve->isVisible())
			continue;

		auto* yColumn = curve->yColumn();
		if (!yColumn)
			continue;

		Range<int> indexRange{0, 0};
		if (!completeRange && d->rangeType == RangeType::Free && curve->xColumn()) {
			const int xIndex = coordinateSystem(curve->coordinateSystemIndex())->xIndex();
			curve->xColumn()->indicesMinMax(xRange(xIndex).start(), xRange(xIndex).end(), indexRange.start(), indexRange.end());
		} else {
			switch (d->rangeType) {
				case RangeType::Free:
					indexRange.setRange(0, yColumn->rowCount());
					break;
				case RangeType::Last:
					indexRange.setRange(yColumn->rowCount() - d->rangeLastValues, yColumn->rowCount());
					break;
				case RangeType::First:
					indexRange.setRange(0, d->rangeFirstValues);
					break;
			}
		}

		curve->minMaxY(indexRange, range, true);

		if (range.start() < d->dataYRange(index).start())
			d->dataYRange(index).start() = range.start();

		if (range.end() > d->dataYRange(index).end())
			d->dataYRange(index).end() = range.end();
	}

	//loop over all histograms and determine the maximum y-value
	for (const auto* curve : this->children<const Histogram>()) {
		if (!curve->isVisible())
			continue;

		const double min = curve->yMinimum();
		if (d->dataYRange(index).start() > min)
			d->dataYRange(index).start() = min;

		const double max = curve->yMaximum();
		if (max > d->dataYRange(index).end())
			d->dataYRange(index).end() = max;
	}

	//loop over all box plots and determine the maximum y-value
	for (const auto* curve : this->children<const BoxPlot>()) {
		if (!curve->isVisible())
			continue;

		const double min = curve->yMinimum();
		if (d->dataYRange(index).start() > min)
			d->dataYRange(index).start() = min;

		const double max = curve->yMaximum();
		if (max > d->dataYRange(index).end())
			d->dataYRange(index).end() = max;
	}

	DEBUG(Q_FUNC_INFO << ", data y range = " << d->dataYRange(index).toStdString())
}

// zoom

void CartesianPlot::zoomIn(int xIndex, int yIndex) {
	setUndoAware(false);
	setAutoScaleX(xIndex, false);
	setAutoScaleY(yIndex, false);
	setUndoAware(true);
	setXRangeDirty(xIndex, true);
	setYRangeDirty(yIndex, true);
	zoom(xIndex, true, true); //zoom in x
	zoom(yIndex, false, true); //zoom in y

	Q_D(CartesianPlot);
	d->retransformScales(xIndex, yIndex);
}

void CartesianPlot::zoomOut(int xIndex, int yIndex) {
	setUndoAware(false);
	setAutoScaleX(xIndex, false);
	setAutoScaleY(yIndex, false);
	setUndoAware(true);
	setXRangeDirty(xIndex, true);
	setYRangeDirty(yIndex, true);
	zoom(xIndex, true, false); //zoom out x
	zoom(yIndex, false, false); //zoom out y

	Q_D(CartesianPlot);
	d->retransformScales(xIndex, yIndex);
}

void CartesianPlot::zoomInX(int index) {
	setUndoAware(false);
	setAutoScaleX(index, false);
	setUndoAware(true);
	setYRangeDirty(index, true);
	zoom(index, true, true); //zoom in x

	bool retransform = false;
	for (int i = 0; i < m_coordinateSystems.count(); i++) {
		const auto cs = coordinateSystem(i);
		if (index == -1 || index == cs->xIndex()) {
			if (autoScaleY(cs->yIndex()))
				scaleAutoY(cs->yIndex(), true);
			retransform = true;
		}
	}

	Q_D(CartesianPlot);
	if (retransform)
		d->retransformScales(index, -1);
}

void CartesianPlot::zoomOutX(int index) {
	setUndoAware(false);
	setAutoScaleX(index, false);
	setUndoAware(true);
	setYRangeDirty(index, true);
	zoom(index, true, false); //zoom out x

	bool retransform = false;
	for (int i = 0; i < m_coordinateSystems.count(); i++) {
		const auto cs = coordinateSystem(i);
		if ((index == -1 || index == cs->xIndex())) {
			if (autoScaleY(cs->yIndex()))
				scaleAutoY(cs->yIndex(), true);
			retransform = true;
		}
	}

	Q_D(CartesianPlot);
	if (retransform)
		d->retransformScales(index, -1);
}

void CartesianPlot::zoomInY(int index) {
	setUndoAware(false);
	setAutoScaleY(index, false);
	setUndoAware(true);
	setXRangeDirty(index, true);
	zoom(index, false, true); //zoom in y

	bool retransform = false;
	for (int i = 0; i < m_coordinateSystems.count(); i++) {
		const auto cs = coordinateSystem(i);
		if ((index == -1 || index == cs->yIndex())) {
			if (autoScaleX(cs->xIndex()))
				scaleAutoX(cs->yIndex(), true);
			retransform = true;
		}
	}

	Q_D(CartesianPlot);
	if (retransform)
		d->retransformScales(-1, index);
}

void CartesianPlot::zoomOutY(int index) {
	setUndoAware(false);
	setAutoScaleY(index, false);
	setUndoAware(true);
	setXRangeDirty(index, true);
	zoom(index, false, false); //zoom out y

	bool retransform = false;
	for (int i = 0; i < m_coordinateSystems.count(); i++) {
		const auto cs = coordinateSystem(i);
		if ((index == -1 || index == cs->yIndex())) {
			if (autoScaleX(cs->xIndex()))
				scaleAutoX(cs->xIndex(), true);
			retransform = true;
		}
	}

	Q_D(CartesianPlot);
	if (retransform)
		d->retransformScales(-1, index);
}

/*!
 * helper function called in other zoom*() functions
 * and doing the actual change of the data ranges.
 * @param x if set to \true the x-range is modified, the y-range for \c false
 * @param in the "zoom in" is performed if set to \c \true, "zoom out" for \c false
 */
void CartesianPlot::zoom(int index, bool x, bool zoom_in) {
	Q_D(CartesianPlot);

	Range<double> range;
	if (index == -1) {
		QVector<int> zoomedIndices;
		for (int i = 0; i < m_coordinateSystems.count(); i++) {
			int idx = x ? coordinateSystem(i)->xIndex() : coordinateSystem(i)->yIndex();
			if (zoomedIndices.contains(idx))
				continue;
			zoom(idx, x, zoom_in);
			zoomedIndices.append(idx);
		}
		return;
	} else if (x)
		range = d->xRanges.at(index).range;
	else
		range = d->yRanges.at(index).range;

	double factor = m_zoomFactor;
	if (zoom_in)
		factor = 1./factor;

	const double start{range.start()}, end{range.end()};
	switch (range.scale()) {
	case RangeT::Scale::Linear: {
		range.extend(range.size() * (factor - 1.) / 2.);
		break;
	}
	case RangeT::Scale::Log10: {
		if (start == 0 || end/start <= 0)
			break;
		const double diff = log10(end/start) * (factor - 1.);
		const double extend = pow(10, diff / 2.);
		range.end() *= extend;
		range.start() /= extend;
		break;
	}
	case RangeT::Scale::Log2: {
		if (start == 0 || end/start <= 0)
			break;
		const double diff = log2(end/start) * (factor - 1.);
		const double extend = exp2(diff / 2.);
		range.end() *= extend;
		range.start() /= extend;
		break;
	}
	case RangeT::Scale::Ln: {
		if (start == 0 || end/start <= 0)
			break;
		const double diff = log(end/start) * (factor - 1.);
		const double extend = exp(diff / 2.);
		range.end() *= extend;
		range.start() /= extend;
		break;
	}
	case RangeT::Scale::Sqrt: {
		if (start < 0 || end < 0)
			break;
		const double diff = (sqrt(end) - sqrt(start)) * (factor - 1.);
		range.extend(diff*diff/4.);
		break;
	}
	case RangeT::Scale::Square: {
		const double diff = (end*end - start*start) * (factor - 1.);
		range.extend( sqrt(qAbs(diff/2.)) );
		break;
	}
	case RangeT::Scale::Inverse: {
		const double diff = (1./start - 1./end) * (factor - 1.);
		range.extend( 1./qAbs(diff/2.) );
		break;
	}
	}

	// make nice again
	if (zoom_in)
		range.niceShrink();
	else
		range.niceExtend();

	if (range.finite())
		x ? d->xRange(index) = range : d->yRange(index) = range;
}

/*!
 * helper function called in other shift*() functions
 * and doing the actual change of the data ranges.
 * @param x if set to \true the x-range is modified, the y-range for \c false
 * @param leftOrDown the "shift left" for x or "shift dows" for y is performed if set to \c \true,
 * "shift right" or "shift up" for \c false
 */
void CartesianPlot::shift(int index, bool x, bool leftOrDown) {
	Q_D(CartesianPlot);

	Range<double> range;
	if (index == -1) {
		QVector<int> shiftedIndices;
		for (int i=0; i < m_coordinateSystems.count(); i++) {
			int idx = x ? coordinateSystem(i)->xIndex() : coordinateSystem(i)->yIndex();
			if (shiftedIndices.contains(idx))
				continue;
			shift(idx, x, leftOrDown);
			shiftedIndices.append(idx);
		}
		return;
	} else if (x)
		range = d->xRanges.at(index).range;
	else
		range = d->yRanges.at(index).range;

	double offset = 0.0, factor = 0.1;

	if (!leftOrDown)
		factor *= -1.;

	const double start{range.start()}, end{range.end()};
	switch (range.scale()) {
	case RangeT::Scale::Linear: {
		offset = range.size() * factor;
		range += offset;
		break;
	}
	case RangeT::Scale::Log10: {
		if (start == 0 || end/start <= 0)
			break;
		offset = log10(end/start) * factor;
		range *= pow(10, offset);
		break;
	}
	case RangeT::Scale::Log2: {
		if (start == 0 || end/start <= 0)
			break;
		offset = log2(end/start) * factor;
		range *= exp2(offset);
		break;
	}
	case RangeT::Scale::Ln: {
		if (start == 0 || end/start <= 0)
			break;
		offset = log(end/start) * factor;
		range *= exp(offset);
		break;
	}
	case RangeT::Scale::Sqrt:
		if (start < 0 || end < 0)
			break;
		offset = (sqrt(end) - sqrt(start)) * factor;
		range += offset*offset;
		break;
	case RangeT::Scale::Square:
		offset = (end*end - start*start) * factor;
		range += sqrt(qAbs(offset));
		break;
	case RangeT::Scale::Inverse:
		offset = (1./start - 1./end) * factor;
		range += 1./qAbs(offset);
		break;
	}

	if (range.finite())
		x ? d->xRange(index) = range : d->yRange(index) = range;
}

void CartesianPlot::shiftLeftX(int index) {
	setUndoAware(false);
	setAutoScaleX(index, false);
	setUndoAware(true);
	shift(index, true, true);

	bool retransform = false;
	for (int i = 0; i < m_coordinateSystems.count(); i++) {
		const auto cs = coordinateSystem(i);
		if ((index == -1 || index == cs->xIndex())) {
			if (autoScaleY(cs->yIndex())) {
				setYRangeDirty(cs->yIndex(), true);
				scaleAutoY(cs->yIndex(), true);
			}
			retransform = true;
		}
	}

	Q_D(CartesianPlot);
	if (retransform)
		d->retransformScales(index, -1);
}

void CartesianPlot::shiftRightX(int index) {
	setUndoAware(false);
	setAutoScaleX(index, false);
	setUndoAware(true);
	shift(index, true, false);

	bool retransform = false;
	for (int i = 0; i < m_coordinateSystems.count(); i++) {
		const auto cs = coordinateSystem(i);
		if ((index == -1 || index == cs->xIndex())) {
			if (autoScaleY(cs->yIndex())) {
				setYRangeDirty(cs->yIndex(), true);
				scaleAutoY(cs->yIndex(), true);
			}
			retransform = true;
		}
	}

	Q_D(CartesianPlot);
	if (retransform)
		d->retransformScales(index, -1);
}

void CartesianPlot::shiftUpY(int index) {
	setUndoAware(false);
	setAutoScaleY(index, false);
	setUndoAware(true);
	shift(index, false, false);

	bool retransform = false;
	for (int i = 0; i < m_coordinateSystems.count(); i++) {
		const auto cs = coordinateSystem(i);
		if ((index == -1 || index == cs->yIndex())) {
			if (autoScaleX(cs->xIndex())) {
				setXRangeDirty(cs->xIndex(), true);
				scaleAutoX(cs->xIndex(), true);
			}
			retransform = true;
		}
	}

	Q_D(CartesianPlot);
	if (retransform)
		d->retransformScales(-1, index);
}

void CartesianPlot::shiftDownY(int index) {
	setUndoAware(false);
	setAutoScaleY(index, false);
	setUndoAware(true);
	shift(index, false, true);

	bool retransform = false;
	for (int i = 0; i < m_coordinateSystems.count(); i++) {
		const auto cs = coordinateSystem(i);
		if ((index == -1 || index == cs->yIndex())) {
			if (autoScaleX(cs->xIndex())) {
				setXRangeDirty(cs->xIndex(), true);
				scaleAutoX(cs->xIndex(), true);
			}
			retransform = true;
		}
	}

	Q_D(CartesianPlot);
	if (retransform)
		d->retransformScales(-1, index);
}

void CartesianPlot::cursor() {
	Q_D(CartesianPlot);
	d->retransformScales(-1, -1); // TODO: needed to retransform all scales?
}

void CartesianPlot::mousePressZoomSelectionMode(QPointF logicPos, int cSystemIndex) {
	Q_D(CartesianPlot);
	d->mousePressZoomSelectionMode(logicPos, cSystemIndex);
}
void CartesianPlot::mousePressCursorMode(int cursorNumber, QPointF logicPos) {
	Q_D(CartesianPlot);
	d->mousePressCursorMode(cursorNumber, logicPos);
}
void CartesianPlot::mouseMoveZoomSelectionMode(QPointF logicPos, int cSystemIndex) {
	Q_D(CartesianPlot);
	d->mouseMoveZoomSelectionMode(logicPos, cSystemIndex);
}

void CartesianPlot::mouseMoveSelectionMode(QPointF logicStart, QPointF logicEnd) {
	Q_D(CartesianPlot);
	d->mouseMoveSelectionMode(logicStart, logicEnd);
}

void CartesianPlot::mouseMoveCursorMode(int cursorNumber, QPointF logicPos) {
	Q_D(CartesianPlot);
	d->mouseMoveCursorMode(cursorNumber, logicPos);
}

void CartesianPlot::mouseReleaseZoomSelectionMode(int cSystemIndex) {
	Q_D(CartesianPlot);
	d->mouseReleaseZoomSelectionMode(cSystemIndex);
}

void CartesianPlot::mouseHoverZoomSelectionMode(QPointF logicPos, int cSystemIndex) {
	Q_D(CartesianPlot);
	d->mouseHoverZoomSelectionMode(logicPos, cSystemIndex);
}

void CartesianPlot::mouseHoverOutsideDataRect() {
	Q_D(CartesianPlot);
	d->mouseHoverOutsideDataRect();
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
	setData(0, static_cast<int>(AspectType::CartesianPlot));
	m_cursor0Text.prepare();
	m_cursor1Text.prepare();
}

CartesianPlotPrivate::~CartesianPlotPrivate() {
}

/*!
	updates the position of plot rectangular in scene coordinates to \c r and recalculates the scales.
	The size of the plot corresponds to the size of the plot area, the area which is filled with the background color etc.
	and which can pose the parent item for several sub-items (like TextLabel).
	Note, the size of the area used to define the coordinate system doesn't need to be equal to this plot area.
	Also, the size (=bounding box) of CartesianPlot can be greater than the size of the plot area.
 */
void CartesianPlotPrivate::retransform() {
	DEBUG(Q_FUNC_INFO)
	if (suppressRetransform)
		return;

	PERFTRACE("CartesianPlotPrivate::retransform()");
	prepareGeometryChange();
	setPos(rect.x() + rect.width()/2, rect.y() + rect.height()/2);

	updateDataRect();
	retransformScales(-1, -1); // TODO: check if all must be retransformed

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

void CartesianPlotPrivate::retransformXScale(int index) {
	static const int breakGap = 20;
	Range<double> plotSceneRange{dataRect.x(), dataRect.x() + dataRect.width()};
	Range<double> sceneRange, logicalRange;

	for (auto cSystem: coordinateSystems()) {
		auto cs = static_cast<CartesianCoordinateSystem*>(cSystem);
		if (cs->xIndex() != index)
			continue;

		// check ranges for nonlinear scales
		if (xRanges.at(index).range.scale() != RangeT::Scale::Linear)
			checkXRange(index);

		const auto xRange{ xRanges.at(index).range };
		//DEBUG(Q_FUNC_INFO << ", coordinate system " << i++ <<  ", x range is x range " << xRangeIndex+1)
		DEBUG(Q_FUNC_INFO << ", x range : " << xRange.toStdString())

		QVector<CartesianScale*> scales;

		//check whether we have x-range breaks - the first break, if available, should be valid
		bool hasValidBreak = (xRangeBreakingEnabled && !xRangeBreaks.list.isEmpty() && xRangeBreaks.list.first().isValid());
		if (!hasValidBreak) {	//no breaks available -> range goes from the start to the end of the plot
			sceneRange = plotSceneRange;
			logicalRange = xRange;

			//TODO: how should we handle the case sceneRange.length() == 0?
			//(to reproduce, create plots and adjust the spacing/pading to get zero size for the plots)
			if (sceneRange.length() > 0)
				scales << this->createScale(xRange.scale(), sceneRange, logicalRange);
		} else {
			double sceneEndLast = plotSceneRange.start();
			double logicalEndLast = xRange.start();
			for (const auto& rb : qAsConst(xRangeBreaks.list)) {
				if (!rb.isValid())
					break;

				//current range goes from the end of the previous one (or from the plot beginning) to curBreak.start
				sceneRange.start() = sceneEndLast;
				if (&rb == &xRangeBreaks.list.first()) sceneRange.start() += breakGap;
				sceneRange.end() = plotSceneRange.start() + plotSceneRange.size() * rb.position;
				logicalRange = Range<double>(logicalEndLast, rb.range.start());

				if (sceneRange.length() > 0)
					scales << this->createScale(xRange.scale(), sceneRange, logicalRange);

				sceneEndLast = sceneRange.end();
				logicalEndLast = rb.range.end();
			}

			//add the remaining range going from the last available range break to the end of the plot (=end of the x-data range)
			sceneRange.setRange(sceneEndLast + breakGap, plotSceneRange.end());
			logicalRange.setRange(logicalEndLast, xRange.end());

			if (sceneRange.length() > 0)
				scales << this->createScale(xRange.scale(), sceneRange, logicalRange);
		}
		cs->setXScales(scales);
	}
}

void CartesianPlotPrivate::retransformYScale(int index) {
	static const int breakGap = 20;
	Range<double> plotSceneRange{dataRect.y() + dataRect.height(), dataRect.y()};
	Range<double> sceneRange, logicalRange;

	for (auto cSystem: coordinateSystems()) {
		auto cs = static_cast<CartesianCoordinateSystem*>(cSystem);
		if (cs->yIndex() != index)
			continue;

		if (yRanges.at(index).range.scale() != RangeT::Scale::Linear)
			checkYRange(index);

		const auto yRange{ yRanges.at(index).range };
		//DEBUG(Q_FUNC_INFO << ", coordinate system " << i++ <<  ", y range is y range " << yRangeIndex+1)
		DEBUG(Q_FUNC_INFO << ", yrange = " << yRange.toStdString())

		QVector<CartesianScale*> scales;

		//check whether we have y-range breaks - the first break, if available, should be valid
		bool hasValidBreak = (yRangeBreakingEnabled && !yRangeBreaks.list.isEmpty() && yRangeBreaks.list.first().isValid());
		if (!hasValidBreak) {	//no breaks available -> range goes from the start to the end of the plot
			sceneRange = plotSceneRange;
			logicalRange = yRange;

			if (sceneRange.length() > 0)
				scales << this->createScale(yRange.scale(), sceneRange, logicalRange);
		} else {
			double sceneEndLast = plotSceneRange.start();
			double logicalEndLast = yRange.start();
			for (const auto& rb : qAsConst(yRangeBreaks.list)) {
				if (!rb.isValid())
					break;

				//current range goes from the end of the previous one (or from the plot beginning) to curBreak.start
				sceneRange.start() = sceneEndLast;
				if (&rb == &yRangeBreaks.list.first()) sceneRange.start() -= breakGap;
				sceneRange.end() = plotSceneRange.start() + plotSceneRange.size() * rb.position;
				logicalRange = Range<double>(logicalEndLast, rb.range.start());

				if (sceneRange.length() > 0)
					scales << this->createScale(yRange.scale(), sceneRange, logicalRange);

				sceneEndLast = sceneRange.end();
				logicalEndLast = rb.range.end();
			}

			//add the remaining range going from the last available range break to the end of the plot (=end of the y-data range)
			sceneRange.setRange(sceneEndLast - breakGap, plotSceneRange.end());
			logicalRange.setRange(logicalEndLast, yRange.end());

			if (sceneRange.length() > 0)
				scales << this->createScale(yRange.scale(), sceneRange, logicalRange);
		}
		cs->setYScales(scales);
	}
}

/*
 * calculate x and y scales from scence range and logical range (x/y range) for all coordinate systems
 */
void CartesianPlotPrivate::retransformScales(int xIndex, int yIndex) {
	for (int i = 0; i < xRanges.count(); i++)
		DEBUG( Q_FUNC_INFO << ", x range " << i+1 << " : " << xRanges.at(i).range.toStdString()
			<< ", scale = " << ENUM_TO_STRING(RangeT, Scale, xRanges.at(i).range.scale()) );
	for (int i = 0; i < yRanges.count(); i++)
		DEBUG( Q_FUNC_INFO << ", y range " << i+1 << " : " << yRanges.at(i).range.toStdString()
			<< ", scale = " << ENUM_TO_STRING(RangeT, Scale, yRanges.at(i).range.scale()) );

	PERFTRACE(Q_FUNC_INFO);

	if (xIndex == -1) {
		for (int i = 0; i < xRanges.count(); i++)
			retransformXScale(i);
	} else
		retransformXScale(xIndex);
	if (yIndex == -1) {
		for (int i = 0; i < yRanges.count(); i++)
			retransformYScale(i);
	} else
		retransformYScale(yIndex);

	// X ranges
	for (int i = 0; i < xRanges.count(); i++) {
		auto& rangep = xRanges[i];
		double deltaXMin = rangep.range.start() - rangep.prev.start();
		double deltaXMax = rangep.range.end() - rangep.prev.end();

		if (!qFuzzyIsNull(deltaXMin))
			emit q->xMinChanged(i, rangep.range.start());
		if (!qFuzzyIsNull(deltaXMax))
			emit q->xMaxChanged(i, rangep.range.end());

		rangep.prev = rangep.range;

		for (auto* axis : q->children<Axis>()) {
			DEBUG(Q_FUNC_INFO << ", auto-scale axis \"" << STDSTRING(axis->name()) << "\"")
			// use ranges of axis
			int axisXIndex = q->coordinateSystem(axis->coordinateSystemIndex())->xIndex();
			if (axis->rangeType() != Axis::RangeType::Auto || axis->orientation() != Axis::Orientation::Horizontal || axisXIndex != i)
				continue;

			if (!qFuzzyIsNull(deltaXMax)) {
				axis->setUndoAware(false);
				axis->setSuppressRetransform(true);
				axis->setEnd(rangep.range.end());
				axis->setUndoAware(true);
				axis->setSuppressRetransform(false);
			}
			if (!qFuzzyIsNull(deltaXMin)) {
				axis->setUndoAware(false);
				axis->setSuppressRetransform(true);
				axis->setStart(rangep.range.start());
				axis->setUndoAware(true);
				axis->setSuppressRetransform(false);
			}
			//TODO;
// 			if (axis->position() == Axis::Position::Centered && deltaYMin != 0) {
// 				axis->setOffset(axis->offset() + deltaYMin, false);
// 			}
		}
	}

	// Y ranges
	for (int i = 0; i < yRanges.count(); i++) {
		auto& rangep = yRanges[i];
		const double deltaYMin = rangep.range.start() - rangep.prev.start();
		const double deltaYMax = rangep.range.end() - rangep.prev.end();

		if (!qFuzzyIsNull(deltaYMin))
			emit q->yMinChanged(i, rangep.range.start());
		if (!qFuzzyIsNull(deltaYMax))
			emit q->yMaxChanged(i, rangep.range.end());

		rangep.prev = rangep.range;

		for (auto* axis : q->children<Axis>()) {
			DEBUG(Q_FUNC_INFO << ", auto-scale axis \"" << STDSTRING(axis->name()) << "\"")
			// use ranges of axis
			int axisYIndex = q->coordinateSystem(axis->coordinateSystemIndex())->yIndex();
			if (axis->rangeType() != Axis::RangeType::Auto || axis->orientation() != Axis::Orientation::Vertical || axisYIndex != i)
				continue;

			if (!qFuzzyIsNull(deltaYMax)) {
				axis->setUndoAware(false);
				axis->setSuppressRetransform(true);
				axis->setEnd(rangep.range.end());
				axis->setUndoAware(true);
				axis->setSuppressRetransform(false);
			}
			if (!qFuzzyIsNull(deltaYMin)) {
				axis->setUndoAware(false);
				axis->setSuppressRetransform(true);
				axis->setStart(rangep.range.start());
				axis->setUndoAware(true);
				axis->setSuppressRetransform(false);
			}
			//TODO;
// 			if (axis->position() == Axis::Position::Centered && deltaYMin != 0) {
// 				axis->setOffset(axis->offset() + deltaYMin, false);
// 			}
		}
	}

	// call retransform() on the parent to trigger the update of all axes and curves.
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

	double paddingLeft = horizontalPadding;
	double paddingRight = rightPadding;
	double paddingTop = verticalPadding;
	double paddingBottom = bottomPadding;
	if (symmetricPadding) {
		paddingRight = horizontalPadding;
		paddingBottom = verticalPadding;
	}

	dataRect.setX(dataRect.x() + paddingLeft);
	dataRect.setY(dataRect.y() + paddingTop);

	double newHeight = dataRect.height() - paddingBottom;
	if (newHeight < 0)
		newHeight = 0;
	dataRect.setHeight(newHeight);

	double newWidth = dataRect.width() - paddingRight;
	if (newWidth < 0)
		newWidth = 0;
	dataRect.setWidth(newWidth);
}

CartesianCoordinateSystem* CartesianPlotPrivate::coordinateSystem(const int index) const {
	if (index < 0)
		return defaultCoordinateSystem();

	return static_cast<CartesianCoordinateSystem*>(q->m_coordinateSystems.at(index));
}

QVector<AbstractCoordinateSystem*> CartesianPlotPrivate::coordinateSystems() const {
	return q->m_coordinateSystems;
}

void CartesianPlotPrivate::rangeChanged() {
	DEBUG(Q_FUNC_INFO)
	for (int i = 0; i < q->m_coordinateSystems.count(); i++) {
		int xIndex = coordinateSystem(i)->xIndex();
		int yIndex = coordinateSystem(i)->yIndex();
		xRanges[xIndex].dirty = true;
		yRanges[yIndex].dirty = true;
		if (autoScaleX(xIndex) && autoScaleY(yIndex))
			q->scaleAuto(xIndex, yIndex);
		else if (autoScaleX(xIndex))
			q->scaleAutoX(xIndex);
		else if (autoScaleY(yIndex))
			q->scaleAutoY(yIndex);
	}
}

void CartesianPlotPrivate::xRangeFormatChanged() {
	DEBUG(Q_FUNC_INFO)
	for (auto* axis : q->children<Axis>()) {
		//TODO: only if x range of axis's plot range is changed
		if (axis->orientation() == Axis::Orientation::Horizontal)
			axis->retransformTickLabelStrings();
	}
}
void CartesianPlotPrivate::yRangeFormatChanged() {
	DEBUG(Q_FUNC_INFO)
	for (auto* axis : q->children<Axis>()) {
		//TODO: only if x range of axis's plot range is changed
		if (axis->orientation() == Axis::Orientation::Vertical)
			axis->retransformTickLabelStrings();
	}
}

/*!
* helper function for checkXRange() and checkYRange()
*/
Range<double> CartesianPlotPrivate::checkRange(Range<double> range) {
	double start = range.start(), end = range.end();
	const auto scale = range.scale();
	if (start > 0 && end > 0)	// nothing to do
		return range;
	if (start >= 0 && end >= 0 && scale == RangeT::Scale::Sqrt)	// nothing to do
		return range;
	//TODO: check if start == end?

	double min = 0.01, max = 1.;

	if (scale == RangeT::Scale::Sqrt) {
		if (start < 0)
			start = 0.;
	} else if (start <= 0)
		start = min;
	if (scale == RangeT::Scale::Sqrt) {
		if (end < 0)
			end = max;
	} else if (end <= 0)
		end = max;

	return Range<double>(start, end);
}

/*!
 * check for negative values in the x range when non-linear scalings are used
 */
void CartesianPlotPrivate::checkXRange(int index) {
	const auto xRange = xRanges.at(index).range;
	DEBUG(Q_FUNC_INFO << ", x range " << index+1 << " : " << xRange.toStdString()
		<< ", scale = " << ENUM_TO_STRING(RangeT, Scale, xRange.scale()))

	const auto newRange = checkRange(xRange);

	const double start = newRange.start(), end = newRange.end();
	if (start != xRange.start()) {
		DEBUG(Q_FUNC_INFO << ", old/new start = " << xRange.start() << "/" << start)
		q->setXMin(index, start);
	}
	if (end != xRange.end()) {
		DEBUG(Q_FUNC_INFO << ", old/new end = " << xRange.end() << "/" << end)
		q->setXMax(index, end);
	}
}

/*!
 * check for negative values in the y range when non-linear scalings are used
 */
void CartesianPlotPrivate::checkYRange(int index) {
	const auto yRange = yRanges.at(index).range;
	DEBUG(Q_FUNC_INFO << ", y range " << index+1 << " : " << yRange.toStdString()
		<< ", scale = " << ENUM_TO_STRING(RangeT, Scale, yRange.scale()))

	const auto newRange = checkRange(yRange);

	const double start = newRange.start(), end = newRange.end();
	if (start != yRange.start()) {
		DEBUG(Q_FUNC_INFO << ", old/new start = " << yRange.start() << "/" << start)
		q->setYMin(index, start);
	}
	if (end != yRange.end()) {
		DEBUG(Q_FUNC_INFO << ", old/new end = " << yRange.end() << "/" << end)
		q->setYMax(index, end);
	}
}

CartesianScale* CartesianPlotPrivate::createScale(RangeT::Scale scale, const Range<double>& sceneRange, const Range<double>& logicalRange) {
	DEBUG( Q_FUNC_INFO << ", scene range : " << sceneRange.toStdString() << ", logical range : " << logicalRange.toStdString() );

	Range<double> range(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max());

	switch (scale) {
	case RangeT::Scale::Linear:
		return CartesianScale::createLinearScale(range, sceneRange, logicalRange);
	case RangeT::Scale::Log10:
	case RangeT::Scale::Log2:
	case RangeT::Scale::Ln:
		return CartesianScale::createLogScale(range, sceneRange, logicalRange, scale);
	case RangeT::Scale::Sqrt:
		return CartesianScale::createSqrtScale(range, sceneRange, logicalRange);
	case RangeT::Scale::Square:
		return CartesianScale::createSquareScale(range, sceneRange, logicalRange);
	case RangeT::Scale::Inverse:
		return CartesianScale::createInverseScale(range, sceneRange, logicalRange);
	}

	return nullptr;
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
		newRect.setX(x - w/2);
		newRect.setY(y - h/2);
		newRect.setWidth(w);
		newRect.setHeight(h);
		emit q->rectChanged(newRect);
	}
	return QGraphicsItem::itemChange(change, value);
}

//##############################################################################
//##################################  Events  ##################################
//##############################################################################

void CartesianPlotPrivate::contextMenuEvent(QGraphicsSceneContextMenuEvent* event) {
	const auto* cSystem{ defaultCoordinateSystem() };
	scenePos = event->pos();
	logicalPos = cSystem->mapSceneToLogical(scenePos, AbstractCoordinateSystem::MappingFlag::Limit);
	calledFromContextMenu = true;
	auto* menu = q->createContextMenu();
	menu->exec(event->screenPos());
}

/*!
 * \brief CartesianPlotPrivate::mousePressEvent
 * In this function only basic stuff is done. The mousePressEvent is forwarded to the Worksheet, which
 * has access to all cartesian plots and can apply the changes to all plots if the option "applyToAll"
 * is set. The worksheet calls then the corresponding mousepressZoomMode/CursorMode function in this class
 * This is done for mousePress, mouseMove and mouseRelease event
 * This function sends a signal with the logical position, because this is the only value which is the same
 * in all plots. Using the scene coordinates is not possible
 * \param event
 */
void CartesianPlotPrivate::mousePressEvent(QGraphicsSceneMouseEvent* event) {
	const auto* cSystem{ defaultCoordinateSystem() };
	auto*w = static_cast<Worksheet*>(q->parent(AspectType::Worksheet))->currentSelection();
	int index = Worksheet::cSystemIndex(w);
	if (!w || w->parent(AspectType::CartesianPlot) != q)
		index = -1;
	else if (index >= 0)
		cSystem = static_cast<CartesianCoordinateSystem*>(q->m_coordinateSystems[index]);
	if (mouseMode == CartesianPlot::MouseMode::Selection) {
		if (!locked && dataRect.contains(event->pos())) {
			panningStarted = true;
			m_panningStart = event->pos();
			setCursor(Qt::ClosedHandCursor);
		}
	} else if (mouseMode == CartesianPlot::MouseMode::ZoomSelection
		|| mouseMode == CartesianPlot::MouseMode::ZoomXSelection
		|| mouseMode == CartesianPlot::MouseMode::ZoomYSelection) {
		const QPointF logicalPos = cSystem->mapSceneToLogical(event->pos(), AbstractCoordinateSystem::MappingFlag::Limit);
		emit q->mousePressZoomSelectionModeSignal(logicalPos);
	} else if (mouseMode == CartesianPlot::MouseMode::Cursor) {
		setCursor(Qt::SizeHorCursor);
		const QPointF logicalPos = cSystem->mapSceneToLogical(event->pos(), AbstractCoordinateSystem::MappingFlag::Limit);
		double cursorPenWidth2 = cursorPen.width()/2.;
		if (cursorPenWidth2 < 10.)
			cursorPenWidth2 = 10.;

		bool visible;
		if (cursor0Enable && qAbs(event->pos().x() - cSystem->mapLogicalToScene(QPointF(cursor0Pos.x(), yRange().start()), visible).x()) < cursorPenWidth2) {
			selectedCursor = 0;
		} else if (cursor1Enable && qAbs(event->pos().x() - cSystem->mapLogicalToScene(QPointF(cursor1Pos.x(), yRange().start()), visible).x()) < cursorPenWidth2) {
			selectedCursor = 1;
		} else if (QApplication::keyboardModifiers() & Qt::ControlModifier) {
			cursor1Enable = true;
			selectedCursor = 1;
			emit q->cursor1EnableChanged(cursor1Enable);
		} else {
			cursor0Enable = true;
			selectedCursor = 0;
			emit q->cursor0EnableChanged(cursor0Enable);
		}
		emit q->mousePressCursorModeSignal(selectedCursor, logicalPos);
	}

	QGraphicsItem::mousePressEvent(event);
}

void CartesianPlotPrivate::mousePressZoomSelectionMode(QPointF logicalPos, int cSystemIndex) {
	//DEBUG(Q_FUNC_INFO << ", csystem index = " << cSystemIndex)
	const CartesianCoordinateSystem* cSystem;
	if (cSystemIndex == -1 || cSystemIndex >= q->m_coordinateSystems.count())
		cSystem = defaultCoordinateSystem();
	else
		cSystem = static_cast<CartesianCoordinateSystem*>(q->m_coordinateSystems.at(cSystemIndex));

	int xIndex = cSystem->xIndex();
	int yIndex = cSystem->yIndex();

	bool visible;
	const QPointF scenePos = cSystem->mapLogicalToScene(logicalPos, visible, AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);
	if (mouseMode == CartesianPlot::MouseMode::ZoomSelection) {
		if (logicalPos.x() < xRange(xIndex).start())
			logicalPos.setX(xRange(xIndex).start());

		if (logicalPos.x() > xRange(xIndex).end())
			logicalPos.setX(xRange(xIndex).end());

		if (logicalPos.y() < yRange(yIndex).start())
			logicalPos.setY(yRange(yIndex).start());

		if (logicalPos.y() > yRange(yIndex).end())
			logicalPos.setY(yRange(yIndex).end());

		m_selectionStart = scenePos;
	} else if (mouseMode == CartesianPlot::MouseMode::ZoomXSelection) {
		logicalPos.setY(yRange(yIndex).start()); // must be done, because the other plots can have other ranges, value must be in the scenes
		m_selectionStart.setX(scenePos.x());
		m_selectionStart.setY(dataRect.y());
	} else if (mouseMode == CartesianPlot::MouseMode::ZoomYSelection) {
		logicalPos.setX(xRange(xIndex).start()); // must be done, because the other plots can have other ranges, value must be in the scenes
		m_selectionStart.setX(dataRect.x());
		m_selectionStart.setY(scenePos.y());
	}
	m_selectionEnd = m_selectionStart;
	m_selectionBandIsShown = true;
}

void CartesianPlotPrivate::mousePressCursorMode(int cursorNumber, QPointF logicalPos) {

	cursorNumber == 0 ? cursor0Enable = true : cursor1Enable = true;

	QPointF p1(logicalPos.x(), yRange().start());
	QPointF p2(logicalPos.x(), yRange().end());

	if (cursorNumber == 0) {
		cursor0Pos.setX(logicalPos.x());
		cursor0Pos.setY(0);
	} else {
		cursor1Pos.setX(logicalPos.x());
		cursor1Pos.setY(0);
	}
	update();
}

void CartesianPlotPrivate::updateCursor() {
	update();
}

void CartesianPlotPrivate::setZoomSelectionBandShow(bool show) {
	m_selectionBandIsShown = show;
}

void CartesianPlotPrivate::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
	const auto* cSystem{ defaultCoordinateSystem() };
	auto*w = static_cast<Worksheet*>(q->parent(AspectType::Worksheet))->currentSelection();
	int index = Worksheet::cSystemIndex(w);
	if (!w || w->parent(AspectType::CartesianPlot) != q)
		index = -1;
	else if (index >= 0)
		cSystem = static_cast<CartesianCoordinateSystem*>(q->m_coordinateSystems[index]);

	if (mouseMode == CartesianPlot::MouseMode::Selection) {
		if (panningStarted && dataRect.contains(event->pos()) ) {
			//don't retransform on small mouse movement deltas
			const int deltaXScene = (m_panningStart.x() - event->pos().x());
			const int deltaYScene = (m_panningStart.y() - event->pos().y());
			if (qAbs(deltaXScene) < 5 && qAbs(deltaYScene) < 5)
				return;

			const QPointF logicalEnd = cSystem->mapSceneToLogical(event->pos());
			const QPointF logicalStart = cSystem->mapSceneToLogical(m_panningStart);
			m_panningStart = event->pos();
			emit q->mouseMoveSelectionModeSignal(logicalStart, logicalEnd);
		} else
			QGraphicsItem::mouseMoveEvent(event);
	} else if (mouseMode == CartesianPlot::MouseMode::ZoomSelection
		|| mouseMode == CartesianPlot::MouseMode::ZoomXSelection
		|| mouseMode == CartesianPlot::MouseMode::ZoomYSelection) {
		QGraphicsItem::mouseMoveEvent(event);
		if ( !boundingRect().contains(event->pos()) ) {
			q->info(QString());
			return;
		}
		const QPointF logicalPos = cSystem->mapSceneToLogical(event->pos(), AbstractCoordinateSystem::MappingFlag::Limit);
		emit q->mouseMoveZoomSelectionModeSignal(logicalPos);

	} else if (mouseMode == CartesianPlot::MouseMode::Cursor) {
		QGraphicsItem::mouseMoveEvent(event);
		if (!boundingRect().contains(event->pos())) {
			q->info(i18n("Not inside of the bounding rect"));
			return;
		}

		// updating treeview data and cursor position
		// updating cursor position is done in Worksheet, because
		// multiple plots must be updated
		const QPointF logicalPos = cSystem->mapSceneToLogical(event->pos(), AbstractCoordinateSystem::MappingFlag::Limit);
		emit q->mouseMoveCursorModeSignal(selectedCursor, logicalPos);
	}
}

bool CartesianPlotPrivate::translateRange(int xIndex, int yIndex, const QPointF& logicalStart, const QPointF& logicalEnd, bool translateX, bool translateY)
{
	//handle the change in x
	bool translationX = false, translationY = false;
	if (translateX && logicalStart.x() - logicalEnd.x() != 0) { // TODO: find better method
		translationX = true;
		double start{ logicalStart.x() }, end{ logicalEnd.x() };
		switch (xRange(xIndex).scale()) {
		case RangeT::Scale::Linear: {
			const double delta = (start - end);
			xRange(xIndex).translate(delta);
			break;
		}
		case RangeT::Scale::Log10: {
			if (end == 0 || start / end <= 0)
				break;
			const double delta = log10(start / end);
			xRange(xIndex) *= pow(10, delta);
			break;
		}
		case RangeT::Scale::Log2: {
			if (end == 0 || start / end <= 0)
				break;
			const double delta = log2(start / end);
			xRange(xIndex) *= exp2(delta);
			break;
		}
		case RangeT::Scale::Ln: {
			if (end == 0 || start / end <= 0)
				break;
			const double delta = log(start / end);
			xRange(xIndex) *= exp(delta);
			break;
		}
		case RangeT::Scale::Sqrt: {
			if (start < 0 || end < 0)
				break;
			const double delta = sqrt(start) - sqrt(end);
			xRange(xIndex).translate(delta*delta);
			break;
		}
		case RangeT::Scale::Square: {
			if (end <= start)
				break;
			const double delta = end*end - start*start;
			xRange(xIndex).translate(sqrt(delta));
			break;
		}
		case RangeT::Scale::Inverse: {
			if (start == 0. || end == 0. || end <= start)
				break;
			const double delta = 1./start - 1./end;
			xRange(xIndex).translate(1./delta);
			break;
		}
		}
	}

	if (translateY && logicalStart.y() - logicalEnd.y() != 0) {
		translationY = true;
		//handle the change in y
		double start = logicalStart.y();
		double end = logicalEnd.y();
		switch (yRange(yIndex).scale()) {
		case RangeT::Scale::Linear: {
			const double deltaY = (start - end);
			yRange(yIndex).translate(deltaY);
			break;
		}
		case RangeT::Scale::Log10: {
			if (end == 0 || start / end <= 0)
				break;
			const double deltaY = log10(start / end);
			yRange(yIndex) *= pow(10, deltaY);
			break;
		}
		case RangeT::Scale::Log2: {
			if (end == 0 || start / end <= 0)
				break;
			const double deltaY = log2(start / end);
			yRange(yIndex) *= exp2(deltaY);
			break;
		}
		case RangeT::Scale::Ln: {
			if (end == 0 || start / end <= 0)
				break;
			const double deltaY = log(start / end);
			yRange(yIndex) *= exp(deltaY);
			break;
		}
		case RangeT::Scale::Sqrt: {
			if (start < 0 || end < 0)
				break;
			const double delta = sqrt(start) - sqrt(end);
			yRange(yIndex).translate(delta*delta);
			break;
		}
		case RangeT::Scale::Square: {
			if (end <= start)
				break;
			const double delta = end*end - start*start;
			yRange(yIndex).translate(sqrt(delta));
			break;
		}
		case RangeT::Scale::Inverse: {
			if (start == 0. || end == 0. || end <= start)
				break;
			const double delta = 1./start - 1./end;
			yRange(yIndex).translate(1./delta);
			break;
		}
		}
	}

	q->setUndoAware(false);
	if (translationX)
		q->setAutoScaleX(xIndex, false);
	if (translationY)
		q->setAutoScaleY(yIndex, false);
	q->setUndoAware(true);

	// If x or y should not be translated, means, that it was done before
	// so the ranges must get dirty.
	if (translationX || translationY || !translateX || !translateY) {
		q->setXRangeDirty(xIndex, true);
		q->setYRangeDirty(yIndex, true);
	}

	return translationX || translationY || !translateX || !translateY;
}

void CartesianPlotPrivate::mouseMoveSelectionMode(QPointF logicalStart, QPointF logicalEnd) {
	const bool autoscaleRanges = true; // consumes a lot of power, maybe making an option to turn off/on!
	auto*w = static_cast<Worksheet*>(q->parent(AspectType::Worksheet))->currentSelection();
	int index = Worksheet::cSystemIndex(w);
	if (!w || w->parent(AspectType::CartesianPlot) != q)
		index = -1;

	bool translated = false;
	int xIndex = -1;
	int yIndex = -1;
	if (index < 0) {
		QVector<int> translatedIndicesX, translatedIndicesY;
		for (int i=0; i < q->m_coordinateSystems.count(); i++) {
			auto cs = coordinateSystem(i);
			int xIndex = cs->xIndex();
			int yIndex = cs->yIndex();
			bool translateX = !translatedIndicesX.contains(xIndex);
			bool translateY = !translatedIndicesY.contains(yIndex);
			if (translateRange(xIndex, yIndex, logicalStart, logicalEnd, translateX, translateY)) {
				translated = true;
				if (autoscaleRanges && logicalStart.y() == logicalEnd.y() && autoScaleY(cs->yIndex())) {
					// only x was changed, so autoscale y
					q->scaleAutoY(cs->yIndex(), false, true);
				}
				if (autoscaleRanges && logicalStart.x() == logicalEnd.x() && autoScaleX(cs->xIndex())) {
					// only y was changed, so autoscale x
					q->scaleAutoX(cs->xIndex(), false, true);
				}
			}
			if (translateX)
				translatedIndicesX.append(static_cast<CartesianCoordinateSystem*>(q->m_coordinateSystems[i])->xIndex());
			if (translateY)
				translatedIndicesY.append(static_cast<CartesianCoordinateSystem*>(q->m_coordinateSystems[i])->yIndex());
		}
	} else {
		auto cs = coordinateSystem(index);
		xIndex = cs->xIndex();
		yIndex = cs->yIndex();
		translated = translateRange(xIndex, yIndex, logicalStart, logicalEnd, true, true);
		if (autoscaleRanges && logicalStart.y() == logicalEnd.y() && autoScaleY(yIndex)) {
			// only x was changed, so autoscale y
			q->scaleAutoY(yIndex, false, true);
		}
		if (autoscaleRanges && logicalStart.x() == logicalEnd.x() && autoScaleX(xIndex)) {
			// only y was changed, so autoscale x
			q->scaleAutoX(xIndex, false, true);
		}
	}

	if (translated)
		retransformScales(xIndex, yIndex);
}

void CartesianPlotPrivate::mouseMoveZoomSelectionMode(QPointF logicalPos, int cSystemIndex) {
	QString info;
	const CartesianCoordinateSystem* cSystem;
	if (cSystemIndex == -1 || cSystemIndex >= q->m_coordinateSystems.count()) {
		cSystem = defaultCoordinateSystem();
	} else
		cSystem = q->coordinateSystem(cSystemIndex);

	int xIndex = cSystem->xIndex();
	int yIndex = cSystem->yIndex();

	const auto xRangeFormat{ xRange(xIndex).format() };
	const auto yRangeFormat{ yRange(yIndex).format() };
	const auto xRangeDateTimeFormat{ xRange(xIndex).dateTimeFormat() };
	const QPointF logicalStart = cSystem->mapSceneToLogical(m_selectionStart, AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);

	if (mouseMode == CartesianPlot::MouseMode::ZoomSelection) {
		bool visible;
		m_selectionEnd = cSystem->mapLogicalToScene(logicalPos, visible, AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);
		QPointF logicalEnd = logicalPos;
		if (xRangeFormat == RangeT::Format::Numeric)
			info = QString::fromUtf8("Δx=") + QString::number(logicalEnd.x()-logicalStart.x());
		else
			info = i18n("from x=%1 to x=%2", QDateTime::fromMSecsSinceEpoch(logicalStart.x()).toString(xRangeDateTimeFormat),
						QDateTime::fromMSecsSinceEpoch(logicalEnd.x()).toString(xRangeDateTimeFormat));

		info += QLatin1String(", ");
		if (yRangeFormat == RangeT::Format::Numeric)
			info += QString::fromUtf8("Δy=") + QString::number(logicalEnd.y()-logicalStart.y());
		else
			info += i18n("from y=%1 to y=%2", QDateTime::fromMSecsSinceEpoch(logicalStart.y()).toString(xRangeDateTimeFormat),
						 QDateTime::fromMSecsSinceEpoch(logicalEnd.y()).toString(xRangeDateTimeFormat));
	} else if (mouseMode == CartesianPlot::MouseMode::ZoomXSelection) {
		logicalPos.setY(yRange(yIndex).start()); // must be done, because the other plots can have other ranges, value must be in the scenes
		bool visible;
		m_selectionEnd.setX(cSystem->mapLogicalToScene(logicalPos, visible, CartesianCoordinateSystem::MappingFlag::SuppressPageClipping).x());//event->pos().x());
		m_selectionEnd.setY(dataRect.bottom());
		QPointF logicalEnd = logicalPos;
	if (xRangeFormat == RangeT::Format::Numeric)
			info = QString::fromUtf8("Δx=") + QString::number(logicalEnd.x()-logicalStart.x());
		else
			info = i18n("from x=%1 to x=%2", QDateTime::fromMSecsSinceEpoch(logicalStart.x()).toString(xRangeDateTimeFormat),
						QDateTime::fromMSecsSinceEpoch(logicalEnd.x()).toString(xRangeDateTimeFormat));
	} else if (mouseMode == CartesianPlot::MouseMode::ZoomYSelection) {
		m_selectionEnd.setX(dataRect.right());
		logicalPos.setX(xRange(xIndex).start()); // must be done, because the other plots can have other ranges, value must be in the scenes
		bool visible;
		m_selectionEnd.setY(cSystem->mapLogicalToScene(logicalPos, visible, CartesianCoordinateSystem::MappingFlag::SuppressPageClipping).y());//event->pos().y());
		QPointF logicalEnd = logicalPos;
		if (yRangeFormat == RangeT::Format::Numeric)
			info = QString::fromUtf8("Δy=") + QString::number(logicalEnd.y()-logicalStart.y());
		else
			info = i18n("from y=%1 to y=%2", QDateTime::fromMSecsSinceEpoch(logicalStart.y()).toString(xRangeDateTimeFormat),
						QDateTime::fromMSecsSinceEpoch(logicalEnd.y()).toString(xRangeDateTimeFormat));
	}
	q->info(info);
	update();
}

void CartesianPlotPrivate::mouseMoveCursorMode(int cursorNumber, QPointF logicalPos) {
	const auto xRangeFormat{ xRange().format() };
	const auto xRangeDateTimeFormat{ xRange().dateTimeFormat() };

	QPointF p1(logicalPos.x(), 0);
	cursorNumber == 0 ? cursor0Pos = p1 : cursor1Pos = p1;

	QString info;
	if (xRangeFormat == RangeT::Format::Numeric)
		info = QString::fromUtf8("x=") + QString::number(logicalPos.x());
	else
		info = i18n("x=%1", QDateTime::fromMSecsSinceEpoch(logicalPos.x()).toString(xRangeDateTimeFormat));
	q->info(info);
	update();
}

void CartesianPlotPrivate::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
	if (mouseMode == CartesianPlot::MouseMode::Selection) {
		setCursor(Qt::ArrowCursor);
		panningStarted = false;

		//TODO: why do we do this all the time?!?!
		const QPointF& itemPos = pos();//item's center point in parent's coordinates;
		const qreal x = itemPos.x();
		const qreal y = itemPos.y();

		//calculate the new rect and set it
		QRectF newRect;
		const qreal w = rect.width();
		const qreal h = rect.height();
		newRect.setX(x - w/2);
		newRect.setY(y - h/2);
		newRect.setWidth(w);
		newRect.setHeight(h);

		// TODO: autoscale

		suppressRetransform = true;
		q->setRect(newRect);
		suppressRetransform = false;

		QGraphicsItem::mouseReleaseEvent(event);
	} else if (mouseMode == CartesianPlot::MouseMode::ZoomSelection || mouseMode == CartesianPlot::MouseMode::ZoomXSelection
			|| mouseMode == CartesianPlot::MouseMode::ZoomYSelection) {
		emit q->mouseReleaseZoomSelectionModeSignal();
	}
}

bool CartesianPlotPrivate::mouseReleaseZoomSelectionMode(int cSystemIndex, bool suppressRetransform) {
	//don't zoom if very small region was selected, avoid occasional/unwanted zooming
	if ( qAbs(m_selectionEnd.x() - m_selectionStart.x()) < 20 && qAbs(m_selectionEnd.y() - m_selectionStart.y()) < 20 ) {
		m_selectionBandIsShown = false;
		return false;
	}
	bool retransformPlot = true;
	int xIndex = -1, yIndex = -1;
	if (cSystemIndex == -1 || cSystemIndex >= q->m_coordinateSystems.count()) {
		retransformPlot = false; // do a retransform only when at least one requested
		for (int i=0; i < q->m_coordinateSystems.count(); i++) {
			if (mouseReleaseZoomSelectionMode(i, true))
				retransformPlot = true;
		}
	} else {

		auto cSystem = coordinateSystem(cSystemIndex);
		xIndex = cSystem->xIndex();
		yIndex = cSystem->yIndex();

		//determine the new plot ranges
		QPointF logicalZoomStart = cSystem->mapSceneToLogical(m_selectionStart, AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);
		QPointF logicalZoomEnd = cSystem->mapSceneToLogical(m_selectionEnd, AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);
		if (m_selectionEnd.x() > m_selectionStart.x())
			xRange(xIndex).setRange(logicalZoomStart.x(), logicalZoomEnd.x());
		else
			xRange(xIndex).setRange(logicalZoomEnd.x(), logicalZoomStart.x());
		xRange(xIndex).niceExtend();

		if (m_selectionEnd.y() > m_selectionStart.y())
			yRange(yIndex).setRange(logicalZoomEnd.y(), logicalZoomStart.y());
		else
			yRange(yIndex).setRange(logicalZoomStart.y(), logicalZoomEnd.y());
		yRange(yIndex).niceExtend();

		if (mouseMode == CartesianPlot::MouseMode::ZoomSelection) {
			q->setXRangeDirty(xIndex, true);
			q->setYRangeDirty(yIndex, true);
			q->setAutoScaleX(xIndex, false);
			q->setAutoScaleY(yIndex, false);
		} else if (mouseMode == CartesianPlot::MouseMode::ZoomXSelection) {
			q->setYRangeDirty(yIndex, true);
			q->setAutoScaleX(xIndex, false);
			if (q->autoScaleY(yIndex) && q->scaleAutoY(yIndex, false, true))
				retransformPlot = false;
		} else if (mouseMode == CartesianPlot::MouseMode::ZoomYSelection) {
			q->setXRangeDirty(xIndex, true);
			q->setAutoScaleY(yIndex, false);
			if (q->autoScaleX(xIndex) && q->scaleAutoX(xIndex, false, true))
				retransformPlot = false;
		}
	}

	if (!suppressRetransform) {
		//if (retransformPlot), because autoscale suppress and therefore a retransform is neccessary
			retransformScales(xIndex, yIndex);

		m_selectionBandIsShown = false;
	}

	return retransformPlot;
}

void CartesianPlotPrivate::wheelEvent(QGraphicsSceneWheelEvent* event) {
	if (locked)
		return;

	auto*w = static_cast<Worksheet*>(q->parent(AspectType::Worksheet))->currentSelection();
	int cSystemIndex = Worksheet::cSystemIndex(w);
	int xIndex = -1, yIndex = -1;
	if (!w || w->parent(AspectType::CartesianPlot) != q)
		cSystemIndex = -1;
	else {
		xIndex = coordinateSystem(cSystemIndex)->xIndex();
		yIndex = coordinateSystem(cSystemIndex)->yIndex();
	}

	//determine first, which axes are selected and zoom only in the corresponding direction.
	//zoom the entire plot if no axes selected.
	bool zoomX = false;
	bool zoomY = false;
	/*for (auto* axis : q->children<Axis>()) {
		if (!axis->graphicsItem()->isSelected() && !axis->isHovered())
			continue;

		if (axis->orientation() == Axis::Orientation::Horizontal) {
			zoomX  = true;
			cSystemIndex = axis->coordinateSystemIndex();
			break;
		} else {
			zoomY = true;
			cSystemIndex = axis->coordinateSystemIndex();
			break;
		}
	}*/

	if (event->delta() > 0) {
		if (!zoomX && !zoomY) {
			//no special axis selected -> zoom in everything
			q->zoomIn(-1, -1);
		} else {
			if (zoomX) q->zoomInX(xIndex);
			if (zoomY) q->zoomInY(yIndex);
		}
	} else {
		if (!zoomX && !zoomY) {
			//no special axis selected -> zoom in everything
			q->zoomOut(-1, -1);
		} else {
			if (zoomX) q->zoomOutX(xIndex);
			if (zoomY) q->zoomOutY(yIndex);
		}
	}
}

void CartesianPlotPrivate::keyPressEvent(QKeyEvent* event) {
	if (event->key() == Qt::Key_Escape) {
		m_selectionBandIsShown = false;
	} else if (event->key() == Qt::Key_Left || event->key() == Qt::Key_Right
		|| event->key() == Qt::Key_Up ||event->key() == Qt::Key_Down) {

		const auto* worksheet = static_cast<const Worksheet*>(q->parentAspect());
		if (worksheet->layout() == Worksheet::Layout::NoLayout) {
			const int delta = 5;
			QRectF rect = q->rect();

			if (event->key() == Qt::Key_Left) {
				rect.setX(rect.x() - delta);
				rect.setWidth(rect.width() - delta);
			} else if (event->key() == Qt::Key_Right) {
				rect.setX(rect.x() + delta);
				rect.setWidth(rect.width() + delta);
			} else if (event->key() == Qt::Key_Up) {
				rect.setY(rect.y() - delta);
				rect.setHeight(rect.height() - delta);
			} else if (event->key() == Qt::Key_Down) {
				rect.setY(rect.y() + delta);
				rect.setHeight(rect.height() + delta);
			}

			q->setRect(rect);
		}
	}

	QGraphicsItem::keyPressEvent(event);
}

void CartesianPlotPrivate::hoverMoveEvent(QGraphicsSceneHoverEvent* event) {
	QPointF point = event->pos();
	QString info;
	const auto* cSystem{ defaultCoordinateSystem() };
	auto* w = static_cast<Worksheet*>(q->parent(AspectType::Worksheet))->currentSelection();
	int index = Worksheet::cSystemIndex(w);
	int xIndex = cSystem->xIndex(), yIndex = cSystem->yIndex();
	if (!w || w->parent(AspectType::CartesianPlot) != q) {
		index = -1;
		xIndex = -1;
		yIndex = -1;
	} else if (index >= 0) {
		cSystem = static_cast<CartesianCoordinateSystem*>(q->m_coordinateSystems[index]);
		xIndex = cSystem->xIndex();
		yIndex = cSystem->yIndex();
	}

	const auto xRangeFormat{ xRange(xIndex).format() };
	const auto yRangeFormat{ yRange(yIndex).format() };
	const auto xRangeDateTimeFormat{ xRange(xIndex).dateTimeFormat() };
	const auto yRangeDateTimeFormat{ yRange(yIndex).dateTimeFormat() };
	if (dataRect.contains(point)) {
		QPointF logicalPoint = cSystem->mapSceneToLogical(point);

		if ((mouseMode == CartesianPlot::MouseMode::ZoomSelection)
			|| mouseMode == CartesianPlot::MouseMode::Selection
			|| mouseMode == CartesianPlot::MouseMode::Crosshair) {
			info = "x=";
			if (xRangeFormat == RangeT::Format::Numeric)
				 info += QString::number(logicalPoint.x());
			else
				info += QDateTime::fromMSecsSinceEpoch(logicalPoint.x()).toString(xRangeDateTimeFormat);

			info += ", y=";
			if (yRangeFormat == RangeT::Format::Numeric)
				info += QString::number(logicalPoint.y());
			else
				info += QDateTime::fromMSecsSinceEpoch(logicalPoint.y()).toString(yRangeDateTimeFormat);
		}

		if (mouseMode == CartesianPlot::MouseMode::ZoomSelection && !m_selectionBandIsShown) {
			emit q->mouseHoverZoomSelectionModeSignal(logicalPoint);
		} else if (mouseMode == CartesianPlot::MouseMode::ZoomXSelection && !m_selectionBandIsShown) {
			info = "x=";
			if (xRangeFormat == RangeT::Format::Numeric)
				 info += QString::number(logicalPoint.x());
			else
				info += QDateTime::fromMSecsSinceEpoch(logicalPoint.x()).toString(xRangeDateTimeFormat);
			emit q->mouseHoverZoomSelectionModeSignal(logicalPoint);
		} else if (mouseMode == CartesianPlot::MouseMode::ZoomYSelection && !m_selectionBandIsShown) {
			info = "y=";
			if (yRangeFormat == RangeT::Format::Numeric)
				info += QString::number(logicalPoint.y());
			else
				info += QDateTime::fromMSecsSinceEpoch(logicalPoint.y()).toString(yRangeDateTimeFormat);
			emit q->mouseHoverZoomSelectionModeSignal(logicalPoint);
		} else if (mouseMode == CartesianPlot::MouseMode::Selection) {
			// hover the nearest curve to the mousepointer
			// hovering curves is implemented in the parent, because no ignoreEvent() exists
			// for it. Checking all curves and hover the first
			bool hovered = false;
			const auto& curves = q->children<Curve>();
			for (int i = curves.count() - 1; i >= 0; i--) { // because the last curve is above the other curves
				auto* curve = curves[i];
				if (hovered) { // if a curve is already hovered, disable hover for the rest
					curve->setHover(false);
					continue;
				}
				if (curve->activateCurve(event->pos())) {
					curve->setHover(true);
					hovered = true;
					continue;
				}
				curve->setHover(false);
			}
		} else if (mouseMode == CartesianPlot::MouseMode::Crosshair) {
			m_crosshairPos = event->pos();
			update();
		} else if (mouseMode == CartesianPlot::MouseMode::Cursor) {
			info = "x=";
			if (yRangeFormat == RangeT::Format::Numeric)
				info += QString::number(logicalPoint.x());
			else
				info += QDateTime::fromMSecsSinceEpoch(logicalPoint.x()).toString(xRangeDateTimeFormat);

			double cursorPenWidth2 = cursorPen.width()/2.;
			if (cursorPenWidth2 < 10.)
				cursorPenWidth2 = 10.;

			bool visible;
			if ((cursor0Enable && qAbs(point.x() - defaultCoordinateSystem()->mapLogicalToScene(QPointF(cursor0Pos.x(), yRange().start()), visible).x()) < cursorPenWidth2) ||
					(cursor1Enable && qAbs(point.x() - defaultCoordinateSystem()->mapLogicalToScene(QPointF(cursor1Pos.x(), yRange().start()), visible).x()) < cursorPenWidth2))
				setCursor(Qt::SizeHorCursor);
			else
				setCursor(Qt::ArrowCursor);

			update();
		}
	} else
		emit q->mouseHoverOutsideDataRectSignal();

	q->info(info);

	QGraphicsItem::hoverMoveEvent(event);
}

void CartesianPlotPrivate::mouseHoverOutsideDataRect() {
	m_insideDataRect = false;
	update();
}

void CartesianPlotPrivate::hoverLeaveEvent(QGraphicsSceneHoverEvent* event) {
	for (auto* curve : q->children<XYCurve>())
		curve->setHover(false);

	m_hovered = false;
	QGraphicsItem::hoverLeaveEvent(event);
}

void CartesianPlotPrivate::mouseHoverZoomSelectionMode(QPointF logicPos, int cSystemIndex) {
	m_insideDataRect = true;

	const CartesianCoordinateSystem* cSystem;
	auto* w = static_cast<Worksheet*>(q->parent(AspectType::Worksheet))->currentSelection();
	int index = Worksheet::cSystemIndex(w);
	if (w && w->parent(AspectType::CartesianPlot) == q && index != -1)
		cSystem = coordinateSystem(index);
	else if (cSystemIndex == -1 || cSystemIndex >= q->m_coordinateSystems.count())
		cSystem = defaultCoordinateSystem();
	else
		cSystem = coordinateSystem(cSystemIndex);

	bool visible;
	if (mouseMode == CartesianPlot::MouseMode::ZoomSelection && !m_selectionBandIsShown) {

	} else if (mouseMode == CartesianPlot::MouseMode::ZoomXSelection && !m_selectionBandIsShown) {
		QPointF p1(logicPos.x(), yRange(cSystem->yIndex()).start());
		QPointF p2(logicPos.x(), yRange(cSystem->yIndex()).end());
		m_selectionStartLine.setP1(cSystem->mapLogicalToScene(p1, visible, CartesianCoordinateSystem::MappingFlag::Limit));
		m_selectionStartLine.setP2(cSystem->mapLogicalToScene(p2, visible, CartesianCoordinateSystem::MappingFlag::Limit));
	} else if (mouseMode == CartesianPlot::MouseMode::ZoomYSelection && !m_selectionBandIsShown) {
		QPointF p1(xRange(cSystem->xIndex()).start(), logicPos.y());
		QPointF p2(xRange(cSystem->xIndex()).end(), logicPos.y());
		m_selectionStartLine.setP1(cSystem->mapLogicalToScene(p1, visible, CartesianCoordinateSystem::MappingFlag::Limit));
		m_selectionStartLine.setP2(cSystem->mapLogicalToScene(p2, visible, CartesianCoordinateSystem::MappingFlag::Limit));
	}

	update(); // because if previous another selection mode was selected, the lines must be deleted
}

void CartesianPlotPrivate::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget*) {
	if (!isVisible() || m_printing)
		return;

	if ((mouseMode == CartesianPlot::MouseMode::ZoomXSelection
		|| mouseMode == CartesianPlot::MouseMode::ZoomYSelection)
		&& (!m_selectionBandIsShown) && m_insideDataRect) {
		painter->setPen(zoomSelectPen);
		painter->drawLine(m_selectionStartLine);
	} else if (m_selectionBandIsShown) {
		QPointF selectionStart = m_selectionStart;
		if (m_selectionStart.x() > dataRect.right())
			selectionStart.setX(dataRect.right());
		if (m_selectionStart.x() < dataRect.left())
			selectionStart.setX(dataRect.left());
		if (m_selectionStart.y() > dataRect.bottom())
			selectionStart.setY(dataRect.bottom());
		if (m_selectionStart.y() < dataRect.top())
			selectionStart.setY(dataRect.top());

		QPointF selectionEnd = m_selectionEnd;
		if (m_selectionEnd.x() > dataRect.right())
			selectionEnd.setX(dataRect.right());
		if (m_selectionEnd.x() < dataRect.left())
			selectionEnd.setX(dataRect.left());
		if (m_selectionEnd.y() > dataRect.bottom())
			selectionEnd.setY(dataRect.bottom());
		if (m_selectionEnd.y() < dataRect.top())
			selectionEnd.setY(dataRect.top());
		painter->save();
		painter->setPen(zoomSelectPen);
		painter->drawRect(QRectF(selectionStart, selectionEnd));
		painter->setBrush(Qt::blue);
		painter->setOpacity(0.2);
		painter->drawRect(QRectF(selectionStart, selectionEnd));
		painter->restore();
	} else if (mouseMode == CartesianPlot::MouseMode::Crosshair) {
		painter->setPen(crossHairPen);

		//horizontal line
		double x1 = dataRect.left();
		double y1 = m_crosshairPos.y();
		double x2 = dataRect.right();
		double y2 = y1;
		painter->drawLine(x1, y1, x2, y2);

		//vertical line
		x1 = m_crosshairPos.x();
		y1 = dataRect.bottom();
		x2 = x1;
		y2 = dataRect.top();
		painter->drawLine(x1, y1, x2, y2);
	}

	//draw cursor lines if available
	if (cursor0Enable || cursor1Enable) {
		painter->save();
		painter->setPen(cursorPen);
		QFont font = painter->font();
		font.setPointSize(font.pointSize() * 4);
		painter->setFont(font);

		bool visible;
		QPointF p1 = defaultCoordinateSystem()->mapLogicalToScene(QPointF(cursor0Pos.x(), yRange().start()), visible);
		if (cursor0Enable && visible) {
			QPointF p2 = defaultCoordinateSystem()->mapLogicalToScene(QPointF(cursor0Pos.x(), yRange().end()), visible);
			painter->drawLine(p1, p2);
			QPointF textPos = p2;
			textPos.setX(p2.x() - m_cursor0Text.size().width()/2);
			textPos.setY(p2.y() - m_cursor0Text.size().height());
			if (textPos.y() < boundingRect().y())
				textPos.setY(boundingRect().y());
			painter->drawStaticText(textPos, m_cursor0Text);
		}

		p1 = defaultCoordinateSystem()->mapLogicalToScene(QPointF(cursor1Pos.x(), yRange().start()), visible);
		if (cursor1Enable && visible) {
			QPointF p2 = defaultCoordinateSystem()->mapLogicalToScene(QPointF(cursor1Pos.x(), yRange().end()), visible);
			painter->drawLine(p1, p2);
			QPointF textPos = p2;
			// TODO: Moving this stuff into other function to not calculate it every time
			textPos.setX(p2.x() - m_cursor1Text.size().width()/2);
			textPos.setY(p2.y() - m_cursor1Text.size().height());
			if (textPos.y() < boundingRect().y())
				textPos.setY(boundingRect().y());
			painter->drawStaticText(textPos, m_cursor1Text);
		}

		painter->restore();
	}

	const bool hovered = (m_hovered && !isSelected());
	const bool selected = isSelected();
	if ((hovered || selected)  && !m_printing) {
		static double penWidth = 2.;	// why static?
		const QRectF& br = q->m_plotArea->graphicsItem()->boundingRect();
		const qreal width = br.width();
		const qreal height = br.height();
		const QRectF rect = QRectF(-width/2 + penWidth/2, -height/2 + penWidth/2,
					  width - penWidth, height - penWidth);

		if (m_hovered)
			painter->setPen(QPen(QApplication::palette().color(QPalette::Shadow), penWidth));
		else
			painter->setPen(QPen(QApplication::palette().color(QPalette::Highlight), penWidth));

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

	//cursor
	writer->writeStartElement( "cursor" );
	WRITE_QPEN(d->cursorPen);
	writer->writeEndElement();
	//geometry
	writer->writeStartElement( "geometry" );
	writer->writeAttribute( "x", QString::number(d->rect.x()) );
	writer->writeAttribute( "y", QString::number(d->rect.y()) );
	writer->writeAttribute( "width", QString::number(d->rect.width()) );
	writer->writeAttribute( "height", QString::number(d->rect.height()) );
	writer->writeAttribute( "visible", QString::number(d->isVisible()) );
	writer->writeEndElement();

	//coordinate system and padding
	//new style
	writer->writeStartElement("xRanges");
	for (const auto& range : d->xRanges) {
		writer->writeStartElement("xRange");
		writer->writeAttribute( "autoScale", QString::number(range.range.autoScale()) );
		writer->writeAttribute( "start", QString::number(range.range.start(), 'g', 16));
		writer->writeAttribute( "end", QString::number(range.range.end(), 'g', 16) );
		writer->writeAttribute( "scale", QString::number(static_cast<int>(range.range.scale())) );
		writer->writeAttribute( "format", QString::number(static_cast<int>(range.range.format())) );
		writer->writeAttribute( "dateTimeFormat", range.range.dateTimeFormat() );
		writer->writeEndElement();
	}
	writer->writeEndElement();
	writer->writeStartElement("yRanges");
	for (const auto& range : d->yRanges) {
		writer->writeStartElement("yRange");
		writer->writeAttribute( "autoScale", QString::number(range.range.autoScale()) );
		writer->writeAttribute( "start", QString::number(range.range.start(), 'g', 16));
		writer->writeAttribute( "end", QString::number(range.range.end(), 'g', 16) );
		writer->writeAttribute( "scale", QString::number(static_cast<int>(range.range.scale())) );
		writer->writeAttribute( "format", QString::number(static_cast<int>(range.range.format())) );
		writer->writeAttribute( "dateTimeFormat", range.range.dateTimeFormat() );
		writer->writeEndElement();
	}
	writer->writeEndElement();
	writer->writeStartElement("coordinateSystems");
	writer->writeAttribute( "defaultCoordinateSystem", QString::number(defaultCoordinateSystemIndex()) );
	// padding
	writer->writeAttribute( "horizontalPadding", QString::number(d->horizontalPadding) );
	writer->writeAttribute( "verticalPadding", QString::number(d->verticalPadding) );
	writer->writeAttribute( "rightPadding", QString::number(d->rightPadding) );
	writer->writeAttribute( "bottomPadding", QString::number(d->bottomPadding) );
	writer->writeAttribute( "symmetricPadding", QString::number(d->symmetricPadding));
	for (const auto& cSystem : m_coordinateSystems) {
		writer->writeStartElement( "coordinateSystem" );
		writer->writeAttribute( "xIndex", QString::number(dynamic_cast<CartesianCoordinateSystem*>(cSystem)->xIndex()) );
		writer->writeAttribute( "yIndex", QString::number(dynamic_cast<CartesianCoordinateSystem*>(cSystem)->yIndex()) );
		writer->writeEndElement();
	}
	writer->writeEndElement();
	// OLD style (pre 2.9.0)
//	writer->writeStartElement( "coordinateSystem" );
//	writer->writeAttribute( "autoScaleX", QString::number(d->autoScaleX) );
//	writer->writeAttribute( "autoScaleY", QString::number(d->autoScaleY) );
//	writer->writeAttribute( "xMin", QString::number(xRange(0).start(), 'g', 16));
//	writer->writeAttribute( "xMax", QString::number(xRange(0).end(), 'g', 16) );
//	writer->writeAttribute( "yMin", QString::number(d->yRange.range.start(), 'g', 16) );
//	writer->writeAttribute( "yMax", QString::number(d->yRange.range.end(), 'g', 16) );
//	writer->writeAttribute( "xScale", QString::number(static_cast<int>(xRange(0).scale())) );
//	writer->writeAttribute( "yScale", QString::number(static_cast<int>(d->yScale)) );
//	writer->writeAttribute( "xRangeFormat", QString::number(static_cast<int>(xRangeFormat(0))) );
//	writer->writeAttribute( "yRangeFormat", QString::number(static_cast<int>(d->yRangeFormat)) );
//	writer->writeAttribute( "horizontalPadding", QString::number(d->horizontalPadding) );
//	writer->writeAttribute( "verticalPadding", QString::number(d->verticalPadding) );
//	writer->writeAttribute( "rightPadding", QString::number(d->rightPadding) );
//	writer->writeAttribute( "bottomPadding", QString::number(d->bottomPadding) );
//	writer->writeAttribute( "symmetricPadding", QString::number(d->symmetricPadding));

	//x-scale breaks
	if (d->xRangeBreakingEnabled || !d->xRangeBreaks.list.isEmpty()) {
		writer->writeStartElement("xRangeBreaks");
		writer->writeAttribute( "enabled", QString::number(d->xRangeBreakingEnabled) );
		for (const auto& rb : d->xRangeBreaks.list) {
			writer->writeStartElement("xRangeBreak");
			writer->writeAttribute("start", QString::number(rb.range.start()));
			writer->writeAttribute("end", QString::number(rb.range.end()));
			writer->writeAttribute("position", QString::number(rb.position));
			writer->writeAttribute("style", QString::number(static_cast<int>(rb.style)));
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
			writer->writeAttribute("start", QString::number(rb.range.start()));
			writer->writeAttribute("end", QString::number(rb.range.end()));
			writer->writeAttribute("position", QString::number(rb.position));
			writer->writeAttribute("style", QString::number(static_cast<int>(rb.style)));
			writer->writeEndElement();
		}
		writer->writeEndElement();
	}

	//serialize all children (plot area, title text label, axes and curves)
	const auto& elements = children<WorksheetElement>(ChildIndexFlag::IncludeHidden);
	for (auto* elem : elements)
		elem->save(writer);

	writer->writeEndElement(); // cartesianPlot
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
	bool hasCoordinateSystems = false;	// new since 2.9.0

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
		} else if (!preview && reader->name() == "cursor") {
			attribs = reader->attributes();
			QPen pen;
			pen.setWidth(attribs.value("width").toInt());
			pen.setStyle(static_cast<Qt::PenStyle>(attribs.value("style").toInt()));
			QColor color;
			color.setRed(attribs.value("color_r").toInt());
			color.setGreen(attribs.value("color_g").toInt());
			color.setBlue(attribs.value("color_b").toInt());
			pen.setColor(color);
			d->cursorPen = pen;
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
		} else if (!preview && reader->name() == "xRanges") {
			d->xRanges.clear();
		} else if (!preview && reader->name() == "xRange") {
			attribs = reader->attributes();

			//TODO: Range<double> range = Range::load(reader)
			Range<double> range;
			str = attribs.value("autoScale").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("autoScale").toString());
			else
				range.setAutoScale( str.toInt() );
			str = attribs.value("start").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("start").toString());
			else
				range.setStart( str.toDouble() );
			str = attribs.value("end").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("end").toString());
			else
				range.setEnd( str.toDouble() );
			str = attribs.value("scale").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("scale").toString());
			else
				range.setScale( static_cast<RangeT::Scale>(str.toInt()) );
			str = attribs.value("format").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("format").toString());
			else
				range.setFormat( static_cast<RangeT::Format>(str.toInt()) );
			str = attribs.value("dateTimeFormat").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("dateTimeFormat").toString());
			else
				range.setDateTimeFormat(str);

			addXRange(range);
		} else if (!preview && reader->name() == "yRanges") {
			d->yRanges.clear();
		} else if (!preview && reader->name() == "yRange") {
			attribs = reader->attributes();

			//TODO: Range<double> range = Range::load(reader)
			Range<double> range;
			str = attribs.value("autoScale").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("autoScale").toString());
			else
				range.setAutoScale( str.toInt() );
			str = attribs.value("start").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("start").toString());
			else
				range.setStart( str.toDouble() );
			str = attribs.value("end").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("end").toString());
			else
				range.setEnd( str.toDouble() );
			str = attribs.value("scale").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("scale").toString());
			else
				range.setScale( static_cast<RangeT::Scale>(str.toInt()) );
			str = attribs.value("format").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("format").toString());
			else
				range.setFormat( static_cast<RangeT::Format>(str.toInt()) );
			str = attribs.value("dateTimeFormat").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("dateTimeFormat").toString());
			else
				range.setDateTimeFormat(str);

			addYRange(range);
		} else if (!preview && reader->name() == "coordinateSystems") {
			attribs = reader->attributes();
			READ_INT_VALUE("defaultCoordinateSystem", defaultCoordinateSystemIndex, int);
			DEBUG(Q_FUNC_INFO << ", got default cSystem index = " << d->defaultCoordinateSystemIndex)

			READ_DOUBLE_VALUE("horizontalPadding", horizontalPadding);
			READ_DOUBLE_VALUE("verticalPadding", verticalPadding);
			READ_DOUBLE_VALUE("rightPadding", rightPadding);
			READ_DOUBLE_VALUE("bottomPadding", bottomPadding);
			READ_INT_VALUE("symmetricPadding", symmetricPadding, bool);
			hasCoordinateSystems = true;

			m_coordinateSystems.clear();
		} else if (!preview && reader->name() == "coordinateSystem") {
			attribs = reader->attributes();
			// new style
			str = attribs.value("xIndex").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("xIndex").toString());
			else {
				CartesianCoordinateSystem* cSystem{ new CartesianCoordinateSystem(this) };
				cSystem->setXIndex( str.toInt() );

				str = attribs.value("yIndex").toString();
				if (str.isEmpty())
					reader->raiseWarning(attributeWarning.subs("yIndex").toString());
				else
					cSystem->setYIndex( str.toInt() );

				addCoordinateSystem(cSystem);
			}

			// old style (pre 2.9.0, to read old projects)
			if (!hasCoordinateSystems) {
				str = attribs.value("autoScaleX").toString();
				if (str.isEmpty())
					reader->raiseWarning(attributeWarning.subs("autoScaleX").toString());
				else
					d->xRanges[0].range.setAutoScale(str.toInt());
				str = attribs.value("autoScaleY").toString();
				if (str.isEmpty())
					reader->raiseWarning(attributeWarning.subs("autoScaleY").toString());
				else
					d->yRanges[0].range.setAutoScale(str.toInt());

				str = attribs.value("xMin").toString();
				if (str.isEmpty())
					reader->raiseWarning(attributeWarning.subs("xMin").toString());
				else {
					d->xRanges[0].range.start() = str.toDouble();
					d->xRanges[0].prev.start() = xRange(0).start();
				}

				str = attribs.value("xMax").toString();
				if (str.isEmpty())
					reader->raiseWarning(attributeWarning.subs("xMax").toString());
				else {
					d->xRanges[0].range.end() = str.toDouble();
					d->xRanges[0].prev.end() = xRange(0).end();
				}

				str = attribs.value("yMin").toString();
				if (str.isEmpty())
					reader->raiseWarning(attributeWarning.subs("yMin").toString());
				else {
					d->yRanges[0].range.start() = str.toDouble();
					d->yRanges[0].prev.start() = yRange(0).start();
				}

				str = attribs.value("yMax").toString();
				if (str.isEmpty())
					reader->raiseWarning(attributeWarning.subs("yMax").toString());
				else {
					d->yRanges[0].range.end() = str.toDouble();
					d->yRanges[0].prev.end() = yRange(0).end();
				}

				str = attribs.value("xScale").toString();
				if (str.isEmpty())
					reader->raiseWarning(attributeWarning.subs("xScale").toString());
				else {
					int scale{ str.toInt() };
					// convert old scale
					if (scale > (int)RangeT::Scale::Ln)
						scale -= 3;
					d->xRanges[0].range.scale() = static_cast<RangeT::Scale>(scale);
				}
				str = attribs.value("yScale").toString();
				if (str.isEmpty())
					reader->raiseWarning(attributeWarning.subs("yScale").toString());
				else {
					int scale{ str.toInt() };
					// convert old scale
					if (scale > (int)RangeT::Scale::Ln)
						scale -= 3;
					d->yRanges[0].range.scale() = static_cast<RangeT::Scale>(scale);
				}

				str = attribs.value("xRangeFormat").toString();
				if (str.isEmpty())
					reader->raiseWarning(attributeWarning.subs("xRangeFormat").toString());
				else
					d->xRanges[0].range.format() = static_cast<RangeT::Format>(str.toInt());
				str = attribs.value("yRangeFormat").toString();
				if (str.isEmpty())
					reader->raiseWarning(attributeWarning.subs("yRangeFormat").toString());
				else
					d->yRanges[0].range.format() = static_cast<RangeT::Format>(str.toInt());

				str = attribs.value("xRangeDateTimeFormat").toString();
				if (!str.isEmpty())
					d->xRanges[0].range.setDateTimeFormat(str);

				str = attribs.value("yRangeDateTimeFormat").toString();
				if (!str.isEmpty())
					d->yRanges[0].range.setDateTimeFormat(str);

				READ_DOUBLE_VALUE("horizontalPadding", horizontalPadding);
				READ_DOUBLE_VALUE("verticalPadding", verticalPadding);
				READ_DOUBLE_VALUE("rightPadding", rightPadding);
				READ_DOUBLE_VALUE("bottomPadding", bottomPadding);
				READ_INT_VALUE("symmetricPadding", symmetricPadding, bool);
			}
		} else if (!preview && reader->name() == "xRangeBreaks") {
			//delete default range break
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
				b.range.start() = str.toDouble();

			str = attribs.value("end").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("end").toString());
			else
				b.range.end() = str.toDouble();

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
			//delete default range break
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
				b.range.start() = str.toDouble();

			str = attribs.value("end").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("end").toString());
			else
				b.range.end() = str.toDouble();

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
		} else if (!preview && reader->name() == "textLabel") {
			if (!titleLabelRead) {
				//the first text label is always the title label
				m_title->load(reader, preview);
				titleLabelRead = true;

				//TODO: the name is read in m_title->load() but we overwrite it here
				//since the old projects don't have this " - Title" appendix yet that we add in init().
				//can be removed in couple of releases
				m_title->setName(name() + QLatin1String(" - ") + i18n("Title"));
			} else {
				TextLabel* label = new TextLabel("text label", this);
				if (label->load(reader, preview)) {
					addChildFast(label);
					label->setParentGraphicsItem(graphicsItem());
				} else {
					delete label;
					return false;
				}
			}
		} else if (!preview && reader->name() == "image") {
			auto* image = new Image(QString());
			if (!image->load(reader, preview)) {
				delete image;
				return false;
			} else
				addChildFast(image);
		} else if (!preview && reader->name() == "infoElement") {
			InfoElement* marker = new InfoElement("Marker", this);
			if (marker->load(reader, preview)) {
				addChildFast(marker);
				marker->setParentGraphicsItem(graphicsItem());
			} else {
				delete marker;
				return false;
			}
		} else if (!preview && reader->name() == "plotArea")
			m_plotArea->load(reader, preview);
		else if (!preview && reader->name() == "axis") {
			auto* axis = new Axis(QString());
			if (axis->load(reader, preview))
				addChildFast(axis);
			else {
				delete axis;
				return false;
			}
		} else if (reader->name() == "xyCurve") {
			auto* curve = new XYCurve(QString());
			if (curve->load(reader, preview))
				addChildFast(curve);
			else {
				removeChild(curve);
				return false;
			}
		} else if (reader->name() == "xyEquationCurve") {
			auto* curve = new XYEquationCurve(QString());
			if (curve->load(reader, preview))
				addChildFast(curve);
			else {
				removeChild(curve);
				return false;
			}
		} else if (reader->name() == "xyDataReductionCurve") {
			auto* curve = new XYDataReductionCurve(QString());
			if (curve->load(reader, preview))
				addChildFast(curve);
			else {
				removeChild(curve);
				return false;
			}
		} else if (reader->name() == "xyDifferentiationCurve") {
			auto* curve = new XYDifferentiationCurve(QString());
			if (curve->load(reader, preview))
				addChildFast(curve);
			else {
				removeChild(curve);
				return false;
			}
		} else if (reader->name() == "xyIntegrationCurve") {
			auto* curve = new XYIntegrationCurve(QString());
			if (curve->load(reader, preview))
				addChildFast(curve);
			else {
				removeChild(curve);
				return false;
			}
		} else if (reader->name() == "xyInterpolationCurve") {
			auto* curve = new XYInterpolationCurve(QString());
			if (curve->load(reader, preview))
				addChildFast(curve);
			else {
				removeChild(curve);
				return false;
			}
		} else if (reader->name() == "xySmoothCurve") {
			auto* curve = new XYSmoothCurve(QString());
			if (curve->load(reader, preview))
				addChildFast(curve);
			else {
				removeChild(curve);
				return false;
			}
		} else if (reader->name() == "xyFitCurve") {
			auto* curve = new XYFitCurve(QString());
			if (curve->load(reader, preview))
				addChildFast(curve);
			else {
				removeChild(curve);
				return false;
			}
		} else if (reader->name() == "xyFourierFilterCurve") {
			auto* curve = new XYFourierFilterCurve(QString());
			if (curve->load(reader, preview))
				addChildFast(curve);
			else {
				removeChild(curve);
				return false;
			}
		} else if (reader->name() == "xyFourierTransformCurve") {
			auto* curve = new XYFourierTransformCurve(QString());
			if (curve->load(reader, preview))
				addChildFast(curve);
			else {
				removeChild(curve);
				return false;
			}
		} else if (reader->name() == "xyHilbertTransformCurve") {
			auto* curve = new XYHilbertTransformCurve(QString());
			if (curve->load(reader, preview))
				addChildFast(curve);
			else {
				removeChild(curve);
				return false;
			}
		} else if (reader->name() == "xyConvolutionCurve") {
			auto* curve = new XYConvolutionCurve(QString());
			if (curve->load(reader, preview))
				addChildFast(curve);
			else {
				removeChild(curve);
				return false;
			}
		} else if (reader->name() == "xyCorrelationCurve") {
			auto* curve = new XYCorrelationCurve(QString());
			if (curve->load(reader, preview))
				addChildFast(curve);
			else {
				removeChild(curve);
				return false;
			}
		} else if (!preview && reader->name() == "cartesianPlotLegend") {
			m_legend = new CartesianPlotLegend(QString());
			if (m_legend->load(reader, preview))
				addChildFast(m_legend);
			else {
				delete m_legend;
				return false;
			}
		} else if (!preview && reader->name() == "customPoint") {
			auto* point = new CustomPoint(this, QString());
			if (point->load(reader, preview))
				addChildFast(point);
			else {
				delete point;
				return false;
			}
		} else if (!preview && reader->name() == "referenceLine") {
			auto* line = new ReferenceLine(this, QString());
			if (line->load(reader, preview))
				addChildFast(line);
			else {
				delete line;
				return false;
			}
		} else if (reader->name() == "boxPlot") {
			auto* boxPlot = new BoxPlot("BoxPlot");
			if (boxPlot->load(reader, preview))
				addChildFast(boxPlot);
			else {
				removeChild(boxPlot);
				return false;
			}
		} else if (reader->name() == "Histogram") {
			auto* hist = new Histogram("Histogram");
			if (hist->load(reader, preview))
				addChildFast(hist);
			else {
				removeChild(hist);
				return false;
			}
		} else { // unknown element
			if (!preview)
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
	if (!theme.isEmpty()) {
		KConfig config(ThemeHandler::themeFilePath(theme), KConfig::SimpleConfig);
		loadThemeConfig(config);
	} else {
		KConfig config;
		loadThemeConfig(config);
	}
}

void CartesianPlot::loadThemeConfig(const KConfig& config) {
	Q_D(CartesianPlot);

	QString theme = QString();
	if (config.hasGroup(QLatin1String("Theme"))) {
		theme = config.name();

		// theme path is saved with UNIX dir separator
		theme = theme.right(theme.length() - theme.lastIndexOf(QLatin1Char('/')) - 1);
		DEBUG(Q_FUNC_INFO << ", set theme to " << STDSTRING(theme));
	}

	//loadThemeConfig() can be called from
	//1. CartesianPlot::setTheme() when the user changes the theme for the plot
	//2. Worksheet::setTheme() -> Worksheet::loadTheme() when the user changes the theme for the worksheet
	//In the second case (i.e. when d->theme is not equal to theme yet),
	///we need to put the new theme name on the undo-stack.
	if (theme != d->theme)
		exec(new CartesianPlotSetThemeCmd(d, theme, ki18n("%1: set theme")));

	//load the color palettes for the curves
	this->setColorPalette(config);

	//load the theme for all the children
	const auto& elements = children<WorksheetElement>(ChildIndexFlag::IncludeHidden);
	for (auto* child : elements)
		child->loadThemeConfig(config);

	d->update(this->rect());
}

void CartesianPlot::saveTheme(KConfig &config) {
	const QVector<Axis*>& axisElements = children<Axis>(ChildIndexFlag::IncludeHidden);
	const QVector<PlotArea*>& plotAreaElements = children<PlotArea>(ChildIndexFlag::IncludeHidden);
	const QVector<TextLabel*>& textLabelElements = children<TextLabel>(ChildIndexFlag::IncludeHidden);

	axisElements.at(0)->saveThemeConfig(config);
	plotAreaElements.at(0)->saveThemeConfig(config);
	textLabelElements.at(0)->saveThemeConfig(config);

	const auto& children = this->children<XYCurve>(ChildIndexFlag::IncludeHidden);
	for (auto *child : children)
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
		std::array<float, 3> fac = {0.25f, 0.45f, 0.65f};

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

	//use the color of the axis lines as the color for the different mouse cursor lines
	Q_D(CartesianPlot);
	const KConfigGroup& group = config.group("Axis");
	const QColor& color = group.readEntry("LineColor", QColor(Qt::black));
	d->zoomSelectPen.setColor(color);
	d->crossHairPen.setColor(color);
}

const QList<QColor>& CartesianPlot::themeColorPalette() const {
	return m_themeColorPalette;
}
