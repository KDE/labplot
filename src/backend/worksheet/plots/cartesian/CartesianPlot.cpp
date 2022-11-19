/*
	File                 : CartesianPlot.cpp
	Project              : LabPlot
	Description          : Cartesian plot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2011-2022 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2016-2021 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-FileCopyrightText: 2017-2018 Garvit Khatri <garvitdelhi@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "CartesianPlot.h"
#include "CartesianPlotPrivate.h"
#include "Histogram.h"
#include "XYConvolutionCurve.h"
#include "XYCorrelationCurve.h"
#include "XYCurve.h"
#include "XYDataReductionCurve.h"
#include "XYDifferentiationCurve.h"
#include "XYEquationCurve.h"
#include "XYFitCurve.h"
#include "XYFourierFilterCurve.h"
#include "XYFourierTransformCurve.h"
#include "XYHilbertTransformCurve.h"
#include "XYIntegrationCurve.h"
#include "XYInterpolationCurve.h"
#include "XYSmoothCurve.h"
#include "backend/core/Project.h"
#include "backend/core/column/Column.h"
#include "backend/core/datatypes/DateTime2StringFilter.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/macros.h"
#include "backend/lib/trace.h"
#include "backend/worksheet/Image.h"
#include "backend/worksheet/InfoElement.h"
#include "backend/worksheet/Line.h"
#include "backend/worksheet/TextLabel.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/PlotArea.h"
#include "backend/worksheet/plots/cartesian/Axis.h"
#include "backend/worksheet/plots/cartesian/BarPlot.h"
#include "backend/worksheet/plots/cartesian/BoxPlot.h"
#include "backend/worksheet/plots/cartesian/CartesianPlotLegend.h"
#include "backend/worksheet/plots/cartesian/CustomPoint.h"
#include "backend/worksheet/plots/cartesian/ReferenceLine.h"
#include "backend/worksheet/plots/cartesian/Symbol.h"
#include "kdefrontend/ThemeHandler.h"
#include "kdefrontend/widgets/ThemesWidget.h"

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>

#include <QDir>
#include <QIcon>
#include <QKeyEvent>
#include <QMenu>
#include <QPainter>
#include <QWidgetAction>

using Dimension = CartesianCoordinateSystem::Dimension;

/**
 * \class CartesianPlot
 * \brief A xy-plot.
 */
CartesianPlot::CartesianPlot(const QString& name)
	: AbstractPlot(name, new CartesianPlotPrivate(this), AspectType::CartesianPlot) {
	init();
}

CartesianPlot::CartesianPlot(const QString& name, CartesianPlotPrivate* dd)
	: AbstractPlot(name, dd, AspectType::CartesianPlot) {
	init();
}

CartesianPlot::~CartesianPlot() {
	if (m_menusInitialized) {
		delete addNewMenu;
		delete themeMenu;
	}

	while (!m_coordinateSystems.isEmpty())
		delete m_coordinateSystems.takeFirst();

	// no need to delete objects added with addChild()

	// no need to delete the d-pointer here - it inherits from QGraphicsItem
	// and is deleted during the cleanup in QGraphicsScene
}

/*!
	initializes all member variables of \c CartesianPlot
*/
void CartesianPlot::init() {
	m_coordinateSystems.append(new CartesianCoordinateSystem(this));
	m_plotArea = new PlotArea(name() + QStringLiteral(" plot area"), this);
	addChildFast(m_plotArea);

	// title
	m_title = new TextLabel(this->name() + QLatin1String("- ") + i18n("Title"), TextLabel::Type::PlotTitle);
	addChild(m_title);
	m_title->setHidden(true);
	m_title->setParentGraphicsItem(m_plotArea->graphicsItem());

	// offset between the plot area and the area defining the coordinate system, in scene units.
	Q_D(CartesianPlot);
	d->horizontalPadding = Worksheet::convertToSceneUnits(1.5, Worksheet::Unit::Centimeter);
	d->verticalPadding = Worksheet::convertToSceneUnits(1.5, Worksheet::Unit::Centimeter);
	d->rightPadding = Worksheet::convertToSceneUnits(1.5, Worksheet::Unit::Centimeter);
	d->bottomPadding = Worksheet::convertToSceneUnits(1.5, Worksheet::Unit::Centimeter);
	d->symmetricPadding = true;

	// cursor line
	d->cursorLine = new Line(QString());
	d->cursorLine->setPrefix(QLatin1String("Cursor"));
	d->cursorLine->setHidden(true);
	addChild(d->cursorLine);
	d->cursorLine->setStyle(Qt::SolidLine);
	d->cursorLine->setColor(Qt::red); // TODO: use theme specific initial settings
	d->cursorLine->setWidth(Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Point));
	connect(d->cursorLine, &Line::updatePixmapRequested, [=] {
		d->update();
	});
	connect(d->cursorLine, &Line::updateRequested, [=] {
		d->update();
	});

	connect(this, &AbstractAspect::aspectAdded, this, &CartesianPlot::childAdded);
	connect(this, &AbstractAspect::aspectRemoved, this, &CartesianPlot::childRemoved);

	graphicsItem()->setFlag(QGraphicsItem::ItemIsMovable, true);
	graphicsItem()->setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
	graphicsItem()->setFlag(QGraphicsItem::ItemIsSelectable, true);
	graphicsItem()->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
	graphicsItem()->setFlag(QGraphicsItem::ItemIsFocusable, true);

	// theme is not set at this point, initialize the color palette with default colors
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
		// Axes
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
		axis->title()->setText(QString());
		axis->setDefault(true);
		axis->setSuppressRetransform(true);

		addChild(axis);
		axis->setPosition(Axis::Position::Top);
		axis->setRange(0., 1.);
		axis->setMajorTicksDirection(Axis::ticksIn);
		axis->setMinorTicksDirection(Axis::ticksIn);
		axis->setMinorTicksNumber(1);
		axis->majorGridLine()->setStyle(Qt::NoPen);
		axis->minorGridLine()->setStyle(Qt::NoPen);
		axis->setLabelsPosition(Axis::LabelsPosition::NoLabels);
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
		axis->title()->setText(QString());
		axis->setDefault(true);
		axis->setSuppressRetransform(true);
		addChild(axis);
		axis->setPosition(Axis::Position::Right);
		axis->setRange(0., 1.);
		axis->setMajorTicksDirection(Axis::ticksIn);
		axis->setMinorTicksDirection(Axis::ticksIn);
		axis->setMinorTicksNumber(1);
		axis->majorGridLine()->setStyle(Qt::NoPen);
		axis->minorGridLine()->setStyle(Qt::NoPen);
		axis->setLabelsPosition(Axis::LabelsPosition::NoLabels);
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

		m_plotArea->borderLine()->setStyle(Qt::NoPen);

		Axis* axis = new Axis(QLatin1String("x"), Axis::Orientation::Horizontal);
		axis->title()->setText(QString());
		axis->setDefault(true);
		axis->setSuppressRetransform(true);
		addChild(axis);
		axis->setPosition(Axis::Position::Centered);
		axis->setRange(-0.5, 0.5);
		axis->setMajorTicksDirection(Axis::ticksBoth);
		axis->setMinorTicksDirection(Axis::ticksBoth);
		axis->setMinorTicksNumber(1);
		axis->setArrowType(Axis::ArrowType::FilledSmall);
		axis->setSuppressRetransform(false);

		axis = new Axis(QLatin1String("y"), Axis::Orientation::Vertical);
		axis->title()->setText(QString());
		axis->setDefault(true);
		axis->setSuppressRetransform(true);
		addChild(axis);
		axis->setPosition(Axis::Position::Centered);
		axis->setRange(-0.5, 0.5);
		axis->setMajorTicksDirection(Axis::ticksBoth);
		axis->setMinorTicksDirection(Axis::ticksBoth);
		axis->setMinorTicksNumber(1);
		axis->setArrowType(Axis::ArrowType::FilledSmall);
		axis->setSuppressRetransform(false);

		break;
	}
	case Type::TwoAxesCenteredZero: {
		d->xRanges[0].range.setRange(-0.5, 0.5);
		d->yRanges[0].range.setRange(-0.5, 0.5);

		d->horizontalPadding = Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Centimeter);
		d->verticalPadding = Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Centimeter);

		m_plotArea->borderLine()->setStyle(Qt::NoPen);

		Axis* axis = new Axis(QLatin1String("x"), Axis::Orientation::Horizontal);
		axis->title()->setText(QString());
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
		axis->setSuppressRetransform(false);

		axis = new Axis(QLatin1String("y"), Axis::Orientation::Vertical);
		axis->title()->setText(QString());
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
		axis->setSuppressRetransform(false);

		break;
	}
	}

	d->xRanges[0].prev = range(Dimension::X);
	d->yRanges[0].prev = range(Dimension::Y);

	// Geometry, specify the plot rect in scene coordinates.
	// TODO: Use default settings for left, top, width, height and for min/max for the coordinate system
	double x = Worksheet::convertToSceneUnits(2, Worksheet::Unit::Centimeter);
	double y = Worksheet::convertToSceneUnits(2, Worksheet::Unit::Centimeter);
	double w = Worksheet::convertToSceneUnits(10, Worksheet::Unit::Centimeter);
	double h = Worksheet::convertToSceneUnits(10, Worksheet::Unit::Centimeter);

	// all plot children are initialized -> set the geometry of the plot in scene coordinates.
	d->rect = QRectF(x, y, w, h);

	const auto* worksheet = static_cast<const Worksheet*>(parentAspect());
	if (worksheet && worksheet->layout() != Worksheet::Layout::NoLayout)
		retransform();
}

CartesianPlot::Type CartesianPlot::type() const {
	Q_D(const CartesianPlot);
	return d->type;
}

void CartesianPlot::initActions() {
	//"add new" actions
	addCurveAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-curve")), i18n("xy-curve"), this);
	addHistogramAction = new QAction(QIcon::fromTheme(QStringLiteral("view-object-histogram-linear")), i18n("Histogram"), this);
	addBarPlotAction = new QAction(QIcon::fromTheme(QStringLiteral("office-chart-bar")), i18n("Bar Plot"), this);
	addBoxPlotAction = new QAction(BoxPlot::staticIcon(), i18n("Box Plot"), this);
	addEquationCurveAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-equation-curve")), i18n("xy-curve from a Formula"), this);
	// no icons yet
	addDataReductionCurveAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-curve")), i18n("Data Reduction"), this);
	addDifferentiationCurveAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-curve")), i18n("Differentiation"), this);
	addIntegrationCurveAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-curve")), i18n("Integration"), this);
	addInterpolationCurveAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-interpolation-curve")), i18n("Interpolation"), this);
	addSmoothCurveAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-smoothing-curve")), i18n("Smooth"), this);
	addFitCurveAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-fit-curve")), i18n("Fit"), this);
	addFourierFilterCurveAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-fourier-filter-curve")), i18n("Fourier Filter"), this);
	addFourierTransformCurveAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-fourier-transform-curve")), i18n("Fourier Transform"), this);
	addHilbertTransformCurveAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-curve")), i18n("Hilbert Transform"), this);
	addConvolutionCurveAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-curve")), i18n("(De-)Convolution"), this);
	addCorrelationCurveAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-curve")), i18n("Auto-/Cross-Correlation"), this);

	addLegendAction = new QAction(QIcon::fromTheme(QStringLiteral("text-field")), i18n("Legend"), this);
	if (children<CartesianPlotLegend>().size() > 0)
		addLegendAction->setEnabled(false); // only one legend is allowed -> disable the action

	addHorizontalAxisAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-axis-horizontal")), i18n("Horizontal Axis"), this);
	addVerticalAxisAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-axis-vertical")), i18n("Vertical Axis"), this);
	addTextLabelAction = new QAction(QIcon::fromTheme(QStringLiteral("draw-text")), i18n("Text"), this);
	addImageAction = new QAction(QIcon::fromTheme(QStringLiteral("viewimage")), i18n("Image"), this);
	addInfoElementAction = new QAction(QIcon::fromTheme(QStringLiteral("draw-text")), i18n("Info Element"), this);
	addCustomPointAction = new QAction(QIcon::fromTheme(QStringLiteral("draw-cross")), i18n("Custom Point"), this);
	addReferenceLineAction = new QAction(QIcon::fromTheme(QStringLiteral("draw-line")), i18n("Reference Line"), this);

	connect(addCurveAction, &QAction::triggered, this, &CartesianPlot::addCurve);
	connect(addHistogramAction, &QAction::triggered, this, &CartesianPlot::addHistogram);
	connect(addBarPlotAction, &QAction::triggered, this, &CartesianPlot::addBarPlot);
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

	// Analysis menu actions
	// 	addDataOperationAction = new QAction(i18n("Data Operation"), this);
	addDataReductionAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-curve")), i18n("Data Reduction"), this);
	addDifferentiationAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-curve")), i18n("Differentiate"), this);
	addIntegrationAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-curve")), i18n("Integrate"), this);
	addInterpolationAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-interpolation-curve")), i18n("Interpolate"), this);
	addSmoothAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-smoothing-curve")), i18n("Smooth"), this);
	addConvolutionAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-curve")), i18n("Convolute/Deconvolute"), this);
	addCorrelationAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-curve")), i18n("Auto-/Cross-Correlation"), this);

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

	addFourierFilterAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-fourier-filter-curve")), i18n("Fourier Filter"), this);
	addFourierTransformAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-fourier-transform-curve")), i18n("Fourier Transform"), this);
	addHilbertTransformAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-curve")), i18n("Hilbert Transform"), this);

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

	// visibility action
	visibilityAction = new QAction(QIcon::fromTheme(QStringLiteral("view-visible")), i18n("Visible"), this);
	visibilityAction->setCheckable(true);
	connect(visibilityAction, &QAction::triggered, this, &CartesianPlot::changeVisibility);
}

void CartesianPlot::initMenus() {
	initActions();

	addNewMenu = new QMenu(i18n("Add New"));
	addNewMenu->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));
	addNewMenu->addAction(addCurveAction);
	addNewMenu->addAction(addHistogramAction);
	addNewMenu->addAction(addBoxPlotAction);
	addNewMenu->addAction(addBarPlotAction);
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

	// Data manipulation menu
	// 	QMenu* dataManipulationMenu = new QMenu(i18n("Data Manipulation"));
	// 	dataManipulationMenu->setIcon(QIcon::fromTheme(QStringLiteral("zoom-draw")));
	// 	dataManipulationMenu->addAction(addDataOperationAction);
	// 	dataManipulationMenu->addAction(addDataReductionAction);

	// Data fit menu
	QMenu* dataFitMenu = new QMenu(i18n("Fit"));
	dataFitMenu->setIcon(QIcon::fromTheme(QStringLiteral("labplot-xy-fit-curve")));
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

	// analysis menu
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

	// theme menu
	themeMenu = new QMenu(i18n("Theme"));
	themeMenu->setIcon(QIcon::fromTheme(QStringLiteral("color-management")));
#ifndef SDK
	connect(themeMenu, &QMenu::aboutToShow, this, [=]() {
		if (!themeMenu->isEmpty())
			return;
		auto* themeWidget = new ThemesWidget(nullptr);
		themeWidget->setFixedMode();
		connect(themeWidget, &ThemesWidget::themeSelected, this, &CartesianPlot::loadTheme);
		connect(themeWidget, &ThemesWidget::themeSelected, themeMenu, &QMenu::close);

		auto* widgetAction = new QWidgetAction(this);
		widgetAction->setDefaultWidget(themeWidget);
		themeMenu->addAction(widgetAction);
	});
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
	menu->insertMenu(firstAction, themeMenu);
	menu->insertSeparator(firstAction);

	visibilityAction->setChecked(isVisible());
	menu->insertAction(firstAction, visibilityAction);
	menu->insertSeparator(firstAction);

	if (children<XYCurve>().isEmpty()) {
		addInfoElementAction->setEnabled(false);
		addInfoElementAction->setToolTip(QStringLiteral("No curve inside plot."));
	} else {
		addInfoElementAction->setEnabled(true);
		addInfoElementAction->setToolTip(QString());
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
	return QIcon::fromTheme(QStringLiteral("office-chart-line"));
}

QVector<AbstractAspect*> CartesianPlot::dependsOn() const {
	// aspects which the plotted data in the worksheet depends on (spreadsheets and later matrices)
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
	QVector<AspectType> types{AspectType::XYCurve,
							  AspectType::Histogram,
							  AspectType::BarPlot,
							  AspectType::BoxPlot,
							  AspectType::Axis,
							  AspectType::XYEquationCurve,
							  AspectType::XYConvolutionCurve,
							  AspectType::XYCorrelationCurve,
							  AspectType::XYDataReductionCurve,
							  AspectType::XYDifferentiationCurve,
							  AspectType::XYFitCurve,
							  AspectType::XYFourierFilterCurve,
							  AspectType::XYFourierTransformCurve,
							  AspectType::XYIntegrationCurve,
							  AspectType::XYInterpolationCurve,
							  AspectType::XYSmoothCurve,
							  AspectType::TextLabel,
							  AspectType::Image,
							  AspectType::InfoElement,
							  AspectType::CustomPoint,
							  AspectType::ReferenceLine};

	// only allow to paste a legend if there is no legend available yet in the plot
	if (!m_legend)
		types << AspectType::CartesianPlotLegend;

	return types;
}

void CartesianPlot::navigate(int cSystemIndex, NavigationOperation op) {
	PERFTRACE(QLatin1String(Q_FUNC_INFO));
	const auto* cSystem = coordinateSystem(cSystemIndex);
	int xIndex = -1, yIndex = -1;
	if (cSystem) {
		xIndex = cSystem->index(Dimension::X);
		yIndex = cSystem->index(Dimension::Y);
	}

	if (op == NavigationOperation::ScaleAuto) {
		if (!cSystem) { // all csystems
			for (int i = 0; i < coordinateSystemCount(); i++) {
				auto* cSystem = coordinateSystem(i);
				auto xDirty = rangeDirty(Dimension::X, cSystem->index(Dimension::X));
				auto yDirty = rangeDirty(Dimension::Y, cSystem->index(Dimension::Y));

				if (xDirty || yDirty || !autoScale(Dimension::X, cSystem->index(Dimension::X)) || !autoScale(Dimension::Y, cSystem->index(Dimension::Y))) {
					setRangeDirty(Dimension::X, cSystem->index(Dimension::X), true);
					setRangeDirty(Dimension::Y, cSystem->index(Dimension::Y), true);
				}
				if (!autoScale(Dimension::X, cSystem->index(Dimension::X)))
					enableAutoScale(Dimension::X, cSystem->index(Dimension::X), true, true);
				else // if already autoscale set, scaleAutoX will not be called anymore, so force it to do
					scaleAuto(Dimension::X, cSystem->index(Dimension::X), true);

				if (!autoScale(Dimension::Y, cSystem->index(Dimension::Y)))
					enableAutoScale(Dimension::Y, cSystem->index(Dimension::Y), true, true);
				else
					scaleAuto(Dimension::Y, cSystem->index(Dimension::Y), true);
			}
			WorksheetElementContainer::retransform();
		} else {
			auto xDirty = rangeDirty(Dimension::X, xIndex);
			auto yDirty = rangeDirty(Dimension::Y, yIndex);

			if (xDirty || yDirty || !autoScale(Dimension::X, xIndex) || !autoScale(Dimension::Y, yIndex)) {
				setRangeDirty(Dimension::X, xIndex, true);
				setRangeDirty(Dimension::Y, yIndex, true);
			}
			if (!autoScale(Dimension::X, cSystem->index(Dimension::X)))
				enableAutoScale(Dimension::X, cSystem->index(Dimension::X), true, true);
			else
				scaleAuto(Dimension::X, cSystem->index(Dimension::X), true);

			if (!autoScale(Dimension::Y, cSystem->index(Dimension::Y)))
				enableAutoScale(Dimension::Y, cSystem->index(Dimension::Y), true, true);
			else
				scaleAuto(Dimension::Y, cSystem->index(Dimension::Y), true);
			WorksheetElementContainer::retransform();
		}
	} else if (op == NavigationOperation::ScaleAutoX) {
		bool update = rangeDirty(Dimension::X, xIndex);
		if (!autoScale(Dimension::X, xIndex)) {
			enableAutoScale(Dimension::X, xIndex, true, true);
			update = true;
		} else
			update |= scaleAuto(Dimension::X, xIndex, true);
		if (update) {
			for (int i = 0; i < m_coordinateSystems.count(); i++) {
				auto cs = coordinateSystem(i);
				if ((cSystemIndex == -1 || xIndex == cs->index(Dimension::X)) && autoScale(Dimension::Y, cs->index(Dimension::Y)))
					scaleAuto(Dimension::Y, cs->index(Dimension::Y), false);
			}
			WorksheetElementContainer::retransform();
		}
	} else if (op == NavigationOperation::ScaleAutoY) {
		bool update = rangeDirty(Dimension::Y, yIndex);
		if (!autoScale(Dimension::Y, yIndex)) {
			enableAutoScale(Dimension::Y, yIndex, true, true);
			update = true;
		} else
			update |= scaleAuto(Dimension::Y, yIndex, true);
		if (update) {
			for (int i = 0; i < m_coordinateSystems.count(); i++) {
				auto cs = coordinateSystem(i);
				if ((cSystemIndex == -1 || yIndex == cs->index(Dimension::Y)) && autoScale(Dimension::X, cs->index(Dimension::X)))
					scaleAuto(Dimension::X, cs->index(Dimension::X), false);
			}
			WorksheetElementContainer::retransform();
		}
	} else if (op == NavigationOperation::ZoomIn)
		zoomIn(xIndex, yIndex);
	else if (op == NavigationOperation::ZoomOut)
		zoomOut(xIndex, yIndex);
	else if (op == NavigationOperation::ZoomInX)
		zoomInX(xIndex);
	else if (op == NavigationOperation::ZoomOutX)
		zoomOutX(xIndex);
	else if (op == NavigationOperation::ZoomInY)
		zoomInY(yIndex);
	else if (op == NavigationOperation::ZoomOutY)
		zoomOutY(yIndex);
	else if (op == NavigationOperation::ShiftLeftX)
		shiftLeftX(xIndex);
	else if (op == NavigationOperation::ShiftRightX)
		shiftRightX(xIndex);
	else if (op == NavigationOperation::ShiftUpY)
		shiftUpY(yIndex);
	else if (op == NavigationOperation::ShiftDownY)
		shiftDownY(yIndex);
}

void CartesianPlot::processDropEvent(const QVector<quintptr>& vec) {
	PERFTRACE(QLatin1String(Q_FUNC_INFO));

	QVector<AbstractColumn*> columns;
	for (auto a : vec) {
		auto* aspect = (AbstractAspect*)a;
		auto* column = qobject_cast<AbstractColumn*>(aspect);
		if (column)
			columns << column;
	}

	// return if there are no columns being dropped.
	// TODO: extend this later when we allow to drag&drop plots, etc.
	if (columns.isEmpty())
		return;

	// determine the first column with "x plot designation" as the x-data column for all curves to be created
	const AbstractColumn* xColumn = nullptr;
	for (const auto* column : qAsConst(columns)) {
		if (column->plotDesignation() == AbstractColumn::PlotDesignation::X) {
			xColumn = column;
			break;
		}
	}

	// if no column with "x plot designation" is available, use the x-data column of the first curve in the plot,
	if (xColumn == nullptr) {
		QVector<XYCurve*> curves = children<XYCurve>();
		if (!curves.isEmpty())
			xColumn = curves.at(0)->xColumn();
	}

	// use the first dropped column if no column with "x plot designation" nor curves are available
	if (xColumn == nullptr)
		xColumn = columns.at(0);

	// create curves
	bool curvesAdded = false;
	for (const auto* column : qAsConst(columns)) {
		if (column == xColumn)
			continue;

		XYCurve* curve = new XYCurve(column->name());
		curve->setSuppressRetransform(true); // suppress retransform, all curved will be recalculated at the end
		curve->setXColumn(xColumn);
		curve->setYColumn(column);
		addChild(curve);
		curve->setSuppressRetransform(false);
		curvesAdded = true;
	}

	if (curvesAdded) {
		// In addChild() the curve gets the coordinatesystem which is the default coordinate system
		dataChanged(defaultCoordinateSystem()->index(Dimension::X), defaultCoordinateSystem()->index(Dimension::Y));
	}
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
BASIC_SHARED_D_READER_IMPL(CartesianPlot, bool, niceExtend, niceExtend)
BASIC_SHARED_D_READER_IMPL(CartesianPlot, int, rangeLastValues, rangeLastValues)
BASIC_SHARED_D_READER_IMPL(CartesianPlot, int, rangeFirstValues, rangeFirstValues)

BASIC_SHARED_D_READER_IMPL(CartesianPlot, bool, xRangeBreakingEnabled, xRangeBreakingEnabled)
BASIC_SHARED_D_READER_IMPL(CartesianPlot, CartesianPlot::RangeBreaks, xRangeBreaks, xRangeBreaks)
BASIC_SHARED_D_READER_IMPL(CartesianPlot, bool, yRangeBreakingEnabled, yRangeBreakingEnabled)
BASIC_SHARED_D_READER_IMPL(CartesianPlot, CartesianPlot::RangeBreaks, yRangeBreaks, yRangeBreaks)

BASIC_SHARED_D_READER_IMPL(CartesianPlot, bool, cursor0Enable, cursor0Enable)
BASIC_SHARED_D_READER_IMPL(CartesianPlot, bool, cursor1Enable, cursor1Enable)
BASIC_SHARED_D_READER_IMPL(CartesianPlot, QString, theme, theme)

Line* CartesianPlot::cursorLine() const {
	Q_D(const CartesianPlot);
	return d->cursorLine;
}

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

const QString CartesianPlot::rangeDateTimeFormat(const Dimension dim) const {
	const int index{defaultCoordinateSystem()->index(dim)};
	return rangeDateTimeFormat(dim, index);
}

const QString CartesianPlot::rangeDateTimeFormat(const Dimension dim, const int index) const {
	Q_D(const CartesianPlot);
	return d->rangeConst(dim, index).dateTimeFormat();
}

//##############################################################################
//######################  setter methods and undo commands  ####################
//##############################################################################
/*!
	set the rectangular, defined in scene coordinates
 */
class CartesianPlotSetRectCmd : public QUndoCommand {
public:
	CartesianPlotSetRectCmd(CartesianPlotPrivate* private_obj, const QRectF& rect)
		: m_private(private_obj)
		, m_rect(rect) {
		setText(i18n("%1: change geometry rect", m_private->name()));
	}

	void redo() override {
		// 		const double horizontalRatio = m_rect.width() / m_private->rect.width();
		// 		const double verticalRatio = m_rect.height() / m_private->rect.height();

		qSwap(m_private->rect, m_rect);

		// 		m_private->q->handleResize(horizontalRatio, verticalRatio, false);
		m_private->retransform();
		Q_EMIT m_private->q->rectChanged(m_private->rect);
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

class CartesianPlotSetPrevRectCmd : public QUndoCommand {
public:
	CartesianPlotSetPrevRectCmd(CartesianPlotPrivate* private_obj, const QRectF& rect)
		: m_private(private_obj)
		, m_rect(rect) {
		setText(i18n("%1: change geometry rect", m_private->name()));
	}

	void redo() override {
		if (m_initilized) {
			qSwap(m_private->rect, m_rect);
			m_private->retransform();
			Q_EMIT m_private->q->rectChanged(m_private->rect);
		} else {
			// this function is called for the first time,
			// nothing to do, we just need to remember what the previous rect was
			// which has happened already in the constructor.
			m_initilized = true;
		}
	}

	void undo() override {
		redo();
	}

private:
	CartesianPlotPrivate* m_private;
	QRectF m_rect;
	bool m_initilized{false};
};

void CartesianPlot::setPrevRect(const QRectF& prevRect) {
	Q_D(CartesianPlot);
	exec(new CartesianPlotSetPrevRectCmd(d, prevRect));
}

STD_SETTER_CMD_IMPL_F_S(CartesianPlot, SetRangeType, CartesianPlot::RangeType, rangeType, rangeChanged)
void CartesianPlot::setRangeType(RangeType type) {
	Q_D(CartesianPlot);
	if (type != d->rangeType)
		exec(new CartesianPlotSetRangeTypeCmd(d, type, ki18n("%1: set range type")));
}

STD_SETTER_CMD_IMPL_F_S(CartesianPlot, SetNiceExtend, bool, niceExtend, niceExtendChanged)
void CartesianPlot::setNiceExtend(const bool value) {
	Q_D(CartesianPlot);
	if (value != d->niceExtend)
		exec(new CartesianPlotSetNiceExtendCmd(d, value, ki18n("%1: set nice extend")));
}

STD_SETTER_CMD_IMPL_F_S(CartesianPlot, SetRangeLastValues, int, rangeLastValues, rangeChanged)
void CartesianPlot::setRangeLastValues(int values) {
	Q_D(CartesianPlot);
	if (values != d->rangeLastValues)
		exec(new CartesianPlotSetRangeLastValuesCmd(d, values, ki18n("%1: set range")));
}

STD_SETTER_CMD_IMPL_F_S(CartesianPlot, SetRangeFirstValues, int, rangeFirstValues, rangeChanged)
void CartesianPlot::setRangeFirstValues(int values) {
	Q_D(CartesianPlot);
	if (values != d->rangeFirstValues)
		exec(new CartesianPlotSetRangeFirstValuesCmd(d, values, ki18n("%1: set range")));
}

// x/y ranges
class CartesianPlotSetRangeFormatIndexCmd : public QUndoCommand {
public:
	CartesianPlotSetRangeFormatIndexCmd(CartesianPlotPrivate* private_obj, const Dimension dim, RangeT::Format format, int index)
		: m_private(private_obj)
		, m_dimension(dim)
		, m_format(format)
		, m_index(index) {
		setText(i18n("%1: change %2-range %3 format", m_private->name(), CartesianCoordinateSystem::dimensionToString(dim), index + 1));
	}

	void redo() override {
		m_formatOld = m_private->rangeConst(m_dimension, m_index).format();
		m_private->setFormat(m_dimension, m_index, m_format);
		Q_EMIT m_private->q->rangeFormatChanged(m_dimension, m_index, m_format);
		m_private->rangeFormatChanged(m_dimension);
	}

	void undo() override {
		m_private->setFormat(m_dimension, m_index, m_formatOld);
		Q_EMIT m_private->q->rangeFormatChanged(m_dimension, m_index, m_formatOld);
		m_private->rangeFormatChanged(m_dimension);
	}

private:
	CartesianPlotPrivate* m_private;
	Dimension m_dimension;
	RangeT::Format m_format;
	int m_index;
	RangeT::Format m_formatOld{RangeT::Format::Numeric};
};

RangeT::Format CartesianPlot::xRangeFormatDefault() const {
	return rangeFormat(Dimension::X, defaultCoordinateSystem()->index(Dimension::X));
}
RangeT::Format CartesianPlot::yRangeFormatDefault() const {
	return rangeFormat(Dimension::Y, defaultCoordinateSystem()->index(Dimension::Y));
}
RangeT::Format CartesianPlot::rangeFormat(const Dimension dim, const int index) const {
	Q_D(const CartesianPlot);
	if (index < 0 || index > rangeCount(dim)) {
		DEBUG(Q_FUNC_INFO << ", index " << index << " out of range")
		return RangeT::Format::Numeric;
	}
	return d->rangeConst(dim, index).format();
}
RangeT::Format CartesianPlot::xRangeFormat(const int index) const {
	return rangeFormat(Dimension::X, index);
}
RangeT::Format CartesianPlot::yRangeFormat(const int index) const {
	return rangeFormat(Dimension::Y, index);
}

void CartesianPlot::setRangeFormat(const Dimension dim, const RangeT::Format format) {
	setRangeFormat(dim, defaultCoordinateSystem()->index(dim), format);
}

void CartesianPlot::setRangeFormat(const Dimension dim, const int index, const RangeT::Format format) {
	Q_D(CartesianPlot);
	if (index < 0 || index > rangeCount(dim)) {
		DEBUG(Q_FUNC_INFO << ", index " << index << " out of range")
		return;
	}
	if (format != rangeFormat(dim, index)) {
		exec(new CartesianPlotSetRangeFormatIndexCmd(d, dim, format, index));
		if (project())
			project()->setChanged(true);
	}
}

void CartesianPlot::setXRangeFormat(const int index, const RangeT::Format format) {
	setRangeFormat(Dimension::X, index, format);
}
void CartesianPlot::setYRangeFormat(const int index, const RangeT::Format format) {
	setRangeFormat(Dimension::Y, index, format);
}

// auto scale

// is auto scale enabled for x axis index (index == -1: all axes)
bool CartesianPlot::autoScale(const Dimension dim, int index) const {
	if (index == -1) {
		for (int i = 0; i < rangeCount(dim); i++) {
			if (!range(dim, i).autoScale())
				return false;
		}
		return true;
	}
	return range(dim, index).autoScale();
}

class CartesianPlotEnableAutoScaleIndexCmd : public QUndoCommand {
public:
	CartesianPlotEnableAutoScaleIndexCmd(CartesianPlotPrivate* private_obj, const Dimension dim, bool autoScale, int index, bool fullRange)
		: m_private(private_obj)
		, m_dimension(dim)
		, m_autoScale(autoScale)
		, m_index(index)
		, m_fullRange(fullRange) {
		setText(i18n("%1: change %2-range %3 auto scaling", m_private->name(), CartesianCoordinateSystem::dimensionToString(dim), m_index + 1));
	}

	void redo() override {
		m_autoScaleOld = m_private->autoScale(m_dimension, m_index);
		m_private->enableAutoScale(m_dimension, m_index, m_autoScale);
		if (m_autoScale) {
			m_oldRange = m_private->range(m_dimension, m_index);
			m_private->q->scaleAuto(m_dimension, m_index, m_fullRange);
		}
		Q_EMIT m_private->q->autoScaleChanged(m_dimension, m_index, m_autoScale);
	}

	void undo() override {
		if (!m_autoScaleOld) {
			m_private->range(m_dimension, m_index) = m_oldRange;
			m_private->retransformScale(m_dimension, m_index);
		}
		m_private->enableAutoScale(m_dimension, m_index, m_autoScaleOld);
		Q_EMIT m_private->q->autoScaleChanged(m_dimension, m_index, m_autoScaleOld);
	}

private:
	CartesianPlotPrivate* m_private;
	Dimension m_dimension;
	bool m_autoScale;
	bool m_autoScaleOld{false};
	int m_index;
	Range<double> m_oldRange = Range<double>(0.0, 0.0);
	bool m_fullRange;
};

// set auto scale for x/y range index (index == -1: all ranges)
void CartesianPlot::enableAutoScale(const Dimension dim, int index, const bool enable, bool fullRange) {
	PERFTRACE(QLatin1String(Q_FUNC_INFO));
	Q_D(CartesianPlot);
	if (index == -1) { // all x ranges
		for (int i = 0; i < rangeCount(dim); i++)
			enableAutoScale(dim, i, enable, fullRange);
		return;
	}

	if (enable != range(dim, index).autoScale()) {
		DEBUG(Q_FUNC_INFO << ", x range " << index << " enable auto scale: " << enable)
		// TODO: maybe using the first and then adding the first one as parent to the next undo command
		exec(new CartesianPlotEnableAutoScaleIndexCmd(d, dim, enable, index, fullRange));
		if (project())
			project()->setChanged(true);
	}
}

int CartesianPlot::rangeCount(const Dimension dim) const {
	Q_D(const CartesianPlot);
	return d ? d->rangeCount(dim) : 0;
}

const Range<double>& CartesianPlot::range(const Dimension dim, int index) const {
	if (index == -1)
		index = defaultCoordinateSystem()->index(dim);
	Q_D(const CartesianPlot);
	return d->rangeConst(dim, index);
}

void CartesianPlot::setRangeDefault(const Dimension dim, const Range<double> range) {
	const int index{defaultCoordinateSystem()->index(dim)};
	setRange(dim, index, range);
}

class CartesianPlotSetRangeIndexCmd : public QUndoCommand {
public:
	CartesianPlotSetRangeIndexCmd(CartesianPlot::Private* target, const Dimension dim, Range<double> newValue, int index)
		: QUndoCommand()
		, m_target(target)
		, m_index(index)
		, m_dimension(dim)
		, m_otherValue(newValue) {
	}
	void redo() override {
		auto tmp = m_target->rangeConst(m_dimension, m_index);
		m_target->setRange(m_dimension, m_index, m_otherValue);
		m_otherValue = tmp;
		finalize();
	}
	void undo() override {
		redo();
	}
	virtual void finalize() {
		m_target->retransformScale(m_dimension, m_index, true);
		Q_EMIT m_target->q->rangeChanged(m_dimension, m_index, m_target->rangeConst(m_dimension, m_index));
	}

private:
	CartesianPlot::Private* m_target;
	int m_index;
	Dimension m_dimension;
	Range<double> m_otherValue;
};

void CartesianPlot::setRange(const Dimension dim, const int index, const Range<double>& range) {
	Q_D(CartesianPlot);
	DEBUG(Q_FUNC_INFO << ", range = " << range.toStdString() << ", auto scale = " << range.autoScale())

	Dimension dim_other = Dimension::Y;
	if (dim == Dimension::Y)
		dim_other = Dimension::X;

	auto r = d->checkRange(range);
	if (index >= 0 && index < rangeCount(dim) && r.finite() && r != d->rangeConst(dim, index)) {
		d->setRangeDirty(dim, index, true);
		exec(new CartesianPlotSetRangeIndexCmd(d, dim, r, index));
		QVector<int> scaledIndices;
		for (int i = 0; i < coordinateSystemCount(); i++) {
			auto cs = coordinateSystem(i);
			auto index_other = cs->index(dim_other);
			if (cs->index(dim) == index && scaledIndices.indexOf(index_other) == -1) {
				scaledIndices << index_other;
				if (autoScale(dim_other, index_other) && scaleAuto(dim_other, index_other, false))
					d->retransformScale(dim_other, index_other);
			}
		}
		WorksheetElementContainer::retransform();
	}
	DEBUG(Q_FUNC_INFO << ", DONE. range = " << range.toStdString() << ", auto scale = " << range.autoScale())
}

const Range<double>& CartesianPlot::dataRange(const Dimension dim, int index) {
	if (index == -1)
		index = defaultCoordinateSystem()->index(dim);

	if (rangeDirty(dim, index))
		calculateDataRange(dim, index, true);

	Q_D(CartesianPlot);
	return d->dataRange(dim, index);
}

bool CartesianPlot::rangeDirty(const Dimension dim, int index) const {
	Q_D(const CartesianPlot);
	if (index >= 0 && index < rangeCount(dim))
		return d->rangeDirty(dim, index);
	else {
		bool dirty = false;
		for (int i = 0; i < rangeCount(dim); i++)
			dirty |= d->rangeDirty(dim, i);
		return dirty;
	}
}

void CartesianPlot::setRangeDirty(const Dimension dim, int index, bool dirty) {
	Q_D(CartesianPlot);
	if (index >= 0 && index < rangeCount(dim))
		d->setRangeDirty(dim, index, dirty);
	else {
		for (int i = 0; i < rangeCount(dim); i++)
			d->setRangeDirty(dim, i, dirty);
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

void CartesianPlot::removeRange(const Dimension dim, int index) {
	Q_D(CartesianPlot);
	if (index < 0 || index > rangeCount(dim)) {
		DEBUG(Q_FUNC_INFO << ", index " << index << " out of range")
		return;
	}

	switch (dim) {
	case Dimension::X:
		d->xRanges.remove(index);
		break;
	case Dimension::Y:
		d->yRanges.remove(index);
		break;
	}

	if (project())
		project()->setChanged(true);
}

void CartesianPlot::setMin(const Dimension dim, int index, double value) {
	DEBUG(Q_FUNC_INFO << ", direction: " << CartesianCoordinateSystem::dimensionToString(dim).toStdString() << "value = " << value)
	Range<double> r{range(dim, index)};
	r.setStart(value);
	DEBUG(Q_FUNC_INFO << ", new range = " << r.toStdString())
	setRange(dim, index, r);
}

void CartesianPlot::setMax(const Dimension dim, int index, double value) {
	DEBUG(Q_FUNC_INFO << ", direction: " << CartesianCoordinateSystem::dimensionToString(dim).toStdString() << "value = " << value)
	Range<double> r{range(dim, index)};
	r.setEnd(value);

	setRange(dim, index, r);
}

// x/y scale

class CartesianPlotSetScaleIndexCmd : public QUndoCommand {
public:
	CartesianPlotSetScaleIndexCmd(CartesianPlotPrivate* private_obj, const Dimension dim, RangeT::Scale scale, int index)
		: m_private(private_obj)
		, m_dimension(dim)
		, m_scale(scale)
		, m_index(index) {
		setText(i18n("%1: change x-range %2 scale", m_private->name(), index + 1));
	}

	void redo() override {
		m_scaleOld = m_private->rangeConst(m_dimension, m_index).scale();
		m_private->setScale(m_dimension, m_index, m_scale);
		m_private->retransformScale(m_dimension, m_index);
		m_private->q->WorksheetElementContainer::retransform();
		Q_EMIT m_private->q->scaleChanged(m_dimension, m_index, m_scale);
	}

	void undo() override {
		m_private->setScale(m_dimension, m_index, m_scaleOld);
		m_private->retransformScale(m_dimension, m_index);
		m_private->q->WorksheetElementContainer::retransform();
		Q_EMIT m_private->q->scaleChanged(m_dimension, m_index, m_scaleOld);
	}

private:
	CartesianPlotPrivate* m_private;
	Dimension m_dimension;
	RangeT::Scale m_scale;
	int m_index;
	RangeT::Scale m_scaleOld{RangeT::Scale::Linear};
};

RangeT::Scale CartesianPlot::rangeScale(const Dimension dim, const int index) const {
	if (index < 0 || index > rangeCount(dim)) {
		DEBUG(Q_FUNC_INFO << ", index " << index << " out of range")
		return RangeT::Scale::Linear;
	}
	return range(dim, index).scale();
}

RangeT::Scale CartesianPlot::xRangeScale() const {
	return xRangeScale(defaultCoordinateSystem()->index(Dimension::X));
}
RangeT::Scale CartesianPlot::yRangeScale() const {
	return yRangeScale(defaultCoordinateSystem()->index(Dimension::Y));
}
RangeT::Scale CartesianPlot::xRangeScale(const int index) const {
	return rangeScale(Dimension::X, index);
}
RangeT::Scale CartesianPlot::yRangeScale(const int index) const {
	return rangeScale(Dimension::Y, index);
}
void CartesianPlot::setXRangeScale(const RangeT::Scale scale) {
	setXRangeScale(defaultCoordinateSystem()->index(Dimension::X), scale);
}

void CartesianPlot::setYRangeScale(const RangeT::Scale scale) {
	setYRangeScale(defaultCoordinateSystem()->index(Dimension::Y), scale);
}

void CartesianPlot::setRangeScale(const Dimension dim, const int index, const RangeT::Scale scale) {
	Q_D(CartesianPlot);
	if (index < 0 || index > rangeCount(dim)) {
		DEBUG(Q_FUNC_INFO << ", index " << index << " out of range")
		return;
	}
	exec(new CartesianPlotSetScaleIndexCmd(d, dim, scale, index));
	if (project())
		project()->setChanged(true);
}

void CartesianPlot::setXRangeScale(const int index, const RangeT::Scale scale) {
	setRangeScale(Dimension::X, index, scale);
}
void CartesianPlot::setYRangeScale(const int index, const RangeT::Scale scale) {
	setRangeScale(Dimension::Y, index, scale);
}

// coordinate systems

int CartesianPlot::coordinateSystemCount() const {
	return m_coordinateSystems.size();
}

CartesianCoordinateSystem* CartesianPlot::coordinateSystem(int index) const {
	// DEBUG(Q_FUNC_INFO << ", nr of cSystems = " << coordinateSystemCount() << ", index = " << index)
	if (index >= coordinateSystemCount() || index < 0)
		return nullptr;

	return dynamic_cast<CartesianCoordinateSystem*>(m_coordinateSystems.at(index));
}

void CartesianPlot::addCoordinateSystem() {
	auto cSystem = new CartesianCoordinateSystem(this);
	addCoordinateSystem(cSystem);
	// retransform scales, because otherwise the CartesianCoordinateSystem
	// does not have any scales
	retransformScale(Dimension::X, cSystem->index(Dimension::X));
	retransformScale(Dimension::Y, cSystem->index(Dimension::Y));
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

STD_SETTER_CMD_IMPL_S(CartesianPlot, SetDefaultCoordinateSystemIndex, int, defaultCoordinateSystemIndex)
int CartesianPlot::defaultCoordinateSystemIndex() const {
	Q_D(const CartesianPlot);
	return d->defaultCoordinateSystemIndex;
}
void CartesianPlot::setDefaultCoordinateSystemIndex(int index) {
	Q_D(CartesianPlot);
	if (index != d->defaultCoordinateSystemIndex)
		exec(new CartesianPlotSetDefaultCoordinateSystemIndexCmd(d, index, ki18n("%1: set default plot range")));
}
CartesianCoordinateSystem* CartesianPlot::defaultCoordinateSystem() const {
	Q_D(const CartesianPlot);
	return d->defaultCoordinateSystem();
}

// range breaks

STD_SETTER_CMD_IMPL_S(CartesianPlot, SetXRangeBreakingEnabled, bool, xRangeBreakingEnabled)
void CartesianPlot::setXRangeBreakingEnabled(bool enabled) {
	Q_D(CartesianPlot);
	if (enabled != d->xRangeBreakingEnabled) {
		exec(new CartesianPlotSetXRangeBreakingEnabledCmd(d, enabled, ki18n("%1: x-range breaking enabled")));
		retransformScales(); // TODO: replace by retransformScale(Dimension::X, ) with the corresponding index!
		WorksheetElementContainer::retransform(); // retransformScales does not contain any retransfrom() anymore
	}
}

STD_SETTER_CMD_IMPL_S(CartesianPlot, SetXRangeBreaks, CartesianPlot::RangeBreaks, xRangeBreaks)
void CartesianPlot::setXRangeBreaks(const RangeBreaks& breakings) {
	Q_D(CartesianPlot);
	exec(new CartesianPlotSetXRangeBreaksCmd(d, breakings, ki18n("%1: x-range breaks changed")));
	retransformScales(); // TODO: replace by retransformScale(Dimension::X, ) with the corresponding index!
	WorksheetElementContainer::retransform(); // retransformScales does not contain any retransfrom() anymore
}

STD_SETTER_CMD_IMPL_S(CartesianPlot, SetYRangeBreakingEnabled, bool, yRangeBreakingEnabled)
void CartesianPlot::setYRangeBreakingEnabled(bool enabled) {
	Q_D(CartesianPlot);
	if (enabled != d->yRangeBreakingEnabled) {
		exec(new CartesianPlotSetYRangeBreakingEnabledCmd(d, enabled, ki18n("%1: y-range breaking enabled")));
		retransformScales(); // TODO: replace by retransformScale(Dimension::Y, ) with the corresponding index!
		WorksheetElementContainer::retransform(); // retransformScales does not contain any retransfrom() anymore
	}
}

STD_SETTER_CMD_IMPL_S(CartesianPlot, SetYRangeBreaks, CartesianPlot::RangeBreaks, yRangeBreaks)
void CartesianPlot::setYRangeBreaks(const RangeBreaks& breaks) {
	Q_D(CartesianPlot);
	exec(new CartesianPlotSetYRangeBreaksCmd(d, breaks, ki18n("%1: y-range breaks changed")));
	retransformScales(); // TODO: replace by retransformScale(Dimension::Y, ) with the corresponding index!
	WorksheetElementContainer::retransform(); // retransformScales does not contain any retransfrom() anymore
}

// cursor
STD_SETTER_CMD_IMPL_F_S(CartesianPlot, SetCursor0Enable, bool, cursor0Enable, updateCursor)
void CartesianPlot::setCursor0Enable(const bool& enable) {
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
void CartesianPlot::setCursor1Enable(const bool& enable) {
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

void CartesianPlot::retransform() {
	Q_D(CartesianPlot);
	d->retransform();
}

//################################################################
//########################## Slots ###############################
//################################################################
void CartesianPlot::addHorizontalAxis() {
	DEBUG(Q_FUNC_INFO)
	Axis* axis = new Axis(QStringLiteral("x-axis"), Axis::Orientation::Horizontal);
	addChild(axis);
	axis->setSuppressRetransform(true); // retransformTicks() needs plot
	axis->setCoordinateSystemIndex(defaultCoordinateSystemIndex());
	if (axis->rangeType() == Axis::RangeType::Auto) {
		axis->setUndoAware(false);
		// use x range of default plot range
		axis->setRange(range(Dimension::X));
		axis->setMajorTicksNumber(range(Dimension::X).autoTickCount());
		axis->setUndoAware(true);
	}
	axis->setSuppressRetransform(false);
	axis->retransform();
}

void CartesianPlot::addVerticalAxis() {
	Axis* axis = new Axis(QStringLiteral("y-axis"), Axis::Orientation::Vertical);
	axis->setSuppressRetransform(true); // retransformTicks() needs plot
	addChild(axis);
	axis->setCoordinateSystemIndex(defaultCoordinateSystemIndex());
	if (axis->rangeType() == Axis::RangeType::Auto) {
		axis->setUndoAware(false);
		// use y range of default plot range
		axis->setRange(range(Dimension::Y));
		axis->setMajorTicksNumber(range(Dimension::Y).autoTickCount());
		axis->setUndoAware(true);
	}
	axis->setSuppressRetransform(false);
	axis->retransform();
}

void CartesianPlot::addCurve() {
	DEBUG(Q_FUNC_INFO)
	auto* curve{new XYCurve(QStringLiteral("xy-curve"))};
	curve->setCoordinateSystemIndex(defaultCoordinateSystemIndex());
	addChild(curve);
}

void CartesianPlot::addEquationCurve() {
	DEBUG(Q_FUNC_INFO << ", to default coordinate system " << defaultCoordinateSystemIndex())
	auto* curve{new XYEquationCurve(QStringLiteral("f(x)"))};
	curve->setCoordinateSystemIndex(defaultCoordinateSystemIndex());
	addChild(curve);
}

void CartesianPlot::addHistogram() {
	DEBUG(Q_FUNC_INFO << ", to default coordinate system " << defaultCoordinateSystemIndex())
	auto* hist{new Histogram(QStringLiteral("Histogram"))};
	hist->setCoordinateSystemIndex(defaultCoordinateSystemIndex());
	addChild(hist);
}

void CartesianPlot::addHistogramFit(Histogram* hist, nsl_sf_stats_distribution type) {
	if (!hist)
		return;

	beginMacro(i18n("%1: distribution fit to '%2'", name(), hist->name()));
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

	// add the child after the fit was calculated so the dock widgets gets the fit results
	// and call retransform() after this to calculate and to paint the data points of the fit-curve
	this->addChild(curve);
	curve->retransform();

	endMacro();
}

void CartesianPlot::addBarPlot() {
	addChild(new BarPlot(QStringLiteral("Bar Plot")));
}

void CartesianPlot::addBoxPlot() {
	addChild(new BoxPlot(QStringLiteral("Box Plot")));
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
	auto* curve = new XYDataReductionCurve(QStringLiteral("Data reduction"));
	const XYCurve* curCurve = currentCurve();
	curve->setCoordinateSystemIndex(defaultCoordinateSystemIndex());
	if (curCurve) {
		beginMacro(i18n("%1: reduce '%2'", name(), curCurve->name()));
		curve->setName(i18n("Reduction of '%1'", curCurve->name()));
		curve->setDataSourceType(XYAnalysisCurve::DataSourceType::Curve);
		curve->setDataSourceCurve(curCurve);
		this->addChild(curve);
		curve->recalculate();
		Q_EMIT curve->dataReductionDataChanged(curve->dataReductionData());
	} else {
		beginMacro(i18n("%1: add data reduction curve", name()));
		this->addChild(curve);
	}

	endMacro();
}

void CartesianPlot::addDifferentiationCurve() {
	auto* curve = new XYDifferentiationCurve(QStringLiteral("Differentiation"));
	const XYCurve* curCurve = currentCurve();
	curve->setCoordinateSystemIndex(defaultCoordinateSystemIndex());
	if (curCurve) {
		beginMacro(i18n("%1: differentiate '%2'", name(), curCurve->name()));
		curve->setName(i18n("Derivative of '%1'", curCurve->name()));
		curve->setDataSourceType(XYAnalysisCurve::DataSourceType::Curve);
		curve->setDataSourceCurve(curCurve);
		this->addChild(curve);
		curve->recalculate();
		Q_EMIT curve->differentiationDataChanged(curve->differentiationData());
	} else {
		beginMacro(i18n("%1: add differentiation curve", name()));
		this->addChild(curve);
	}

	endMacro();
}

void CartesianPlot::addIntegrationCurve() {
	auto* curve = new XYIntegrationCurve(QStringLiteral("Integration"));
	const XYCurve* curCurve = currentCurve();
	curve->setCoordinateSystemIndex(defaultCoordinateSystemIndex());
	if (curCurve) {
		beginMacro(i18n("%1: integrate '%2'", name(), curCurve->name()));
		curve->setName(i18n("Integral of '%1'", curCurve->name()));
		curve->setDataSourceType(XYAnalysisCurve::DataSourceType::Curve);
		curve->setDataSourceCurve(curCurve);
		this->addChild(curve);
		curve->recalculate();
		Q_EMIT curve->integrationDataChanged(curve->integrationData());
	} else {
		beginMacro(i18n("%1: add integration curve", name()));
		this->addChild(curve);
	}

	endMacro();
}

void CartesianPlot::addInterpolationCurve() {
	auto* curve = new XYInterpolationCurve(QStringLiteral("Interpolation"));
	const XYCurve* curCurve = currentCurve();
	curve->setCoordinateSystemIndex(defaultCoordinateSystemIndex());
	if (curCurve) {
		beginMacro(i18n("%1: interpolate '%2'", name(), curCurve->name()));
		curve->setName(i18n("Interpolation of '%1'", curCurve->name()));
		curve->setDataSourceType(XYAnalysisCurve::DataSourceType::Curve);
		curve->setDataSourceCurve(curCurve);
		curve->recalculate();
		this->addChild(curve);
		Q_EMIT curve->interpolationDataChanged(curve->interpolationData());
	} else {
		beginMacro(i18n("%1: add interpolation curve", name()));
		this->addChild(curve);
	}

	endMacro();
}

void CartesianPlot::addSmoothCurve() {
	auto* curve = new XYSmoothCurve(QStringLiteral("Smooth"));
	const XYCurve* curCurve = currentCurve();
	curve->setCoordinateSystemIndex(defaultCoordinateSystemIndex());
	if (curCurve) {
		beginMacro(i18n("%1: smooth '%2'", name(), curCurve->name()));
		curve->setName(i18n("Smoothing of '%1'", curCurve->name()));
		curve->setDataSourceType(XYAnalysisCurve::DataSourceType::Curve);
		curve->setDataSourceCurve(curCurve);
		this->addChild(curve);
		curve->recalculate();
		Q_EMIT curve->smoothDataChanged(curve->smoothData());
	} else {
		beginMacro(i18n("%1: add smoothing curve", name()));
		this->addChild(curve);
	}

	endMacro();
}

void CartesianPlot::addFitCurve() {
	auto* curve = new XYFitCurve(QStringLiteral("fit"));
	const auto* curCurve = currentCurve();
	curve->setCoordinateSystemIndex(defaultCoordinateSystemIndex());
	if (curCurve) {
		beginMacro(i18n("%1: fit to '%2'", name(), curCurve->name()));
		curve->setName(i18n("Fit to '%1'", curCurve->name()));
		curve->setDataSourceType(XYAnalysisCurve::DataSourceType::Curve);
		curve->setDataSourceCurve(curCurve);

		// set the fit model category and type
		const auto* action = qobject_cast<const QAction*>(QObject::sender());
		if (action) {
			auto type = static_cast<XYAnalysisCurve::AnalysisAction>(action->data().toInt());
			curve->initFitData(type);
		} else {
			DEBUG(Q_FUNC_INFO << "WARNING: no action found!")
		}
		curve->initStartValues(curCurve);

		// fit with weights for y if the curve has error bars for y
		if (curCurve->yErrorType() == XYCurve::ErrorType::Symmetric && curCurve->yErrorPlusColumn()) {
			auto fitData = curve->fitData();
			fitData.yWeightsType = nsl_fit_weight_instrumental;
			curve->setFitData(fitData);
			curve->setYErrorColumn(curCurve->yErrorPlusColumn());
		}

		curve->recalculate();

		// add the child after the fit was calculated so the dock widgets gets the fit results
		// and call retransform() after this to calculate and to paint the data points of the fit-curve
		this->addChild(curve);
		curve->retransform();
	} else {
		beginMacro(i18n("%1: add fit curve", name()));
		this->addChild(curve);
	}

	endMacro();
}

void CartesianPlot::addFourierFilterCurve() {
	auto* curve = new XYFourierFilterCurve(QStringLiteral("Fourier filter"));
	const XYCurve* curCurve = currentCurve();
	if (curCurve) {
		beginMacro(i18n("%1: Fourier filtering of '%2'", name(), curCurve->name()));
		curve->setName(i18n("Fourier filtering of '%1'", curCurve->name()));
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
	auto* curve = new XYFourierTransformCurve(QStringLiteral("Fourier transform"));
	curve->setCoordinateSystemIndex(defaultCoordinateSystemIndex());
	this->addChild(curve);
}

void CartesianPlot::addHilbertTransformCurve() {
	auto* curve = new XYHilbertTransformCurve(QStringLiteral("Hilbert transform"));
	curve->setCoordinateSystemIndex(defaultCoordinateSystemIndex());
	this->addChild(curve);
}

void CartesianPlot::addConvolutionCurve() {
	auto* curve = new XYConvolutionCurve(QStringLiteral("Convolution"));
	curve->setCoordinateSystemIndex(defaultCoordinateSystemIndex());
	this->addChild(curve);
}

void CartesianPlot::addCorrelationCurve() {
	auto* curve = new XYCorrelationCurve(QStringLiteral("Auto-/Cross-Correlation"));
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
	// don't do anything if there's already a legend
	if (m_legend)
		return;

	m_legend = new CartesianPlotLegend(QStringLiteral("legend"));
	this->addChild(m_legend);
	m_legend->retransform();

	// only one legend is allowed -> disable the action
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
		pos = range(Dimension::X).center();

	auto* element = new InfoElement(QStringLiteral("Info Element"), this, curve, pos);
	this->addChild(element);
	element->setParentGraphicsItem(graphicsItem());
	element->retransform(); // must be done, because the element must be retransformed (see https://invent.kde.org/marmsoler/labplot/issues/9)
}

void CartesianPlot::addTextLabel() {
	auto* label = new TextLabel(QStringLiteral("text label"), this);

	Q_D(CartesianPlot);
	if (d->calledFromContextMenu) {
		auto position = label->position();
		position.point = label->parentPosToRelativePos(d->scenePos, position);
		position.point = label->align(position.point, label->graphicsItem()->boundingRect(), label->horizontalAlignment(), label->verticalAlignment(), false);
		label->setPosition(position);
		d->calledFromContextMenu = false;
	}

	this->addChild(label);
	label->setParentGraphicsItem(graphicsItem());
	label->retransform();
}

void CartesianPlot::addImage() {
	auto* image = new Image(QStringLiteral("image"));

	Q_D(CartesianPlot);
	if (d->calledFromContextMenu) {
		auto position = image->position();
		position.point = image->parentPosToRelativePos(d->scenePos, position);
		position.point = image->align(position.point, image->graphicsItem()->boundingRect(), image->horizontalAlignment(), image->verticalAlignment(), false);
		image->setPosition(position);
		d->calledFromContextMenu = false;
	}

	// make the new image somewhat smaller so it's completely visible also on smaller plots
	image->setWidth((int)Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Centimeter));

	this->addChild(image);
	image->retransform();
}

void CartesianPlot::addCustomPoint() {
	Q_D(CartesianPlot);
	auto* point = new CustomPoint(this, QStringLiteral("custom point"));
	point->setCoordinateSystemIndex(defaultCoordinateSystemIndex());

	beginMacro(i18n("%1: add %2", name(), point->name()));

	// must be before setting the position
	this->addChild(point);

	if (d->calledFromContextMenu) {
		point->setCoordinateBindingEnabled(true);
		point->setPositionLogical(d->logicalPos);
		d->calledFromContextMenu = false;
	} else {
		auto p = point->position();
		p.point = QPointF(0, 0);
		point->setPosition(p);
		point->setCoordinateBindingEnabled(true);
	}

	endMacro();
	point->retransform();
}

void CartesianPlot::addReferenceLine() {
	Q_D(CartesianPlot);
	auto* line = new ReferenceLine(this, QStringLiteral("reference line"));
	line->setCoordinateSystemIndex(defaultCoordinateSystemIndex());

	if (d->calledFromContextMenu) {
		line->setPositionLogical(d->logicalPos);
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
	count += children<BarPlot>().size();
	return count;
}

const XYCurve* CartesianPlot::getCurve(int index) {
	return children<XYCurve>().at(index);
}

double CartesianPlot::cursorPos(int cursorNumber) {
	Q_D(const CartesianPlot);
	return (cursorNumber == 0 ? d->cursor0Pos.x() : d->cursor1Pos.x());
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

		if (child->inherits(AspectType::XYCurve) || child->type() == AspectType::Histogram || child->type() == AspectType::BarPlot
			|| child->type() == AspectType::BoxPlot || child->inherits(AspectType::XYAnalysisCurve))
			++index;
	}

	return index;
}

void CartesianPlot::childAdded(const AbstractAspect* child) {
	Q_D(CartesianPlot);

	const auto* curve = dynamic_cast<const XYCurve*>(child);
	const auto* hist = dynamic_cast<const Histogram*>(child);
	const auto* boxPlot = dynamic_cast<const BoxPlot*>(child);
	const auto* barPlot = dynamic_cast<const BarPlot*>(child);

	int cSystemIndex = -1;
	bool checkRanges = false; // check/change ranges when adding new children like curves for example

	const auto* elem = dynamic_cast<const WorksheetElement*>(child);
	// TODO: why is child->type() == AspectType::XYCurve, etc. not working here?
	if (elem
		&& (child->inherits(AspectType::XYCurve) || child->type() == AspectType::Histogram || child->type() == AspectType::BarPlot
			|| child->type() == AspectType::BoxPlot)) {
		auto* elem = static_cast<const WorksheetElement*>(child);
		connect(elem, &WorksheetElement::visibleChanged, this, &CartesianPlot::curveVisibilityChanged);
		connect(elem, &WorksheetElement::aspectDescriptionChanged, this, &CartesianPlot::updateLegend);

		updateLegend();
		cSystemIndex = elem->coordinateSystemIndex();
		checkRanges = true;
	}

	if (curve) {
		DEBUG(Q_FUNC_INFO << ", CURVE")
		// x and y data
		connect(curve, &XYCurve::dataChanged, this, [this, curve]() {
			this->dataChanged(const_cast<XYCurve*>(curve));
		});

		// x data
		connect(curve, &XYCurve::xColumnChanged, this, [this, curve](const AbstractColumn* column) {
			if (curveTotalCount() == 1) // first curve addded
				checkAxisFormat(curve->coordinateSystemIndex(), column, Axis::Orientation::Horizontal);
		});
		connect(curve, &XYCurve::xDataChanged, [this, curve]() {
			this->dataChanged(const_cast<XYCurve*>(curve), Dimension::X);
		});
		connect(curve, &XYCurve::xErrorTypeChanged, [this, curve]() {
			this->dataChanged(const_cast<XYCurve*>(curve), Dimension::X);
		});
		connect(curve, &XYCurve::xErrorPlusColumnChanged, [this, curve]() {
			this->dataChanged(const_cast<XYCurve*>(curve), Dimension::X);
		});
		connect(curve, &XYCurve::xErrorMinusColumnChanged, [this, curve]() {
			this->dataChanged(const_cast<XYCurve*>(curve), Dimension::X);
		});

		// y data
		connect(curve, &XYCurve::yColumnChanged, this, [this, curve](const AbstractColumn* column) {
			if (curveTotalCount() == 1)
				checkAxisFormat(curve->coordinateSystemIndex(), column, Axis::Orientation::Vertical);
		});
		connect(curve, &XYCurve::yDataChanged, [this, curve]() {
			this->dataChanged(const_cast<XYCurve*>(curve), Dimension::Y);
		});
		connect(curve, &XYCurve::yErrorTypeChanged, [this, curve]() {
			this->dataChanged(const_cast<XYCurve*>(curve), Dimension::Y);
		});
		connect(curve, &XYCurve::yErrorPlusColumnChanged, [this, curve]() {
			this->dataChanged(const_cast<XYCurve*>(curve), Dimension::Y);
		});
		connect(curve, &XYCurve::yErrorMinusColumnChanged, [this, curve]() {
			this->dataChanged(const_cast<XYCurve*>(curve), Dimension::Y);
		});

		// update the legend on line and symbol properties changes
		connect(curve, &XYCurve::aspectDescriptionChanged, this, &CartesianPlot::curveNameChanged);
		connect(curve, &XYCurve::legendVisibleChanged, this, &CartesianPlot::updateLegend);
		connect(curve, &XYCurve::lineTypeChanged, this, &CartesianPlot::updateLegend);
		connect(curve->line(), &Line::updateRequested, this, &CartesianPlot::updateLegend);
		connect(curve->symbol(), &Symbol::updateRequested, this, &CartesianPlot::updateLegend);

		// in case the first curve is added, check whether we start plotting datetime data
		if (curveTotalCount() == 1) {
			checkAxisFormat(curve->coordinateSystemIndex(), curve->xColumn(), Axis::Orientation::Horizontal);
			checkAxisFormat(curve->coordinateSystemIndex(), curve->yColumn(), Axis::Orientation::Vertical);
		}

		Q_EMIT curveAdded(curve);
	} else if (hist) {
		DEBUG(Q_FUNC_INFO << ", HISTOGRAM")
		// TODO: check if all ranges must be updated
		connect(hist, &Histogram::dataChanged, [this, hist] {
			this->dataChanged(const_cast<Histogram*>(hist));
		});
		connect(hist, &Histogram::visibleChanged, this, &CartesianPlot::curveVisibilityChanged);
		connect(hist, &Histogram::aspectDescriptionChanged, this, &CartesianPlot::updateLegend);

		updateLegend();
		cSystemIndex = hist->coordinateSystemIndex();
		checkRanges = true;

		if (curveTotalCount() == 1)
			checkAxisFormat(hist->coordinateSystemIndex(), hist->dataColumn(), Axis::Orientation::Horizontal);
	} else if (boxPlot) {
		DEBUG(Q_FUNC_INFO << ", BOX PLOT")
		connect(boxPlot, &BoxPlot::dataChanged, [this, boxPlot] {
			this->dataChanged(const_cast<BoxPlot*>(boxPlot));
		});

		if (curveTotalCount() == 1) {
			connect(boxPlot, &BoxPlot::orientationChanged, this, &CartesianPlot::boxPlotOrientationChanged);
			boxPlotOrientationChanged(boxPlot->orientation());
			if (!boxPlot->dataColumns().isEmpty())
				checkAxisFormat(boxPlot->coordinateSystemIndex(), boxPlot->dataColumns().constFirst(), Axis::Orientation::Vertical);
		}
	} else if (barPlot) {
		DEBUG(Q_FUNC_INFO << ", BAR PLOT")
		// TODO: check if all ranges must be updated
		connect(barPlot, &BarPlot::dataChanged, [this, barPlot] {
			this->dataChanged(const_cast<BarPlot*>(barPlot));
		});

		// update the legend on data column and formatting changes
		connect(barPlot, &BarPlot::dataColumnsChanged, this, &CartesianPlot::updateLegend);
		connect(barPlot, &BarPlot::updateLegendRequested, this, &CartesianPlot::updateLegend);
	} else {
		const auto* infoElement = dynamic_cast<const InfoElement*>(child);
		if (infoElement)
			connect(this, &CartesianPlot::curveRemoved, infoElement, &InfoElement::removeCurve);

		// if an element is hovered, the curves which are handled manually in this class
		// must be unhovered
		if (elem)
			connect(elem, &WorksheetElement::hovered, this, &CartesianPlot::childHovered);
	}

	auto rangeChanged = false;
	if (checkRanges && INRANGE(cSystemIndex, 0, m_coordinateSystems.count())) {
		auto xIndex = coordinateSystem(cSystemIndex)->index(Dimension::X);
		auto yIndex = coordinateSystem(cSystemIndex)->index(Dimension::Y);
		setRangeDirty(Dimension::X, xIndex, true);
		setRangeDirty(Dimension::Y, yIndex, true);

		if (autoScale(Dimension::X, xIndex) && autoScale(Dimension::Y, yIndex))
			rangeChanged = scaleAuto(xIndex, yIndex);
		else if (autoScale(Dimension::X, xIndex))
			rangeChanged = scaleAuto(Dimension::X, xIndex);
		else if (autoScale(Dimension::Y, yIndex))
			rangeChanged = scaleAuto(Dimension::Y, yIndex);

		if (rangeChanged)
			WorksheetElementContainer::retransform();
	}

	if (!isLoading() && !this->pasted() && !child->pasted() && !child->isMoved()) {
		// new child was added which might change the ranges and the axis tick labels.
		// adjust the plot area padding if the axis label is outside of the plot area
		if (rangeChanged) {
			const auto& axes = children<Axis>();
			for (auto* axis : axes) {
				if (axis->orientation() == WorksheetElement::Orientation::Vertical) {
					double delta = plotArea()->graphicsItem()->boundingRect().x() - axis->graphicsItem()->boundingRect().x();
					if (delta > 0) {
						setUndoAware(false);
						// 					setSuppressRetransform(true);
						setSymmetricPadding(false);
						setHorizontalPadding(horizontalPadding() + delta);
						// 					setSuppressRetransform(false);
						setUndoAware(true);
					}
					break;
				}
			}
		}

		// if a theme was selected, apply the theme settings for newly added children,
		// load default theme settings otherwise.
		if (elem) {
			// TODO			const_cast<WorksheetElement*>(elem)->setCoordinateSystemIndex(defaultCoordinateSystemIndex());
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
void CartesianPlot::checkAxisFormat(const int cSystemIndex, const AbstractColumn* column, Axis::Orientation orientation) {
	if (isLoading())
		return;

	const auto* col = qobject_cast<const Column*>(column);
	if (!col)
		return;

	const int xIndex = coordinateSystem(cSystemIndex)->index(Dimension::X);
	const int yIndex = coordinateSystem(cSystemIndex)->index(Dimension::Y);

	Q_D(CartesianPlot);
	if (col->columnMode() == AbstractColumn::ColumnMode::DateTime) {
		setUndoAware(false);
		if (orientation == Axis::Orientation::Horizontal)
			setXRangeFormat(xIndex, RangeT::Format::DateTime);
		else
			setYRangeFormat(yIndex, RangeT::Format::DateTime);
		setUndoAware(true);

		// set column's datetime format for all horizontal axis
		for (auto* axis : children<Axis>()) {
			if (axis->orientation() == orientation) {
				const auto* cSystem{coordinateSystem(axis->coordinateSystemIndex())};
				const auto* filter = static_cast<DateTime2StringFilter*>(col->outputFilter());
				d->xRanges[cSystem ? cSystem->index(Dimension::X) : 0].range.setDateTimeFormat(filter->format());
				axis->setUndoAware(false);
				axis->setLabelsDateTimeFormat(rangeDateTimeFormat(Dimension::X, xIndex));
				axis->setUndoAware(true);
			}
		}
	} else {
		setUndoAware(false);
		if (orientation == Axis::Orientation::Horizontal)
			setXRangeFormat(xIndex, RangeT::Format::Numeric);
		else
			setYRangeFormat(yIndex, RangeT::Format::Numeric);

		setUndoAware(true);
	}
}

void CartesianPlot::boxPlotOrientationChanged(BoxPlot::Orientation orientation) {
	const auto& axes = children<Axis>();

	// don't show any labels for the first axis orthogonal to the orientation of the boxplot
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

	// don't show any labels for the first axis parallel to the orientation of the boxplot
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
	QDEBUG(Q_FUNC_INFO << ", CHILD = " << child)
	if (m_legend == child) {
		DEBUG(Q_FUNC_INFO << ", a legend")
		if (m_menusInitialized)
			addLegendAction->setEnabled(true);
		m_legend = nullptr;
	} else {
		const auto* curve = qobject_cast<const XYCurve*>(child);
		Q_D(CartesianPlot);
		if (curve) {
			DEBUG(Q_FUNC_INFO << ", a curve")
			updateLegend();
			Q_EMIT curveRemoved(curve);
			const auto cs = coordinateSystem(curve->coordinateSystemIndex());
			const auto xIndex = cs->index(Dimension::X);
			const auto yIndex = cs->index(Dimension::Y);
			d->xRanges[xIndex].dirty = true;
			d->yRanges[yIndex].dirty = true;

			bool rangeChanged = false;
			if (autoScale(Dimension::X, xIndex) && autoScale(Dimension::Y, yIndex))
				rangeChanged = scaleAuto(xIndex, yIndex);
			else if (autoScale(Dimension::X, xIndex))
				rangeChanged = scaleAuto(Dimension::X, xIndex);
			else if (autoScale(Dimension::Y, yIndex))
				rangeChanged = scaleAuto(Dimension::Y, yIndex);

			if (rangeChanged)
				WorksheetElementContainer::retransform();
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
		for (auto curve : children<XYCurve>())
			curve->setHover(false);
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
void CartesianPlot::dataChanged(int xIndex, int yIndex, WorksheetElement* sender) {
	DEBUG(Q_FUNC_INFO << ", x/y index = " << xIndex << "/" << yIndex)
	if (isLoading())
		return;

	Q_D(CartesianPlot);
	if (d->suppressRetransform)
		return;

	if (xIndex == -1) {
		for (int i = 0; i < rangeCount(Dimension::X); i++)
			d->setRangeDirty(Dimension::X, i, true);
	} else
		d->setRangeDirty(Dimension::X, xIndex, true);

	if (yIndex == -1) {
		for (int i = 0; i < rangeCount(Dimension::Y); i++)
			d->setRangeDirty(Dimension::Y, i, true);
	} else
		d->setRangeDirty(Dimension::Y, yIndex, true);

	bool updated = false;
	if (autoScale(Dimension::X, xIndex) && autoScale(Dimension::Y, yIndex))
		updated = scaleAuto(xIndex, yIndex);
	else if (autoScale(Dimension::X, xIndex))
		updated = scaleAuto(Dimension::X, xIndex);
	else if (autoScale(Dimension::Y, yIndex))
		updated = scaleAuto(Dimension::Y, yIndex);

	if (updated)
		WorksheetElementContainer::retransform();
	else {
		// even if the plot ranges were not changed, either no auto scale active or the new data
		// is within the current ranges and no change of the ranges is required,
		// retransform the curve in order to show the changes
		if (sender)
			sender->retransform();
		else {
			// no sender available, the function was called directly in the file filter (live data source got new data)
			// or in Project::load() -> retransform all available curves since we don't know which curves are affected.
			// TODO: this logic can be very expensive
			for (auto* child : children<XYCurve>()) {
				child->recalcLogicalPoints();
				child->retransform();
			}
		}
	}
}

void CartesianPlot::dataChanged(WorksheetElement* element) {
	DEBUG(Q_FUNC_INFO)
	if (project() && project()->isLoading())
		return;

	Q_D(CartesianPlot);
	if (d->suppressRetransform)
		return;

	if (!element)
		return;

	int cSystemIndex = element->coordinateSystemIndex();
	if (cSystemIndex == -1)
		return;

	auto xIndex = coordinateSystem(cSystemIndex)->index(Dimension::X);
	auto yIndex = coordinateSystem(cSystemIndex)->index(Dimension::Y);

	dataChanged(xIndex, yIndex, element);
}

/*!
	called when in one of the curves the data in one direction was changed.
	Autoscales the coordinate system and the x/y-axes, when "auto-scale" is active.
*/
void CartesianPlot::dataChanged(XYCurve* curve, const Dimension dim) {
	DEBUG(Q_FUNC_INFO)
	if (project() && project()->isLoading())
		return;

	Q_D(CartesianPlot);
	if (d->suppressRetransform)
		return;

	if (!curve)
		return;

	int cSystemIndex = curve->coordinateSystemIndex();
	if (cSystemIndex == -1)
		return;
	auto index = coordinateSystem(cSystemIndex)->index(dim);
	Dimension dim_other = Dimension::Y;
	switch (dim) {
	case Dimension::X:
		d->xRanges[index].dirty = true;
		break;
	case Dimension::Y:
		dim_other = Dimension::X;
		d->yRanges[index].dirty = true;
		break;
	}

	bool updated = false;
	if (autoScale(dim, index))
		updated = this->scaleAuto(dim, index);

	QVector<int> scaled;
	for (auto* acSystem : m_coordinateSystems) {
		auto* cSystem = static_cast<CartesianCoordinateSystem*>(acSystem);
		if (cSystem->index(dim) == index && scaled.indexOf(cSystem->index(dim_other)) == -1 && // do not scale again
			autoScale(dim_other, cSystem->index(dim_other))) {
			scaled << cSystem->index(dim_other);
			updated |= scaleAuto(dim_other, cSystem->index(dim_other), false);
		}
	}
	DEBUG(Q_FUNC_INFO << ", updated = " << updated)

	if (updated)
		WorksheetElementContainer::retransform();
	else {
		// even if the plot ranges were not changed, either no auto scale active or the new data
		// is within the current ranges and no change of the ranges is required,
		// retransform the curve in order to show the changes
		curve->retransform();
	}

	// in case there is only one curve and its column mode was changed, check whether we start plotting datetime data
	if (children<XYCurve>().size() == 1) {
		const auto* col = curve->column(dim);
		const auto rangeFormat{range(dim, index).format()};
		if (col && col->columnMode() == AbstractColumn::ColumnMode::DateTime && rangeFormat != RangeT::Format::DateTime) {
			setUndoAware(false);
			setRangeFormat(dim, index, RangeT::Format::DateTime);
			setUndoAware(true);
		}
	}
	Q_EMIT curveDataChanged(curve);
}

void CartesianPlot::curveVisibilityChanged() {
	int index = static_cast<WorksheetElement*>(QObject::sender())->coordinateSystemIndex();
	int xIndex = coordinateSystem(index)->index(Dimension::X);
	int yIndex = coordinateSystem(index)->index(Dimension::Y);
	setRangeDirty(Dimension::X, xIndex, true);
	setRangeDirty(Dimension::Y, yIndex, true);
	updateLegend();
	if (autoScale(Dimension::X, xIndex) && autoScale(Dimension::Y, yIndex))
		this->scaleAuto(xIndex, yIndex);
	else if (autoScale(Dimension::X, xIndex))
		this->scaleAuto(Dimension::X, xIndex, false);
	else if (autoScale(Dimension::Y, yIndex))
		this->scaleAuto(Dimension::Y, yIndex, false);

	WorksheetElementContainer::retransform();

	Q_EMIT curveVisibilityChangedSignal();
}

void CartesianPlot::curveLinePenChanged(QPen pen) {
	const auto* curve = qobject_cast<const XYCurve*>(QObject::sender());
	Q_EMIT curveLinePenChanged(pen, curve->name());
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

	// when doing zoom selection, prevent the graphics item from being movable
	// if it's currently movable (no worksheet layout available)
	const auto* worksheet = qobject_cast<const Worksheet*>(parentAspect());
	if (worksheet) {
		if (mouseMode == MouseMode::Selection) {
			if (worksheet->layout() != Worksheet::Layout::NoLayout)
				graphicsItem()->setFlag(QGraphicsItem::ItemIsMovable, false);
			else
				graphicsItem()->setFlag(QGraphicsItem::ItemIsMovable, true);
		} else // zoom m_selection
			graphicsItem()->setFlag(QGraphicsItem::ItemIsMovable, false);
	}

	Q_EMIT mouseModeChanged(mouseMode);
}

BASIC_SHARED_D_ACCESSOR_IMPL(CartesianPlot, bool, isLocked, Locked, locked)

// auto scale x axis 'index' when auto scale is enabled (index == -1: all x axes)
bool CartesianPlot::scaleAuto(const Dimension dim, int index, bool fullRange, bool suppressRetransformScale) {
	DEBUG(Q_FUNC_INFO << ", dim = " << int(dim) << ", index = " << index << ", full range = " << fullRange)
	PERFTRACE(QLatin1String(Q_FUNC_INFO));
	Q_D(CartesianPlot);
	if (index == -1) { // all ranges
		bool updated = false;
		for (int i = 0; i < rangeCount(dim); i++) {
			if (autoScale(dim, i) && scaleAuto(dim, i, fullRange, true)) {
				if (!suppressRetransformScale)
					d->retransformScale(dim, i);
				updated = true; // at least one was updated
			}
		}
		return updated;
	}

	auto& r{d->range(dim, index)};
	DEBUG(Q_FUNC_INFO << ", " << CartesianCoordinateSystem::dimensionToString(dim).toStdString() << " range dirty = " << rangeDirty(dim, index))
	if (rangeDirty(dim, index)) {
		calculateDataRange(dim, index, fullRange);
		setRangeDirty(dim, index, false);

		if (fullRange) {
			// If not fullrange the y range will be used. So that means
			// the yrange would not change and therefore it must not be dirty
			for (const auto* c : m_coordinateSystems) {
				// All x ranges with this xIndex must be dirty
				const auto* cs = dynamic_cast<const CartesianCoordinateSystem*>(c);
				if (cs->index(dim) == index) {
					switch (dim) {
					case Dimension::X:
						setRangeDirty(Dimension::Y, cs->index(Dimension::Y), true);
						break;
					case Dimension::Y:
						setRangeDirty(Dimension::X, cs->index(Dimension::X), true);
						break;
					}
				}
			}
		}
	}
	auto dataRange = d->dataRange(dim, index);
	if (dataRange.finite() && d->niceExtend)
		dataRange.niceExtend(); // auto scale to nice data range

	// if no curve: do not reset to [0, 1]

	DEBUG(Q_FUNC_INFO << ", range " << index << " = " << r.toStdString() << "., data range = " << d->dataRange(dim, index).toStdString())
	bool update = false;
	if (!qFuzzyCompare(dataRange.start(), r.start()) && !qIsInf(dataRange.start())) {
		r.start() = dataRange.start();
		update = true;
	}

	if (!qFuzzyCompare(dataRange.end(), r.end()) && !qIsInf(dataRange.end())) {
		r.end() = dataRange.end();
		update = true;
	}

	if (update) {
		switch (dim) {
		case Dimension::X:
			DEBUG(Q_FUNC_INFO << ", set new x range = " << r.toStdString())
			break;
		case Dimension::Y:
			DEBUG(Q_FUNC_INFO << ", set new y range = " << r.toStdString())
			break;
		}
		// in case min and max are equal (e.g. if we plot a single point), subtract/add 10% of the value
		if (r.isZero()) {
			const double value{r.start()};
			if (!qFuzzyIsNull(value))
				r.setRange(value * 0.9, value * 1.1);
			else
				r.setRange(-0.1, 0.1);
		} else
			r.extend(r.size() * d->autoScaleOffsetFactor);

		if (!suppressRetransformScale)
			d->retransformScale(dim, index);
	}

	return update;
}

// auto scale all x axis xIndex and y axis yIndex when auto scale is enabled (index == -1: all x/y axes)
bool CartesianPlot::scaleAuto(int xIndex, int yIndex, bool fullRange, bool suppressRetransformScale) {
	DEBUG(Q_FUNC_INFO << " x/y index = " << xIndex << " / " << yIndex)
	PERFTRACE(QLatin1String(Q_FUNC_INFO));
	bool updateX = scaleAuto(Dimension::X, xIndex, fullRange, suppressRetransformScale);
	bool updateY = scaleAuto(Dimension::Y, yIndex, fullRange, suppressRetransformScale);
	DEBUG(Q_FUNC_INFO << ", update X/Y = " << updateX << "/" << updateY)

	// x range is dirty, because scaleAutoY sets it to dirty.
	// TODO: check if it can be removed
	if (xIndex < 0) {
		for (int i = 0; i < m_coordinateSystems.count(); i++) {
			setRangeDirty(Dimension::X, coordinateSystem(i)->index(Dimension::X), false);
			// setRangeDirty(Dimension::Y, coordinateSystem(i)->index(Dimension::Y), false);
		}
	} else {
		setRangeDirty(Dimension::X, xIndex, false);
		// setRangeDirty(Dimension::Y, coordinateSystem(cSystemIndex)->index(Dimension::Y), false);
	}

	return (updateX || updateY);
}

/*!
 * Calculates and saves data x range.
 * The range of the y axis is not considered.
 */
void CartesianPlot::calculateDataRange(const Dimension dim, const int index, bool completeRange) {
	DEBUG(Q_FUNC_INFO << ", direction = " << CartesianCoordinateSystem::dimensionToString(dim).toStdString() << ", index = " << index
					  << ", complete range = " << completeRange)
	Q_D(CartesianPlot);

	d->dataRange(dim, index).setRange(qInf(), -qInf());
	auto range{d->range(dim, index)}; // get reference to range from private

	// loop over all xy-curves and determine the maximum and minimum dir-values
	for (const auto* curve : this->children<const XYCurve>()) {
		// only curves with correct xIndex
		if (coordinateSystem(curve->coordinateSystemIndex())->index(dim) != index)
			continue;
		if (!curve->isVisible())
			continue;

		DEBUG("CURVE \"" << STDSTRING(curve->name()) << "\"")

		auto* column = curve->column(dim);
		if (!column) {
			DEBUG(" NO column!")
			continue;
		}

		Dimension dim_other = Dimension::Y;
		switch (dim) {
		case Dimension::X:
			break;
		case Dimension::Y:
			dim_other = Dimension::X;
			break;
		}

		// range of indices
		Range<int> indexRange{0, 0};
		if (!completeRange && d->rangeType == RangeType::Free && curve->column(dim_other)) { // only data within y range
			const int index = coordinateSystem(curve->coordinateSystemIndex())->index(dim_other);
			DEBUG(Q_FUNC_INFO << ", free incomplete range with y column. y range = " << d->range(dim_other, index).toStdString())
			curve->column(dim_other)->indicesMinMax(d->range(dim_other, index).start(), d->range(dim_other, index).end(), indexRange.start(), indexRange.end());
		} else { // all data
			DEBUG(Q_FUNC_INFO << ", else. range type = " << (int)d->rangeType)
			switch (d->rangeType) {
			case RangeType::Free:
				indexRange.setRange(0, column->rowCount() - 1);
				break;
			case RangeType::Last:
				indexRange.setRange(column->rowCount() - d->rangeLastValues, column->rowCount() - 1);
				break;
			case RangeType::First:
				indexRange.setRange(0, d->rangeFirstValues - 1);
				break;
			}
		}
		DEBUG(Q_FUNC_INFO << ", index range = " << indexRange.toStdString())

		curve->minMax(dim, indexRange, range, true);

		if (range.start() < d->dataRange(dim, index).start())
			d->dataRange(dim, index).start() = range.start();

		if (range.end() > d->dataRange(dim, index).end())
			d->dataRange(dim, index).end() = range.end();
		DEBUG(Q_FUNC_INFO << ", curves range i = " << d->dataRange(dim, index).toStdString(false))
	}

	// loop over all histograms and determine the maximum and minimum x-value
	for (const auto* curve : this->children<const Histogram>()) {
		if (!curve->isVisible() || !curve->dataColumn())
			continue;

		const double min = curve->minimum(dim);
		if (d->dataRange(dim, index).start() > min)
			d->dataRange(dim, index).start() = min;

		const double max = curve->maximum(dim);
		if (max > d->dataRange(dim, index).end())
			d->dataRange(dim, index).end() = max;
	}

	// loop over all box plots and determine the maximum and minimum x-values
	for (const auto* curve : this->children<const BoxPlot>()) {
		if (!curve->isVisible() || curve->dataColumns().isEmpty())
			continue;

		const double min = curve->minimum(dim);
		if (d->dataRange(dim, index).start() > min)
			d->dataRange(dim, index).start() = min;

		const double max = curve->maximum(dim);
		if (max > d->dataRange(dim, index).end())
			d->dataRange(dim, index).end() = max;
	}

	// loop over all box plots and determine the maximum and minimum x-values
	for (const auto* curve : this->children<const BarPlot>()) {
		if (!curve->isVisible() || curve->dataColumns().isEmpty())
			continue;

		const double min = curve->minimum(dim);
		if (d->dataRange(dim, index).start() > min)
			d->dataRange(dim, index).start() = min;

		const double max = curve->maximum(dim);
		if (max > d->dataRange(dim, index).end())
			d->dataRange(dim, index).end() = max;
	}

	// check ranges for nonlinear scales
	if (d->dataRange(dim, index).scale() != RangeT::Scale::Linear)
		d->dataRange(dim, index) = d->checkRange(d->dataRange(dim, index));

	DEBUG(Q_FUNC_INFO << ", data " << CartesianCoordinateSystem::dimensionToString(dim).toStdString()
					  << " range = " << d->dataRange(dim, index).toStdString(false))
}

void CartesianPlot::retransformScales() {
	Q_D(CartesianPlot);
	d->retransformScales(-1, -1);
}
void CartesianPlot::retransformScale(Dimension dim, int index) {
	Q_D(CartesianPlot);
	d->retransformScale(dim, index);
}

// zoom

void CartesianPlot::zoomIn(int xIndex, int yIndex) {
	setUndoAware(false);
	enableAutoScale(Dimension::X, xIndex, false);
	enableAutoScale(Dimension::Y, yIndex, false);
	setUndoAware(true);
	setRangeDirty(Dimension::X, xIndex, true);
	setRangeDirty(Dimension::Y, yIndex, true);
	zoom(xIndex, Dimension::X, true); // zoom in x
	zoom(yIndex, Dimension::Y, true); // zoom in y

	Q_D(CartesianPlot);
	d->retransformScales(xIndex, yIndex);
	WorksheetElementContainer::retransform();
}

void CartesianPlot::zoomOut(int xIndex, int yIndex) {
	setUndoAware(false);
	enableAutoScale(Dimension::X, xIndex, false);
	enableAutoScale(Dimension::Y, yIndex, false);
	setUndoAware(true);
	setRangeDirty(Dimension::X, xIndex, true);
	setRangeDirty(Dimension::Y, yIndex, true);
	zoom(xIndex, Dimension::X, false); // zoom out x
	zoom(yIndex, Dimension::Y, false); // zoom out y

	Q_D(CartesianPlot);
	d->retransformScales(xIndex, yIndex);
	WorksheetElementContainer::retransform();
}

void CartesianPlot::zoomInX(int index) {
	zoomInOut(index, Dimension::X, true);
}

void CartesianPlot::zoomOutX(int index) {
	zoomInOut(index, Dimension::X, false);
}

void CartesianPlot::zoomInY(int index) {
	zoomInOut(index, Dimension::Y, true);
}

void CartesianPlot::zoomOutY(int index) {
	zoomInOut(index, Dimension::Y, false);
}

void CartesianPlot::zoomInOut(const int index, const Dimension dim, const bool zoomIn) {
	Dimension dim_other = Dimension::Y;
	if (dim == Dimension::Y)
		dim_other = Dimension::X;

	setUndoAware(false);
	enableAutoScale(dim, index, false);
	setUndoAware(true);
	setRangeDirty(dim_other, index, true);
	zoom(index, dim, zoomIn);

	bool retrans = false;
	for (int i = 0; i < m_coordinateSystems.count(); i++) {
		const auto cs = coordinateSystem(i);
		if (index == -1 || index == cs->index(dim)) {
			if (autoScale(dim_other, cs->index(dim_other)))
				scaleAuto(dim_other, cs->index(dim_other), false);
			retrans = true;
		}
	}

	Q_D(CartesianPlot);
	if (retrans) {
		// If the other dimension is autoScale it will be scaled and then
		// retransformScale() will be called. So here we just have to do
		// it for the nontransformed scale because in zoom it will not be done
		if (index == -1) {
			for (int i = 0; i < rangeCount(dim); i++)
				d->retransformScale(dim, i);
		} else
			d->retransformScale(dim, index);
		WorksheetElementContainer::retransform();
	}
}

/*!
 * helper function called in other zoom*() functions
 * and doing the actual change of the data ranges.
 * @param x if set to \true the x-range is modified, the y-range for \c false
 * @param in the "zoom in" is performed if set to \c \true, "zoom out" for \c false
 */
void CartesianPlot::zoom(int index, const Dimension dim, bool zoom_in) {
	Q_D(CartesianPlot);

	Range<double> range;
	if (index == -1) {
		QVector<int> zoomedIndices;
		for (int i = 0; i < m_coordinateSystems.count(); i++) {
			int idx = coordinateSystem(i)->index(dim);
			if (zoomedIndices.contains(idx))
				continue;
			zoom(idx, dim, zoom_in);
			zoomedIndices.append(idx);
		}
		return;
	}
	range = d->range(dim, index);

	double factor = m_zoomFactor;
	if (zoom_in)
		factor = 1. / factor;

	const double start{range.start()}, end{range.end()};
	switch (range.scale()) {
	case RangeT::Scale::Linear: {
		range.extend(range.size() * (factor - 1.) / 2.);
		break;
	}
	case RangeT::Scale::Log10: {
		if (start == 0 || end / start <= 0)
			break;
		const double diff = log10(end / start) * (factor - 1.);
		const double extend = pow(10, diff / 2.);
		range.end() *= extend;
		range.start() /= extend;
		break;
	}
	case RangeT::Scale::Log2: {
		if (start == 0 || end / start <= 0)
			break;
		const double diff = log2(end / start) * (factor - 1.);
		const double extend = exp2(diff / 2.);
		range.end() *= extend;
		range.start() /= extend;
		break;
	}
	case RangeT::Scale::Ln: {
		if (start == 0 || end / start <= 0)
			break;
		const double diff = log(end / start) * (factor - 1.);
		const double extend = exp(diff / 2.);
		range.end() *= extend;
		range.start() /= extend;
		break;
	}
	case RangeT::Scale::Sqrt: {
		if (start < 0 || end < 0)
			break;
		const double diff = (sqrt(end) - sqrt(start)) * (factor - 1.);
		range.extend(diff * diff / 4.);
		break;
	}
	case RangeT::Scale::Square: {
		const double diff = (end * end - start * start) * (factor - 1.);
		range.extend(sqrt(qAbs(diff / 2.)));
		break;
	}
	case RangeT::Scale::Inverse: {
		const double diff = (1. / start - 1. / end) * (factor - 1.);
		range.extend(1. / qAbs(diff / 2.));
		break;
	}
	}

	// make nice again
	if (d->niceExtend) {
		if (zoom_in)
			range.niceShrink();
		else
			range.niceExtend();
	}

	if (range.finite())
		d->setRange(dim, index, range);
}

/*!
 * helper function called in other shift*() functions
 * and doing the actual change of the data ranges.
 * @param x if set to \true the x-range is modified, the y-range for \c false
 * @param leftOrDown the "shift left" for x or "shift dows" for y is performed if set to \c \true,
 * "shift right" or "shift up" for \c false
 */
void CartesianPlot::shift(int index, const Dimension dim, bool leftOrDown) {
	Q_D(CartesianPlot);

	Range<double> range;
	if (index == -1) {
		QVector<int> shiftedIndices;
		for (int i = 0; i < m_coordinateSystems.count(); i++) {
			int idx = coordinateSystem(i)->index(dim);
			if (shiftedIndices.contains(idx))
				continue;
			shift(idx, dim, leftOrDown);
			shiftedIndices.append(idx);
		}
		return;
	}
	range = d->range(dim, index);

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
		if (start == 0 || end / start <= 0)
			break;
		offset = log10(end / start) * factor;
		range *= pow(10, offset);
		break;
	}
	case RangeT::Scale::Log2: {
		if (start == 0 || end / start <= 0)
			break;
		offset = log2(end / start) * factor;
		range *= exp2(offset);
		break;
	}
	case RangeT::Scale::Ln: {
		if (start == 0 || end / start <= 0)
			break;
		offset = log(end / start) * factor;
		range *= exp(offset);
		break;
	}
	case RangeT::Scale::Sqrt:
		if (start < 0 || end < 0)
			break;
		offset = (sqrt(end) - sqrt(start)) * factor;
		range += offset * offset;
		break;
	case RangeT::Scale::Square:
		offset = (end * end - start * start) * factor;
		range += sqrt(qAbs(offset));
		break;
	case RangeT::Scale::Inverse:
		offset = (1. / start - 1. / end) * factor;
		range += 1. / qAbs(offset);
		break;
	}

	if (range.finite())
		d->setRange(dim, index, range);

	d->retransformScale(dim, index);
}

void CartesianPlot::shiftLeftX(int index) {
	setUndoAware(false);
	enableAutoScale(Dimension::X, index, false);
	setUndoAware(true);
	shift(index, Dimension::X, true);

	bool retrans = false;
	for (const auto cSystem : m_coordinateSystems) {
		const auto cs = static_cast<CartesianCoordinateSystem*>(cSystem);
		if ((index == -1 || index == cs->index(Dimension::X))) {
			if (autoScale(Dimension::Y, cs->index(Dimension::Y))) {
				setRangeDirty(Dimension::Y, cs->index(Dimension::Y), true);
				scaleAuto(Dimension::Y, cs->index(Dimension::Y), false);
			}
			retrans = true;
		}
	}

	if (retrans)
		WorksheetElementContainer::retransform();
}

void CartesianPlot::shiftRightX(int index) {
	setUndoAware(false);
	enableAutoScale(Dimension::X, index, false);
	setUndoAware(true);
	shift(index, Dimension::X, false);

	bool retrans = false;
	for (const auto cSystem : m_coordinateSystems) {
		const auto cs = static_cast<CartesianCoordinateSystem*>(cSystem);
		if ((index == -1 || index == cs->index(Dimension::X))) {
			if (autoScale(Dimension::Y, cs->index(Dimension::Y))) {
				setRangeDirty(Dimension::Y, cs->index(Dimension::Y), true);
				scaleAuto(Dimension::Y, cs->index(Dimension::Y), false);
			}
			retrans = true;
		}
	}

	if (retrans)
		WorksheetElementContainer::retransform();
}

void CartesianPlot::shiftUpY(int index) {
	setUndoAware(false);
	enableAutoScale(Dimension::Y, index, false);
	setUndoAware(true);
	shift(index, Dimension::Y, false);

	bool retrans = false;
	for (const auto cSystem : m_coordinateSystems) {
		const auto cs = static_cast<CartesianCoordinateSystem*>(cSystem);
		if ((index == -1 || index == cs->index(Dimension::Y))) {
			if (autoScale(Dimension::X, cs->index(Dimension::X))) {
				setRangeDirty(Dimension::X, cs->index(Dimension::X), true);
				scaleAuto(Dimension::X, cs->index(Dimension::X), false);
			}
			retrans = true;
		}
	}

	if (retrans)
		WorksheetElementContainer::retransform();
}

void CartesianPlot::shiftDownY(int index) {
	setUndoAware(false);
	enableAutoScale(Dimension::Y, index, false);
	setUndoAware(true);
	shift(index, Dimension::Y, true);

	bool retrans = false;
	for (const auto cSystem : m_coordinateSystems) {
		const auto cs = static_cast<CartesianCoordinateSystem*>(cSystem);
		if ((index == -1 || index == cs->index(Dimension::Y))) {
			if (autoScale(Dimension::X, cs->index(Dimension::X))) {
				setRangeDirty(Dimension::X, cs->index(Dimension::X), true);
				scaleAuto(Dimension::X, cs->index(Dimension::X), false);
			}
			retrans = true;
		}
	}

	if (retrans)
		WorksheetElementContainer::retransform();
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

//#####################################################################
//################### Private implementation ##########################
//#####################################################################
CartesianPlotPrivate::CartesianPlotPrivate(CartesianPlot* plot)
	: AbstractPlotPrivate(plot)
	, q(plot) {
	m_cursor0Text.prepare();
	m_cursor1Text.prepare();
}

CartesianPlotPrivate::~CartesianPlotPrivate() = default;

/*!
	updates the position of plot rectangular in scene coordinates to \c r and recalculates the scales.
	The size of the plot corresponds to the size of the plot area, the area which is filled with the background color etc.
	and which can pose the parent item for several sub-items (like TextLabel).
	Note, the size of the area used to define the coordinate system doesn't need to be equal to this plot area.
	Also, the size (=bounding box) of CartesianPlot can be greater than the size of the plot area.
 */
void CartesianPlotPrivate::retransform() {
	const bool suppress = suppressRetransform || q->isLoading();
	trackRetransformCalled(suppress);
	for (int i = 0; i < xRanges.count(); i++)
		DEBUG(Q_FUNC_INFO << ", x range " << i + 1 << " : " << xRanges.at(i).range.toStdString()
						  << ", scale = " << ENUM_TO_STRING(RangeT, Scale, xRanges.at(i).range.scale()));
	if (suppress)
		return;

	PERFTRACE(QLatin1String(Q_FUNC_INFO));
	prepareGeometryChange();
	setPos(rect.x() + rect.width() / 2, rect.y() + rect.height() / 2);

	updateDataRect();

	// plotArea position is always (0, 0) in parent's coordinates, don't need to update here
	q->plotArea()->setRect(rect);

	WorksheetElementContainerPrivate::recalcShapeAndBoundingRect();

	retransformScales(-1, -1);

	q->WorksheetElementContainer::retransform();
}

/*!
 * \brief CartesianPlotPrivate::retransformScale
 * Sets new Scales to coordinate systems and updates the ranges of the axis
 * Needed when the range (xRange/yRange) changed
 * \param index
 */
void CartesianPlotPrivate::retransformScale(const Dimension dim, int index, bool suppressSignals) {
	if (index == -1) {
		for (int i = 0; i < rangeCount(dim); i++)
			retransformScale(dim, i);
		return;
	}

	static const int breakGap = 20;
	Range<double> plotSceneRange;
	switch (dim) {
	case Dimension::X:
		plotSceneRange = {dataRect.x(), dataRect.x() + dataRect.width()};
		break;
	case Dimension::Y:
		plotSceneRange = {dataRect.y() + dataRect.height(), dataRect.y()};
		break;
	};
	Range<double> sceneRange, logicalRange;
	bool scaleChanged = false;
	for (const auto cSystem : coordinateSystems()) {
		const auto cs = static_cast<CartesianCoordinateSystem*>(cSystem);
		if (cs->index(dim) != index)
			continue;

		const auto r = range(dim, index);
		DEBUG(Q_FUNC_INFO << ", " << CartesianCoordinateSystem::dimensionToString(dim).toStdString() << "range = " << r.toStdString()
						  << ", auto scale = " << r.autoScale())

		QVector<CartesianScale*> scales;

		// check whether we have x/y-range breaks - the first break, if available, should be valid
		bool hasValidBreak = (rangeBreakingEnabled(dim) && !rangeBreaks(dim).list.isEmpty() && rangeBreaks(dim).list.first().isValid());
		if (!hasValidBreak) { // no breaks available -> range goes from the start to the end of the plot
			sceneRange = plotSceneRange;
			logicalRange = r;

			if (sceneRange.length() > 0)
				scales << this->createScale(r.scale(), sceneRange, logicalRange);
		} else {
			double sceneEndLast = plotSceneRange.start();
			double logicalEndLast = r.start();
			auto rbs = rangeBreaks(dim);
			for (const auto& rb : qAsConst(rbs.list)) {
				if (!rb.isValid())
					break;

				// current range goes from the end of the previous one (or from the plot beginning) to curBreak.start
				sceneRange.start() = sceneEndLast;
				if (&rb == &rangeBreaks(dim).list.first())
					sceneRange.start() -= breakGap;
				sceneRange.end() = plotSceneRange.start() + plotSceneRange.size() * rb.position;
				logicalRange = Range<double>(logicalEndLast, rb.range.start());

				if (sceneRange.length() > 0)
					scales << this->createScale(r.scale(), sceneRange, logicalRange);

				sceneEndLast = sceneRange.end();
				logicalEndLast = rb.range.end();
			}

			// add the remaining range going from the last available range break to the end of the plot (=end of the y-data range)
			sceneRange.setRange(sceneEndLast - breakGap, plotSceneRange.end());
			logicalRange.setRange(logicalEndLast, r.end());

			if (sceneRange.length() > 0)
				scales << this->createScale(r.scale(), sceneRange, logicalRange);
		}
		cs->setScales(dim, scales);
		scaleChanged = true;
	}

	if (scaleChanged)
		emit q->scaleRetransformed(q, dim, index);

	// Set ranges in the axis
	for (int i = 0; i < q->rangeCount(dim); i++) {
		auto& rangep = ranges(dim)[i];
		const double deltaMin = rangep.range.start() - rangep.prev.start();
		const double deltaMax = rangep.range.end() - rangep.prev.end();

		switch (dim) {
		case Dimension::X: {
			if (!qFuzzyIsNull(deltaMin) && !suppressSignals)
				Q_EMIT q->xMinChanged(i, rangep.range.start());
			if (!qFuzzyIsNull(deltaMax) && !suppressSignals)
				Q_EMIT q->xMaxChanged(i, rangep.range.end());
			break;
		}
		case Dimension::Y: {
			if (!qFuzzyIsNull(deltaMin) && !suppressSignals)
				Q_EMIT q->yMinChanged(i, rangep.range.start());
			if (!qFuzzyIsNull(deltaMax) && !suppressSignals)
				Q_EMIT q->yMaxChanged(i, rangep.range.end());
			break;
		}
		}

		rangep.prev = rangep.range;

		for (auto* axis : q->children<Axis>()) {
			DEBUG(Q_FUNC_INFO << ", auto-scale axis \"" << STDSTRING(axis->name()) << "\"")
			// use ranges of axis
			int axisIndex = q->coordinateSystem(axis->coordinateSystemIndex())->index(dim);
			if (axis->rangeType() != Axis::RangeType::Auto || axisIndex != i)
				continue;
			if ((dim == Dimension::Y && axis->orientation() != Axis::Orientation::Vertical)
				|| (dim == Dimension::X && axis->orientation() != Axis::Orientation::Horizontal))
				continue;

			if (!qFuzzyIsNull(deltaMax)) {
				axis->setUndoAware(false);
				axis->setSuppressRetransform(true);
				axis->setEnd(rangep.range.end());
				axis->setUndoAware(true);
				axis->setSuppressRetransform(false);
			}
			if (!qFuzzyIsNull(deltaMin)) {
				axis->setUndoAware(false);
				axis->setSuppressRetransform(true);
				axis->setStart(rangep.range.start());
				axis->setUndoAware(true);
				axis->setSuppressRetransform(false);
			}
			// TODO;
			// 			if (axis->position() == Axis::Position::Centered && deltaYMin != 0) {
			// 				axis->setOffset(axis->offset() + deltaYMin, false);
			// 			}
		}
	}
}

/*
 * calculate x and y scales from scence range and logical range (x/y range) for all coordinate systems
 */
void CartesianPlotPrivate::retransformScales(int xIndex, int yIndex) {
	DEBUG(Q_FUNC_INFO << ", SCALES x/y index = " << xIndex << "/" << yIndex)
	for (int i = 0; i < xRanges.count(); i++)
		DEBUG(Q_FUNC_INFO << ", x range " << i + 1 << " : " << xRanges.at(i).range.toStdString() << ", scale = "
						  << ENUM_TO_STRING(RangeT, Scale, xRanges.at(i).range.scale()) << ", auto scale = " << xRanges.at(i).range.autoScale());
	for (int i = 0; i < yRanges.count(); i++)
		DEBUG(Q_FUNC_INFO << ", y range " << i + 1 << " : " << yRanges.at(i).range.toStdString() << ", scale = "
						  << ENUM_TO_STRING(RangeT, Scale, yRanges.at(i).range.scale()) << ", auto scale = " << yRanges.at(i).range.autoScale());

	PERFTRACE(QLatin1String(Q_FUNC_INFO));

	retransformScale(Dimension::X, xIndex);
	retransformScale(Dimension::Y, yIndex);
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
	for (const auto* cSystem : q->m_coordinateSystems) {
		const auto cs = static_cast<const CartesianCoordinateSystem*>(cSystem);
		int xIndex = cs->index(Dimension::X);
		int yIndex = cs->index(Dimension::Y);
		xRanges[xIndex].dirty = true;
		yRanges[yIndex].dirty = true;
		if (autoScale(Dimension::X, xIndex) && autoScale(Dimension::Y, yIndex))
			q->scaleAuto(xIndex, yIndex);
		else if (autoScale(Dimension::X, xIndex))
			q->scaleAuto(Dimension::X, xIndex, false);
		else if (autoScale(Dimension::Y, yIndex))
			q->scaleAuto(Dimension::Y, yIndex, false);
	}
	q->WorksheetElementContainer::retransform();
}

void CartesianPlotPrivate::niceExtendChanged() {
	DEBUG(Q_FUNC_INFO)
	for (const auto* cSystem : q->m_coordinateSystems) {
		const auto cs = static_cast<const CartesianCoordinateSystem*>(cSystem);
		int xIndex = cs->index(Dimension::X);
		int yIndex = cs->index(Dimension::Y);
		xRanges[xIndex].dirty = true;
		yRanges[yIndex].dirty = true;
		if (autoScale(Dimension::X, xIndex) && autoScale(Dimension::Y, yIndex))
			q->scaleAuto(xIndex, yIndex);
		else if (autoScale(Dimension::X, xIndex))
			q->scaleAuto(Dimension::X, xIndex, false);
		else if (autoScale(Dimension::Y, yIndex))
			q->scaleAuto(Dimension::Y, yIndex, false);
	}
	q->WorksheetElementContainer::retransform();
}

void CartesianPlotPrivate::rangeFormatChanged(const Dimension dim) {
	DEBUG(Q_FUNC_INFO)
	switch (dim) {
	case Dimension::X: {
		for (auto* axis : q->children<Axis>()) {
			// TODO: only if x range of axis's plot range is changed
			if (axis->orientation() == Axis::Orientation::Horizontal)
				axis->retransformTickLabelStrings();
		}
		break;
	}
	case Dimension::Y: {
		for (auto* axis : q->children<Axis>()) {
			// TODO: only if x range of axis's plot range is changed
			if (axis->orientation() == Axis::Orientation::Horizontal)
				axis->retransformTickLabelStrings();
		}
		break;
	}
	}
}

CartesianPlot::RangeBreaks CartesianPlotPrivate::rangeBreaks(const Dimension dim) {
	switch (dim) {
	case Dimension::X:
		return xRangeBreaks;
	case Dimension::Y:
		return yRangeBreaks;
	}
	return CartesianPlot::RangeBreaks();
}

bool CartesianPlotPrivate::rangeBreakingEnabled(const Dimension dim) {
	switch (dim) {
	case Dimension::X:
		return xRangeBreakingEnabled;
	case Dimension::Y:
		return yRangeBreakingEnabled;
	}
	return false;
}

/*!
 * helper function for checkXRange() and checkYRange()
 */
Range<double> CartesianPlotPrivate::checkRange(const Range<double>& range) {
	double start = range.start(), end = range.end();
	const auto scale = range.scale();
	if (scale == RangeT::Scale::Linear || (start > 0 && end > 0)) // nothing to do
		return range;
	if (start >= 0 && end >= 0 && scale == RangeT::Scale::Sqrt) // nothing to do
		return range;
	// TODO: check if start == end?

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

	return {start, end};
}

/*!
 * check for negative values in the range when non-linear scalings are used
 */
void CartesianPlotPrivate::checkRange(Dimension dim, int index) {
	const auto range = ranges(dim).at(index).range;
	DEBUG(Q_FUNC_INFO << ", " << CartesianCoordinateSystem::dimensionToString(dim).toStdString() << " range " << index + 1 << " : " << range.toStdString()
					  << ", scale = " << ENUM_TO_STRING(RangeT, Scale, range.scale()))

	const auto newRange = checkRange(range);

	const double start = newRange.start(), end = newRange.end();
	if (start != range.start()) {
		DEBUG(Q_FUNC_INFO << ", old/new start = " << range.start() << "/" << start)
		q->setMin(dim, index, start);
	}
	if (end != range.end()) {
		DEBUG(Q_FUNC_INFO << ", old/new end = " << range.end() << "/" << end)
		q->setMax(dim, index, end);
	}
}

CartesianScale* CartesianPlotPrivate::createScale(RangeT::Scale scale, const Range<double>& sceneRange, const Range<double>& logicalRange) {
	DEBUG(Q_FUNC_INFO << ", scene range : " << sceneRange.toStdString() << ", logical range : " << logicalRange.toStdString());

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
QVariant CartesianPlotPrivate::itemChange(GraphicsItemChange change, const QVariant& value) {
	if (change == QGraphicsItem::ItemPositionChange) {
		const QPointF& itemPos = value.toPointF(); // item's center point in parent's coordinates;
		const qreal x = itemPos.x();
		const qreal y = itemPos.y();

		// calculate the new rect and forward the changes to the frontend
		QRectF newRect;
		const qreal w = rect.width();
		const qreal h = rect.height();
		newRect.setX(x - w / 2);
		newRect.setY(y - h / 2);
		newRect.setWidth(w);
		newRect.setHeight(h);
		Q_EMIT q->rectChanged(newRect);
	}
	return QGraphicsItem::itemChange(change, value);
}

//##############################################################################
//##################################  Events  ##################################
//##############################################################################

void CartesianPlotPrivate::contextMenuEvent(QGraphicsSceneContextMenuEvent* event) {
	const auto* cSystem{defaultCoordinateSystem()};
	scenePos = event->pos();
	logicalPos = cSystem->mapSceneToLogical(scenePos, AbstractCoordinateSystem::MappingFlag::Limit);
	calledFromContextMenu = true;
	auto* menu = q->createContextMenu();
	emit q->contextMenuRequested(q->AbstractAspect::type(), menu);
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
	const auto* cSystem{defaultCoordinateSystem()};
	auto* w = static_cast<Worksheet*>(q->parent(AspectType::Worksheet))->currentSelection();
	int index = Worksheet::cSystemIndex(w);
	if (index >= 0)
		cSystem = static_cast<CartesianCoordinateSystem*>(q->m_coordinateSystems.at(index));
	if (mouseMode == CartesianPlot::MouseMode::Selection) {
		if (!locked && dataRect.contains(event->pos())) {
			panningStarted = true;
			m_panningStart = event->pos();
			setCursor(Qt::ClosedHandCursor);
		}
	} else if (mouseMode == CartesianPlot::MouseMode::ZoomSelection || mouseMode == CartesianPlot::MouseMode::ZoomXSelection
			   || mouseMode == CartesianPlot::MouseMode::ZoomYSelection) {
		const QPointF logicalPos = cSystem->mapSceneToLogical(event->pos(), AbstractCoordinateSystem::MappingFlag::Limit);
		Q_EMIT q->mousePressZoomSelectionModeSignal(logicalPos);
		return;
	} else if (mouseMode == CartesianPlot::MouseMode::Cursor) {
		setCursor(Qt::SizeHorCursor);
		const QPointF logicalPos = cSystem->mapSceneToLogical(event->pos(), AbstractCoordinateSystem::MappingFlag::Limit);
		double cursorPenWidth2 = cursorLine->pen().width() / 2.;
		if (cursorPenWidth2 < 10.)
			cursorPenWidth2 = 10.;

		bool visible;
		if (cursor0Enable
			&& qAbs(event->pos().x() - cSystem->mapLogicalToScene(QPointF(cursor0Pos.x(), range(Dimension::Y).start()), visible).x()) < cursorPenWidth2) {
			selectedCursor = 0;
		} else if (cursor1Enable
				   && qAbs(event->pos().x() - cSystem->mapLogicalToScene(QPointF(cursor1Pos.x(), range(Dimension::Y).start()), visible).x())
					   < cursorPenWidth2) {
			selectedCursor = 1;
		} else if (QApplication::keyboardModifiers() & Qt::ControlModifier) {
			cursor1Enable = true;
			selectedCursor = 1;
			Q_EMIT q->cursor1EnableChanged(cursor1Enable);
		} else {
			cursor0Enable = true;
			selectedCursor = 0;
			Q_EMIT q->cursor0EnableChanged(cursor0Enable);
		}
		Q_EMIT q->mousePressCursorModeSignal(selectedCursor, logicalPos);
	}

	QGraphicsItem::mousePressEvent(event);
}

void CartesianPlotPrivate::mousePressZoomSelectionMode(QPointF logicalPos, int cSystemIndex) {
	// DEBUG(Q_FUNC_INFO << ", csystem index = " << cSystemIndex)
	const CartesianCoordinateSystem* cSystem;
	if (cSystemIndex == -1 || cSystemIndex >= q->m_coordinateSystems.count())
		cSystem = defaultCoordinateSystem();
	else
		cSystem = static_cast<CartesianCoordinateSystem*>(q->m_coordinateSystems.at(cSystemIndex));

	int xIndex = cSystem->index(Dimension::X);
	int yIndex = cSystem->index(Dimension::Y);

	bool visible;
	const QPointF scenePos = cSystem->mapLogicalToScene(logicalPos, visible, AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);
	if (mouseMode == CartesianPlot::MouseMode::ZoomSelection) {
		if (logicalPos.x() < range(Dimension::X, xIndex).start())
			logicalPos.setX(range(Dimension::X, xIndex).start());

		if (logicalPos.x() > range(Dimension::X, xIndex).end())
			logicalPos.setX(range(Dimension::X, xIndex).end());

		if (logicalPos.y() < range(Dimension::Y, yIndex).start())
			logicalPos.setY(range(Dimension::Y, yIndex).start());

		if (logicalPos.y() > range(Dimension::Y, yIndex).end())
			logicalPos.setY(range(Dimension::Y, yIndex).end());

		m_selectionStart = scenePos;
	} else if (mouseMode == CartesianPlot::MouseMode::ZoomXSelection) {
		logicalPos.setY(range(Dimension::Y, yIndex).start()); // must be done, because the other plots can have other ranges, value must be in the scenes
		m_selectionStart.setX(scenePos.x());
		m_selectionStart.setY(dataRect.y());
	} else if (mouseMode == CartesianPlot::MouseMode::ZoomYSelection) {
		logicalPos.setX(range(Dimension::X, xIndex).start()); // must be done, because the other plots can have other ranges, value must be in the scenes
		m_selectionStart.setX(dataRect.x());
		m_selectionStart.setY(scenePos.y());
	}
	m_selectionEnd = m_selectionStart;
	m_selectionBandIsShown = true;
}

void CartesianPlotPrivate::mousePressCursorMode(int cursorNumber, QPointF logicalPos) {
	cursorNumber == 0 ? cursor0Enable = true : cursor1Enable = true;

	QPointF p1(logicalPos.x(), range(Dimension::Y).start());
	QPointF p2(logicalPos.x(), range(Dimension::Y).end());

	if (cursorNumber == 0)
		cursor0Pos = QPointF(logicalPos.x(), 0);
	else
		cursor1Pos = QPointF(logicalPos.x(), 0);

	update();
}

void CartesianPlotPrivate::updateCursor() {
	update();
}

void CartesianPlotPrivate::setZoomSelectionBandShow(bool show) {
	m_selectionBandIsShown = show;
}

void CartesianPlotPrivate::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
	const auto* cSystem{defaultCoordinateSystem()};
	auto* w = static_cast<Worksheet*>(q->parent(AspectType::Worksheet))->currentSelection();
	int index = Worksheet::cSystemIndex(w);
	if (index >= 0)
		cSystem = static_cast<CartesianCoordinateSystem*>(q->m_coordinateSystems.at(index));

	if (mouseMode == CartesianPlot::MouseMode::Selection) {
		if (panningStarted && dataRect.contains(event->pos())) {
			// don't retransform on small mouse movement deltas
			const int deltaXScene = (m_panningStart.x() - event->pos().x());
			const int deltaYScene = (m_panningStart.y() - event->pos().y());
			if (qAbs(deltaXScene) < 5 && qAbs(deltaYScene) < 5)
				return;

			const QPointF logicalEnd = cSystem->mapSceneToLogical(event->pos());
			const QPointF logicalStart = cSystem->mapSceneToLogical(m_panningStart);
			m_panningStart = event->pos();
			Q_EMIT q->mouseMoveSelectionModeSignal(logicalStart, logicalEnd);
		} else
			QGraphicsItem::mouseMoveEvent(event);
	} else if (mouseMode == CartesianPlot::MouseMode::ZoomSelection || mouseMode == CartesianPlot::MouseMode::ZoomXSelection
			   || mouseMode == CartesianPlot::MouseMode::ZoomYSelection) {
		QGraphicsItem::mouseMoveEvent(event);
		if (!boundingRect().contains(event->pos())) {
			q->info(QString());
			return;
		}
		const QPointF logicalPos = cSystem->mapSceneToLogical(event->pos(), AbstractCoordinateSystem::MappingFlag::Limit);
		Q_EMIT q->mouseMoveZoomSelectionModeSignal(logicalPos);

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
		Q_EMIT q->mouseMoveCursorModeSignal(selectedCursor, logicalPos);
	}
}

bool CartesianPlotPrivate::translateRange(int xIndex, int yIndex, const QPointF& logicalStart, const QPointF& logicalEnd, bool translateX, bool translateY) {
	// handle the change in x
	bool translationX = false, translationY = false;
	if (translateX && logicalStart.x() - logicalEnd.x() != 0) { // TODO: find better method
		translationX = true;
		double start{logicalStart.x()}, end{logicalEnd.x()};
		switch (range(Dimension::X, xIndex).scale()) {
		case RangeT::Scale::Linear: {
			const double delta = (start - end);
			range(Dimension::X, xIndex).translate(delta);
			break;
		}
		case RangeT::Scale::Log10: {
			if (end == 0 || start / end <= 0)
				break;
			const double delta = log10(start / end);
			range(Dimension::X, xIndex) *= pow(10, delta);
			break;
		}
		case RangeT::Scale::Log2: {
			if (end == 0 || start / end <= 0)
				break;
			const double delta = log2(start / end);
			range(Dimension::X, xIndex) *= exp2(delta);
			break;
		}
		case RangeT::Scale::Ln: {
			if (end == 0 || start / end <= 0)
				break;
			const double delta = log(start / end);
			range(Dimension::X, xIndex) *= exp(delta);
			break;
		}
		case RangeT::Scale::Sqrt: {
			if (start < 0 || end < 0)
				break;
			const double delta = sqrt(start) - sqrt(end);
			range(Dimension::X, xIndex).translate(delta * delta);
			break;
		}
		case RangeT::Scale::Square: {
			if (end <= start)
				break;
			const double delta = end * end - start * start;
			range(Dimension::X, xIndex).translate(sqrt(delta));
			break;
		}
		case RangeT::Scale::Inverse: {
			if (start == 0. || end == 0. || end <= start)
				break;
			const double delta = 1. / start - 1. / end;
			range(Dimension::X, xIndex).translate(1. / delta);
			break;
		}
		}
	}

	if (translateY && logicalStart.y() - logicalEnd.y() != 0) {
		translationY = true;
		// handle the change in y
		double start = logicalStart.y();
		double end = logicalEnd.y();
		switch (range(Dimension::Y, yIndex).scale()) {
		case RangeT::Scale::Linear: {
			const double deltaY = (start - end);
			range(Dimension::Y, yIndex).translate(deltaY);
			break;
		}
		case RangeT::Scale::Log10: {
			if (end == 0 || start / end <= 0)
				break;
			const double deltaY = log10(start / end);
			range(Dimension::Y, yIndex) *= pow(10, deltaY);
			break;
		}
		case RangeT::Scale::Log2: {
			if (end == 0 || start / end <= 0)
				break;
			const double deltaY = log2(start / end);
			range(Dimension::Y, yIndex) *= exp2(deltaY);
			break;
		}
		case RangeT::Scale::Ln: {
			if (end == 0 || start / end <= 0)
				break;
			const double deltaY = log(start / end);
			range(Dimension::Y, yIndex) *= exp(deltaY);
			break;
		}
		case RangeT::Scale::Sqrt: {
			if (start < 0 || end < 0)
				break;
			const double delta = sqrt(start) - sqrt(end);
			range(Dimension::Y, yIndex).translate(delta * delta);
			break;
		}
		case RangeT::Scale::Square: {
			if (end <= start)
				break;
			const double delta = end * end - start * start;
			range(Dimension::Y, yIndex).translate(sqrt(delta));
			break;
		}
		case RangeT::Scale::Inverse: {
			if (start == 0. || end == 0. || end <= start)
				break;
			const double delta = 1. / start - 1. / end;
			range(Dimension::Y, yIndex).translate(1. / delta);
			break;
		}
		}
	}

	q->setUndoAware(false);
	if (translationX) {
		q->enableAutoScale(Dimension::X, xIndex, false);
		retransformScale(Dimension::X, xIndex);
	}
	if (translationY) {
		q->enableAutoScale(Dimension::Y, yIndex, false);
		retransformScale(Dimension::Y, yIndex);
	}
	q->setUndoAware(true);

	// If x or y should not be translated, means, that it was done before
	// so the ranges must get dirty.
	if (translationX || translationY || !translateX || !translateY) {
		q->setRangeDirty(Dimension::X, xIndex, true);
		q->setRangeDirty(Dimension::Y, yIndex, true);
	}

	return translationX || translationY || !translateX || !translateY;
}

void CartesianPlotPrivate::mouseMoveSelectionMode(QPointF logicalStart, QPointF logicalEnd) {
	const bool autoscaleRanges = true; // consumes a lot of power, maybe making an option to turn off/on!
	auto* w = static_cast<Worksheet*>(q->parent(AspectType::Worksheet))->currentSelection();
	int index = Worksheet::cSystemIndex(w);
	if (!w || w->parent(AspectType::CartesianPlot) != q)
		index = -1;

	bool translated = false;
	if (index < 0) {
		QVector<int> translatedIndicesX, translatedIndicesY;
		for (int i = 0; i < q->m_coordinateSystems.count(); i++) {
			auto cs = coordinateSystem(i);
			int xIndex = cs->index(Dimension::X);
			int yIndex = cs->index(Dimension::Y);
			bool translateX = !translatedIndicesX.contains(xIndex);
			bool translateY = !translatedIndicesY.contains(yIndex);
			if (translateRange(xIndex, yIndex, logicalStart, logicalEnd, translateX, translateY)) {
				translated = true;
				if (autoscaleRanges && logicalStart.y() == logicalEnd.y() && autoScale(Dimension::Y, cs->index(Dimension::Y))) {
					// only x was changed, so autoscale y
					q->scaleAuto(Dimension::Y, cs->index(Dimension::Y), false);
				}
				if (autoscaleRanges && logicalStart.x() == logicalEnd.x() && autoScale(Dimension::X, cs->index(Dimension::X))) {
					// only y was changed, so autoscale x
					q->scaleAuto(Dimension::X, cs->index(Dimension::X), false);
				}
			}
			if (translateX)
				translatedIndicesX.append(static_cast<CartesianCoordinateSystem*>(q->m_coordinateSystems[i])->index(Dimension::X));
			if (translateY)
				translatedIndicesY.append(static_cast<CartesianCoordinateSystem*>(q->m_coordinateSystems[i])->index(Dimension::Y));
		}
	} else {
		auto cs = coordinateSystem(index);
		int xIndex = cs->index(Dimension::X);
		int yIndex = cs->index(Dimension::Y);
		translated = translateRange(xIndex, yIndex, logicalStart, logicalEnd, true, true);
		if (autoscaleRanges && logicalStart.y() == logicalEnd.y() && autoScale(Dimension::Y, yIndex)) {
			// only x was changed, so autoscale y
			q->scaleAuto(Dimension::Y, yIndex, false);
		}
		if (autoscaleRanges && logicalStart.x() == logicalEnd.x() && autoScale(Dimension::X, xIndex)) {
			// only y was changed, so autoscale x
			q->scaleAuto(Dimension::X, xIndex, false);
		}
	}

	if (translated)
		q->WorksheetElementContainer::retransform();
}

void CartesianPlotPrivate::mouseMoveZoomSelectionMode(QPointF logicalPos, int cSystemIndex) {
	QString info;
	const CartesianCoordinateSystem* cSystem;
	if (cSystemIndex == -1 || cSystemIndex >= q->m_coordinateSystems.count())
		cSystem = defaultCoordinateSystem();
	else
		cSystem = q->coordinateSystem(cSystemIndex);

	int xIndex = cSystem->index(Dimension::X);
	int yIndex = cSystem->index(Dimension::Y);

	const auto xRangeFormat{range(Dimension::X, xIndex).format()};
	const auto yRangeFormat{range(Dimension::Y, yIndex).format()};
	const auto xRangeDateTimeFormat{range(Dimension::X, xIndex).dateTimeFormat()};
	const QPointF logicalStart = cSystem->mapSceneToLogical(m_selectionStart, AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);

	if (mouseMode == CartesianPlot::MouseMode::ZoomSelection) {
		bool visible;
		m_selectionEnd = cSystem->mapLogicalToScene(logicalPos, visible, AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);
		QPointF logicalEnd = logicalPos;
		if (xRangeFormat == RangeT::Format::Numeric)
			info = QString::fromUtf8("Δx=") + QString::number(logicalEnd.x() - logicalStart.x());
		else
			info = i18n("from x=%1 to x=%2",
						QDateTime::fromMSecsSinceEpoch(logicalStart.x(), Qt::UTC).toString(xRangeDateTimeFormat),
						QDateTime::fromMSecsSinceEpoch(logicalEnd.x(), Qt::UTC).toString(xRangeDateTimeFormat));

		info += QLatin1String(", ");
		if (yRangeFormat == RangeT::Format::Numeric)
			info += QString::fromUtf8("Δy=") + QString::number(logicalEnd.y() - logicalStart.y());
		else
			info += i18n("from y=%1 to y=%2",
						 QDateTime::fromMSecsSinceEpoch(logicalStart.y(), Qt::UTC).toString(xRangeDateTimeFormat),
						 QDateTime::fromMSecsSinceEpoch(logicalEnd.y(), Qt::UTC).toString(xRangeDateTimeFormat));
	} else if (mouseMode == CartesianPlot::MouseMode::ZoomXSelection) {
		logicalPos.setY(range(Dimension::Y, yIndex).start()); // must be done, because the other plots can have other ranges, value must be in the scenes
		bool visible;
		m_selectionEnd.setX(
			cSystem->mapLogicalToScene(logicalPos, visible, CartesianCoordinateSystem::MappingFlag::SuppressPageClipping).x()); // event->pos().x());
		m_selectionEnd.setY(dataRect.bottom());
		QPointF logicalEnd = logicalPos;
		if (xRangeFormat == RangeT::Format::Numeric)
			info = QString::fromUtf8("Δx=") + QString::number(logicalEnd.x() - logicalStart.x());
		else
			info = i18n("from x=%1 to x=%2",
						QDateTime::fromMSecsSinceEpoch(logicalStart.x(), Qt::UTC).toString(xRangeDateTimeFormat),
						QDateTime::fromMSecsSinceEpoch(logicalEnd.x(), Qt::UTC).toString(xRangeDateTimeFormat));
	} else if (mouseMode == CartesianPlot::MouseMode::ZoomYSelection) {
		m_selectionEnd.setX(dataRect.right());
		logicalPos.setX(range(Dimension::X, xIndex).start()); // must be done, because the other plots can have other ranges, value must be in the scenes
		bool visible;
		m_selectionEnd.setY(
			cSystem->mapLogicalToScene(logicalPos, visible, CartesianCoordinateSystem::MappingFlag::SuppressPageClipping).y()); // event->pos().y());
		QPointF logicalEnd = logicalPos;
		if (yRangeFormat == RangeT::Format::Numeric)
			info = QString::fromUtf8("Δy=") + QString::number(logicalEnd.y() - logicalStart.y());
		else
			info = i18n("from y=%1 to y=%2",
						QDateTime::fromMSecsSinceEpoch(logicalStart.y(), Qt::UTC).toString(xRangeDateTimeFormat),
						QDateTime::fromMSecsSinceEpoch(logicalEnd.y(), Qt::UTC).toString(xRangeDateTimeFormat));
	}
	q->info(info);
	update();
}

void CartesianPlotPrivate::mouseMoveCursorMode(int cursorNumber, QPointF logicalPos) {
	const auto xRangeFormat{range(Dimension::X).format()};
	const auto xRangeDateTimeFormat{range(Dimension::X).dateTimeFormat()};

	QPointF p1(logicalPos.x(), 0);
	cursorNumber == 0 ? cursor0Pos = p1 : cursor1Pos = p1;

	QString info;
	if (xRangeFormat == RangeT::Format::Numeric)
		info = QString::fromUtf8("x=") + QString::number(logicalPos.x());
	else
		info = i18n("x=%1", QDateTime::fromMSecsSinceEpoch(logicalPos.x(), Qt::UTC).toString(xRangeDateTimeFormat));
	q->info(info);
	update();
}

void CartesianPlotPrivate::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
	if (mouseMode == CartesianPlot::MouseMode::Selection) {
		setCursor(Qt::ArrowCursor);
		panningStarted = false;

		// TODO: why do we do this all the time?!?!
		const QPointF& itemPos = pos(); // item's center point in parent's coordinates;
		const qreal x = itemPos.x();
		const qreal y = itemPos.y();

		// calculate the new rect and set it
		QRectF newRect;
		const qreal w = rect.width();
		const qreal h = rect.height();
		newRect.setX(x - w / 2);
		newRect.setY(y - h / 2);
		newRect.setWidth(w);
		newRect.setHeight(h);

		// TODO: autoscale

		suppressRetransform = true;
		q->setRect(newRect);
		suppressRetransform = false;

		QGraphicsItem::mouseReleaseEvent(event);
	} else if (mouseMode == CartesianPlot::MouseMode::ZoomSelection || mouseMode == CartesianPlot::MouseMode::ZoomXSelection
			   || mouseMode == CartesianPlot::MouseMode::ZoomYSelection) {
		Q_EMIT q->mouseReleaseZoomSelectionModeSignal();
	}
}

void CartesianPlotPrivate::mouseReleaseZoomSelectionMode(int cSystemIndex, bool suppressRetransform) {
	m_selectionBandIsShown = false;
	// don't zoom if very small region was selected, avoid occasional/unwanted zooming
	if (qAbs(m_selectionEnd.x() - m_selectionStart.x()) < 20 && qAbs(m_selectionEnd.y() - m_selectionStart.y()) < 20)
		return;

	int xIndex = -1, yIndex = -1;
	if (cSystemIndex == -1 || cSystemIndex >= q->m_coordinateSystems.count()) {
		for (int i = 0; i < q->m_coordinateSystems.count(); i++)
			mouseReleaseZoomSelectionMode(i, true);
	} else {
		auto cSystem = coordinateSystem(cSystemIndex);
		xIndex = cSystem->index(Dimension::X);
		yIndex = cSystem->index(Dimension::Y);

		// determine the new plot ranges
		QPointF logicalZoomStart = cSystem->mapSceneToLogical(m_selectionStart, AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);
		QPointF logicalZoomEnd = cSystem->mapSceneToLogical(m_selectionEnd, AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);
		if (m_selectionEnd.x() > m_selectionStart.x())
			range(Dimension::X, xIndex).setRange(logicalZoomStart.x(), logicalZoomEnd.x());
		else
			range(Dimension::X, xIndex).setRange(logicalZoomEnd.x(), logicalZoomStart.x());

		if (niceExtend)
			range(Dimension::X, xIndex).niceExtend();

		if (m_selectionEnd.y() > m_selectionStart.y())
			range(Dimension::Y, yIndex).setRange(logicalZoomEnd.y(), logicalZoomStart.y());
		else
			range(Dimension::Y, yIndex).setRange(logicalZoomStart.y(), logicalZoomEnd.y());

		if (niceExtend)
			range(Dimension::Y, yIndex).niceExtend();

		if (mouseMode == CartesianPlot::MouseMode::ZoomSelection) {
			q->setRangeDirty(Dimension::X, xIndex, true);
			q->setRangeDirty(Dimension::Y, yIndex, true);
			q->enableAutoScale(Dimension::X, xIndex, false);
			q->enableAutoScale(Dimension::Y, yIndex, false);
		} else if (mouseMode == CartesianPlot::MouseMode::ZoomXSelection) {
			q->setRangeDirty(Dimension::X, xIndex, true);
			q->setRangeDirty(Dimension::Y, yIndex, true);
			q->enableAutoScale(Dimension::X, xIndex, false);
			if (q->autoScale(Dimension::Y, yIndex))
				q->scaleAuto(Dimension::Y, yIndex, false, true);
		} else if (mouseMode == CartesianPlot::MouseMode::ZoomYSelection) {
			q->setRangeDirty(Dimension::X, xIndex, true);
			q->setRangeDirty(Dimension::Y, yIndex, true);
			q->enableAutoScale(Dimension::Y, yIndex, false);
			if (q->autoScale(Dimension::X, xIndex))
				q->scaleAuto(Dimension::X, xIndex, false, true);
		}
	}

	if (!suppressRetransform) {
		retransformScales(xIndex, yIndex);
		q->WorksheetElementContainer::retransform();
	}
}

void CartesianPlotPrivate::wheelEvent(QGraphicsSceneWheelEvent* event) {
	if (locked)
		return;

	auto* w = static_cast<Worksheet*>(q->parent(AspectType::Worksheet))->currentSelection();
	int cSystemIndex = Worksheet::cSystemIndex(w);
	int xIndex = -1, yIndex = -1;
	if (w && w->parent(AspectType::CartesianPlot) == q) {
		xIndex = coordinateSystem(cSystemIndex)->index(Dimension::X);
		yIndex = coordinateSystem(cSystemIndex)->index(Dimension::Y);
	}

	bool zoomX = false;
	bool zoomY = false;

	// If an axis was selected, only the corresponding range
	// should be changed
	if (w && w->type() == AspectType::Axis) {
		auto axis = static_cast<Axis*>(w);

		if (axis->orientation() == Axis::Orientation::Horizontal) {
			zoomX = true;
			xIndex = coordinateSystem(axis->coordinateSystemIndex())->index(Dimension::X);
		} else {
			zoomY = true;
			yIndex = coordinateSystem(axis->coordinateSystemIndex())->index(Dimension::Y);
		}
	}

	if (event->delta() > 0) {
		if (!zoomX && !zoomY) {
			q->zoomIn(xIndex, yIndex);
		} else {
			if (zoomX)
				q->zoomInX(xIndex);
			if (zoomY)
				q->zoomInY(yIndex);
		}
	} else {
		if (!zoomX && !zoomY) {
			q->zoomOut(xIndex, yIndex);
		} else {
			if (zoomX)
				q->zoomOutX(xIndex);
			if (zoomY)
				q->zoomOutY(yIndex);
		}
	}
}

void CartesianPlotPrivate::keyPressEvent(QKeyEvent* event) {
	auto key = event->key();
	if (key == Qt::Key_Escape) {
		m_selectionBandIsShown = false;
	} else if (key == Qt::Key_Left || key == Qt::Key_Right || key == Qt::Key_Up || key == Qt::Key_Down) {
		const auto* worksheet = static_cast<const Worksheet*>(q->parentAspect());
		if (worksheet->layout() == Worksheet::Layout::NoLayout) {
			// no layout is active -> use arrow keys to move the plot on the worksheet
			const int delta = 5;
			QRectF rect = q->rect();

			if (key == Qt::Key_Left) {
				rect.setX(rect.x() - delta);
				rect.setWidth(rect.width() - delta);
			} else if (key == Qt::Key_Right) {
				rect.setX(rect.x() + delta);
				rect.setWidth(rect.width() + delta);
			} else if (key == Qt::Key_Up) {
				rect.setY(rect.y() - delta);
				rect.setHeight(rect.height() - delta);
			} else if (key == Qt::Key_Down) {
				rect.setY(rect.y() + delta);
				rect.setHeight(rect.height() + delta);
			}

			q->setRect(rect);
		}
	} else if (key == Qt::Key_N) // (key == Qt::Key_Tab)
		navigateNextPrevCurve();
	else if (key == Qt::Key_P) // (key == Qt::SHIFT + Qt::Key_Tab)
		navigateNextPrevCurve(false /*next*/);

	QGraphicsItem::keyPressEvent(event);
}

void CartesianPlotPrivate::navigateNextPrevCurve(bool next) const {
	const auto& curves = q->children<XYCurve>();
	if (curves.isEmpty())
		return;

	// determine the current selected curve
	const XYCurve* selectedCurve = nullptr;
	int index = 0;
	for (const auto* curve : curves) {
		if (curve->graphicsItem()->isSelected()) {
			selectedCurve = curve;
			break;
		}
		++index;
	}

	int newIndex = 0;
	if (selectedCurve) {
		if (next) { // havigate to the next curve
			if (index < curves.size() - 1)
				newIndex = index + 1;
		} else { // navigate to the previous curve
			if (index > 0)
				newIndex = index - 1;
			else
				newIndex = curves.size() - 1;
		}
	}

	auto* w = static_cast<Worksheet*>(q->parent(AspectType::Worksheet));

	// deselect the current curve
	if (selectedCurve)
		w->setItemSelectedInView(selectedCurve->graphicsItem(), false);
	else {
		// no curve is selected, either the plot itself or some
		// other children like axis, etc. are selected.
		// deselect all of them
		w->setItemSelectedInView(this, false); // deselect the plot

		// deselect children
		const auto& elements = q->children<WorksheetElement>(AbstractAspect::ChildIndexFlag::IncludeHidden);
		for (auto* element : elements)
			w->setItemSelectedInView(element->graphicsItem(), false);
	}

	// select the new curve
	w->setItemSelectedInView(curves.at(newIndex)->graphicsItem(), true);
}

void CartesianPlotPrivate::hoverMoveEvent(QGraphicsSceneHoverEvent* event) {
	QPointF point = event->pos();
	QString info;
	const auto* cSystem{defaultCoordinateSystem()};
	auto* w = static_cast<Worksheet*>(q->parent(AspectType::Worksheet))->currentSelection();
	int index = Worksheet::cSystemIndex(w);
	int xIndex = cSystem->index(Dimension::X), yIndex = cSystem->index(Dimension::Y);
	if (!w || w->parent(AspectType::CartesianPlot) != q) {
		xIndex = -1;
		yIndex = -1;
	} else if (index >= 0) {
		cSystem = static_cast<CartesianCoordinateSystem*>(q->m_coordinateSystems.at(index));
		xIndex = cSystem->index(Dimension::X);
		yIndex = cSystem->index(Dimension::Y);
	}

	const auto xRangeFormat{range(Dimension::X, xIndex).format()};
	const auto yRangeFormat{range(Dimension::Y, yIndex).format()};
	const auto xRangeDateTimeFormat{range(Dimension::X, xIndex).dateTimeFormat()};
	const auto yRangeDateTimeFormat{range(Dimension::Y, yIndex).dateTimeFormat()};
	if (dataRect.contains(point)) {
		QPointF logicalPoint = cSystem->mapSceneToLogical(point);

		if ((mouseMode == CartesianPlot::MouseMode::ZoomSelection) || mouseMode == CartesianPlot::MouseMode::Selection
			|| mouseMode == CartesianPlot::MouseMode::Crosshair) {
			info = QStringLiteral("x=");
			if (xRangeFormat == RangeT::Format::Numeric)
				info += QString::number(logicalPoint.x());
			else
				info += QDateTime::fromMSecsSinceEpoch(logicalPoint.x(), Qt::UTC).toString(xRangeDateTimeFormat);

			info += QStringLiteral(", y=");
			if (yRangeFormat == RangeT::Format::Numeric)
				info += QString::number(logicalPoint.y());
			else
				info += QDateTime::fromMSecsSinceEpoch(logicalPoint.y(), Qt::UTC).toString(yRangeDateTimeFormat);
		}

		if (mouseMode == CartesianPlot::MouseMode::ZoomSelection && !m_selectionBandIsShown) {
			Q_EMIT q->mouseHoverZoomSelectionModeSignal(logicalPoint);
		} else if (mouseMode == CartesianPlot::MouseMode::ZoomXSelection && !m_selectionBandIsShown) {
			info = QStringLiteral("x=");
			if (xRangeFormat == RangeT::Format::Numeric)
				info += QString::number(logicalPoint.x());
			else
				info += QDateTime::fromMSecsSinceEpoch(logicalPoint.x(), Qt::UTC).toString(xRangeDateTimeFormat);
			Q_EMIT q->mouseHoverZoomSelectionModeSignal(logicalPoint);
		} else if (mouseMode == CartesianPlot::MouseMode::ZoomYSelection && !m_selectionBandIsShown) {
			info = QStringLiteral("y=");
			if (yRangeFormat == RangeT::Format::Numeric)
				info += QString::number(logicalPoint.y());
			else
				info += QDateTime::fromMSecsSinceEpoch(logicalPoint.y(), Qt::UTC).toString(yRangeDateTimeFormat);
			Q_EMIT q->mouseHoverZoomSelectionModeSignal(logicalPoint);
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
			info = QStringLiteral("x=");
			if (yRangeFormat == RangeT::Format::Numeric)
				info += QString::number(logicalPoint.x());
			else
				info += QDateTime::fromMSecsSinceEpoch(logicalPoint.x(), Qt::UTC).toString(xRangeDateTimeFormat);

			double cursorPenWidth2 = cursorLine->pen().width() / 2.;
			if (cursorPenWidth2 < 10.)
				cursorPenWidth2 = 10.;

			bool visible;
			if ((cursor0Enable
				 && qAbs(point.x() - defaultCoordinateSystem()->mapLogicalToScene(QPointF(cursor0Pos.x(), range(Dimension::Y).start()), visible).x())
					 < cursorPenWidth2)
				|| (cursor1Enable
					&& qAbs(point.x() - defaultCoordinateSystem()->mapLogicalToScene(QPointF(cursor1Pos.x(), range(Dimension::Y).start()), visible).x())
						< cursorPenWidth2))
				setCursor(Qt::SizeHorCursor);
			else
				setCursor(Qt::ArrowCursor);

			update();
		}
	} else
		Q_EMIT q->mouseHoverOutsideDataRectSignal();

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
		QPointF p1(logicPos.x(), range(Dimension::Y, cSystem->index(Dimension::Y)).start());
		QPointF p2(logicPos.x(), range(Dimension::Y, cSystem->index(Dimension::Y)).end());
		m_selectionStartLine.setP1(cSystem->mapLogicalToScene(p1, visible, CartesianCoordinateSystem::MappingFlag::Limit));
		m_selectionStartLine.setP2(cSystem->mapLogicalToScene(p2, visible, CartesianCoordinateSystem::MappingFlag::Limit));
	} else if (mouseMode == CartesianPlot::MouseMode::ZoomYSelection && !m_selectionBandIsShown) {
		QPointF p1(range(Dimension::X, cSystem->index(Dimension::X)).start(), logicPos.y());
		QPointF p2(range(Dimension::X, cSystem->index(Dimension::X)).end(), logicPos.y());
		m_selectionStartLine.setP1(cSystem->mapLogicalToScene(p1, visible, CartesianCoordinateSystem::MappingFlag::Limit));
		m_selectionStartLine.setP2(cSystem->mapLogicalToScene(p2, visible, CartesianCoordinateSystem::MappingFlag::Limit));
	}

	update(); // because if previous another selection mode was selected, the lines must be deleted
}

void CartesianPlotPrivate::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget*) {
	if (!isVisible() || m_printing)
		return;

	if ((mouseMode == CartesianPlot::MouseMode::ZoomXSelection || mouseMode == CartesianPlot::MouseMode::ZoomYSelection) && (!m_selectionBandIsShown)
		&& m_insideDataRect) {
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

		// horizontal line
		double x1 = dataRect.left();
		double y1 = m_crosshairPos.y();
		double x2 = dataRect.right();
		double y2 = y1;
		painter->drawLine(x1, y1, x2, y2);

		// vertical line
		x1 = m_crosshairPos.x();
		y1 = dataRect.bottom();
		x2 = x1;
		y2 = dataRect.top();
		painter->drawLine(x1, y1, x2, y2);
	}

	// draw cursor lines if available
	if (cursor0Enable || cursor1Enable) {
		painter->save();
		painter->setPen(cursorLine->pen());
		painter->setOpacity(cursorLine->opacity());
		QFont font = painter->font();
		font.setPointSize(font.pointSize() * 4);
		painter->setFont(font);

		bool visible;
		QPointF p1 = defaultCoordinateSystem()->mapLogicalToScene(QPointF(cursor0Pos.x(), range(Dimension::Y).start()), visible);
		if (cursor0Enable && visible) {
			QPointF p2 = defaultCoordinateSystem()->mapLogicalToScene(QPointF(cursor0Pos.x(), range(Dimension::Y).end()), visible);
			painter->drawLine(p1, p2);
			QPointF textPos = p2;
			textPos.setX(p2.x() - m_cursor0Text.size().width() / 2);
			textPos.setY(p2.y() - m_cursor0Text.size().height());
			if (textPos.y() < boundingRect().y())
				textPos.setY(boundingRect().y());
			painter->drawStaticText(textPos, m_cursor0Text);
		}

		p1 = defaultCoordinateSystem()->mapLogicalToScene(QPointF(cursor1Pos.x(), range(Dimension::Y).start()), visible);
		if (cursor1Enable && visible) {
			QPointF p2 = defaultCoordinateSystem()->mapLogicalToScene(QPointF(cursor1Pos.x(), range(Dimension::Y).end()), visible);
			painter->drawLine(p1, p2);
			QPointF textPos = p2;
			// TODO: Moving this stuff into other function to not calculate it every time
			textPos.setX(p2.x() - m_cursor1Text.size().width() / 2);
			textPos.setY(p2.y() - m_cursor1Text.size().height());
			if (textPos.y() < boundingRect().y())
				textPos.setY(boundingRect().y());
			painter->drawStaticText(textPos, m_cursor1Text);
		}

		painter->restore();
	}

	const bool selected = isSelected();
	const bool hovered = (m_hovered && !selected);
	if ((hovered || selected) && !m_printing) {
		static double penWidth = 2.; // why static?
		const QRectF& br = q->m_plotArea->graphicsItem()->boundingRect();
		const qreal width = br.width();
		const qreal height = br.height();
		const QRectF rect = QRectF(-width / 2 + penWidth / 2, -height / 2 + penWidth / 2, width - penWidth, height - penWidth);

		if (hovered)
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

	writer->writeStartElement(QStringLiteral("cartesianPlot"));
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	// applied theme
	if (!d->theme.isEmpty()) {
		writer->writeStartElement(QStringLiteral("theme"));
		writer->writeAttribute(QStringLiteral("name"), d->theme);
		writer->writeEndElement();
	}

	// cursor
	d->cursorLine->save(writer);

	// geometry
	writer->writeStartElement(QStringLiteral("geometry"));
	writer->writeAttribute(QStringLiteral("x"), QString::number(d->rect.x()));
	writer->writeAttribute(QStringLiteral("y"), QString::number(d->rect.y()));
	writer->writeAttribute(QStringLiteral("width"), QString::number(d->rect.width()));
	writer->writeAttribute(QStringLiteral("height"), QString::number(d->rect.height()));
	writer->writeAttribute(QStringLiteral("visible"), QString::number(d->isVisible()));
	writer->writeEndElement();

	// coordinate system and padding
	// new style
	writer->writeStartElement(QStringLiteral("xRanges"));
	for (const auto& range : d->xRanges) {
		writer->writeStartElement(QStringLiteral("xRange"));
		writer->writeAttribute(QStringLiteral("autoScale"), QString::number(range.range.autoScale()));
		writer->writeAttribute(QStringLiteral("start"), QString::number(range.range.start(), 'g', 16));
		writer->writeAttribute(QStringLiteral("end"), QString::number(range.range.end(), 'g', 16));
		writer->writeAttribute(QStringLiteral("scale"), QString::number(static_cast<int>(range.range.scale())));
		writer->writeAttribute(QStringLiteral("format"), QString::number(static_cast<int>(range.range.format())));
		writer->writeAttribute(QStringLiteral("dateTimeFormat"), range.range.dateTimeFormat());
		writer->writeEndElement();
	}
	writer->writeEndElement();
	writer->writeStartElement(QStringLiteral("yRanges"));
	for (const auto& range : d->yRanges) {
		writer->writeStartElement(QStringLiteral("yRange"));
		writer->writeAttribute(QStringLiteral("autoScale"), QString::number(range.range.autoScale()));
		writer->writeAttribute(QStringLiteral("start"), QString::number(range.range.start(), 'g', 16));
		writer->writeAttribute(QStringLiteral("end"), QString::number(range.range.end(), 'g', 16));
		writer->writeAttribute(QStringLiteral("scale"), QString::number(static_cast<int>(range.range.scale())));
		writer->writeAttribute(QStringLiteral("format"), QString::number(static_cast<int>(range.range.format())));
		writer->writeAttribute(QStringLiteral("dateTimeFormat"), range.range.dateTimeFormat());
		writer->writeEndElement();
	}
	writer->writeEndElement();
	writer->writeStartElement(QStringLiteral("coordinateSystems"));
	writer->writeAttribute(QStringLiteral("defaultCoordinateSystem"), QString::number(defaultCoordinateSystemIndex()));
	// padding
	writer->writeAttribute(QStringLiteral("horizontalPadding"), QString::number(d->horizontalPadding));
	writer->writeAttribute(QStringLiteral("verticalPadding"), QString::number(d->verticalPadding));
	writer->writeAttribute(QStringLiteral("rightPadding"), QString::number(d->rightPadding));
	writer->writeAttribute(QStringLiteral("bottomPadding"), QString::number(d->bottomPadding));
	writer->writeAttribute(QStringLiteral("symmetricPadding"), QString::number(d->symmetricPadding));
	writer->writeAttribute(QStringLiteral("niceExtend"), QString::number(d->niceExtend));
	for (const auto& cSystem : m_coordinateSystems) {
		writer->writeStartElement(QStringLiteral("coordinateSystem"));
		writer->writeAttribute(QStringLiteral("xIndex"), QString::number(static_cast<CartesianCoordinateSystem*>(cSystem)->index(Dimension::X)));
		writer->writeAttribute(QStringLiteral("yIndex"), QString::number(static_cast<CartesianCoordinateSystem*>(cSystem)->index(Dimension::Y)));
		writer->writeEndElement();
	}
	writer->writeEndElement();
	// OLD style (pre 2.9.0)
	//	writer->writeStartElement( QStringLiteral("coordinateSystem") );
	//	writer->writeAttribute( QStringLiteral("autoScaleX"), QString::number(d->autoScaleX) );
	//	writer->writeAttribute( QStringLiteral("autoScaleY"), QString::number(d->autoScaleY) );
	//	writer->writeAttribute( QStringLiteral("xMin"), QString::number(d->range(Dimension::X, 0).start(), 'g', 16));
	//	writer->writeAttribute( QStringLiteral("xMax"), QString::number(d->range(Dimension::X, 0).end(), 'g', 16) );
	//	writer->writeAttribute( QStringLiteral("yMin"), QString::number(d->yRange.range.start(), 'g', 16) );
	//	writer->writeAttribute( QStringLiteral("yMax"), QString::number(d->yRange.range.end(), 'g', 16) );
	//	writer->writeAttribute( QStringLiteral("xScale"), QString::number(static_cast<int>(d->range(Dimension::X, 0).scale())) );
	//	writer->writeAttribute( QStringLiteral("yScale"), QString::number(static_cast<int>(d->yScale)) );
	//	writer->writeAttribute( QStringLiteral("xRangeFormat"), QString::number(static_cast<int>(xRangeFormat(0))) );
	//	writer->writeAttribute( QStringLiteral("yRangeFormat"), QString::number(static_cast<int>(d->yRangeFormat)) );
	//	writer->writeAttribute( QStringLiteral("horizontalPadding"), QString::number(d->horizontalPadding) );
	//	writer->writeAttribute( QStringLiteral("verticalPadding"), QString::number(d->verticalPadding) );
	//	writer->writeAttribute( QStringLiteral("rightPadding"), QString::number(d->rightPadding) );
	//	writer->writeAttribute( QStringLiteral("bottomPadding"), QString::number(d->bottomPadding) );
	//	writer->writeAttribute( QStringLiteral("symmetricPadding"), QString::number(d->symmetricPadding));

	// x-scale breaks
	if (d->xRangeBreakingEnabled || !d->xRangeBreaks.list.isEmpty()) {
		writer->writeStartElement(QStringLiteral("xRangeBreaks"));
		writer->writeAttribute(QStringLiteral("enabled"), QString::number(d->xRangeBreakingEnabled));
		for (const auto& rb : d->xRangeBreaks.list) {
			writer->writeStartElement(QStringLiteral("xRangeBreak"));
			writer->writeAttribute(QStringLiteral("start"), QString::number(rb.range.start()));
			writer->writeAttribute(QStringLiteral("end"), QString::number(rb.range.end()));
			writer->writeAttribute(QStringLiteral("position"), QString::number(rb.position));
			writer->writeAttribute(QStringLiteral("style"), QString::number(static_cast<int>(rb.style)));
			writer->writeEndElement();
		}
		writer->writeEndElement();
	}

	// y-scale breaks
	if (d->yRangeBreakingEnabled || !d->yRangeBreaks.list.isEmpty()) {
		writer->writeStartElement(QStringLiteral("yRangeBreaks"));
		writer->writeAttribute(QStringLiteral("enabled"), QString::number(d->yRangeBreakingEnabled));
		for (const auto& rb : d->yRangeBreaks.list) {
			writer->writeStartElement(QStringLiteral("yRangeBreak"));
			writer->writeAttribute(QStringLiteral("start"), QString::number(rb.range.start()));
			writer->writeAttribute(QStringLiteral("end"), QString::number(rb.range.end()));
			writer->writeAttribute(QStringLiteral("position"), QString::number(rb.position));
			writer->writeAttribute(QStringLiteral("style"), QString::number(static_cast<int>(rb.style)));
			writer->writeEndElement();
		}
		writer->writeEndElement();
	}

	// serialize all children (plot area, title text label, axes and curves)
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
	bool hasCoordinateSystems = false; // new since 2.9.0

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == QLatin1String("cartesianPlot"))
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == QLatin1String("comment")) {
			if (!readCommentElement(reader))
				return false;
		} else if (!preview && reader->name() == QLatin1String("theme")) {
			attribs = reader->attributes();
			d->theme = attribs.value(QStringLiteral("name")).toString();
		} else if (!preview && reader->name() == QLatin1String("cursor")) {
			d->cursorLine->load(reader, preview);
		} else if (!preview && reader->name() == QLatin1String("geometry")) {
			attribs = reader->attributes();

			str = attribs.value(QStringLiteral("x")).toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs(QStringLiteral("x")).toString());
			else
				d->rect.setX(str.toDouble());

			str = attribs.value(QStringLiteral("y")).toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs(QStringLiteral("y")).toString());
			else
				d->rect.setY(str.toDouble());

			str = attribs.value(QStringLiteral("width")).toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs(QStringLiteral("width")).toString());
			else
				d->rect.setWidth(str.toDouble());

			str = attribs.value(QStringLiteral("height")).toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs(QStringLiteral("height")).toString());
			else
				d->rect.setHeight(str.toDouble());

			str = attribs.value(QStringLiteral("visible")).toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs(QStringLiteral("visible")).toString());
			else
				d->setVisible(str.toInt());
		} else if (!preview && reader->name() == QLatin1String("xRanges")) {
			d->xRanges.clear();
		} else if (!preview && reader->name() == QLatin1String("xRange")) {
			attribs = reader->attributes();

			// TODO: Range<double> range = Range::load(reader)
			Range<double> range;
			str = attribs.value(QStringLiteral("autoScale")).toString();
			QDEBUG(Q_FUNC_INFO << ", str =" << str << ", value = " << str.toInt())
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs(QStringLiteral("autoScale")).toString());
			else
				range.setAutoScale(str.toInt());
			str = attribs.value(QStringLiteral("start")).toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs(QStringLiteral("start")).toString());
			else
				range.setStart(str.toDouble());
			str = attribs.value(QStringLiteral("end")).toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs(QStringLiteral("end")).toString());
			else
				range.setEnd(str.toDouble());
			str = attribs.value(QStringLiteral("scale")).toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs(QStringLiteral("scale")).toString());
			else
				range.setScale(static_cast<RangeT::Scale>(str.toInt()));
			str = attribs.value(QStringLiteral("format")).toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs(QStringLiteral("format")).toString());
			else
				range.setFormat(static_cast<RangeT::Format>(str.toInt()));
			str = attribs.value(QStringLiteral("dateTimeFormat")).toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs(QStringLiteral("dateTimeFormat")).toString());
			else
				range.setDateTimeFormat(str);

			DEBUG(Q_FUNC_INFO << ", auto scale =" << range.autoScale())
			addXRange(range);
		} else if (!preview && reader->name() == QLatin1String("yRanges")) {
			d->yRanges.clear();
		} else if (!preview && reader->name() == QLatin1String("yRange")) {
			attribs = reader->attributes();

			// TODO: Range<double> range = Range::load(reader)
			Range<double> range;
			str = attribs.value(QStringLiteral("autoScale")).toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs(QStringLiteral("autoScale")).toString());
			else
				range.setAutoScale(str.toInt());
			str = attribs.value(QStringLiteral("start")).toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs(QStringLiteral("start")).toString());
			else
				range.setStart(str.toDouble());
			str = attribs.value(QStringLiteral("end")).toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs(QStringLiteral("end")).toString());
			else
				range.setEnd(str.toDouble());
			str = attribs.value(QStringLiteral("scale")).toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs(QStringLiteral("scale")).toString());
			else
				range.setScale(static_cast<RangeT::Scale>(str.toInt()));
			str = attribs.value(QStringLiteral("format")).toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs(QStringLiteral("format")).toString());
			else
				range.setFormat(static_cast<RangeT::Format>(str.toInt()));
			str = attribs.value(QStringLiteral("dateTimeFormat")).toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs(QStringLiteral("dateTimeFormat")).toString());
			else
				range.setDateTimeFormat(str);

			addYRange(range);
		} else if (!preview && reader->name() == QLatin1String("coordinateSystems")) {
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

			if (Project::xmlVersion() < 7) {
				d->niceExtend = true;
			} else {
				str = attribs.value(QStringLiteral("niceExtend")).toString();
				if (str.isEmpty())
					reader->raiseWarning(attributeWarning.subs(QStringLiteral("niceExtend")).toString());
				else
					d->niceExtend = str.toInt();
			}

		} else if (!preview && reader->name() == QLatin1String("coordinateSystem")) {
			attribs = reader->attributes();
			// new style
			str = attribs.value(QStringLiteral("xIndex")).toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs(QStringLiteral("xIndex")).toString());
			else {
				CartesianCoordinateSystem* cSystem{new CartesianCoordinateSystem(this)};
				cSystem->setIndex(Dimension::X, str.toInt());

				str = attribs.value(QStringLiteral("yIndex")).toString();
				if (str.isEmpty())
					reader->raiseWarning(attributeWarning.subs(QStringLiteral("yIndex")).toString());
				else
					cSystem->setIndex(Dimension::Y, str.toInt());

				addCoordinateSystem(cSystem);
			}

			// old style (pre 2.9.0, to read old projects)
			if (!hasCoordinateSystems) {
				str = attribs.value(QStringLiteral("autoScaleX")).toString();
				if (str.isEmpty())
					reader->raiseWarning(attributeWarning.subs(QStringLiteral("autoScaleX")).toString());
				else
					d->xRanges[0].range.setAutoScale(str.toInt());
				str = attribs.value(QStringLiteral("autoScaleY")).toString();
				if (str.isEmpty())
					reader->raiseWarning(attributeWarning.subs(QStringLiteral("autoScaleY")).toString());
				else
					d->yRanges[0].range.setAutoScale(str.toInt());

				str = attribs.value(QStringLiteral("xMin")).toString();
				if (str.isEmpty())
					reader->raiseWarning(attributeWarning.subs(QStringLiteral("xMin")).toString());
				else {
					d->xRanges[0].range.start() = str.toDouble();
					d->xRanges[0].prev.start() = d->range(Dimension::X, 0).start();
				}

				str = attribs.value(QStringLiteral("xMax")).toString();
				if (str.isEmpty())
					reader->raiseWarning(attributeWarning.subs(QStringLiteral("xMax")).toString());
				else {
					d->xRanges[0].range.end() = str.toDouble();
					d->xRanges[0].prev.end() = d->range(Dimension::X, 0).end();
				}

				str = attribs.value(QStringLiteral("yMin")).toString();
				if (str.isEmpty())
					reader->raiseWarning(attributeWarning.subs(QStringLiteral("yMin")).toString());
				else {
					d->yRanges[0].range.start() = str.toDouble();
					d->yRanges[0].prev.start() = range(Dimension::Y, 0).start();
				}

				str = attribs.value(QStringLiteral("yMax")).toString();
				if (str.isEmpty())
					reader->raiseWarning(attributeWarning.subs(QStringLiteral("yMax")).toString());
				else {
					d->yRanges[0].range.end() = str.toDouble();
					d->yRanges[0].prev.end() = range(Dimension::Y, 0).end();
				}

				str = attribs.value(QStringLiteral("xScale")).toString();
				if (str.isEmpty())
					reader->raiseWarning(attributeWarning.subs(QStringLiteral("xScale")).toString());
				else {
					int scale{str.toInt()};
					// convert old scale
					if (scale > (int)RangeT::Scale::Ln)
						scale -= 3;
					d->xRanges[0].range.scale() = static_cast<RangeT::Scale>(scale);
				}
				str = attribs.value(QStringLiteral("yScale")).toString();
				if (str.isEmpty())
					reader->raiseWarning(attributeWarning.subs(QStringLiteral("yScale")).toString());
				else {
					int scale{str.toInt()};
					// convert old scale
					if (scale > (int)RangeT::Scale::Ln)
						scale -= 3;
					d->yRanges[0].range.scale() = static_cast<RangeT::Scale>(scale);
				}

				str = attribs.value(QStringLiteral("xRangeFormat")).toString();
				if (str.isEmpty())
					reader->raiseWarning(attributeWarning.subs(QStringLiteral("xRangeFormat")).toString());
				else
					d->xRanges[0].range.format() = static_cast<RangeT::Format>(str.toInt());
				str = attribs.value(QStringLiteral("yRangeFormat")).toString();
				if (str.isEmpty())
					reader->raiseWarning(attributeWarning.subs(QStringLiteral("yRangeFormat")).toString());
				else
					d->yRanges[0].range.format() = static_cast<RangeT::Format>(str.toInt());

				str = attribs.value(QStringLiteral("xRangeDateTimeFormat")).toString();
				if (!str.isEmpty())
					d->xRanges[0].range.setDateTimeFormat(str);

				str = attribs.value(QStringLiteral("yRangeDateTimeFormat")).toString();
				if (!str.isEmpty())
					d->yRanges[0].range.setDateTimeFormat(str);

				READ_DOUBLE_VALUE("horizontalPadding", horizontalPadding);
				READ_DOUBLE_VALUE("verticalPadding", verticalPadding);
				READ_DOUBLE_VALUE("rightPadding", rightPadding);
				READ_DOUBLE_VALUE("bottomPadding", bottomPadding);
				READ_INT_VALUE("symmetricPadding", symmetricPadding, bool);
			}
		} else if (!preview && reader->name() == QLatin1String("xRangeBreaks")) {
			// delete default range break
			d->xRangeBreaks.list.clear();

			attribs = reader->attributes();
			READ_INT_VALUE("enabled", xRangeBreakingEnabled, bool);
		} else if (!preview && reader->name() == QLatin1String("xRangeBreak")) {
			attribs = reader->attributes();

			RangeBreak b;
			str = attribs.value(QStringLiteral("start")).toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs(QStringLiteral("start")).toString());
			else
				b.range.start() = str.toDouble();

			str = attribs.value(QStringLiteral("end")).toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs(QStringLiteral("end")).toString());
			else
				b.range.end() = str.toDouble();

			str = attribs.value(QStringLiteral("position")).toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs(QStringLiteral("position")).toString());
			else
				b.position = str.toDouble();

			str = attribs.value(QStringLiteral("style")).toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs(QStringLiteral("style")).toString());
			else
				b.style = CartesianPlot::RangeBreakStyle(str.toInt());

			d->xRangeBreaks.list << b;
		} else if (!preview && reader->name() == QLatin1String("yRangeBreaks")) {
			// delete default range break
			d->yRangeBreaks.list.clear();

			attribs = reader->attributes();
			READ_INT_VALUE("enabled", yRangeBreakingEnabled, bool);
		} else if (!preview && reader->name() == QLatin1String("yRangeBreak")) {
			attribs = reader->attributes();

			RangeBreak b;
			str = attribs.value(QStringLiteral("start")).toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs(QStringLiteral("start")).toString());
			else
				b.range.start() = str.toDouble();

			str = attribs.value(QStringLiteral("end")).toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs(QStringLiteral("end")).toString());
			else
				b.range.end() = str.toDouble();

			str = attribs.value(QStringLiteral("position")).toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs(QStringLiteral("position")).toString());
			else
				b.position = str.toDouble();

			str = attribs.value(QStringLiteral("style")).toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs(QStringLiteral("style")).toString());
			else
				b.style = CartesianPlot::RangeBreakStyle(str.toInt());

			d->yRangeBreaks.list << b;
		} else if (!preview && reader->name() == QLatin1String("textLabel")) {
			if (!titleLabelRead) {
				m_title->setIsLoading(true);
				// the first text label is always the title label
				m_title->load(reader, preview);
				titleLabelRead = true;

				// TODO: the name is read in m_title->load() but we overwrite it here
				// since the old projects don't have this " - Title" appendix yet that we add in init().
				// can be removed in couple of releases
				m_title->setName(name() + QLatin1String(" - ") + i18n("Title"));
			} else {
				auto* label = new TextLabel(QStringLiteral("text label"), this);
				label->setIsLoading(true);
				if (label->load(reader, preview)) {
					addChildFast(label);
					label->setParentGraphicsItem(graphicsItem());
				} else {
					delete label;
					return false;
				}
			}
		} else if (!preview && reader->name() == QLatin1String("image")) {
			auto* image = new Image(QString());
			image->setIsLoading(true);
			if (!image->load(reader, preview)) {
				delete image;
				return false;
			} else
				addChildFast(image);
		} else if (!preview && reader->name() == QLatin1String("infoElement")) {
			auto* marker = new InfoElement(QStringLiteral("Marker"), this);
			marker->setIsLoading(true);
			if (marker->load(reader, preview)) {
				addChildFast(marker);
				marker->setParentGraphicsItem(graphicsItem());
			} else {
				delete marker;
				return false;
			}
		} else if (!preview && reader->name() == QLatin1String("plotArea")) {
			m_plotArea->setIsLoading(true);
			m_plotArea->load(reader, preview);
		} else if (!preview && reader->name() == QLatin1String("axis")) {
			auto* axis = new Axis(QString());
			axis->setIsLoading(true);
			if (axis->load(reader, preview))
				addChildFast(axis);
			else {
				delete axis;
				return false;
			}
		} else if (reader->name() == QLatin1String("xyCurve")) {
			auto* curve = new XYCurve(QString());
			curve->setIsLoading(true);
			if (curve->load(reader, preview))
				addChildFast(curve);
			else {
				removeChild(curve);
				return false;
			}
		} else if (reader->name() == QLatin1String("xyEquationCurve")) {
			auto* curve = new XYEquationCurve(QString());
			curve->setIsLoading(true);
			if (curve->load(reader, preview))
				addChildFast(curve);
			else {
				removeChild(curve);
				return false;
			}
		} else if (reader->name() == QLatin1String("xyDataReductionCurve")) {
			auto* curve = new XYDataReductionCurve(QString());
			curve->setIsLoading(true);
			if (curve->load(reader, preview))
				addChildFast(curve);
			else {
				removeChild(curve);
				return false;
			}
		} else if (reader->name() == QLatin1String("xyDifferentiationCurve")) {
			auto* curve = new XYDifferentiationCurve(QString());
			curve->setIsLoading(true);
			if (curve->load(reader, preview))
				addChildFast(curve);
			else {
				removeChild(curve);
				return false;
			}
		} else if (reader->name() == QLatin1String("xyIntegrationCurve")) {
			auto* curve = new XYIntegrationCurve(QString());
			curve->setIsLoading(true);
			if (curve->load(reader, preview))
				addChildFast(curve);
			else {
				removeChild(curve);
				return false;
			}
		} else if (reader->name() == QLatin1String("xyInterpolationCurve")) {
			auto* curve = new XYInterpolationCurve(QString());
			curve->setIsLoading(true);
			if (curve->load(reader, preview))
				addChildFast(curve);
			else {
				removeChild(curve);
				return false;
			}
		} else if (reader->name() == QLatin1String("xySmoothCurve")) {
			auto* curve = new XYSmoothCurve(QString());
			curve->setIsLoading(true);
			if (curve->load(reader, preview))
				addChildFast(curve);
			else {
				removeChild(curve);
				return false;
			}
		} else if (reader->name() == QLatin1String("xyFitCurve")) {
			auto* curve = new XYFitCurve(QString());
			curve->setIsLoading(true);
			if (curve->load(reader, preview))
				addChildFast(curve);
			else {
				removeChild(curve);
				return false;
			}
		} else if (reader->name() == QLatin1String("xyFourierFilterCurve")) {
			auto* curve = new XYFourierFilterCurve(QString());
			curve->setIsLoading(true);
			if (curve->load(reader, preview))
				addChildFast(curve);
			else {
				removeChild(curve);
				return false;
			}
		} else if (reader->name() == QLatin1String("xyFourierTransformCurve")) {
			auto* curve = new XYFourierTransformCurve(QString());
			curve->setIsLoading(true);
			if (curve->load(reader, preview))
				addChildFast(curve);
			else {
				removeChild(curve);
				return false;
			}
		} else if (reader->name() == QLatin1String("xyHilbertTransformCurve")) {
			auto* curve = new XYHilbertTransformCurve(QString());
			curve->setIsLoading(true);
			if (curve->load(reader, preview))
				addChildFast(curve);
			else {
				removeChild(curve);
				return false;
			}
		} else if (reader->name() == QLatin1String("xyConvolutionCurve")) {
			auto* curve = new XYConvolutionCurve(QString());
			curve->setIsLoading(true);
			if (curve->load(reader, preview))
				addChildFast(curve);
			else {
				removeChild(curve);
				return false;
			}
		} else if (reader->name() == QLatin1String("xyCorrelationCurve")) {
			auto* curve = new XYCorrelationCurve(QString());
			curve->setIsLoading(true);
			if (curve->load(reader, preview))
				addChildFast(curve);
			else {
				removeChild(curve);
				return false;
			}
		} else if (!preview && reader->name() == QLatin1String("cartesianPlotLegend")) {
			m_legend = new CartesianPlotLegend(QString());
			m_legend->setIsLoading(true);
			if (m_legend->load(reader, preview))
				addChildFast(m_legend);
			else {
				delete m_legend;
				return false;
			}
		} else if (!preview && reader->name() == QLatin1String("customPoint")) {
			auto* point = new CustomPoint(this, QString());
			point->setIsLoading(true);
			if (point->load(reader, preview))
				addChildFast(point);
			else {
				delete point;
				return false;
			}
		} else if (!preview && reader->name() == QLatin1String("referenceLine")) {
			auto* line = new ReferenceLine(this, QString());
			line->setIsLoading(true);
			if (line->load(reader, preview))
				addChildFast(line);
			else {
				delete line;
				return false;
			}
		} else if (reader->name() == QLatin1String("boxPlot")) {
			auto* boxPlot = new BoxPlot(QStringLiteral("BoxPlot"));
			boxPlot->setIsLoading(true);
			if (boxPlot->load(reader, preview))
				addChildFast(boxPlot);
			else {
				removeChild(boxPlot);
				return false;
			}
		} else if (reader->name() == QLatin1String("barPlot")) {
			auto* barPlot = new BarPlot(QStringLiteral("BarPlot"));
			barPlot->setIsLoading(true);
			if (barPlot->load(reader, preview))
				addChildFast(barPlot);
			else {
				removeChild(barPlot);
				return false;
			}
		} else if (reader->name() == QLatin1String("Histogram")) {
			auto* hist = new Histogram(QStringLiteral("Histogram"));
			hist->setIsLoading(true);
			if (hist->load(reader, preview))
				addChildFast(hist);
			else {
				removeChild(hist);
				return false;
			}
		} else { // unknown element
			if (!preview)
				reader->raiseWarning(i18n("unknown cartesianPlot element '%1'", reader->name().toString()));
			if (!reader->skipToEndElement())
				return false;
		}
	}

	if (preview)
		return true;

	// if a theme was used, initialize the color palette
	if (!d->theme.isEmpty()) {
		// TODO: check whether the theme config really exists
		KConfig config(ThemeHandler::themeFilePath(d->theme), KConfig::SimpleConfig);
		this->setColorPalette(config);
	} else {
		// initialize the color palette with default colors
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

	// loadThemeConfig() can be called from
	// 1. CartesianPlot::setTheme() when the user changes the theme for the plot
	// 2. Worksheet::setTheme() -> Worksheet::loadTheme() when the user changes the theme for the worksheet
	// In the second case (i.e. when d->theme is not equal to theme yet),
	/// we need to put the new theme name on the undo-stack.
	if (theme != d->theme)
		exec(new CartesianPlotSetThemeCmd(d, theme, ki18n("%1: set theme")));

	// load the color palettes for the curves
	this->setColorPalette(config);

	// load the theme for all the children
	const auto& elements = children<WorksheetElement>(ChildIndexFlag::IncludeHidden);
	for (auto* child : elements)
		child->loadThemeConfig(config);

	d->update(this->rect());
}

void CartesianPlot::saveTheme(KConfig& config) {
	const QVector<Axis*>& axisElements = children<Axis>(ChildIndexFlag::IncludeHidden);
	const QVector<PlotArea*>& plotAreaElements = children<PlotArea>(ChildIndexFlag::IncludeHidden);
	const QVector<TextLabel*>& textLabelElements = children<TextLabel>(ChildIndexFlag::IncludeHidden);

	axisElements.at(0)->saveThemeConfig(config);
	plotAreaElements.at(0)->saveThemeConfig(config);
	textLabelElements.at(0)->saveThemeConfig(config);

	const auto& children = this->children<XYCurve>(ChildIndexFlag::IncludeHidden);
	for (auto* child : children)
		child->saveThemeConfig(config);
}

// Generating colors from 5-color theme palette
void CartesianPlot::setColorPalette(const KConfig& config) {
	if (config.hasGroup(QLatin1String("Theme"))) {
		KConfigGroup group = config.group(QLatin1String("Theme"));

		// read the five colors defining the palette
		m_themeColorPalette.clear();
		m_themeColorPalette.append(group.readEntry("ThemePaletteColor1", QColor()));
		m_themeColorPalette.append(group.readEntry("ThemePaletteColor2", QColor()));
		m_themeColorPalette.append(group.readEntry("ThemePaletteColor3", QColor()));
		m_themeColorPalette.append(group.readEntry("ThemePaletteColor4", QColor()));
		m_themeColorPalette.append(group.readEntry("ThemePaletteColor5", QColor()));
	} else {
		// no theme is available, provide "default colors"
		m_themeColorPalette.clear();

		m_themeColorPalette.append(QColor(28, 113, 216));
		m_themeColorPalette.append(QColor(255, 120, 0));
		m_themeColorPalette.append(QColor(224, 27, 36));
		m_themeColorPalette.append(QColor(46, 194, 126));
		m_themeColorPalette.append(QColor(246, 211, 45));
		m_themeColorPalette.append(QColor(143, 19, 178));
		m_themeColorPalette.append(QColor(0, 255, 255));
		m_themeColorPalette.append(QColor(235, 26, 209));
		m_themeColorPalette.append(QColor(41, 221, 37));
		m_themeColorPalette.append(QColor(33, 6, 227));
		m_themeColorPalette.append(QColor(14, 136, 22));
		m_themeColorPalette.append(QColor(147, 97, 22));
		m_themeColorPalette.append(QColor(85, 85, 91));
		m_themeColorPalette.append(QColor(156, 4, 4));
		// TODO: maybe removing black?
		m_themeColorPalette.append(QColor(0, 0, 0));
	}

	// use the color of the axis lines as the color for the different mouse cursor lines
	Q_D(CartesianPlot);
	const KConfigGroup& group = config.group("Axis");
	const QColor& color = group.readEntry("LineColor", QColor(Qt::black));
	d->zoomSelectPen.setColor(color);
	d->crossHairPen.setColor(color);
}

const QList<QColor>& CartesianPlot::themeColorPalette() const {
	return m_themeColorPalette;
}

const QColor CartesianPlot::themeColorPalette(int index) const {
	const int i = index % m_themeColorPalette.count();
	return m_themeColorPalette.at(i);
}
