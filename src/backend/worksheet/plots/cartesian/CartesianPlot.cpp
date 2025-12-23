/*
	File                 : CartesianPlot.cpp
	Project              : LabPlot
	Description          : Cartesian plot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2011-2025 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2016-2021 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-FileCopyrightText: 2017-2018 Garvit Khatri <garvitdelhi@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "CartesianPlot.h"
#include "CartesianPlotPrivate.h"

#include "backend/core/Project.h"
#include "backend/core/column/Column.h"
#include "backend/core/datatypes/DateTime2StringFilter.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/trace.h"
#include "backend/worksheet/DefaultColorTheme.h"
#include "backend/worksheet/Image.h"
#include "backend/worksheet/InfoElement.h"
#include "backend/worksheet/Line.h"
#include "backend/worksheet/TextLabel.h"
#include "backend/worksheet/plots/PlotArea.h"
#include "backend/worksheet/plots/cartesian/CartesianPlotLegend.h"
#include "backend/worksheet/plots/cartesian/CustomPoint.h"
#include "backend/worksheet/plots/cartesian/ErrorBar.h"
#include "backend/worksheet/plots/cartesian/ReferenceLine.h"
#include "backend/worksheet/plots/cartesian/ReferenceRange.h"
#include "backend/worksheet/plots/cartesian/Symbol.h"
#include "backend/worksheet/plots/cartesian/plots.h"
#include "frontend/ThemeHandler.h"
#include "frontend/widgets/ThemesWidget.h"
#include "tools/ColorMapsManager.h"

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>

#include <QActionGroup>
#include <QIcon>
#include <QKeyEvent>
#include <QMenu>
#include <QPainter>
#include <QWidgetAction>

namespace {
enum Action {
	New = 0x1000,
	NewTextLabel = 0x1001,
	NewCustomPoint = 0x1002,
	NewReferenceRange = 0x1003,
	NewReferenceLine = 0x1004,
	NewImage = 0x1005,

	NavigateNextCurve = 0x2001,
	NavigatePrevCurve = 0x2002,

	Move = 0x4000,
	MoveLeft = 0x4001,
	MoveRight = 0x4002,
	MoveUp = 0x4003,
	MoveDown = 0x4004,

	Abort = 0x8000,
};

Action evaluateKeys(int key, Qt::KeyboardModifiers) {
	if (key == Qt::Key_N)
		return Action::NavigateNextCurve;
	else if (key == Qt::Key_P)
		return Action::NavigatePrevCurve;
	else if (key == Qt::Key_T)
		return Action::NewTextLabel;
	else if (key == Qt::Key_R)
		return Action::NewReferenceRange;
	else if (key == Qt::Key_L)
		return Action::NewReferenceLine;
	else if (key == Qt::Key_I)
		return Action::NewImage;
	else if (key == Qt::Key_M)
		return Action::NewCustomPoint;
	else if (key == Qt::Key_Escape)
		return Action::Abort;
	else if (key == Qt::Key_Left)
		return Action::MoveLeft;
	else if (key == Qt::Key_Right)
		return Action::MoveRight;
	else if (key == Qt::Key_Up)
		return Action::MoveUp;
	else if (key == Qt::Key_Down)
		return Action::MoveDown;
	return Action::Abort;
}
}

/**
 * \class CartesianPlot
 * \brief This class implements the cartesian plot and the actual plot area that is visualized on the \c Worksheet.
 *
 * The definition of the cartesian coordinate systems as well as of the to be plotted data ranges is done in this class.
 *
 * \ingroup CartesianPlotArea
 */
CartesianPlot::CartesianPlot(const QString& name, bool loading)
	: AbstractPlot(name, new CartesianPlotPrivate(this), AspectType::CartesianPlot) {
	init(loading);
}

CartesianPlot::CartesianPlot(const QString& name, CartesianPlotPrivate* dd)
	: AbstractPlot(name, dd, AspectType::CartesianPlot) {
	init(false);
}

CartesianPlot::~CartesianPlot() {
	if (m_menusInitialized) {
		delete m_addNewMenu;
		delete dataAnalysisMenu;
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
void CartesianPlot::init(bool loading) {
	// initialize the children objects
	m_plotArea = new PlotArea(name() + QStringLiteral(" plot area"), this);
	connect(m_plotArea, &WorksheetElement::changed, this, &WorksheetElement::changed);
	addChildFast(m_plotArea);

	// title
	m_title = new TextLabel(this->name() + QLatin1String(" - ") + i18n("Title"), TextLabel::Type::PlotTitle);
	addChild(m_title);
	m_title->setHidden(true);
	m_title->setParentGraphicsItem(m_plotArea->graphicsItem());

	Q_D(CartesianPlot);

	// cursor line
	d->cursorLine = new Line(QString());
	d->cursorLine->setPrefix(QLatin1String("Cursor"));
	d->cursorLine->setHidden(true);
	addChild(d->cursorLine);
	connect(d->cursorLine, &Line::updatePixmapRequested, [=] {
		d->update();
	});
	connect(d->cursorLine, &Line::updateRequested, [=] {
		d->update();
	});

	connect(this, &AbstractAspect::childAspectAdded, this, &CartesianPlot::childAdded);
	connect(this, &AbstractAspect::childAspectRemoved, this, &CartesianPlot::childRemoved);

	// if not loading, initialize the default properties (read in load() otherwise)
	if (loading)
		return;

	m_coordinateSystems << new CartesianCoordinateSystem(this);

	KConfig config;
	KConfigGroup group = config.group(QStringLiteral("CartesianPlot"));

	// TODO: load from KConfigGroup
	// offset between the plot area and the area defining the coordinate system, in scene units.
	d->horizontalPadding = Worksheet::convertToSceneUnits(1.5, Worksheet::Unit::Centimeter);
	d->verticalPadding = Worksheet::convertToSceneUnits(1.5, Worksheet::Unit::Centimeter);
	d->rightPadding = Worksheet::convertToSceneUnits(1.5, Worksheet::Unit::Centimeter);
	d->bottomPadding = Worksheet::convertToSceneUnits(1.5, Worksheet::Unit::Centimeter);

	d->cursorLine->setStyle(Qt::SolidLine);
	d->cursorLine->setColor(Qt::red); // TODO: use theme specific initial settings
	d->cursorLine->setWidth(Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Point));

	d->plotColorMode = (PlotColorMode)group.readEntry(QStringLiteral("PlotColorMode"), (int)PlotColorMode::Theme);
	d->theme = group.readEntry(QStringLiteral("Theme"), QString());
	d->plotColorMap = group.readEntry(QStringLiteral("ColorMap"), QStringLiteral("batlowS10"));

	// initialize the color palette with default colors
	d->updatePlotColorPalette();
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

		// x1
		Axis* axis = new Axis(QLatin1String("x"), Axis::Orientation::Horizontal);
		axis->setDefault(true);
		axis->setSuppressRetransform(true);
		addChild(axis);
		axis->setPosition(Axis::Position::Bottom);
		axis->setRange(0., 1.);
		axis->setMajorTicksDirection(Axis::ticksIn);
		axis->setMinorTicksDirection(Axis::ticksIn);
		axis->setMinorTicksNumber(1);
		axis->minorGridLine()->setStyle(Qt::NoPen);
		axis->setSuppressRetransform(false);

		// x2
		axis = new Axis(QLatin1String("x2"), Axis::Orientation::Horizontal);
		axis->title()->setText(QString());
		axis->setDefault(true);
		axis->setSuppressRetransform(true);
		addChild(axis);
		axis->setPosition(Axis::Position::Top);
		axis->setRange(0., 1.);
		axis->setMajorTicksDirection(Axis::noTicks);
		axis->setMinorTicksDirection(Axis::noTicks);
		axis->setMinorTicksNumber(1);
		axis->majorGridLine()->setStyle(Qt::NoPen);
		axis->minorGridLine()->setStyle(Qt::NoPen);
		axis->setLabelsPosition(Axis::LabelsPosition::NoLabels);
		axis->setSuppressRetransform(false);

		// y1
		axis = new Axis(QLatin1String("y"), Axis::Orientation::Vertical);
		axis->setDefault(true);
		axis->setSuppressRetransform(true);
		addChild(axis);
		axis->setPosition(Axis::Position::Left);
		axis->setRange(0., 1.);
		axis->setMajorTicksDirection(Axis::ticksIn);
		axis->setMinorTicksDirection(Axis::ticksIn);
		axis->setMinorTicksNumber(1);
		axis->minorGridLine()->setStyle(Qt::NoPen);
		axis->setSuppressRetransform(false);

		// y2
		axis = new Axis(QLatin1String("y2"), Axis::Orientation::Vertical);
		axis->title()->setText(QString());
		axis->setDefault(true);
		axis->setSuppressRetransform(true);
		addChild(axis);
		axis->setPosition(Axis::Position::Right);
		axis->setRange(0., 1.);
		axis->setMajorTicksDirection(Axis::noTicks);
		axis->setMinorTicksDirection(Axis::noTicks);
		axis->setMinorTicksNumber(1);
		axis->majorGridLine()->setStyle(Qt::NoPen);
		axis->minorGridLine()->setStyle(Qt::NoPen);
		axis->setLabelsPosition(Axis::LabelsPosition::NoLabels);
		axis->setSuppressRetransform(false);

		d->horizontalPadding = Worksheet::convertToSceneUnits(1.5, Worksheet::Unit::Centimeter);
		d->verticalPadding = Worksheet::convertToSceneUnits(0.5, Worksheet::Unit::Centimeter);
		d->rightPadding = Worksheet::convertToSceneUnits(0.5, Worksheet::Unit::Centimeter);
		d->bottomPadding = Worksheet::convertToSceneUnits(1.5, Worksheet::Unit::Centimeter);

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

		d->horizontalPadding = Worksheet::convertToSceneUnits(1.5, Worksheet::Unit::Centimeter);
		d->verticalPadding = Worksheet::convertToSceneUnits(0.5, Worksheet::Unit::Centimeter);
		d->rightPadding = Worksheet::convertToSceneUnits(0.5, Worksheet::Unit::Centimeter);
		d->bottomPadding = Worksheet::convertToSceneUnits(1.5, Worksheet::Unit::Centimeter);

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

		d->horizontalPadding = Worksheet::convertToSceneUnits(0.5, Worksheet::Unit::Centimeter);
		d->verticalPadding = Worksheet::convertToSceneUnits(0.5, Worksheet::Unit::Centimeter);
		d->rightPadding = Worksheet::convertToSceneUnits(0.5, Worksheet::Unit::Centimeter);
		d->bottomPadding = Worksheet::convertToSceneUnits(0.5, Worksheet::Unit::Centimeter);

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

		d->horizontalPadding = Worksheet::convertToSceneUnits(0.5, Worksheet::Unit::Centimeter);
		d->verticalPadding = Worksheet::convertToSceneUnits(0.5, Worksheet::Unit::Centimeter);
		d->rightPadding = Worksheet::convertToSceneUnits(0.5, Worksheet::Unit::Centimeter);
		d->bottomPadding = Worksheet::convertToSceneUnits(0.5, Worksheet::Unit::Centimeter);

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
	// analysis curves, no icons yet
	addLineSimplificationCurveAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-curve")), i18n("Line Simplification"), this);
	addDifferentiationCurveAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-curve")), i18n("Differentiation"), this);
	addIntegrationCurveAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-curve")), i18n("Integration"), this);
	addInterpolationCurveAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-interpolation-curve")), i18n("Interpolation"), this);
	addSmoothCurveAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-smoothing-curve")), i18n("Smooth"), this);
	addFitCurveAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-fit-curve")), i18nc("Curve fitting", "Fit"), this);
	addFourierFilterCurveAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-fourier-filter-curve")), i18n("Fourier Filter"), this);
	addFourierTransformCurveAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-fourier-transform-curve")), i18n("Fourier Transform"), this);
	addHilbertTransformCurveAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-curve")), i18n("Hilbert Transform"), this);
	addConvolutionCurveAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-curve")), i18n("(De-)Convolution"), this);
	addCorrelationCurveAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-curve")), i18n("Auto-/Cross-Correlation"), this);
	addBaselineCorrectionCurveAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-curve")), i18n("Baseline Correction"), this);

	connect(addLineSimplificationCurveAction, &QAction::triggered, this, &CartesianPlot::addLineSimplificationCurve);
	connect(addDifferentiationCurveAction, &QAction::triggered, this, &CartesianPlot::addDifferentiationCurve);
	connect(addIntegrationCurveAction, &QAction::triggered, this, &CartesianPlot::addIntegrationCurve);
	connect(addInterpolationCurveAction, &QAction::triggered, this, &CartesianPlot::addInterpolationCurve);
	connect(addSmoothCurveAction, &QAction::triggered, this, &CartesianPlot::addSmoothCurve);
	connect(addFitCurveAction, &QAction::triggered, this, &CartesianPlot::addFitCurve);
	connect(addFourierFilterCurveAction, &QAction::triggered, this, &CartesianPlot::addFourierFilterCurve);
	connect(addFourierTransformCurveAction, &QAction::triggered, this, [=]() {
		addChild(new XYFourierTransformCurve(i18n("Fourier Transform")));
	});
	connect(addHilbertTransformCurveAction, &QAction::triggered, this, [=]() {
		addChild(new XYHilbertTransformCurve(i18n("Hilbert Transform")));
	});
	connect(addConvolutionCurveAction, &QAction::triggered, this, [=]() {
		addChild(new XYConvolutionCurve(i18n("Convolution")));
	});
	connect(addCorrelationCurveAction, &QAction::triggered, this, [=]() {
		addChild(new XYCorrelationCurve(i18n("Auto-/Cross-Correlation")));
	});
	connect(addBaselineCorrectionCurveAction, &QAction::triggered, this, &CartesianPlot::addBaselineCorrectionCurve);

	addFunctionCurveAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-equation-curve")), i18n("Function"), this);
	addFunctionCurveAction->setToolTip(i18n("Add a new xy-curve that is defined as a function of other xy-curves (scaled, shifted, etc.)"));
	connect(addFunctionCurveAction, &QAction::triggered, this, &CartesianPlot::addFunctionCurve);

	// Analysis menu actions, used in the spreadsheet
	addLineSimplificationAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-curve")), i18n("Line Simplification"), this);
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

	connect(addLineSimplificationAction, &QAction::triggered, this, &CartesianPlot::addLineSimplificationCurve);
	connect(addDifferentiationAction, &QAction::triggered, this, &CartesianPlot::addDifferentiationCurve);
	connect(addIntegrationAction, &QAction::triggered, this, &CartesianPlot::addIntegrationCurve);
	connect(addInterpolationAction, &QAction::triggered, this, &CartesianPlot::addInterpolationCurve);
	connect(addSmoothAction, &QAction::triggered, this, &CartesianPlot::addSmoothCurve);
	connect(addConvolutionAction, &QAction::triggered, this, [=]() {
		addChild(new XYConvolutionCurve(i18n("Convolution")));
	});
	connect(addCorrelationAction, &QAction::triggered, this, [=]() {
		addChild(new XYCorrelationCurve(i18n("Auto-/Cross-Correlation")));
	});
	connect(addBaselineCorrectionAction, &QAction::triggered, this, &CartesianPlot::addBaselineCorrectionCurve);
	for (const auto& action : addFitActions)
		connect(action, &QAction::triggered, this, &CartesianPlot::addFitCurve);
	connect(addFourierFilterAction, &QAction::triggered, this, &CartesianPlot::addFourierFilterCurve);
	connect(addFourierTransformAction, &QAction::triggered, this, [=]() {
		addChild(new XYFourierTransformCurve(i18n("Fourier Transform")));
	});
	connect(addHilbertTransformAction, &QAction::triggered, this, [=]() {
		addChild(new XYHilbertTransformCurve(i18n("Hilbert Transform")));
	});

	// other objects
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
	addReferenceRangeAction = new QAction(QIcon::fromTheme(QStringLiteral("draw-rectangle")), i18n("Reference Range"), this);

	// inset plot action, use the proper icon for the current plot type
	QIcon icon;
	Q_D(CartesianPlot);
	switch (d->type) {
	case Type::FourAxes:
		icon = QIcon::fromTheme(QStringLiteral("labplot-xy-plot-four-axes"));
		break;
	case Type::TwoAxes:
		icon = QIcon::fromTheme(QStringLiteral("labplot-xy-plot-two-axes"));
		break;
	case Type::TwoAxesCentered:
		icon = QIcon::fromTheme(QStringLiteral("labplot-xy-plot-two-axes-centered"));
		break;
	case Type::TwoAxesCenteredZero:
		icon = QIcon::fromTheme(QStringLiteral("labplot-xy-plot-two-axes-centered-origin"));
		break;
	}
	addInsetPlotAction = new QAction(icon, i18n("Inset Plot Area"), this);
	addInsetPlotWithDataAction = new QAction(icon, i18n("Inset Plot Area with Data"), this);

	connect(addLegendAction, &QAction::triggered, this, static_cast<void (CartesianPlot::*)()>(&CartesianPlot::addLegend));
	connect(addHorizontalAxisAction, &QAction::triggered, this, &CartesianPlot::addHorizontalAxis);
	connect(addVerticalAxisAction, &QAction::triggered, this, &CartesianPlot::addVerticalAxis);
	connect(addTextLabelAction, &QAction::triggered, this, &CartesianPlot::addTextLabel);
	connect(addImageAction, &QAction::triggered, this, &CartesianPlot::addImage);
	connect(addInfoElementAction, &QAction::triggered, this, &CartesianPlot::addInfoElement);
	connect(addCustomPointAction, &QAction::triggered, this, &CartesianPlot::addCustomPoint);
	connect(addReferenceLineAction, &QAction::triggered, this, &CartesianPlot::addReferenceLine);
	connect(addReferenceRangeAction, &QAction::triggered, this, &CartesianPlot::addReferenceRange);
	connect(addInsetPlotAction, &QAction::triggered, this, &CartesianPlot::addInsetPlot);
	connect(addInsetPlotWithDataAction, &QAction::triggered, this, &CartesianPlot::addInsetPlotWithData);
}

void CartesianPlot::initMenus() {
	initActions();

	m_addNewMenu = new QMenu(i18n("Add New"));
	m_addNewMenu->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));
	auto* actionGroup = new QActionGroup(this);
	connect(actionGroup, &QActionGroup::triggered, this, &CartesianPlot::addPlot);

	// add all available plot types
	CartesianPlot::fillAddNewPlotMenu(m_addNewMenu, actionGroup);

	// formula plot
	auto* action = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-equation-curve")), i18n("Formula Plot"), actionGroup);
	action->setToolTip(i18n("Add a new xy-plot that is defined via a mathematical expression."));
	action->setData(static_cast<int>(Plot::PlotType::Formula));
	m_addNewMenu->addSeparator();
	m_addNewMenu->addAction(action);
	m_addNewMenu->addSeparator();

	// analysis curves
	addNewAnalysisMenu = new QMenu(i18n("Analysis Plots"), m_addNewMenu);
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
	addNewAnalysisMenu->addAction(addLineSimplificationCurveAction);
	addNewAnalysisMenu->addSeparator();
	addNewAnalysisMenu->addAction(addBaselineCorrectionCurveAction);
	addNewAnalysisMenu->addSeparator();
	addNewAnalysisMenu->addAction(addFunctionCurveAction);
	m_addNewMenu->addMenu(addNewAnalysisMenu);

	m_addNewMenu->addSeparator();
	m_addNewMenu->addAction(addLegendAction);
	m_addNewMenu->addSeparator();
	m_addNewMenu->addAction(addHorizontalAxisAction);
	m_addNewMenu->addAction(addVerticalAxisAction);
	m_addNewMenu->addSeparator();
	m_addNewMenu->addAction(addTextLabelAction);
	m_addNewMenu->addAction(addImageAction);
	m_addNewMenu->addAction(addInfoElementAction);
	m_addNewMenu->addSeparator();
	m_addNewMenu->addAction(addCustomPointAction);
	m_addNewMenu->addAction(addReferenceLineAction);
	m_addNewMenu->addAction(addReferenceRangeAction);
	if (parentAspect()->type() != AspectType::CartesianPlot) { // don't allow to add an inset plot if it's already an inset plot
		m_addNewMenu->addSeparator();
		m_addNewMenu->addAction(addInsetPlotAction);
		m_addNewMenu->addAction(addInsetPlotWithDataAction);
	}

	// analysis menu, used in the context menu of XYCurve to allow direct application of analysis functions on the curves
	dataAnalysisMenu = new QMenu(i18n("Analysis"));

	QMenu* dataFitMenu = new QMenu(i18nc("Curve fitting", "Fit"), dataAnalysisMenu);
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
	dataAnalysisMenu->addMenu(dataFitMenu);

	// TODO: re-use addNewAnalysisMenu?
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
	dataAnalysisMenu->addSeparator();
	dataAnalysisMenu->addAction(addLineSimplificationAction);
	dataAnalysisMenu->addSeparator();
	dataAnalysisMenu->addAction(addBaselineCorrectionAction);
	dataAnalysisMenu->addSeparator();
	dataAnalysisMenu->addAction(addFunctionCurveAction);

	// theme menu
	themeMenu = new QMenu(i18n("Theme"));
	themeMenu->setIcon(QIcon::fromTheme(QStringLiteral("color-management")));
#ifndef SDK
	connect(themeMenu, &QMenu::aboutToShow, this, [=]() {
		if (!themeMenu->isEmpty())
			return;
		auto* themeWidget = new ThemesWidget(nullptr);
		themeWidget->setFixedMode();
		connect(themeWidget, &ThemesWidget::themeSelected, this, &CartesianPlot::setTheme);
		connect(themeWidget, &ThemesWidget::themeSelected, themeMenu, &QMenu::close);

		auto* widgetAction = new QWidgetAction(this);
		widgetAction->setDefaultWidget(themeWidget);
		themeMenu->addAction(widgetAction);
	});
#endif

	m_menusInitialized = true;
}

void CartesianPlot::fillAddNewPlotMenu(QMenu* addNewPlotMenu, QActionGroup* actionGroup) {
	// line plots
	auto* menu = new QMenu(i18n("Line Plots"), addNewPlotMenu);

	auto* action = new QAction(XYCurve::staticIcon(XYCurve::PlotType::Line), i18n("Line"), actionGroup);
	action->setData(static_cast<int>(Plot::PlotType::Line));
	menu->addAction(action);

	action = new QAction(XYCurve::staticIcon(XYCurve::PlotType::LineHorizontalStep), i18n("Horizontal Step"), actionGroup);
	action->setData(static_cast<int>(Plot::PlotType::LineHorizontalStep));
	menu->addAction(action);

	action = new QAction(XYCurve::staticIcon(XYCurve::PlotType::LineVerticalStep), i18n("Vertical Step"), actionGroup);
	action->setData(static_cast<int>(Plot::PlotType::LineVerticalStep));
	menu->addAction(action);

	action = new QAction(XYCurve::staticIcon(XYCurve::PlotType::LineSpline), i18n("Spline Interpolated"), actionGroup);
	action->setData(static_cast<int>(Plot::PlotType::LineSpline));
	menu->addAction(action);

	addNewPlotMenu->addMenu(menu);

	// scatter plots
	menu = new QMenu(i18n("Scatter Plots"), addNewPlotMenu);

	action = new QAction(XYCurve::staticIcon(XYCurve::PlotType::Scatter), i18n("Scatter"), actionGroup);
	action->setData(static_cast<int>(Plot::PlotType::Scatter));
	menu->addAction(action);

	action = new QAction(XYCurve::staticIcon(XYCurve::PlotType::ScatterYError), i18n("Y Error"), actionGroup);
	action->setData(static_cast<int>(Plot::PlotType::ScatterYError));
	menu->addAction(action);

	action = new QAction(XYCurve::staticIcon(XYCurve::PlotType::ScatterXYError), i18n("XY Error"), actionGroup);
	action->setData(static_cast<int>(XYCurve::PlotType::ScatterXYError));
	menu->addAction(action);

	addNewPlotMenu->addMenu(menu);

	// line + symbol plots
	menu = new QMenu(i18n("Line and Symbol Plots"), addNewPlotMenu);

	action = new QAction(XYCurve::staticIcon(XYCurve::PlotType::LineSymbol), i18n("Line + Symbol"), actionGroup);
	action->setData(static_cast<int>(Plot::PlotType::LineSymbol));
	menu->addAction(action);

	action = new QAction(XYCurve::staticIcon(XYCurve::PlotType::LineSymbol2PointSegment), i18n("2 Point Segment"), actionGroup);
	action->setData(static_cast<int>(Plot::PlotType::LineSymbol2PointSegment));
	menu->addAction(action);

	action = new QAction(XYCurve::staticIcon(XYCurve::PlotType::LineSymbol3PointSegment), i18n("3 Point Segment"), actionGroup);
	action->setData(static_cast<int>(Plot::PlotType::LineSymbol3PointSegment));
	menu->addAction(action);
	addNewPlotMenu->addMenu(menu);

	// statistical plots
	menu->addSeparator();
	auto* addNewStatisticalPlotsMenu = new QMenu(i18n("Statistical Plots"), addNewPlotMenu);

	action = new QAction(QIcon::fromTheme(QStringLiteral("view-object-histogram-linear")), i18n("Histogram"), actionGroup);
	action->setData(static_cast<int>(Plot::PlotType::Histogram));
	addNewStatisticalPlotsMenu->addAction(action);

	action = new QAction(BoxPlot::staticIcon(), i18n("Box Plot"), actionGroup);
	action->setData(static_cast<int>(Plot::PlotType::BoxPlot));
	addNewStatisticalPlotsMenu->addAction(action);

	action = new QAction(i18n("Q-Q Plot"), actionGroup);
	action->setData(static_cast<int>(Plot::PlotType::QQPlot));
	addNewStatisticalPlotsMenu->addAction(action);

	action = new QAction(i18n("KDE Plot"), actionGroup);
	action->setData(static_cast<int>(Plot::PlotType::KDEPlot));
	addNewStatisticalPlotsMenu->addAction(action);

	addNewPlotMenu->addMenu(addNewStatisticalPlotsMenu);

	// // bar plots
	auto* addNewBarPlotsMenu = new QMenu(i18n("Bar Plots"), addNewPlotMenu);

	action = new QAction(QIcon::fromTheme(QStringLiteral("office-chart-bar")), i18n("Bar Plot"), actionGroup);
	action->setData(static_cast<int>(Plot::PlotType::BarPlot));
	addNewBarPlotsMenu->addAction(action);

	action = new QAction(LollipopPlot::staticIcon(), i18n("Lollipop Plot"), actionGroup);
	action->setData(static_cast<int>(Plot::PlotType::LollipopPlot));
	addNewBarPlotsMenu->addAction(action);

	addNewPlotMenu->addMenu(addNewBarPlotsMenu);

	// continuous improvement plots
	auto* addNewCIPlotsMenu = new QMenu(i18n("Continual Improvement Plots"), addNewPlotMenu);

	action = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-curve")), i18n("Run Chart"), actionGroup);
	action->setData(static_cast<int>(Plot::PlotType::RunChart));
	addNewCIPlotsMenu->addAction(action);

	action = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-curve")), i18n("Process Behavior Chart"), actionGroup);
	action->setData(static_cast<int>(Plot::PlotType::ProcessBehaviorChart));
	addNewCIPlotsMenu->addAction(action);

	addNewPlotMenu->addMenu(addNewCIPlotsMenu);
}

QMenu* CartesianPlot::createContextMenu() {
	if (!m_menusInitialized)
		initMenus();

	QMenu* menu = WorksheetElement::createContextMenu();
	// seems to be a bug, because the tooltips are not shown
	menu->setToolTipsVisible(true);
	QAction* visibilityAction = this->visibilityAction();

	menu->insertMenu(visibilityAction, m_addNewMenu);
	menu->insertSeparator(visibilityAction);
	menu->insertMenu(visibilityAction, themeMenu);
	menu->insertSeparator(visibilityAction);

	if (children<XYCurve>().isEmpty()) {
		addInfoElementAction->setEnabled(false);
		addInfoElementAction->setToolTip(i18n("No curves inside the plot area."));
	} else {
		addInfoElementAction->setEnabled(true);
		addInfoElementAction->setToolTip(QString());
	}

	return menu;
}

QMenu* CartesianPlot::addNewMenu() {
	if (!m_menusInitialized)
		initMenus();

	return m_addNewMenu;
}

QMenu* CartesianPlot::analysisMenu() {
	if (!m_menusInitialized)
		initMenus();

	return dataAnalysisMenu;
}

int CartesianPlot::cSystemIndex(WorksheetElement* e) {
	if (!e)
		return -1;

	auto type = e->type();
	if (type == AspectType::CartesianPlot)
		return -1;
	else if (dynamic_cast<Plot*>(e) || e->coordinateBindingEnabled() || type == AspectType::Axis)
		return e->coordinateSystemIndex();
	return -1;
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

	// TODO: handle other plot types
	for (const auto* curve : children<XYCurve>()) {
		if (curve->xColumn() && curve->xColumn()->parentAspect()->type() == AspectType::Spreadsheet)
			aspects << curve->xColumn()->parentAspect();

		if (curve->yColumn() && curve->yColumn()->parentAspect()->type() == AspectType::Spreadsheet)
			aspects << curve->yColumn()->parentAspect();
	}

	return aspects;
}

QVector<AspectType> CartesianPlot::pasteTypes() const {
	QVector<AspectType> types{AspectType::CartesianPlot,
							  AspectType::XYCurve,
							  AspectType::Histogram,
							  AspectType::BarPlot,
							  AspectType::LollipopPlot,
							  AspectType::BoxPlot,
							  AspectType::KDEPlot,
							  AspectType::QQPlot,
							  AspectType::RunChart,
							  AspectType::ProcessBehaviorChart,
							  AspectType::Axis,
							  AspectType::XYEquationCurve,
							  AspectType::XYFunctionCurve,
							  AspectType::XYConvolutionCurve,
							  AspectType::XYCorrelationCurve,
							  AspectType::XYLineSimplificationCurve,
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
					scaleAuto(Dimension::X, cSystem->index(Dimension::X));

				if (!autoScale(Dimension::Y, cSystem->index(Dimension::Y)))
					enableAutoScale(Dimension::Y, cSystem->index(Dimension::Y), true, true);
				else
					scaleAuto(Dimension::Y, cSystem->index(Dimension::Y));
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
			update |= scaleAuto(Dimension::X, xIndex);
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
			update |= scaleAuto(Dimension::Y, yIndex);
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
	for (const auto* column : std::as_const(columns)) {
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
	for (const auto* column : std::as_const(columns)) {
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

bool CartesianPlot::isPrinted() const {
	Q_D(const CartesianPlot);
	return d->m_printing;
}

bool CartesianPlot::isSelected() const {
	Q_D(const CartesianPlot);
	return d->isSelected();
}

// ##############################################################################
// ################################  getter methods  ############################
// ##############################################################################
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

BASIC_SHARED_D_READER_IMPL(CartesianPlot, CartesianPlot::PlotColorMode, plotColorMode, plotColorMode)
BASIC_SHARED_D_READER_IMPL(CartesianPlot, QString, theme, theme)
BASIC_SHARED_D_READER_IMPL(CartesianPlot, QString, plotColorMap, plotColorMap)

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

// ##############################################################################
// ######################  setter methods and undo commands  ####################
// ##############################################################################
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
		if (m_initialized) {
			qSwap(m_private->rect, m_rect);
			m_private->retransform();
			Q_EMIT m_private->q->rectChanged(m_private->rect);
		} else {
			// this function is called for the first time,
			// nothing to do, we just need to remember what the previous rect was
			// which has happened already in the constructor.
			m_initialized = true;
		}
	}

	void undo() override {
		redo();
	}

private:
	CartesianPlotPrivate* m_private;
	QRectF m_rect;
	bool m_initialized{false};
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
		setProjectChanged(true);
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
	if (index < -1 || index >= rangeCount(dim)) {
		DEBUG(Q_FUNC_INFO << QStringLiteral("Warning: Invalid index: ").arg(index).toStdString());
		return;
	}

	if (index == -1) { // all x ranges
		for (int i = 0; i < rangeCount(dim); i++)
			enableAutoScale(dim, i, enable, fullRange);
		return;
	}

	if (enable != range(dim, index).autoScale()) {
		DEBUG(Q_FUNC_INFO << ", x range " << index << " enable auto scale: " << enable)
		// TODO: maybe using the first and then adding the first one as parent to the next undo command
		exec(new CartesianPlotEnableAutoScaleIndexCmd(d, dim, enable, index, fullRange));
		setProjectChanged(true);
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
		m_target->setRangeDirty(m_dimension, m_index, true);
		const auto& tmp = m_target->rangeConst(m_dimension, m_index);
		m_target->setRange(m_dimension, m_index, m_otherValue);
		m_otherValue = tmp;
		finalize();
	}
	void undo() override {
		redo();
	}
	virtual void finalize() {
		m_target->retransformScale(m_dimension, m_index, true);
		Dimension dim_other = Dimension::Y;
		if (m_dimension == Dimension::Y)
			dim_other = Dimension::X;

		QVector<int> scaledIndices;
		for (int i = 0; i < m_target->q->coordinateSystemCount(); i++) {
			auto cs = m_target->q->coordinateSystem(i);
			auto index_other = cs->index(dim_other);
			if (cs->index(m_dimension) == m_index && scaledIndices.indexOf(index_other) == -1) {
				scaledIndices << index_other;
				if (m_target->q->autoScale(dim_other, index_other) && m_target->q->scaleAuto(dim_other, index_other, false))
					m_target->retransformScale(dim_other, index_other);
			}
		}
		m_target->q->WorksheetElementContainer::retransform();
		Q_EMIT m_target->q->rangeChanged(m_dimension, m_index, m_target->rangeConst(m_dimension, m_index));
	}

private:
	CartesianPlot::Private* m_target;
	int m_index;
	Dimension m_dimension;
	Range<double> m_otherValue; // old value in redo, new value in undo
};

void CartesianPlot::setRange(const Dimension dim, const int index, const Range<double>& range) {
	Q_D(CartesianPlot);
	DEBUG(Q_FUNC_INFO << ", range = " << range.toStdString() << ", auto scale = " << range.autoScale())

	if (range.start() == range.end()) {
		// User entered invalid range
		Q_EMIT rangeChanged(dim, index, this->range(dim, index)); // Feedback
		return;
	}

	auto r = range.checkRange();
	if (index >= 0 && index < rangeCount(dim) && r.finite() && r != d->rangeConst(dim, index)) {
		exec(new CartesianPlotSetRangeIndexCmd(d, dim, r, index));
	} else if (index < 0 || index >= rangeCount(dim))
		DEBUG(Q_FUNC_INFO << QStringLiteral("Warning: wrong index: %1").arg(index).toStdString());

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
	if (index >= 0)
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
	if (index >= rangeCount(dim))
		return;
	if (index >= 0)
		d->setRangeDirty(dim, index, dirty);
	else {
		for (int i = 0; i < rangeCount(dim); i++)
			d->setRangeDirty(dim, i, dirty);
	}
}

void CartesianPlot::addXRange() {
	Q_D(CartesianPlot);
	d->xRanges.append(CartesianPlot::Private::RichRange());
	setProjectChanged(true);
}
void CartesianPlot::addYRange() {
	Q_D(CartesianPlot);
	d->yRanges.append(CartesianPlot::Private::RichRange());
	setProjectChanged(true);
}
void CartesianPlot::addXRange(const Range<double>& range) {
	Q_D(CartesianPlot);
	d->xRanges.append(CartesianPlot::Private::RichRange(range));
	setProjectChanged(true);
}
void CartesianPlot::addYRange(const Range<double>& range) {
	Q_D(CartesianPlot);
	d->yRanges.append(CartesianPlot::Private::RichRange(range));
	setProjectChanged(true);
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

	setProjectChanged(true);
}

void CartesianPlot::setMin(const Dimension dim, int index, double value) {
	DEBUG(Q_FUNC_INFO << ", direction: " << CartesianCoordinateSystem::dimensionToString(dim).toStdString() << "value = " << value)
	if (index >= rangeCount(dim))
		return;
	Range<double> r{range(dim, index)};
	r.setStart(value);
	DEBUG(Q_FUNC_INFO << ", new range = " << r.toStdString())
	setRange(dim, index, r);
}

void CartesianPlot::setMax(const Dimension dim, int index, double value) {
	DEBUG(Q_FUNC_INFO << ", direction: " << CartesianCoordinateSystem::dimensionToString(dim).toStdString() << "value = " << value)
	if (index >= rangeCount(dim))
		return;
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

	auto newRange = range(Dimension::X, index);
	newRange.setScale(scale);
	auto r = newRange.checkRange();
	if (index >= 0 && index < rangeCount(dim) && r.finite() && r != d->rangeConst(dim, index)) {
		if (r == newRange) {
			exec(new CartesianPlotSetScaleIndexCmd(d, dim, scale, index));
			if (project())
				setProjectChanged(true);
		} else
			setRange(dim, index, r);
	}
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
		setProjectChanged(true);
}
void CartesianPlot::removeCoordinateSystem(int index) {
	if (index < 0 || index > coordinateSystemCount()) {
		DEBUG(Q_FUNC_INFO << ", index " << index << " out of range")
		return;
	}

	// TODO: deleting cSystem?
	m_coordinateSystems.remove(index);
	if (project())
		setProjectChanged(true);
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

void CartesianPlot::setCoordinateSystemRangeIndex(int cSystemIndex, Dimension dim, int rangeIndex) {
	coordinateSystem(cSystemIndex)->setIndex(dim, rangeIndex);
	retransformScale(dim, rangeIndex);
}

// range breaks

STD_SETTER_CMD_IMPL_S(CartesianPlot, SetXRangeBreakingEnabled, bool, xRangeBreakingEnabled)
void CartesianPlot::setXRangeBreakingEnabled(bool enabled) {
	Q_D(CartesianPlot);
	if (enabled != d->xRangeBreakingEnabled) {
		exec(new CartesianPlotSetXRangeBreakingEnabledCmd(d, enabled, ki18n("%1: x-range breaking enabled")));
		retransformScales(); // TODO: replace by retransformScale(Dimension::X, ) with the corresponding index!
		WorksheetElementContainer::retransform(); // retransformScales does not contain any retransform() anymore
	}
}

STD_SETTER_CMD_IMPL_S(CartesianPlot, SetXRangeBreaks, CartesianPlot::RangeBreaks, xRangeBreaks)
void CartesianPlot::setXRangeBreaks(const RangeBreaks& breakings) {
	Q_D(CartesianPlot);
	exec(new CartesianPlotSetXRangeBreaksCmd(d, breakings, ki18n("%1: x-range breaks changed")));
	retransformScales(); // TODO: replace by retransformScale(Dimension::X, ) with the corresponding index!
	WorksheetElementContainer::retransform(); // retransformScales does not contain any retransform() anymore
}

STD_SETTER_CMD_IMPL_S(CartesianPlot, SetYRangeBreakingEnabled, bool, yRangeBreakingEnabled)
void CartesianPlot::setYRangeBreakingEnabled(bool enabled) {
	Q_D(CartesianPlot);
	if (enabled != d->yRangeBreakingEnabled) {
		exec(new CartesianPlotSetYRangeBreakingEnabledCmd(d, enabled, ki18n("%1: y-range breaking enabled")));
		retransformScales(); // TODO: replace by retransformScale(Dimension::Y, ) with the corresponding index!
		WorksheetElementContainer::retransform(); // retransformScales does not contain any retransform() anymore
	}
}

STD_SETTER_CMD_IMPL_S(CartesianPlot, SetYRangeBreaks, CartesianPlot::RangeBreaks, yRangeBreaks)
void CartesianPlot::setYRangeBreaks(const RangeBreaks& breaks) {
	Q_D(CartesianPlot);
	exec(new CartesianPlotSetYRangeBreaksCmd(d, breaks, ki18n("%1: y-range breaks changed")));
	retransformScales(); // TODO: replace by retransformScale(Dimension::Y, ) with the corresponding index!
	WorksheetElementContainer::retransform(); // retransformScales does not contain any retransform() anymore
}

// cursor
STD_SETTER_CMD_IMPL_F_S(CartesianPlot, SetCursor0Enable, bool, cursor0Enable, updateCursor)
void CartesianPlot::setCursor0Enable(const bool& enable) {
	Q_D(CartesianPlot);
	if (enable != d->cursor0Enable && defaultCoordinateSystem()->isValid()) {
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
	if (enable != d->cursor1Enable && defaultCoordinateSystem()->isValid()) {
		if (std::isnan(d->cursor1Pos.x())) { // if never set, set initial position
			d->cursor1Pos.setX(defaultCoordinateSystem()->mapSceneToLogical(QPointF(0, 0)).x());
			mousePressCursorModeSignal(1, d->cursor1Pos); // simulate mousePress to update values in the cursor dock
		}
		exec(new CartesianPlotSetCursor1EnableCmd(d, enable, ki18n("%1: Cursor1 enable")));
	}
}

// theme and plot colors
STD_SETTER_CMD_IMPL_S(CartesianPlot, SetPlotColorMode, CartesianPlot::PlotColorMode, plotColorMode)
void CartesianPlot::setPlotColorMode(PlotColorMode mode) {
	Q_D(CartesianPlot);
	if (mode != d->plotColorMode) {
		beginMacro(i18n("%1: set plot color mode", name()));
		exec(new CartesianPlotSetPlotColorModeCmd(d, mode, ki18n("%1: set plot color mode")));
		d->updatePlotColorPalette();
		endMacro();
	}
}

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

STD_SETTER_CMD_IMPL_S(CartesianPlot, SetPlotColorMap, QString, plotColorMap)
void CartesianPlot::setPlotColorMap(QString colorMap) {
	Q_D(CartesianPlot);
	if (colorMap != d->plotColorMap) {
		beginMacro(i18n("%1: set plot color map", name()));
		exec(new CartesianPlotSetPlotColorMapCmd(d, colorMap, ki18n("%1: set plot color map")));
		d->updatePlotColorPalette();
		endMacro();
	}
}

void CartesianPlot::retransform() {
	Q_D(CartesianPlot);
	d->retransform();
}

// ################################################################
// ########################## Slots ###############################
// ################################################################
void CartesianPlot::addPlot(QAction* action) {
	const auto type = static_cast<Plot::PlotType>(action->data().toInt());

	switch (type) {
	// basic plots
	case Plot::PlotType::Line: {
		auto* plot = new XYCurve(i18n("Plot"));
		plot->setPlotType(Plot::PlotType::Line);
		addChild(plot);
		break;
	}
	case Plot::PlotType::LineHorizontalStep: {
		auto* plot = new XYCurve(i18n("Plot"));
		plot->setPlotType(Plot::PlotType::LineHorizontalStep);
		addChild(plot);
		break;
	}
	case Plot::PlotType::LineVerticalStep: {
		auto* plot = new XYCurve(i18n("Plot"));
		plot->setPlotType(Plot::PlotType::LineVerticalStep);
		addChild(plot);
		break;
	}
	case Plot::PlotType::LineSpline: {
		auto* plot = new XYCurve(i18n("Plot"));
		plot->setPlotType(Plot::PlotType::LineSpline);
		addChild(plot);
		break;
	}
	case Plot::PlotType::Scatter: {
		auto* plot = new XYCurve(i18n("Plot"));
		plot->setPlotType(Plot::PlotType::Scatter);
		addChild(plot);
		break;
	}
	case Plot::PlotType::ScatterYError: {
		auto* plot = new XYCurve(i18n("Plot"));
		plot->setPlotType(Plot::PlotType::ScatterYError);
		addChild(plot);
		break;
	}
	case Plot::PlotType::ScatterXYError: {
		auto* plot = new XYCurve(i18n("Plot"));
		plot->setPlotType(Plot::PlotType::ScatterXYError);
		addChild(plot);
		break;
	}
	case Plot::PlotType::LineSymbol: {
		auto* plot = new XYCurve(i18n("Plot"));
		plot->setPlotType(Plot::PlotType::LineSymbol);
		addChild(plot);
		break;
	}
	case Plot::PlotType::LineSymbol2PointSegment: {
		auto* plot = new XYCurve(i18n("Plot"));
		plot->setPlotType(Plot::PlotType::LineSymbol2PointSegment);
		addChild(plot);
		break;
	}
	case Plot::PlotType::LineSymbol3PointSegment: {
		auto* plot = new XYCurve(i18n("Plot"));
		plot->setPlotType(Plot::PlotType::LineSymbol3PointSegment);
		addChild(plot);
		break;
	}

	// formula
	case Plot::PlotType::Formula:
		addChild(new XYEquationCurve(QStringLiteral("f(x)")));
		break;

	// statistical plots
	case Plot::PlotType::BoxPlot:
		addChild(new BoxPlot(i18n("Box Plot")));
		break;
	case Plot::PlotType::Histogram:
		addChild(new Histogram(i18n("Histogram")));
		break;
	case Plot::PlotType::QQPlot:
		addChild(new QQPlot(i18n("Q-Q Plot")));
		break;
	case Plot::PlotType::KDEPlot:
		addChild(new KDEPlot(i18n("KDE Plot")));
		break;

	// bar plots
	case Plot::PlotType::BarPlot:
		addChild(new BarPlot(i18n("Bar Plot")));
		break;
	case Plot::PlotType::LollipopPlot:
		addChild(new LollipopPlot(i18n("Lollipop Plot")));
		break;

	// continuous improvement plots
	case Plot::PlotType::ProcessBehaviorChart:
		addChild(new ProcessBehaviorChart(i18n("Process Behavior Chart")));
		break;
	case Plot::PlotType::RunChart:
		addChild(new RunChart(i18n("Run Chart")));
		break;
	}
}

/*!
 * returns the first horizontal axis on the plot area,
 * usually used to set the axis title for the plot area and in case
 * multiple axes of the same orientation are present.
 */
Axis* CartesianPlot::horizontalAxis() const {
	const auto& axes = children(AspectType::Axis);
	for (auto a : axes) {
		auto axis = static_cast<Axis*>(a);
		if (axis->orientation() == Axis::Orientation::Horizontal)
			return axis;
	}

	return nullptr;
}

/*!
 * returns the first horizontal axis on the plot area,
 * usually used to set the axis title for the plot area and in case
 * multiple axes of the same orientation are present.
 */
Axis* CartesianPlot::verticalAxis() const {
	const auto& axes = children(AspectType::Axis);
	for (auto a : axes) {
		auto axis = static_cast<Axis*>(a);
		if (axis->orientation() == Axis::Orientation::Vertical)
			return axis;
	}

	return nullptr;
}

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

void CartesianPlot::addHistogramFit(Histogram* hist, nsl_sf_stats_distribution type) {
	if (!hist)
		return;

	beginMacro(i18n("%1: distribution fit to '%2'", name(), hist->name()));
	auto* curve = new XYFitCurve(i18n("Distribution Fit to '%1'", hist->name()));
	// curve->setCoordinateSystemIndex(defaultCoordinateSystemIndex());
	curve->setDataSourceType(XYAnalysisCurve::DataSourceType::Histogram);
	curve->setDataSourceHistogram(hist);

	// set fit model category and type and initialize fit
	XYFitCurve::FitData fitData = curve->fitData();
	fitData.modelCategory = nsl_fit_model_distribution;
	fitData.modelType = (int)type;
	DEBUG("TYPE = " << type)
	fitData.algorithm = nsl_fit_algorithm_ml; // ML distribution fit
	DEBUG("INITFITDATA:")
	XYFitCurve::initFitData(fitData);
	DEBUG("SETFITDATA:")
	curve->setFitData(fitData);

	DEBUG("RECALCULATE:")
	curve->recalculate();

	// add the child after the fit was calculated so the dock widgets gets the fit results
	// and call retransform() after this to calculate and to paint the data points of the fit-curve
	DEBUG("ADDCHILD:")
	this->addChild(curve);
	DEBUG("RETRANSFORM:")
	curve->retransform();
	DEBUG("DONE:")

	endMacro();
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

void CartesianPlot::addLineSimplificationCurve() {
	auto* curve = new XYLineSimplificationCurve(i18n("Line Simplification"));
	const XYCurve* curCurve = currentCurve();
	if (curCurve) {
		beginMacro(i18n("%1: simplify '%2'", name(), curCurve->name()));
		curve->setName(i18n("Simplification of '%1'", curCurve->name()));
		curve->setDataSourceType(XYAnalysisCurve::DataSourceType::Curve);
		curve->setDataSourceCurve(curCurve);
		this->addChild(curve);
		curve->recalculate();
		Q_EMIT curve->lineSimplificationDataChanged(curve->lineSimplificationData());
	} else {
		beginMacro(i18n("%1: add line simplification curve", name()));
		this->addChild(curve);
	}

	endMacro();
}

void CartesianPlot::addDifferentiationCurve() {
	auto* curve = new XYDifferentiationCurve(i18n("Differentiation"));
	const XYCurve* curCurve = currentCurve();
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
	auto* curve = new XYIntegrationCurve(i18n("Integration"));
	const XYCurve* curCurve = currentCurve();
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
	auto* curve = new XYInterpolationCurve(i18n("Interpolation"));
	const XYCurve* curCurve = currentCurve();
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
	auto* curve = new XYSmoothCurve(i18n("Smooth"));
	const XYCurve* curCurve = currentCurve();
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

void CartesianPlot::addBaselineCorrectionCurve() {
	auto* curve = new XYBaselineCorrectionCurve(i18n("Baseline Correction"));
	const XYCurve* curCurve = currentCurve();
	if (curCurve) {
		beginMacro(i18n("%1: baseline correction for '%2'", name(), curCurve->name()));
		curve->setName(i18n("Baseline correction for '%1'", curCurve->name()));
		curve->setDataSourceType(XYAnalysisCurve::DataSourceType::Curve);
		curve->setDataSourceCurve(curCurve);
		this->addChild(curve);
		curve->recalculate();
		// Q_EMIT curve->baselineCorrectionDataChanged(curve->baselineCorrectionData());
	} else {
		beginMacro(i18n("%1: add baseline correction curve", name()));
		this->addChild(curve);
	}

	endMacro();
}

void CartesianPlot::addFitCurve() {
	auto* curve = new XYFitCurve(i18nc("Curve fitting", "Fit"));
	const auto* curCurve = currentCurve();
	if (curCurve) {
		beginMacro(i18n("%1: fit to '%2'", name(), curCurve->name()));
		curve->setName(i18nc("Curve fitting", "Fit to '%1'", curCurve->name()));
		curve->setDataSourceType(XYAnalysisCurve::DataSourceType::Curve);
		curve->setDataSourceCurve(curCurve);

		// set the fit model category and type
		const auto* action = qobject_cast<const QAction*>(QObject::sender());
		if (action) {
			auto type = static_cast<XYAnalysisCurve::AnalysisAction>(action->data().toInt());
			curve->initFitData(type);
		} else
			DEBUG(Q_FUNC_INFO << "WARNING: no action found!")

		// fit with weights for y if the curve has error bars for y
		if (curCurve->errorBar()->yErrorType() == ErrorBar::ErrorType::Symmetric && curCurve->errorBar()->yPlusColumn()) {
			auto fitData = curve->fitData();
			fitData.yWeightsType = nsl_fit_weight_instrumental;
			curve->setFitData(fitData);
			curve->errorBar()->setYPlusColumn(curCurve->errorBar()->yPlusColumn());
		}

		curve->recalculate();

		// add the child after the fit was calculated so the dock widgets gets the fit results
		// and call retransform() after this to calculate and to paint the data points of the fit-curve
		this->addChild(curve);
		curve->retransform();
	} else {
		beginMacro(i18n("%1: add fit curve", name()));
		curve->initFitData(XYAnalysisCurve::AnalysisAction::FitLinear); // TODO: should happen directly in the constructor of XYFitCurve
		this->addChild(curve);
	}

	endMacro();
}

void CartesianPlot::addFourierFilterCurve() {
	auto* curve = new XYFourierFilterCurve(i18n("Fourier Filter"));
	const XYCurve* curCurve = currentCurve();
	if (curCurve) {
		beginMacro(i18n("%1: Fourier filtering of '%2'", name(), curCurve->name()));
		curve->setName(i18n("Fourier filtering of '%1'", curCurve->name()));
		curve->setDataSourceType(XYAnalysisCurve::DataSourceType::Curve);
		curve->setDataSourceCurve(curCurve);
	} else {
		beginMacro(i18n("%1: add Fourier filter curve", name()));
	}
	this->addChild(curve);

	endMacro();
}

void CartesianPlot::addFunctionCurve() {
	auto* curve = new XYFunctionCurve(i18n("Function"));
	const auto* curCurve = currentCurve();
	if (curCurve) {
		beginMacro(i18n("%1: add function of '%2'", name(), curCurve->name()));
		curve->setName(i18n("Function of '%1'", curCurve->name()));
		curve->setFunction(QString(), {QStringLiteral("x")}, {curCurve});
	} else
		beginMacro(i18n("%1: add function curve", name()));

	this->addChild(curve);
	endMacro();
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

	m_legend = new CartesianPlotLegend(i18n("Legend"));
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

	auto* element = new InfoElement(i18n("Info Element"), this, curve, pos);
	this->addChild(element);
	element->setParentGraphicsItem(graphicsItem());
	element->retransform(); // must be done, because the element must be retransformed (see https://invent.kde.org/marmsoler/labplot/issues/9)
}

void CartesianPlot::addTextLabel() {
	auto* label = new TextLabel(i18n("Text Label"), this);

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
	auto* image = new Image(i18n("Image"));

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
	auto* point = new CustomPoint(this, i18n("Custom Point"));
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
		p.point = QPointF(0, 0); // Exactly in the middle of the plot in scene coordinates
		point->setPosition(p);
		point->setCoordinateBindingEnabled(true);
	}

	endMacro();
	point->retransform();
}

void CartesianPlot::addReferenceLine() {
	Q_D(CartesianPlot);
	auto* line = new ReferenceLine(this, i18n("Reference Line"));
	line->setCoordinateSystemIndex(defaultCoordinateSystemIndex());

	if (d->calledFromContextMenu) {
		line->setPositionLogical(d->logicalPos);
		d->calledFromContextMenu = false;
	}

	this->addChild(line);
	line->retransform();
}

/*!
 * adds an empty default plot area as a child ("inset plot"),
 * the child only inherits the type (four axes, etc.)
 * and the theme of the parent plot area.
 */
void CartesianPlot::addInsetPlot() {
	beginMacro(i18n("%1: add inset plot", name()));
	auto* insetPlot = new CartesianPlot(i18n("Inset Plot Area"));
	Q_D(const CartesianPlot);
	insetPlot->setType(d->type);
	resizeInsetPlot(insetPlot);
	addChild(insetPlot);
	endMacro();
}

/*!
 * adds a copy of the plot area as a child to it, the child inherits all properties
 * of the parent plot area including the visualization of the data
 */
void CartesianPlot::addInsetPlotWithData() {
	beginMacro(i18n("%1: add inset plot with data", name()));

	// add a copy of the current plot as a child
	copy();
	paste(true);

	// rename and resize the new child plot
	const auto& plots = children<CartesianPlot>();
	auto* insetPlot = plots.last();
	insetPlot->setName(i18n("Inset Plot Area"));
	resizeInsetPlot(insetPlot);
	endMacro();
}

/*!
 * sets the size of the inset plot to 30% of the parent plot's size
 * and allows to resize it with the mouse (not controlled by worksheet's layout)
 */
void CartesianPlot::resizeInsetPlot(CartesianPlot* insetPlot) {
	auto insetRect = rect();
	insetRect.setWidth(insetRect.width() * 0.3);
	insetRect.setHeight(insetRect.height() * 0.3);
	insetPlot->setRect(insetRect);
	insetPlot->setResizeEnabled(true);
}

void CartesianPlot::addReferenceRange() {
	auto* range = new ReferenceRange(this, i18n("Reference Range"));
	range->setCoordinateSystemIndex(defaultCoordinateSystemIndex());

	// Q_D(CartesianPlot);
	// 	if (d->calledFromContextMenu) {
	// 		range->setPositionLogical(d->logicalPos);
	// 		d->calledFromContextMenu = false;
	// 	}

	this->addChild(range);
	range->retransform();
}

int CartesianPlot::curveCount() const {
	return children<XYCurve>().size();
}

int CartesianPlot::curveTotalCount() const {
	return children<Plot>().size();
}

const XYCurve* CartesianPlot::getCurve(int index) const {
	return children<XYCurve>().at(index);
}

double CartesianPlot::cursorPos(int cursorNumber) const {
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
	const auto& plots = this->children<Plot>();
	for (auto* plot : plots) {
		if (plot == curve)
			break;

		++index;

		// for the process behavior and run charts two colors are used - for the data and for the control line(s)
		if (plot->type() == AspectType::ProcessBehaviorChart || plot->type() == AspectType::RunChart)
			++index;
	}

	return index;
}

void CartesianPlot::childAdded(const AbstractAspect* child) {
	auto* elem = dynamic_cast<const WorksheetElement*>(child);
	if (!elem)
		return;

	Q_D(CartesianPlot);
	int cSystemIndex = defaultCoordinateSystemIndex();
	// check/change ranges when adding new children like curves for example.
	// The ranges must not be checked if just an element like a TextLabel, Custompoint, ... was added
	bool checkRanges = false;
	const auto* plot = dynamic_cast<const Plot*>(child);

	if (plot) {
		connect(plot, &WorksheetElement::visibleChanged, this, &CartesianPlot::curveVisibilityChanged);
		connect(plot, &WorksheetElement::aspectDescriptionChanged, this, &CartesianPlot::updateLegend);
		connect(plot, &Plot::legendVisibleChanged, this, &CartesianPlot::updateLegend);
		connect(plot, &Plot::appearanceChanged, this, &CartesianPlot::updateLegend);
		connect(plot, &Plot::appearanceChanged, this, QOverload<>::of(&CartesianPlot::plotColorChanged)); // forward to Worksheet to update CursorDock

		connect(plot, &Plot::dataChanged, [this, elem] {
			this->dataChanged(const_cast<WorksheetElement*>(elem));
		});

		// don't set the default coordinate system index during the project load,
		// the index is read in child's load().
		// same for range and legend updates - settings are read in load().
		if (!isLoading()) {
			const_cast<Plot*>(plot)->setCoordinateSystemIndex(cSystemIndex);
			checkRanges = true;
			updateLegend();
		}
	} else {
		// hover events for plots are handled here in CartesianPlot, for other elements in WorksheetElement.
		// in case a non-plot element like axis, etc. was hovered, unhover the plots
		connect(elem, &WorksheetElement::hoveredChanged, [=](bool on) {
			if (on) {
				for (auto* plot : children<Plot>())
					plot->setHover(false);
			}
		});
	}

	const auto* curve = dynamic_cast<const XYCurve*>(child);
	const auto* hist = dynamic_cast<const Histogram*>(child);
	const auto* boxPlot = dynamic_cast<const BoxPlot*>(child);
	const auto* barPlot = dynamic_cast<const BarPlot*>(child);
	const auto* lollipopPlot = dynamic_cast<const LollipopPlot*>(child);

	const auto* axis = dynamic_cast<const Axis*>(child);

	if (curve) {
		DEBUG(Q_FUNC_INFO << ", CURVE")
		// x data
		connect(curve, &XYCurve::xColumnChanged, this, [this, curve](const AbstractColumn* column) {
			if (curveTotalCount() == 1) // first curve added
				checkAxisFormat(curve->coordinateSystemIndex(), column, Axis::Orientation::Horizontal);
		});
		connect(curve, &XYCurve::xDataChanged, [this, curve]() {
			this->dataChanged(const_cast<XYCurve*>(curve), Dimension::X);
		});
		connect(curve->errorBar(), &ErrorBar::xErrorTypeChanged, [this, curve]() {
			this->dataChanged(const_cast<XYCurve*>(curve), Dimension::X);
		});
		connect(curve->errorBar(), &ErrorBar::xPlusColumnChanged, [this, curve]() {
			this->dataChanged(const_cast<XYCurve*>(curve), Dimension::X);
		});
		connect(curve->errorBar(), &ErrorBar::xMinusColumnChanged, [this, curve]() {
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
		connect(curve->errorBar(), &ErrorBar::yErrorTypeChanged, [this, curve]() {
			this->dataChanged(const_cast<XYCurve*>(curve), Dimension::Y);
		});
		connect(curve->errorBar(), &ErrorBar::yPlusColumnChanged, [this, curve]() {
			this->dataChanged(const_cast<XYCurve*>(curve), Dimension::Y);
		});
		connect(curve->errorBar(), &ErrorBar::yMinusColumnChanged, [this, curve]() {
			this->dataChanged(const_cast<XYCurve*>(curve), Dimension::Y);
		});

		// update the legend on line and symbol properties changes
		connect(curve, &XYCurve::aspectDescriptionChanged, this, &CartesianPlot::curveNameChanged);
		connect(curve, &XYCurve::lineTypeChanged, this, &CartesianPlot::updateLegend);

		// in case the first curve is added, check whether we start plotting datetime data
		if (!isLoading() && curveTotalCount() == 1) {
			checkAxisFormat(curve->coordinateSystemIndex(), curve->xColumn(), Axis::Orientation::Horizontal);
			checkAxisFormat(curve->coordinateSystemIndex(), curve->yColumn(), Axis::Orientation::Vertical);
		}

		Q_EMIT curveAdded(curve);
	} else if (hist) {
		DEBUG(Q_FUNC_INFO << ", HISTOGRAM")
		if (!isLoading() && curveTotalCount() == 1)
			checkAxisFormat(hist->coordinateSystemIndex(), hist->dataColumn(), Axis::Orientation::Horizontal);
	} else if (boxPlot) {
		DEBUG(Q_FUNC_INFO << ", BOX PLOT")
		if (curveTotalCount() == 1) {
			connect(boxPlot, &BoxPlot::orientationChanged, this, &CartesianPlot::boxPlotOrientationChanged);
			if (!isLoading()) {
				boxPlotOrientationChanged(boxPlot->orientation());
				if (!boxPlot->dataColumns().isEmpty())
					checkAxisFormat(boxPlot->coordinateSystemIndex(), boxPlot->dataColumns().constFirst(), Axis::Orientation::Vertical);
			}
		}
	} else if (barPlot) {
		DEBUG(Q_FUNC_INFO << ", BAR PLOT")
		connect(barPlot, &BarPlot::dataColumnsChanged, this, &CartesianPlot::updateLegend);
		connect(barPlot, &BarPlot::xDataChanged, [this, barPlot]() {
			this->dataChanged(const_cast<BarPlot*>(barPlot), Dimension::X);
		});
	} else if (lollipopPlot) {
		DEBUG(Q_FUNC_INFO << ", LOLLIPOP PLOT")
		connect(lollipopPlot, &LollipopPlot::dataColumnsChanged, this, &CartesianPlot::updateLegend);
		connect(lollipopPlot, &LollipopPlot::xDataChanged, [this, lollipopPlot]() {
			this->dataChanged(const_cast<LollipopPlot*>(lollipopPlot), Dimension::X);
		});
	} else if (axis) {
		connect(axis, &Axis::shiftSignal, this, &CartesianPlot::axisShiftSignal);
	} else {
		// if an element is hovered, the curves which are handled manually in this class
		// must be unhovered
		if (elem)
			connect(elem, &WorksheetElement::hovered, this, &CartesianPlot::childHovered);
	}

	if (isLoading())
		return;

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

	if (!isLoading() && !isPasted() && !child->isPasted() && !child->isMoved()) {
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
		// no need to put these changes onto the undo stack, temporarily deactivate the undo awareness
		// for the project globally so it's ignored for all elements below when applying the theme recursively.
		if (project())
			project()->setUndoAware(false);

		if (!d->theme.isEmpty()) {
			KConfig config(ThemeHandler::themeFilePath(d->theme), KConfig::SimpleConfig);
			const_cast<WorksheetElement*>(elem)->loadThemeConfig(config);
		} else {
			KConfig config;
			const_cast<WorksheetElement*>(elem)->loadThemeConfig(config);
		}

		if (project())
			project()->setUndoAware(true);
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
		if (isHovered())
			setHover(false);
		else
			d->update(); // really needed?
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
			// no sender available, the function was called directly in the file filter (live data source got new data) or in Project::load()-.
			// -> recalculate the internal structures in all plots and retransform them since we don't know which plots are affected.
			// TODO: this logic can be very expensive
			for (auto* plot : children<Plot>()) {
				plot->recalc();
				plot->retransform();
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

	const auto xIndex = coordinateSystem(cSystemIndex)->index(Dimension::X);
	const auto yIndex = coordinateSystem(cSystemIndex)->index(Dimension::Y);

	dataChanged(xIndex, yIndex, element);
}

/*!
	called when in one of the curves the data in one direction was changed.
	Autoscales the coordinate system and the x/y-axes, when "auto-scale" is active.
*/
void CartesianPlot::dataChanged(Plot* curve, const Dimension dim) {
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

	auto* c = dynamic_cast<XYCurve*>(curve);
	if (c) {
		// in case there is only one curve and its column mode was changed, check whether we start plotting datetime data
		if (children<XYCurve>().size() == 1) {
			const auto* col = c->column(dim);
			const auto rangeFormat{range(dim, index).format()};
			if (col && col->columnMode() == AbstractColumn::ColumnMode::DateTime && rangeFormat != RangeT::Format::DateTime) {
				setUndoAware(false);
				setRangeFormat(dim, index, RangeT::Format::DateTime);
				setUndoAware(true);
			}
		}
		Q_EMIT curveDataChanged(c);
	}
}

void CartesianPlot::curveVisibilityChanged() {
	const int index = static_cast<WorksheetElement*>(QObject::sender())->coordinateSystemIndex();
	const int xIndex = coordinateSystem(index)->index(Dimension::X);
	const int yIndex = coordinateSystem(index)->index(Dimension::Y);
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

void CartesianPlot::plotColorChanged() {
	const auto* curve = qobject_cast<const Plot*>(QObject::sender());
	Q_EMIT plotColorChanged(curve->color(), curve->name());
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

BASIC_SHARED_D_ACCESSOR_IMPL(CartesianPlot, bool, isInteractive, Interactive, interactive)

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
				if (cs && cs->index(dim) == index) {
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
	auto dataRange = d->dataRange(dim, index); // dataRange used for nice extend
	if (dataRange.finite() && d->niceExtend)
		dataRange.niceExtend(); // auto scale to nice range

	// if no curve: do not reset to [0, 1]

	DEBUG(Q_FUNC_INFO << ", range " << index << " = " << r.toStdString() << "., data range = " << d->dataRange(dim, index).toStdString())
	bool update = false;
	if (!qFuzzyCompare(dataRange.start(), r.start()) && std::isfinite(dataRange.start())) {
		r.start() = dataRange.start();
		update = true;
	}

	if (!qFuzzyCompare(dataRange.end(), r.end()) && std::isfinite(dataRange.end())) {
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
			const double value = r.start();
			if (!qFuzzyIsNull(value))
				r.setRange(value * 0.9, value * 1.1);
			else
				r.setRange(-0.1, 0.1);
		} else
			r.extend(r.size() * d->autoScaleOffsetFactor);

		Q_EMIT rangeChanged(dim, index, r);

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
 * Calculates and saves data range.
 */
void CartesianPlot::calculateDataRange(const Dimension dim, const int index, bool completeRange) {
	DEBUG(Q_FUNC_INFO << ", direction = " << CartesianCoordinateSystem::dimensionToString(dim).toStdString() << ", index = " << index
					  << ", complete range = " << completeRange)
	Q_D(CartesianPlot);

	d->dataRange(dim, index).setRange(INFINITY, -INFINITY);
	auto range{d->range(dim, index)}; // get reference to range from private

	// loop over all curves/plots and determine the maximum and minimum values
	for (const auto* plot : this->children<const Plot>()) {
		if (!plot->isVisible() || !plot->hasData())
			continue;

		if (coordinateSystem(plot->coordinateSystemIndex())->index(dim) != index)
			continue;

		if (dynamic_cast<const XYCurve*>(plot)) {
			auto* curve = static_cast<const XYCurve*>(plot);
			DEBUG("CURVE \"" << STDSTRING(curve->name()) << "\"")
			auto* column = curve->column(dim);
			if (!column) {
				DEBUG(" NO column!")
				continue;
			}

			// range of indices
			Range<int> indexRange{0, 0};
			if (!completeRange && d->rangeType == RangeType::Free) {
				Dimension dim_other = Dimension::Y;
				switch (dim) {
				case Dimension::X:
					break;
				case Dimension::Y:
					dim_other = Dimension::X;
					break;
				}

				if (curve->column(dim_other)) { // only data within y range
					const int index = coordinateSystem(curve->coordinateSystemIndex())->index(dim_other);
					DEBUG(Q_FUNC_INFO << ", free incomplete range with y column. y range = " << d->range(dim_other, index).toStdString())
					curve->column(dim_other)->indicesMinMax(d->range(dim_other, index).start(),
															d->range(dim_other, index).end(),
															indexRange.start(),
															indexRange.end());
				}
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
		} else if (plot->type() == AspectType::KDEPlot) {
			const int minIndex = 0;
			const int maxIndex = static_cast<const KDEPlot*>(plot)->gridPointsCount() - 1;
			Range<int> indexRange{minIndex, maxIndex};
			plot->minMax(dim, indexRange, range, true);
		} else if (plot->type() == AspectType::QQPlot) {
			Range<int> indexRange{0, 99}; // 100 percentile values are calculated, max index is 99
			plot->minMax(dim, indexRange, range, true);
		} else if (plot->type() == AspectType::ProcessBehaviorChart) {
			const int maxIndex = static_cast<const ProcessBehaviorChart*>(plot)->xIndexCount() - 1;
			Range<int> indexRange{0, maxIndex};
			plot->minMax(dim, indexRange, range, true);
		} else if (plot->type() == AspectType::RunChart) {
			const int maxIndex = static_cast<const RunChart*>(plot)->xIndexCount() - 1;
			Range<int> indexRange{0, maxIndex};
			plot->minMax(dim, indexRange, range, true);
		} else {
			range.setStart(plot->minimum(dim));
			range.setEnd(plot->maximum(dim));
		}

		// check ranges for nonlinear scales
		if (range.scale() != RangeT::Scale::Linear)
			range = range.checkRange();

		if (range.start() < d->dataRange(dim, index).start())
			d->dataRange(dim, index).start() = range.start();

		if (range.end() > d->dataRange(dim, index).end())
			d->dataRange(dim, index).end() = range.end();

		DEBUG(Q_FUNC_INFO << ", plot's range i = " << d->dataRange(dim, index).toStdString(false))
	}

	// data range is used to nice extend, so set correct scale
	d->dataRange(dim, index).setScale(range.scale());

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

void CartesianPlot::zoomIn(int xIndex, int yIndex, const QPointF& sceneRelPos) {
	setUndoAware(false);
	enableAutoScale(Dimension::X, xIndex, false);
	enableAutoScale(Dimension::Y, yIndex, false);
	setUndoAware(true);
	setRangeDirty(Dimension::X, xIndex, true);
	setRangeDirty(Dimension::Y, yIndex, true);
	zoom(xIndex, Dimension::X, true, sceneRelPos.x()); // zoom in x
	zoom(yIndex, Dimension::Y, true, sceneRelPos.y()); // zoom in y

	Q_D(CartesianPlot);
	d->retransformScales(xIndex, yIndex);
	WorksheetElementContainer::retransform();
}

void CartesianPlot::zoomOut(int xIndex, int yIndex, const QPointF& sceneRelPos) {
	setUndoAware(false);
	enableAutoScale(Dimension::X, xIndex, false);
	enableAutoScale(Dimension::Y, yIndex, false);
	setUndoAware(true);
	setRangeDirty(Dimension::X, xIndex, true);
	setRangeDirty(Dimension::Y, yIndex, true);
	zoom(xIndex, Dimension::X, false, sceneRelPos.x()); // zoom out x
	zoom(yIndex, Dimension::Y, false, sceneRelPos.y()); // zoom out y

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

void CartesianPlot::zoomInOut(const int index, const Dimension dim, const bool zoomIn, const double relScenePosRange) {
	Dimension dim_other = Dimension::Y;
	if (dim == Dimension::Y)
		dim_other = Dimension::X;

	setUndoAware(false);
	enableAutoScale(dim, index, false);
	setUndoAware(true);
	setRangeDirty(dim_other, index, true);
	zoom(index, dim, zoomIn, relScenePosRange);

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
void CartesianPlot::zoom(int index, const Dimension dim, bool zoom_in, const double relPosSceneRange) {
	Q_D(CartesianPlot);

	Range<double> range;
	if (index == -1) {
		QVector<int> zoomedIndices;
		for (int i = 0; i < m_coordinateSystems.count(); i++) {
			int idx = coordinateSystem(i)->index(dim);
			if (zoomedIndices.contains(idx))
				continue;
			zoom(idx, dim, zoom_in, relPosSceneRange);
			zoomedIndices.append(idx);
		}
		return;
	}
	range = d->range(dim, index);

	double factor = m_zoomFactor;
	if (zoom_in)
		factor = 1. / factor;
	range.zoom(factor, d->niceExtend, relPosSceneRange);

	if (range.finite())
		d->setRange(dim, index, range);
}

/*!
 * helper function called in other shift*() functions
 * and doing the actual change of the data ranges.
 * @param x if set to \true the x-range is modified, the y-range for \c false
 * @param leftOrDown the "shift left" for x or "shift down" for y is performed if set to \c \true,
 * "shift right" or "shift up" for \c false
 */
void CartesianPlot::shift(int index, const Dimension dim, bool leftOrDown) {
	setUndoAware(false);
	enableAutoScale(dim, index, false);
	setUndoAware(true);
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
		range += sqrt(std::abs(offset));
		break;
	case RangeT::Scale::Inverse:
		offset = (1. / start - 1. / end) * factor;
		range += 1. / std::abs(offset);
		break;
	}

	if (range.finite())
		d->setRange(dim, index, range);

	d->retransformScale(dim, index);

	auto dim_other = Dimension::X;
	switch (dim) {
	case Dimension::X:
		dim_other = Dimension::Y;
		break;
	case Dimension::Y:
		dim_other = Dimension::X;
		break;
	}

	bool retrans = false;
	for (const auto cSystem : m_coordinateSystems) {
		const auto cs = static_cast<CartesianCoordinateSystem*>(cSystem);
		if ((index == -1 || index == cs->index(dim))) {
			if (autoScale(dim_other, cs->index(dim_other))) {
				setRangeDirty(dim_other, cs->index(dim_other), true);
				scaleAuto(dim_other, cs->index(dim_other), false);
			}
			retrans = true;
		}
	}

	if (retrans)
		WorksheetElementContainer::retransform();
}

void CartesianPlot::shiftLeftX(int index) {
	shift(index, Dimension::X, true);
}

void CartesianPlot::shiftRightX(int index) {
	shift(index, Dimension::X, false);
}

void CartesianPlot::shiftUpY(int index) {
	shift(index, Dimension::Y, false);
}

void CartesianPlot::shiftDownY(int index) {
	shift(index, Dimension::Y, true);
}

void CartesianPlot::cursor() {
	Q_D(CartesianPlot);
	d->retransformScales(-1, -1); // TODO: needed to retransform all scales?
}

void CartesianPlot::wheelEvent(const QPointF& sceneRelPos, int delta, int xIndex, int yIndex, bool considerDimension, Dimension dim) {
	Q_D(CartesianPlot);
	d->wheelEvent(sceneRelPos, delta, xIndex, yIndex, considerDimension, dim);
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

// #####################################################################
// ################### Private implementation ##########################
// #####################################################################
CartesianPlotPrivate::CartesianPlotPrivate(CartesianPlot* plot)
	: AbstractPlotPrivate(plot)
	, q(plot) {
	setFlag(QGraphicsItem::ItemIsMovable, true);
	setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
	setFlag(QGraphicsItem::ItemIsSelectable, true);
	setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
	setFlag(QGraphicsItem::ItemIsFocusable, true);

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
	if (q->parentAspect() && q->parentAspect()->type() == AspectType::CartesianPlot)
		q->plotArea()->setRect(mapRectToParent(rect));
	else
		q->plotArea()->setRect(mapRectFromScene(rect));

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

	// we have to recalculate the data range and auto scale in case of scale changes
	if (range(dim, index).scale() != dataRange(dim, index).scale() && autoScale(dim, index)) {
		q->calculateDataRange(dim, index);
		q->scaleAuto(dim, index, true, true);
	}
	auto r = range(dim, index);
	QDEBUG(Q_FUNC_INFO << CartesianCoordinateSystem::dimensionToString(dim) << "range =" << r.toString() << ", scale =" << r.scale()
					   << ", auto scale = " << r.autoScale())

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

		QVector<CartesianScale*> scales;

		// check whether we have x/y-range breaks - the first break, if available, should be valid
		bool hasValidBreak = (rangeBreakingEnabled(dim) && !rangeBreaks(dim).list.isEmpty() && rangeBreaks(dim).list.first().isValid());
		if (!hasValidBreak) { // no breaks available -> range goes from the start to the end of the plot
			sceneRange = plotSceneRange;
			logicalRange = r;

			if (sceneRange.length() > 0)
				scales << this->createScale(logicalRange.scale(), sceneRange, logicalRange);
		} else {
			double sceneEndLast = plotSceneRange.start();
			double logicalEndLast = r.start();
			auto rbs = rangeBreaks(dim);
			for (const auto& rb : std::as_const(rbs.list)) {
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
		Q_EMIT q->scaleRetransformed(q, dim, index);

	// Set ranges in the axis
	for (int i = 0; i < q->rangeCount(dim); i++) {
		auto& rangep = ranges(dim)[i];
		// Fix range when values not valid
		rangep.range.fixLimits();

		QDEBUG(Q_FUNC_INFO << ", range" << i << ":" << rangep.range.toString())
		const double deltaMin = rangep.range.start() - rangep.prev.start();
		const double deltaMax = rangep.range.end() - rangep.prev.end();

		if (!qFuzzyIsNull(deltaMin) && !suppressSignals)
			Q_EMIT q->minChanged(dim, i, rangep.range.start());
		if (!qFuzzyIsNull(deltaMax) && !suppressSignals)
			Q_EMIT q->maxChanged(dim, i, rangep.range.end());

		rangep.prev = rangep.range;

		for (auto* axis : q->children<Axis>()) {
			QDEBUG(Q_FUNC_INFO << ", auto-scale axis" << axis->name() << "of scale" << axis->scale())
			// use ranges of axis
			auto r = axis->range();
			r.setScale(rangep.range.scale());

			int axisIndex = q->coordinateSystem(axis->coordinateSystemIndex())->index(dim);
			if (axis->rangeType() != Axis::RangeType::Auto || axisIndex != i)
				continue;
			if ((dim == Dimension::Y && axis->orientation() != Axis::Orientation::Vertical)
				|| (dim == Dimension::X && axis->orientation() != Axis::Orientation::Horizontal))
				continue;

			if (!qFuzzyIsNull(deltaMax))
				r.setEnd(rangep.range.end());
			if (!qFuzzyIsNull(deltaMin))
				r.setStart(rangep.range.start());

			axis->setUndoAware(false);
			axis->setSuppressRetransform(true);
			axis->setRange(r);
			axis->setUndoAware(true);
			axis->setSuppressRetransform(false);
			// TODO;
			// 			if (axis->position() == Axis::Position::Centered && deltaYMin != 0) {
			// 				axis->setOffset(axis->offset() + deltaYMin, false);
			// 			}
		}
	}
}

/*
 * calculate x and y scales from scene range and logical range (x/y range) for all coordinate systems
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
 * calculates the rectangular of the area showing the actual data (plot's rect minus padding),
 * in plot's coordinates.
 */
void CartesianPlotPrivate::updateDataRect() {
	// map the rectangle rect, which is in scene coordinates, to this item's coordinate system
	dataRect = mapRectFromScene(rect);

	// for plot in a plot, transfer x and y coordinates
	if (q->parentAspect() && q->parentAspect()->type() == AspectType::CartesianPlot) {
		dataRect.setX(-rect.width() / 2);
		dataRect.setY(-rect.height() / 2);
		dataRect.setWidth(rect.width());
		dataRect.setHeight(rect.height());
	}

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

/*!
 * \brief CartesianPlotPrivate::rangeChanged
 * This function will be called if the range shall be updated, because some other parameters (like the datarange type, datarange points)
 * changed. In this case the ranges must be updated if autoscale is turned on
 * At the end signals for all ranges are send out that they changed.
 */
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

CartesianScale* CartesianPlotPrivate::createScale(RangeT::Scale scale, const Range<double>& sceneRange, const Range<double>& logicalRange) {
	QDEBUG(Q_FUNC_INFO << ", scale =" << scale << ", scene range : " << sceneRange.toString() << ", logical range : " << logicalRange.toString());

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

// ##############################################################################
// ##################################  Events  ##################################
// ##############################################################################

void CartesianPlotPrivate::contextMenuEvent(QGraphicsSceneContextMenuEvent* event) {
	const auto* cSystem{defaultCoordinateSystem()};
	scenePos = event->pos();
	if (!cSystem->isValid())
		return;
	logicalPos = cSystem->mapSceneToLogical(scenePos, AbstractCoordinateSystem::MappingFlag::Limit);
	calledFromContextMenu = true;
	auto* menu = q->createContextMenu();
	if (q->parentAspect()->type() == AspectType::CartesianPlot)
		Q_EMIT q->parentAspect()->contextMenuRequested(q->AbstractAspect::type(),
													   menu); // for inset plots emit the signal for the parent to handle it in Worksheet
	else
		Q_EMIT q->contextMenuRequested(q->AbstractAspect::type(), menu);
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
	auto* w = static_cast<Worksheet*>(q->parent<Worksheet>())->currentSelection();
	int index = CartesianPlot::cSystemIndex(w);
	if (index >= 0)
		cSystem = static_cast<CartesianCoordinateSystem*>(q->m_coordinateSystems.at(index));
	if (mouseMode == CartesianPlot::MouseMode::Selection) {
		if (interactive && dataRect.contains(event->pos())) {
			panningStarted = true;
			m_panningStart = event->pos();
			setCursor(Qt::ClosedHandCursor);
		}
	} else if (mouseMode == CartesianPlot::MouseMode::ZoomSelection || mouseMode == CartesianPlot::MouseMode::ZoomXSelection
			   || mouseMode == CartesianPlot::MouseMode::ZoomYSelection) {
		if (!cSystem->isValid())
			return;
		const QPointF logicalPos = cSystem->mapSceneToLogical(event->pos(), AbstractCoordinateSystem::MappingFlag::Limit);
		Q_EMIT q->mousePressZoomSelectionModeSignal(logicalPos);
		return;
	} else if (mouseMode == CartesianPlot::MouseMode::Cursor) {
		if (!cSystem->isValid())
			return;
		const QPointF logicalPos = cSystem->mapSceneToLogical(event->pos(), AbstractCoordinateSystem::MappingFlag::Limit);
		setCursor(Qt::SizeHorCursor);
		double cursorPenWidth2 = cursorLine->pen().width() / 2.;
		if (cursorPenWidth2 < 10.)
			cursorPenWidth2 = 10.;

		bool visible;
		if (cursor0Enable
			&& std::abs(event->pos().x() - cSystem->mapLogicalToScene(QPointF(cursor0Pos.x(), range(Dimension::Y).start()), visible).x()) < cursorPenWidth2) {
			selectedCursor = 0;
		} else if (cursor1Enable
				   && std::abs(event->pos().x() - cSystem->mapLogicalToScene(QPointF(cursor1Pos.x(), range(Dimension::Y).start()), visible).x())
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
	auto* w = static_cast<Worksheet*>(q->parent<Worksheet>())->currentSelection();
	int index = CartesianPlot::cSystemIndex(w);
	if (index >= 0)
		cSystem = static_cast<CartesianCoordinateSystem*>(q->m_coordinateSystems.at(index));

	if (mouseMode == CartesianPlot::MouseMode::Selection) {
		if (panningStarted && dataRect.contains(event->pos())) {
			// don't retransform on small mouse movement deltas
			const int deltaXScene = (m_panningStart.x() - event->pos().x());
			const int deltaYScene = (m_panningStart.y() - event->pos().y());
			if (std::abs(deltaXScene) < 5 && std::abs(deltaYScene) < 5)
				return;

			if (!cSystem->isValid())
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
		if (!cSystem->isValid())
			return;
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
		if (!cSystem->isValid())
			return;
		const QPointF logicalPos = cSystem->mapSceneToLogical(event->pos(), AbstractCoordinateSystem::MappingFlag::Limit);
		Q_EMIT q->mouseMoveCursorModeSignal(selectedCursor, logicalPos);
	}
}

bool CartesianPlotPrivate::translateRange(int xIndex, int yIndex, const QPointF& logicalStart, const QPointF& logicalEnd, bool translateX, bool translateY) {
	// handle the change in x
	bool translatedX = false, translatedY = false;
	if (translateX && logicalStart.x() - logicalEnd.x() != 0) { // TODO: find better method
		translatedX = true;
		range(Dimension::X, xIndex).translate(logicalStart.x(), logicalEnd.x());
	}

	if (translateY && logicalStart.y() - logicalEnd.y() != 0) {
		translatedY = true;
		// handle the change in y
		range(Dimension::Y, yIndex).translate(logicalStart.y(), logicalEnd.y());
	}

	q->setUndoAware(false);
	if (translatedX) {
		q->enableAutoScale(Dimension::X, xIndex, false);
		retransformScale(Dimension::X, xIndex);
	}
	if (translatedY) {
		q->enableAutoScale(Dimension::Y, yIndex, false);
		retransformScale(Dimension::Y, yIndex);
	}
	q->setUndoAware(true);

	// If x or y should not be translated, means, that it was done before
	// so the ranges must get dirty.
	if (translatedX || translatedY || !translateX || !translateY) {
		q->setRangeDirty(Dimension::X, xIndex, true);
		q->setRangeDirty(Dimension::Y, yIndex, true);
	}

	return translatedX || translatedY || !translateX || !translateY;
}

void CartesianPlotPrivate::mouseMoveSelectionMode(QPointF logicalStart, QPointF logicalEnd) {
	const bool autoscaleRanges = true; // consumes a lot of power, maybe making an option to turn off/on!
	auto* w = static_cast<Worksheet*>(q->parent<Worksheet>())->currentSelection();
	int index = CartesianPlot::cSystemIndex(w);
	if (!w || w->parent<CartesianPlot>() != q)
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

	const auto& xRangeFormat{range(Dimension::X, xIndex).format()};
	const auto& yRangeFormat{range(Dimension::Y, yIndex).format()};
	const auto& xRangeDateTimeFormat{range(Dimension::X, xIndex).dateTimeFormat()};
	if (!cSystem->isValid())
		return;
	const QPointF logicalStart = cSystem->mapSceneToLogical(m_selectionStart, AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);

	if (mouseMode == CartesianPlot::MouseMode::ZoomSelection) {
		bool visible;
		m_selectionEnd = cSystem->mapLogicalToScene(logicalPos, visible, AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);
		QPointF logicalEnd = logicalPos;
		if (xRangeFormat == RangeT::Format::Numeric)
			info = QString::fromUtf8("Δx=") + QString::number(logicalEnd.x() - logicalStart.x());
		else
			info = i18n("from x=%1 to x=%2",
						QDateTime::fromMSecsSinceEpoch(logicalStart.x(), QTimeZone::UTC).toString(xRangeDateTimeFormat),
						QDateTime::fromMSecsSinceEpoch(logicalEnd.x(), QTimeZone::UTC).toString(xRangeDateTimeFormat));

		info += QLatin1String(", ");
		if (yRangeFormat == RangeT::Format::Numeric)
			info += QString::fromUtf8("Δy=") + QString::number(logicalEnd.y() - logicalStart.y());
		else
			info += i18n("from y=%1 to y=%2",
						 QDateTime::fromMSecsSinceEpoch(logicalStart.y(), QTimeZone::UTC).toString(xRangeDateTimeFormat),
						 QDateTime::fromMSecsSinceEpoch(logicalEnd.y(), QTimeZone::UTC).toString(xRangeDateTimeFormat));
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
						QDateTime::fromMSecsSinceEpoch(logicalStart.x(), QTimeZone::UTC).toString(xRangeDateTimeFormat),
						QDateTime::fromMSecsSinceEpoch(logicalEnd.x(), QTimeZone::UTC).toString(xRangeDateTimeFormat));
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
						QDateTime::fromMSecsSinceEpoch(logicalStart.y(), QTimeZone::UTC).toString(xRangeDateTimeFormat),
						QDateTime::fromMSecsSinceEpoch(logicalEnd.y(), QTimeZone::UTC).toString(xRangeDateTimeFormat));
	}
	q->info(info);
	update();
}

void CartesianPlotPrivate::mouseMoveCursorMode(int cursorNumber, QPointF logicalPos) {
	const auto& xRangeFormat{range(Dimension::X).format()};
	const auto& xRangeDateTimeFormat{range(Dimension::X).dateTimeFormat()};

	QPointF p1(logicalPos.x(), 0);
	cursorNumber == 0 ? cursor0Pos = p1 : cursor1Pos = p1;

	QString info;
	if (xRangeFormat == RangeT::Format::Numeric)
		info = QString::fromUtf8("x=") + QString::number(logicalPos.x());
	else
		info = i18n("x=%1", QDateTime::fromMSecsSinceEpoch(logicalPos.x(), QTimeZone::UTC).toString(xRangeDateTimeFormat));
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
		Q_EMIT q->changed(); // notify about the position change
	} else if (mouseMode == CartesianPlot::MouseMode::ZoomSelection || mouseMode == CartesianPlot::MouseMode::ZoomXSelection
			   || mouseMode == CartesianPlot::MouseMode::ZoomYSelection) {
		Q_EMIT q->mouseReleaseZoomSelectionModeSignal();
	}
}

void CartesianPlotPrivate::mouseReleaseZoomSelectionMode(int cSystemIndex, bool suppressRetransform) {
	m_selectionBandIsShown = false;
	// don't zoom if very small region was selected, avoid occasional/unwanted zooming
	if (std::abs(m_selectionEnd.x() - m_selectionStart.x()) < 20 && std::abs(m_selectionEnd.y() - m_selectionStart.y()) < 20)
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
		if (!cSystem->isValid())
			return;
		QPointF logicalZoomStart = cSystem->mapSceneToLogical(m_selectionStart, AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);
		QPointF logicalZoomEnd = cSystem->mapSceneToLogical(m_selectionEnd, AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);

		if (mouseMode == CartesianPlot::MouseMode::ZoomSelection || mouseMode == CartesianPlot::MouseMode::ZoomXSelection) {
			if (m_selectionEnd.x() > m_selectionStart.x())
				range(Dimension::X, xIndex).setRange(logicalZoomStart.x(), logicalZoomEnd.x());
			else
				range(Dimension::X, xIndex).setRange(logicalZoomEnd.x(), logicalZoomStart.x());

			if (niceExtend)
				range(Dimension::X, xIndex).niceExtend();
		}

		if (mouseMode == CartesianPlot::MouseMode::ZoomSelection || mouseMode == CartesianPlot::MouseMode::ZoomYSelection) {
			if (m_selectionEnd.y() > m_selectionStart.y())
				range(Dimension::Y, yIndex).setRange(logicalZoomEnd.y(), logicalZoomStart.y());
			else
				range(Dimension::Y, yIndex).setRange(logicalZoomStart.y(), logicalZoomEnd.y());

			if (niceExtend)
				range(Dimension::Y, yIndex).niceExtend();
		}

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
	if (!interactive)
		return;

	auto* w = static_cast<Worksheet*>(q->parent<Worksheet>())->currentSelection();
	int cSystemIndex = CartesianPlot::cSystemIndex(w);
	int xIndex = -1, yIndex = -1;
	if (w && w->parent<CartesianPlot>() == q) {
		xIndex = coordinateSystem(cSystemIndex)->index(Dimension::X);
		yIndex = coordinateSystem(cSystemIndex)->index(Dimension::Y);
	}

	const auto pos = event->pos();
	const auto posLogical = coordinateSystem(0)->mapLogicalToScene({pos});
	const double xSceneRelPos = (pos.x() - dataRect.left()) / dataRect.width();
	// if (xSceneRelPos < 0)
	// 	xSceneRelPos = 0;
	// if (xSceneRelPos > 1)
	//	xSceneRelPos = 1;
	const double ySceneRelPos = (dataRect.bottom() - pos.y()) / dataRect.height();
	// if (ySceneRelPos < 0)
	//	ySceneRelPos = 0;
	// if (ySceneRelPos > 1)
	//	ySceneRelPos = 1;
	const QPointF sceneRelPos(xSceneRelPos, ySceneRelPos);

	bool considerDimension = false;
	Dimension dim = Dimension::X;
	if (w && w->type() == AspectType::Axis) {
		const auto* axis = static_cast<Axis*>(w);
		considerDimension = true;
		if (axis->orientation() == Axis::Orientation::Vertical)
			dim = Dimension::Y;
	}

	Q_EMIT q->wheelEventSignal(sceneRelPos, event->delta(), xIndex, yIndex, considerDimension, dim);
}

void CartesianPlotPrivate::wheelEvent(const QPointF& sceneRelPos, int delta, int xIndex, int yIndex, bool considerDimension, Dimension dim) {
	if (considerDimension) {
		// Only one dimension
		switch (dim) {
		case Dimension::X:
			q->zoomInOut(xIndex, dim, delta > 0, sceneRelPos.x());
			break;
		case Dimension::Y:
			q->zoomInOut(yIndex, dim, delta > 0, sceneRelPos.y());
			break;
		}
		return;
	}

	if (delta > 0)
		q->zoomIn(xIndex, yIndex, sceneRelPos);
	else
		q->zoomOut(xIndex, yIndex, sceneRelPos);
}

void CartesianPlotPrivate::keyPressEvent(QKeyEvent* event) {
	const auto key = event->key();
	// const bool ctrl = event->modifiers() & Qt::KeyboardModifier::ControlModifier;
	//	const bool shift = event->modifiers() & Qt::KeyboardModifier::ShiftModifier;
	//	const bool alt = event->modifiers() & Qt::KeyboardModifier::AltModifier;
	Action a = evaluateKeys(key, event->modifiers());

	if (a == Action::Abort) {
		m_selectionBandIsShown = false;
	} else if (a & Action::Move) {
		const auto* worksheet = static_cast<const Worksheet*>(q->parentAspect());
		if (worksheet->layout() == Worksheet::Layout::NoLayout) {
			// no layout is active -> use arrow keys to move the plot on the worksheet
			const int delta = 5;
			QRectF rect = q->rect();

			if (a == Action::MoveLeft) {
				rect.setX(rect.x() - delta);
				rect.setWidth(rect.width() - delta);
			} else if (a == Action::MoveRight) {
				rect.setX(rect.x() + delta);
				rect.setWidth(rect.width() + delta);
			} else if (a == Action::MoveUp) {
				rect.setY(rect.y() - delta);
				rect.setHeight(rect.height() - delta);
			} else if (a == Action::MoveDown) {
				rect.setY(rect.y() + delta);
				rect.setHeight(rect.height() + delta);
			}

			q->setRect(rect);
		}
	} else if (a == Action::NavigateNextCurve) // (key == Qt::Key_Tab)
		navigateNextPrevCurve();
	else if (a == Action::NavigatePrevCurve) // (key == Qt::SHIFT + Qt::Key_Tab)
		navigateNextPrevCurve(false /*next*/);
	else if (a & Action::New) {
		const auto* cSystem{defaultCoordinateSystem()};
		if (cSystem->isValid()) {
			logicalPos = cSystem->mapSceneToLogical(scenePos, AbstractCoordinateSystem::MappingFlag::Limit);
			calledFromContextMenu = true;
		}
		if (a == Action::NewTextLabel)
			q->addTextLabel();
		else if (a == Action::NewReferenceLine)
			q->addReferenceLine();
		else if (a == Action::NewReferenceRange)
			q->addReferenceRange();
		else if (a == Action::NewCustomPoint)
			q->addCustomPoint();
		else if (a == Action::NewImage)
			q->addImage();
	}
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

	auto* w = static_cast<Worksheet*>(q->parent<Worksheet>());

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
	scenePos = point;
	QString info;
	const auto* cSystem{defaultCoordinateSystem()};
	auto* w = static_cast<Worksheet*>(q->parent<Worksheet>())->currentSelection();
	int index = CartesianPlot::cSystemIndex(w);
	int xIndex = cSystem->index(Dimension::X), yIndex = cSystem->index(Dimension::Y);
	if (!w || w->parent<CartesianPlot>() != q) {
		xIndex = -1;
		yIndex = -1;
	} else if (index >= 0) {
		cSystem = static_cast<CartesianCoordinateSystem*>(q->m_coordinateSystems.at(index));
		xIndex = cSystem->index(Dimension::X);
		yIndex = cSystem->index(Dimension::Y);
	}

	const auto& xRangeFormat{range(Dimension::X, xIndex).format()};
	const auto& yRangeFormat{range(Dimension::Y, yIndex).format()};
	const auto& xRangeDateTimeFormat{range(Dimension::X, xIndex).dateTimeFormat()};
	const auto& yRangeDateTimeFormat{range(Dimension::Y, yIndex).dateTimeFormat()};
	if (dataRect.contains(point)) {
		if (!cSystem->isValid())
			return;
		QPointF logicalPoint = cSystem->mapSceneToLogical(point);

		if ((mouseMode == CartesianPlot::MouseMode::ZoomSelection) || mouseMode == CartesianPlot::MouseMode::Selection
			|| mouseMode == CartesianPlot::MouseMode::Crosshair) {
			info = QStringLiteral("x=");
			if (xRangeFormat == RangeT::Format::Numeric)
				info += QString::number(logicalPoint.x());
			else
				info += QDateTime::fromMSecsSinceEpoch(logicalPoint.x(), QTimeZone::UTC).toString(xRangeDateTimeFormat);

			info += QStringLiteral(", y=");
			if (yRangeFormat == RangeT::Format::Numeric)
				info += QString::number(logicalPoint.y());
			else
				info += QDateTime::fromMSecsSinceEpoch(logicalPoint.y(), QTimeZone::UTC).toString(yRangeDateTimeFormat);
		}

		if (mouseMode == CartesianPlot::MouseMode::ZoomSelection && !m_selectionBandIsShown) {
			Q_EMIT q->mouseHoverZoomSelectionModeSignal(logicalPoint);
		} else if (mouseMode == CartesianPlot::MouseMode::ZoomXSelection && !m_selectionBandIsShown) {
			info = QStringLiteral("x=");
			if (xRangeFormat == RangeT::Format::Numeric)
				info += QString::number(logicalPoint.x());
			else
				info += QDateTime::fromMSecsSinceEpoch(logicalPoint.x(), QTimeZone::UTC).toString(xRangeDateTimeFormat);
			Q_EMIT q->mouseHoverZoomSelectionModeSignal(logicalPoint);
		} else if (mouseMode == CartesianPlot::MouseMode::ZoomYSelection && !m_selectionBandIsShown) {
			info = QStringLiteral("y=");
			if (yRangeFormat == RangeT::Format::Numeric)
				info += QString::number(logicalPoint.y());
			else
				info += QDateTime::fromMSecsSinceEpoch(logicalPoint.y(), QTimeZone::UTC).toString(yRangeDateTimeFormat);
			Q_EMIT q->mouseHoverZoomSelectionModeSignal(logicalPoint);
		} else if (mouseMode == CartesianPlot::MouseMode::Selection) {
			// hover the nearest curve to the mousepointer
			// hovering curves is implemented in the parent, because no ignoreEvent() exists
			// for it. Checking all curves and hover the first
			bool hovered = false;
			const auto& curves = q->children<Plot>();
			for (int i = curves.count() - 1; i >= 0; i--) { // because the last curve is above the other curves
				auto* curve = curves.at(i);
				if (hovered) { // if a curve is already hovered, disable hover for the rest
					curve->setHover(false);
					continue;
				}
				if (curve->activatePlot(event->pos()) && !curve->isLocked()) {
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
				info += QDateTime::fromMSecsSinceEpoch(logicalPoint.x(), QTimeZone::UTC).toString(xRangeDateTimeFormat);

			double cursorPenWidth2 = cursorLine->pen().width() / 2.;
			if (cursorPenWidth2 < 10.)
				cursorPenWidth2 = 10.;

			bool visible;
			if ((cursor0Enable
				 && std::abs(point.x() - defaultCoordinateSystem()->mapLogicalToScene(QPointF(cursor0Pos.x(), range(Dimension::Y).start()), visible).x())
					 < cursorPenWidth2)
				|| (cursor1Enable
					&& std::abs(point.x() - defaultCoordinateSystem()->mapLogicalToScene(QPointF(cursor1Pos.x(), range(Dimension::Y).start()), visible).x())
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
	for (auto* plot : q->children<Plot>())
		plot->setHover(false);

	m_hovered = false;
	QGraphicsItem::hoverLeaveEvent(event);
}

void CartesianPlotPrivate::mouseHoverZoomSelectionMode(QPointF logicPos, int cSystemIndex) {
	m_insideDataRect = true;

	const CartesianCoordinateSystem* cSystem;
	auto* w = static_cast<Worksheet*>(q->parent<Worksheet>())->currentSelection();
	int index = CartesianPlot::cSystemIndex(w);
	if (w && w->parent<CartesianPlot>() == q && index != -1)
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
}

// ##############################################################################
// ##################  Serialization/Deserialization  ###########################
// ##############################################################################

//! Save as XML
void CartesianPlot::save(QXmlStreamWriter* writer) const {
	Q_D(const CartesianPlot);

	writer->writeStartElement(QStringLiteral("cartesianPlot"));
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	writer->writeStartElement(QStringLiteral("general"));
	writer->writeAttribute(QStringLiteral("plotColorMode"), QString::number(static_cast<int>(d->plotColorMode)));
	writer->writeAttribute(QStringLiteral("plotColorMap"), d->plotColorMap);
	if (!d->theme.isEmpty())
		writer->writeAttribute(QStringLiteral("theme"), d->theme);
	writer->writeEndElement();

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

	// x-ranges
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

	// y-ranges
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

	// coordinate systems, data range settings and padding
	writer->writeStartElement(QStringLiteral("coordinateSystems"));
	writer->writeAttribute(QStringLiteral("defaultCoordinateSystem"), QString::number(defaultCoordinateSystemIndex()));
	writer->writeAttribute(QStringLiteral("horizontalPadding"), QString::number(d->horizontalPadding));
	writer->writeAttribute(QStringLiteral("verticalPadding"), QString::number(d->verticalPadding));
	writer->writeAttribute(QStringLiteral("rightPadding"), QString::number(d->rightPadding));
	writer->writeAttribute(QStringLiteral("bottomPadding"), QString::number(d->bottomPadding));
	writer->writeAttribute(QStringLiteral("symmetricPadding"), QString::number(d->symmetricPadding));
	writer->writeAttribute(QStringLiteral("rangeType"), QString::number(static_cast<int>(d->rangeType)));
	writer->writeAttribute(QStringLiteral("rangeFirstValues"), QString::number(d->rangeFirstValues));
	writer->writeAttribute(QStringLiteral("rangeLastValues"), QString::number(d->rangeLastValues));
	writer->writeAttribute(QStringLiteral("niceExtend"), QString::number(d->niceExtend));
	for (const auto& cSystem : m_coordinateSystems) {
		writer->writeStartElement(QStringLiteral("coordinateSystem"));
		writer->writeAttribute(QStringLiteral("name"), cSystem->name());
		writer->writeAttribute(QStringLiteral("xIndex"), QString::number(static_cast<CartesianCoordinateSystem*>(cSystem)->index(Dimension::X)));
		writer->writeAttribute(QStringLiteral("yIndex"), QString::number(static_cast<CartesianCoordinateSystem*>(cSystem)->index(Dimension::Y)));
		writer->writeEndElement();
	}
	writer->writeEndElement();

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
		} else if (!preview && reader->name() == QLatin1String("general")) {
			attribs = reader->attributes();
			READ_INT_VALUE("plotColorMode", plotColorMode, PlotColorMode);
			READ_STRING_VALUE("theme", theme);
			READ_STRING_VALUE("plotColorMap", plotColorMap);
		} else if (!preview && Project::xmlVersion() < 17 && reader->name() == QLatin1String("theme")) {
			attribs = reader->attributes();
			d->theme = attribs.value(QStringLiteral("name")).toString();
		} else if (!preview && reader->name() == QLatin1String("cursor")) {
			d->cursorLine->load(reader, preview);
		} else if (!preview && reader->name() == QLatin1String("geometry")) {
			attribs = reader->attributes();

			str = attribs.value(QStringLiteral("x")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("x"));
			else
				d->rect.setX(str.toDouble());

			str = attribs.value(QStringLiteral("y")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("y"));
			else
				d->rect.setY(str.toDouble());

			str = attribs.value(QStringLiteral("width")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("width"));
			else
				d->rect.setWidth(str.toDouble());

			str = attribs.value(QStringLiteral("height")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("height"));
			else
				d->rect.setHeight(str.toDouble());

			str = attribs.value(QStringLiteral("visible")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("visible"));
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
				reader->raiseMissingAttributeWarning(QStringLiteral("autoScale"));
			else
				range.setAutoScale(str.toInt());
			str = attribs.value(QStringLiteral("start")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("start"));
			else
				range.setStart(str.toDouble());
			str = attribs.value(QStringLiteral("end")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("end"));
			else
				range.setEnd(str.toDouble());
			str = attribs.value(QStringLiteral("scale")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("scale"));
			else
				range.setScale(static_cast<RangeT::Scale>(str.toInt()));
			str = attribs.value(QStringLiteral("format")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("format"));
			else
				range.setFormat(static_cast<RangeT::Format>(str.toInt()));
			str = attribs.value(QStringLiteral("dateTimeFormat")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("dateTimeFormat"));
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
				reader->raiseMissingAttributeWarning(QStringLiteral("autoScale"));
			else
				range.setAutoScale(str.toInt());
			str = attribs.value(QStringLiteral("start")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("start"));
			else
				range.setStart(str.toDouble());
			str = attribs.value(QStringLiteral("end")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("end"));
			else
				range.setEnd(str.toDouble());
			str = attribs.value(QStringLiteral("scale")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("scale"));
			else
				range.setScale(static_cast<RangeT::Scale>(str.toInt()));
			str = attribs.value(QStringLiteral("format")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("format"));
			else
				range.setFormat(static_cast<RangeT::Format>(str.toInt()));
			str = attribs.value(QStringLiteral("dateTimeFormat")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("dateTimeFormat"));
			else
				range.setDateTimeFormat(str);

			addYRange(range);
		} else if (!preview && reader->name() == QLatin1String("coordinateSystems")) {
			attribs = reader->attributes();
			READ_INT_VALUE("defaultCoordinateSystem", defaultCoordinateSystemIndex, int);
			DEBUG(Q_FUNC_INFO << ", got default cSystem index = " << d->defaultCoordinateSystemIndex)

			// the file can be corrupted, either because of bugs like in
			// https://invent.kde.org/education/labplot/-/issues/598, https://invent.kde.org/education/labplot/-/issues/869
			// or because it was manually compromised.
			// In order not to crash because of the wrong indices, add a safety check here.
			// TODO: check the ranges and the coordinate system to make sure they're available.
			if (d->defaultCoordinateSystemIndex > m_coordinateSystems.size() - 1)
				d->defaultCoordinateSystemIndex = 0;

			READ_DOUBLE_VALUE("horizontalPadding", horizontalPadding);
			READ_DOUBLE_VALUE("verticalPadding", verticalPadding);
			READ_DOUBLE_VALUE("rightPadding", rightPadding);
			READ_DOUBLE_VALUE("bottomPadding", bottomPadding);
			READ_INT_VALUE("symmetricPadding", symmetricPadding, bool);
			hasCoordinateSystems = true;

			READ_INT_VALUE("rangeType", rangeType, CartesianPlot::RangeType);
			READ_INT_VALUE("rangeFirstValues", rangeFirstValues, int);
			READ_INT_VALUE("rangeLastValues", rangeLastValues, int);

			if (Project::xmlVersion() < 7) {
				d->niceExtend = true;
			} else {
				str = attribs.value(QStringLiteral("niceExtend")).toString();
				if (str.isEmpty())
					reader->raiseMissingAttributeWarning(QStringLiteral("niceExtend"));
				else
					d->niceExtend = str.toInt();
			}
		} else if (!preview && reader->name() == QLatin1String("coordinateSystem")) {
			attribs = reader->attributes();
			// new style
			str = attribs.value(QStringLiteral("xIndex")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("xIndex"));
			else {
				auto* cSystem = new CartesianCoordinateSystem(this);
				cSystem->setIndex(Dimension::X, str.toInt());

				str = attribs.value(QStringLiteral("yIndex")).toString();
				if (str.isEmpty())
					reader->raiseMissingAttributeWarning(QStringLiteral("yIndex"));
				else
					cSystem->setIndex(Dimension::Y, str.toInt());

				addCoordinateSystem(cSystem);

				str = attribs.value(QStringLiteral("name")).toString();
				cSystem->setName(str);
			}

			// old style (pre 2.9.0, to read old projects)
			if (!hasCoordinateSystems) {
				str = attribs.value(QStringLiteral("autoScaleX")).toString();
				if (str.isEmpty())
					reader->raiseMissingAttributeWarning(QStringLiteral("autoScaleX"));
				else
					d->xRanges[0].range.setAutoScale(str.toInt());
				str = attribs.value(QStringLiteral("autoScaleY")).toString();
				if (str.isEmpty())
					reader->raiseMissingAttributeWarning(QStringLiteral("autoScaleY"));
				else
					d->yRanges[0].range.setAutoScale(str.toInt());

				str = attribs.value(QStringLiteral("xMin")).toString();
				if (str.isEmpty())
					reader->raiseMissingAttributeWarning(QStringLiteral("xMin"));
				else {
					d->xRanges[0].range.start() = str.toDouble();
					d->xRanges[0].prev.start() = d->range(Dimension::X, 0).start();
				}

				str = attribs.value(QStringLiteral("xMax")).toString();
				if (str.isEmpty())
					reader->raiseMissingAttributeWarning(QStringLiteral("xMax"));
				else {
					d->xRanges[0].range.end() = str.toDouble();
					d->xRanges[0].prev.end() = d->range(Dimension::X, 0).end();
				}

				str = attribs.value(QStringLiteral("yMin")).toString();
				if (str.isEmpty())
					reader->raiseMissingAttributeWarning(QStringLiteral("yMin"));
				else {
					d->yRanges[0].range.start() = str.toDouble();
					d->yRanges[0].prev.start() = range(Dimension::Y, 0).start();
				}

				str = attribs.value(QStringLiteral("yMax")).toString();
				if (str.isEmpty())
					reader->raiseMissingAttributeWarning(QStringLiteral("yMax"));
				else {
					d->yRanges[0].range.end() = str.toDouble();
					d->yRanges[0].prev.end() = range(Dimension::Y, 0).end();
				}

				str = attribs.value(QStringLiteral("xScale")).toString();
				if (str.isEmpty())
					reader->raiseMissingAttributeWarning(QStringLiteral("xScale"));
				else {
					int scale{str.toInt()};
					// convert old scale
					if (scale > (int)RangeT::Scale::Ln)
						scale -= 3;
					d->xRanges[0].range.scale() = static_cast<RangeT::Scale>(scale);
				}
				str = attribs.value(QStringLiteral("yScale")).toString();
				if (str.isEmpty())
					reader->raiseMissingAttributeWarning(QStringLiteral("yScale"));
				else {
					int scale{str.toInt()};
					// convert old scale
					if (scale > (int)RangeT::Scale::Ln)
						scale -= 3;
					d->yRanges[0].range.scale() = static_cast<RangeT::Scale>(scale);
				}

				str = attribs.value(QStringLiteral("xRangeFormat")).toString();
				if (str.isEmpty())
					reader->raiseMissingAttributeWarning(QStringLiteral("xRangeFormat"));
				else
					d->xRanges[0].range.format() = static_cast<RangeT::Format>(str.toInt());
				str = attribs.value(QStringLiteral("yRangeFormat")).toString();
				if (str.isEmpty())
					reader->raiseMissingAttributeWarning(QStringLiteral("yRangeFormat"));
				else
					d->yRanges[0].range.format() = static_cast<RangeT::Format>(str.toInt());

				str = attribs.value(QStringLiteral("xRangeDateTimeFormat")).toString();
				if (!str.isEmpty())
					d->xRanges[0].range.setDateTimeFormat(str);

				str = attribs.value(QStringLiteral("yRangeDateTimeFormat")).toString();
				if (!str.isEmpty())
					d->yRanges[0].range.setDateTimeFormat(str);

				auto* cSystem = new CartesianCoordinateSystem(this);
				cSystem->setIndex(Dimension::X, 0);
				cSystem->setIndex(Dimension::Y, 0);
				addCoordinateSystem(cSystem);

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
				reader->raiseMissingAttributeWarning(QStringLiteral("start"));
			else
				b.range.start() = str.toDouble();

			str = attribs.value(QStringLiteral("end")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("end"));
			else
				b.range.end() = str.toDouble();

			str = attribs.value(QStringLiteral("position")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("position"));
			else
				b.position = str.toDouble();

			str = attribs.value(QStringLiteral("style")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("style"));
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
				reader->raiseMissingAttributeWarning(QStringLiteral("start"));
			else
				b.range.start() = str.toDouble();

			str = attribs.value(QStringLiteral("end")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("end"));
			else
				b.range.end() = str.toDouble();

			str = attribs.value(QStringLiteral("position")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("position"));
			else
				b.position = str.toDouble();

			str = attribs.value(QStringLiteral("style")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("style"));
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
			if (image->load(reader, preview))
				addChildFast(image);
			else {
				delete image;
				return false;
			}
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
			auto* axis = new Axis(QString(), Axis::Orientation::Horizontal, true);
			axis->setIsLoading(true);
			if (axis->load(reader, preview))
				addChildFast(axis);
			else {
				delete axis;
				return false;
			}
		} else if (reader->name() == QLatin1String("xyCurve")) {
			auto* curve = new XYCurve(QString(), AspectType::XYCurve, true);
			curve->setIsLoading(true);
			if (curve->load(reader, preview))
				addChildFast(curve);
			else {
				delete curve;
				return false;
			}
		} else if (reader->name() == QLatin1String("xyEquationCurve")) {
			auto* curve = new XYEquationCurve(QString());
			curve->setIsLoading(true);
			if (curve->load(reader, preview))
				addChildFast(curve);
			else {
				delete curve;
				return false;
			}
		} else if (reader->name() == XYFunctionCurve::saveName) {
			auto* curve = new XYFunctionCurve(QString());
			curve->setIsLoading(true);
			if (curve->load(reader, preview))
				addChildFast(curve);
			else {
				delete curve;
				return false;
			}
		} else if (reader->name() == QLatin1String("xyLineSimplificationCurve")) {
			auto* curve = new XYLineSimplificationCurve(QString());
			curve->setIsLoading(true);
			if (curve->load(reader, preview))
				addChildFast(curve);
			else {
				delete curve;
				return false;
			}
		} else if (reader->name() == QLatin1String("xyDifferentiationCurve")) {
			auto* curve = new XYDifferentiationCurve(QString());
			curve->setIsLoading(true);
			if (curve->load(reader, preview))
				addChildFast(curve);
			else {
				delete curve;
				return false;
			}
        } else if (reader->name() == QLatin1String("xyBaselineCorrectionCurve")) {
            auto* curve = new XYBaselineCorrectionCurve(QString());
            curve->setIsLoading(true);
            if (curve->load(reader, preview))
                addChildFast(curve);
            else {
                delete curve;
                return false;
            }
		} else if (reader->name() == QLatin1String("xyIntegrationCurve")) {
			auto* curve = new XYIntegrationCurve(QString());
			curve->setIsLoading(true);
			if (curve->load(reader, preview))
				addChildFast(curve);
			else {
				delete curve;
				return false;
			}
		} else if (reader->name() == QLatin1String("xyInterpolationCurve")) {
			auto* curve = new XYInterpolationCurve(QString());
			curve->setIsLoading(true);
			if (curve->load(reader, preview))
				addChildFast(curve);
			else {
				delete curve;
				return false;
			}
		} else if (reader->name() == QLatin1String("xySmoothCurve")) {
			auto* curve = new XYSmoothCurve(QString());
			curve->setIsLoading(true);
			if (curve->load(reader, preview))
				addChildFast(curve);
			else {
				delete curve;
				return false;
			}
		} else if (reader->name() == QLatin1String("xyFitCurve")) {
			auto* curve = new XYFitCurve(QString());
			curve->setIsLoading(true);
			if (curve->load(reader, preview))
				addChildFast(curve);
			else {
				delete curve;
				return false;
			}
		} else if (reader->name() == QLatin1String("xyFourierFilterCurve")) {
			auto* curve = new XYFourierFilterCurve(QString());
			curve->setIsLoading(true);
			if (curve->load(reader, preview))
				addChildFast(curve);
			else {
				delete curve;
				return false;
			}
		} else if (reader->name() == QLatin1String("xyFourierTransformCurve")) {
			auto* curve = new XYFourierTransformCurve(QString());
			curve->setIsLoading(true);
			if (curve->load(reader, preview))
				addChildFast(curve);
			else {
				delete curve;
				return false;
			}
		} else if (reader->name() == QLatin1String("xyHilbertTransformCurve")) {
			auto* curve = new XYHilbertTransformCurve(QString());
			curve->setIsLoading(true);
			if (curve->load(reader, preview))
				addChildFast(curve);
			else {
				delete curve;
				return false;
			}
		} else if (reader->name() == QLatin1String("xyConvolutionCurve")) {
			auto* curve = new XYConvolutionCurve(QString());
			curve->setIsLoading(true);
			if (curve->load(reader, preview))
				addChildFast(curve);
			else {
				delete curve;
				return false;
			}
		} else if (reader->name() == QLatin1String("xyCorrelationCurve")) {
			auto* curve = new XYCorrelationCurve(QString());
			curve->setIsLoading(true);
			if (curve->load(reader, preview))
				addChildFast(curve);
			else {
				delete curve;
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
			auto* point = new CustomPoint(this, QString(), true);
			point->setIsLoading(true);
			if (point->load(reader, preview))
				addChildFast(point);
			else {
				delete point;
				return false;
			}
		} else if (!preview && reader->name() == QLatin1String("referenceLine")) {
			auto* line = new ReferenceLine(this, QString(), true);
			line->setIsLoading(true);
			if (line->load(reader, preview))
				addChildFast(line);
			else {
				delete line;
				return false;
			}
		} else if (!preview && reader->name() == QLatin1String("referenceRange")) {
			auto* range = new ReferenceRange(this, QString(), true);
			range->setIsLoading(true);
			if (range->load(reader, preview))
				addChildFast(range);
			else {
				delete range;
				return false;
			}
		} else if (reader->name() == QLatin1String("boxPlot")) {
			auto* boxPlot = new BoxPlot(QStringLiteral("BoxPlot"), true);
			boxPlot->setIsLoading(true);
			if (boxPlot->load(reader, preview))
				addChildFast(boxPlot);
			else {
				delete boxPlot;
				return false;
			}
		} else if (reader->name() == QLatin1String("barPlot")) {
			auto* barPlot = new BarPlot(QStringLiteral("BarPlot"));
			barPlot->setIsLoading(true);
			if (barPlot->load(reader, preview))
				addChildFast(barPlot);
			else {
				delete barPlot;
				return false;
			}
		} else if (reader->name() == QLatin1String("lollipopPlot")) {
			auto* plot = new LollipopPlot(QStringLiteral("LollipopPlot"));
			plot->setIsLoading(true);
			if (plot->load(reader, preview))
				addChildFast(plot);
			else {
				delete plot;
				return false;
			}
		} else if (reader->name() == QLatin1String("Histogram")) {
			auto* hist = new Histogram(QStringLiteral("Histogram"), true);
			hist->setIsLoading(true);
			if (hist->load(reader, preview))
				addChildFast(hist);
			else {
				delete hist;
				return false;
			}
		} else if (reader->name() == QLatin1String("QQPlot")) {
			auto* plot = new QQPlot(QStringLiteral("Q-Q Plot"));
			plot->setIsLoading(true);
			if (plot->load(reader, preview))
				addChildFast(plot);
			else {
				delete plot;
				return false;
			}
		} else if (reader->name() == QLatin1String("KDEPlot")) {
			auto* plot = new KDEPlot(QStringLiteral("KDE Plot"));
			plot->setIsLoading(true);
			if (plot->load(reader, preview))
				addChildFast(plot);
			else
				return false;
		} else if (reader->name() == QLatin1String("ProcessBehaviorChart")) {
			auto* plot = new ProcessBehaviorChart(QStringLiteral("Process Behavior Chart"), true);
			plot->setIsLoading(true);
			if (plot->load(reader, preview))
				addChildFast(plot);
			else
				return false;
		} else if (reader->name() == QLatin1String("RunChart")) {
			auto* plot = new RunChart(QStringLiteral("Run Chart"));
			plot->setIsLoading(true);
			if (plot->load(reader, preview))
				addChildFast(plot);
			else
				return false;
		} else if (reader->name() == QLatin1String("cartesianPlot")) {
			auto* plot = new CartesianPlot(QString(), true);
			plot->setIsLoading(true);
			if (!plot->load(reader, preview)) {
				delete plot;
				return false;
			} else
				addChildFast(plot);
		} else { // unknown element
			if (!preview)
				reader->raiseUnknownElementWarning();
			if (!reader->skipToEndElement())
				return false;
		}
	}

	if (preview)
		return true;

	d->updatePlotColorPalette();

	return true;
}

// ##############################################################################
// #########################  Theme management ##################################
// ##############################################################################
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
	if (config.hasGroup(QStringLiteral("Theme"))) {
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
	if (d->plotColorMode == CartesianPlot::PlotColorMode::Theme)
		d->updatePlotColorPalette();

	// load the theme for all the children
	const auto& elements = children<WorksheetElement>(ChildIndexFlag::IncludeHidden);
	for (auto* child : elements)
		child->loadThemeConfig(config);

	d->update(this->rect());

	Q_EMIT changed();
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

void CartesianPlotPrivate::updatePlotColorPalette() {
	if (plotColorMode == CartesianPlot::PlotColorMode::Theme) {
		// initialize colors from the current theme
		// if a theme was used, initialize the color palette
		if (!theme.isEmpty()) {
			KConfig config(ThemeHandler::themeFilePath(theme), KConfig::SimpleConfig);
			if (config.hasGroup(QStringLiteral("Theme"))) {
				KConfigGroup group = config.group(QStringLiteral("Theme"));

				// read the five colors defining the palette
				plotColors.clear();
				plotColors.append(group.readEntry(QStringLiteral("ThemePaletteColor1"), QColor()));
				plotColors.append(group.readEntry(QStringLiteral("ThemePaletteColor2"), QColor()));
				plotColors.append(group.readEntry(QStringLiteral("ThemePaletteColor3"), QColor()));
				plotColors.append(group.readEntry(QStringLiteral("ThemePaletteColor4"), QColor()));
				plotColors.append(group.readEntry(QStringLiteral("ThemePaletteColor5"), QColor()));
			} else
				plotColors = defaultColorPalette; // initialize with default colors
		} else
			plotColors = defaultColorPalette; // initialize with default colors
	} else {
		// initialize colors from the current color map
		plotColors = ColorMapsManager::instance()->colors(plotColorMap);
	}

	if (!q->isLoading()) {
		const auto& plots = q->children<Plot>();
		if (!theme.isEmpty()) {
			KConfig config(ThemeHandler::themeFilePath(theme), KConfig::SimpleConfig);
			for (auto* plot : plots)
				plot->loadThemeConfig(config);
		} else {
			KConfig config;
			for (auto* plot : plots)
				plot->loadThemeConfig(config);
		}
	}
}

const QList<QColor>& CartesianPlot::plotColors() const {
	Q_D(const CartesianPlot);
	return d->plotColors;
}

const QColor CartesianPlot::plotColor(int index) const {
	Q_D(const CartesianPlot);
	const int i = index % d->plotColors.count();
	return d->plotColors.at(i);
}

void CartesianPlot::setXRange(int index, const Range<double>& range) {
	setRange(Dimension::X, index, range);
}

void CartesianPlot::setYRange(int index, const Range<double>& range) {
	setRange(Dimension::Y, index, range);
}
