/***************************************************************************
    File                 : CartesianPlot.cpp
    Project              : LabPlot
    Description          : Cartesian plot
    --------------------------------------------------------------------
    Copyright            : (C) 2011-2020 by Alexander Semke (alexander.semke@web.de)
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
#include "backend/worksheet/plots/cartesian/ReferenceLine.h"
#include "backend/worksheet/plots/PlotArea.h"
#include "backend/worksheet/plots/AbstractPlotPrivate.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/Axis.h"
#include "backend/worksheet/Image.h"
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

#include <array>
#include <cmath>

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

	d->rangeType = RangeType::Free;
	d->xRangeFormat = CartesianPlot::Numeric;
	d->yRangeFormat = CartesianPlot::Numeric;
	d->xRangeDateTimeFormat = "yyyy-MM-dd hh:mm:ss";
	d->yRangeDateTimeFormat = "yyyy-MM-dd hh:mm:ss";
	d->rangeFirstValues = 1000;
	d->rangeLastValues = 1000;
	d->autoScaleX = true;
	d->autoScaleY = true;
	d->xScale = Scale::Linear;
	d->yScale = Scale::Linear;
	d->xRangeBreakingEnabled = false;
	d->yRangeBreakingEnabled = false;

	//the following factor determines the size of the offset between the min/max points of the curves
	//and the coordinate system ranges, when doing auto scaling
	//Factor 0 corresponds to the exact match - min/max values of the curves correspond to the start/end values of the ranges.
	//TODO: make this factor optional.
	//Provide in the UI the possibility to choose between "exact" or 0% offset, 2%, 5% and 10% for the auto fit option
	d->autoScaleOffsetFactor = 0.0f;

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
	case FourAxes: {
			d->xMin = 0.0;
			d->xMax = 1.0;
			d->yMin = 0.0;
			d->yMax = 1.0;

			//Axes
			Axis* axis = new Axis("x axis 1", Axis::AxisHorizontal);
			axis->setDefault(true);
			axis->setSuppressRetransform(true);
			addChild(axis);
			axis->setPosition(Axis::AxisBottom);
			axis->setStart(0);
			axis->setEnd(1);
			axis->setMajorTicksDirection(Axis::ticksIn);
			axis->setMajorTicksNumber(6);
			axis->setMinorTicksDirection(Axis::ticksIn);
			axis->setMinorTicksNumber(1);
			axis->setSuppressRetransform(false);

			axis = new Axis("x axis 2", Axis::AxisHorizontal);
			axis->setDefault(true);
			axis->setSuppressRetransform(true);
			addChild(axis);
			axis->setPosition(Axis::AxisTop);
			axis->setStart(0);
			axis->setEnd(1);
			axis->setMajorTicksDirection(Axis::ticksIn);
			axis->setMajorTicksNumber(6);
			axis->setMinorTicksDirection(Axis::ticksIn);
			axis->setMinorTicksNumber(1);
			QPen pen = axis->minorGridPen();
			pen.setStyle(Qt::NoPen);
			axis->setMajorGridPen(pen);
			pen = axis->minorGridPen();
			pen.setStyle(Qt::NoPen);
			axis->setMinorGridPen(pen);
			axis->setLabelsPosition(Axis::NoLabels);
			axis->title()->setText(QString());
			axis->setSuppressRetransform(false);

			axis = new Axis("y axis 1", Axis::AxisVertical);
			axis->setDefault(true);
			axis->setSuppressRetransform(true);
			addChild(axis);
			axis->setPosition(Axis::AxisLeft);
			axis->setStart(0);
			axis->setEnd(1);
			axis->setMajorTicksDirection(Axis::ticksIn);
			axis->setMajorTicksNumber(6);
			axis->setMinorTicksDirection(Axis::ticksIn);
			axis->setMinorTicksNumber(1);
			axis->setSuppressRetransform(false);

			axis = new Axis("y axis 2", Axis::AxisVertical);
			axis->setDefault(true);
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
			axis->setMinorGridPen(pen);
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
			axis->setDefault(true);
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
			axis->setDefault(true);
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

			d->horizontalPadding = Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Centimeter);
			d->verticalPadding = Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Centimeter);

			QPen pen = m_plotArea->borderPen();
			pen.setStyle(Qt::NoPen);
			m_plotArea->setBorderPen(pen);

			Axis* axis = new Axis("x axis 1", Axis::AxisHorizontal);
			axis->setDefault(true);
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
			axis->setDefault(true);
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

			d->horizontalPadding = Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Centimeter);
			d->verticalPadding = Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Centimeter);

			QPen pen = m_plotArea->borderPen();
			pen.setStyle(Qt::NoPen);
			m_plotArea->setBorderPen(pen);

			Axis* axis = new Axis("x axis 1", Axis::AxisHorizontal);
			axis->setDefault(true);
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
			axis->setDefault(true);
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
	float x = Worksheet::convertToSceneUnits(2, Worksheet::Unit::Centimeter);
	float y = Worksheet::convertToSceneUnits(2, Worksheet::Unit::Centimeter);
	float w = Worksheet::convertToSceneUnits(10, Worksheet::Unit::Centimeter);
	float h = Worksheet::convertToSceneUnits(10, Worksheet::Unit::Centimeter);

	//all plot children are initialized -> set the geometry of the plot in scene coordinates.
	d->rect = QRectF(x,y,w,h);
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
	addConvolutionCurveAction = new QAction(QIcon::fromTheme("labplot-xy-curve"),i18n("(De-)Convolution"), this);
	addCorrelationCurveAction = new QAction(QIcon::fromTheme("labplot-xy-curve"),i18n("Auto-/Cross-Correlation"), this);

	addLegendAction = new QAction(QIcon::fromTheme("text-field"), i18n("Legend"), this);
	if (children<CartesianPlotLegend>().size()>0)
		addLegendAction->setEnabled(false);	//only one legend is allowed -> disable the action

	addHorizontalAxisAction = new QAction(QIcon::fromTheme("labplot-axis-horizontal"), i18n("Horizontal Axis"), this);
	addVerticalAxisAction = new QAction(QIcon::fromTheme("labplot-axis-vertical"), i18n("Vertical Axis"), this);
	addTextLabelAction = new QAction(QIcon::fromTheme("draw-text"), i18n("Text Label"), this);
	addImageAction = new QAction(QIcon::fromTheme("viewimage"), i18n("Image"), this);
	addCustomPointAction = new QAction(QIcon::fromTheme("draw-cross"), i18n("Custom Point"), this);
	addReferenceLineAction = new QAction(QIcon::fromTheme("draw-line"), i18n("Reference Line"), this);

	connect(addCurveAction, &QAction::triggered, this, &CartesianPlot::addCurve);
	connect(addHistogramAction,&QAction::triggered, this, &CartesianPlot::addHistogram);
	connect(addEquationCurveAction, &QAction::triggered, this, &CartesianPlot::addEquationCurve);
	connect(addDataReductionCurveAction, &QAction::triggered, this, &CartesianPlot::addDataReductionCurve);
	connect(addDifferentiationCurveAction, &QAction::triggered, this, &CartesianPlot::addDifferentiationCurve);
	connect(addIntegrationCurveAction, &QAction::triggered, this, &CartesianPlot::addIntegrationCurve);
	connect(addInterpolationCurveAction, &QAction::triggered, this, &CartesianPlot::addInterpolationCurve);
	connect(addSmoothCurveAction, &QAction::triggered, this, &CartesianPlot::addSmoothCurve);
	connect(addFitCurveAction, &QAction::triggered, this, &CartesianPlot::addFitCurve);
	connect(addFourierFilterCurveAction, &QAction::triggered, this, &CartesianPlot::addFourierFilterCurve);
	connect(addFourierTransformCurveAction, &QAction::triggered, this, &CartesianPlot::addFourierTransformCurve);
	connect(addConvolutionCurveAction, &QAction::triggered, this, &CartesianPlot::addConvolutionCurve);
	connect(addCorrelationCurveAction, &QAction::triggered, this, &CartesianPlot::addCorrelationCurve);

	connect(addLegendAction, &QAction::triggered, this, static_cast<void (CartesianPlot::*)()>(&CartesianPlot::addLegend));
	connect(addHorizontalAxisAction, &QAction::triggered, this, &CartesianPlot::addHorizontalAxis);
	connect(addVerticalAxisAction, &QAction::triggered, this, &CartesianPlot::addVerticalAxis);
	connect(addTextLabelAction, &QAction::triggered, this, &CartesianPlot::addTextLabel);
	connect(addImageAction, &QAction::triggered, this, &CartesianPlot::addImage);
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

	addFourierFilterAction = new QAction(QIcon::fromTheme("labplot-xy-fourier-filter-curve"), i18n("Fourier Filter"), this);
	addFourierTransformAction = new QAction(QIcon::fromTheme("labplot-xy-fourier-transform-curve"), i18n("Fourier Transform"), this);

	connect(addDataReductionAction, &QAction::triggered, this, &CartesianPlot::addDataReductionCurve);
	connect(addDifferentiationAction, &QAction::triggered, this, &CartesianPlot::addDifferentiationCurve);
	connect(addIntegrationAction, &QAction::triggered, this, &CartesianPlot::addIntegrationCurve);
	connect(addInterpolationAction, &QAction::triggered, this, &CartesianPlot::addInterpolationCurve);
	connect(addSmoothAction, &QAction::triggered, this, &CartesianPlot::addSmoothCurve);
	connect(addConvolutionAction, &QAction::triggered, this, &CartesianPlot::addConvolutionCurve);
	connect(addCorrelationAction, &QAction::triggered, this, &CartesianPlot::addCorrelationCurve);
	for (const auto& action : addFitAction)
		connect(action, &QAction::triggered, this, &CartesianPlot::addFitCurve);
	connect(addFourierFilterAction, &QAction::triggered, this, &CartesianPlot::addFourierFilterCurve);
	connect(addFourierTransformAction, &QAction::triggered, this, &CartesianPlot::addFourierTransformCurve);

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
	connect(zoomInAction, &QAction::triggered, this, &CartesianPlot::zoomIn);
	connect(zoomOutAction, &QAction::triggered, this, &CartesianPlot::zoomOut);
	connect(zoomInXAction, &QAction::triggered, this, &CartesianPlot::zoomInX);
	connect(zoomOutXAction, &QAction::triggered, this, &CartesianPlot::zoomOutX);
	connect(zoomInYAction, &QAction::triggered, this, &CartesianPlot::zoomInY);
	connect(zoomOutYAction, &QAction::triggered, this, &CartesianPlot::zoomOutY);
	connect(shiftLeftXAction, &QAction::triggered, this, &CartesianPlot::shiftLeftX);
	connect(shiftRightXAction, &QAction::triggered, this, &CartesianPlot::shiftRightX);
	connect(shiftUpYAction, &QAction::triggered, this, &CartesianPlot::shiftUpY);
	connect(shiftDownYAction, &QAction::triggered, this, &CartesianPlot::shiftDownY);

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
	dataAnalysisMenu->addSeparator();
	dataAnalysisMenu->addAction(addConvolutionAction);
	dataAnalysisMenu->addAction(addCorrelationAction);
	dataAnalysisMenu->addSeparator();
// 	dataAnalysisMenu->insertMenu(nullptr, dataManipulationMenu);
	dataAnalysisMenu->addAction(addDataReductionAction);

	//themes menu
	themeMenu = new QMenu(i18n("Apply Theme"));
	themeMenu->setIcon(QIcon::fromTheme("color-management"));
	auto* themeWidget = new ThemesWidget(nullptr);
	connect(themeWidget, &ThemesWidget::themeSelected, this, &CartesianPlot::loadTheme);
	connect(themeWidget, &ThemesWidget::themeSelected, themeMenu, &QMenu::close);

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


	menu->insertMenu(firstAction, addNewMenu);
	menu->insertSeparator(firstAction);
	menu->insertMenu(firstAction, zoomMenu);
	menu->insertSeparator(firstAction);
	menu->insertMenu(firstAction, themeMenu);
	menu->insertSeparator(firstAction);

	visibilityAction->setChecked(isVisible());
	menu->insertAction(firstAction, visibilityAction);

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
	Q_D(CartesianPlot);
	if (op == ScaleAuto) {
		if (d->curvesXMinMaxIsDirty || d->curvesYMinMaxIsDirty || !autoScaleX() || !autoScaleY()) {
			d->curvesXMinMaxIsDirty = true;
			d->curvesYMinMaxIsDirty = true;
		}
		scaleAuto();
	} else if (op == ScaleAutoX) setAutoScaleX(true);
	else if (op == ScaleAutoY) setAutoScaleY(true);
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

CLASS_SHARED_D_READER_IMPL(CartesianPlot, QPen, cursorPen, cursorPen);
CLASS_SHARED_D_READER_IMPL(CartesianPlot, bool, cursor0Enable, cursor0Enable);
CLASS_SHARED_D_READER_IMPL(CartesianPlot, bool, cursor1Enable, cursor1Enable);
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
// 		const double horizontalRatio = m_rect.width() / m_private->rect.width();
// 		const double verticalRatio = m_rect.height() / m_private->rect.height();

		qSwap(m_private->rect, m_rect);

// 		m_private->q->handleResize(horizontalRatio, verticalRatio, false);
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
	if (xMin != d->xMin && xMin != -INFINITY && xMin != INFINITY) {
		d->curvesYMinMaxIsDirty = true;
		exec(new CartesianPlotSetXMinCmd(d, xMin, ki18n("%1: set min x")));
		if (d->autoScaleY)
			scaleAutoY();
	}
}

STD_SETTER_CMD_IMPL_F_S(CartesianPlot, SetXMax, double, xMax, retransformScales)
void CartesianPlot::setXMax(double xMax) {
	Q_D(CartesianPlot);
	if (xMax != d->xMax && xMax != -INFINITY && xMax != INFINITY) {
		d->curvesYMinMaxIsDirty = true;
		exec(new CartesianPlotSetXMaxCmd(d, xMax, ki18n("%1: set max x")));
		if (d->autoScaleY)
			scaleAutoY();
	}
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
	if (yMin != d->yMin) {
		d->curvesXMinMaxIsDirty = true;
		exec(new CartesianPlotSetYMinCmd(d, yMin, ki18n("%1: set min y")));
		if (d->autoScaleX)
			scaleAutoX();
	}
}

STD_SETTER_CMD_IMPL_F_S(CartesianPlot, SetYMax, double, yMax, retransformScales)
void CartesianPlot::setYMax(double yMax) {
	Q_D(CartesianPlot);
	if (yMax != d->yMax) {
		d->curvesXMinMaxIsDirty = true;
		exec(new CartesianPlotSetYMaxCmd(d, yMax, ki18n("%1: set max y")));
		if (d->autoScaleX)
			scaleAutoX();
	}
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
			d->cursor0Pos.setX(d->cSystem->mapSceneToLogical(QPointF(0,0)).x());
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
			d->cursor1Pos.setX(d->cSystem->mapSceneToLogical(QPointF(0,0)).x());
			mousePressCursorModeSignal(1, d->cursor1Pos); // simulate mousePress to update values in the cursor dock
		}
		exec(new CartesianPlotSetCursor1EnableCmd(d, enable, ki18n("%1: Cursor1 enable")));
	}
}

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
	XYDifferentiationCurve* curve = new XYDifferentiationCurve("Differentiation");
	const XYCurve* curCurve = currentCurve();
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
	XYIntegrationCurve* curve = new XYIntegrationCurve("Integration");
	const XYCurve* curCurve = currentCurve();
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
	XYInterpolationCurve* curve = new XYInterpolationCurve("Interpolation");
	const XYCurve* curCurve = currentCurve();
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
	XYSmoothCurve* curve = new XYSmoothCurve("Smooth");
	const XYCurve* curCurve = currentCurve();
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
	DEBUG("CartesianPlot::addFitCurve()");
	XYFitCurve* curve = new XYFitCurve("fit");
	const XYCurve* curCurve = currentCurve();
	if (curCurve) {
		beginMacro( i18n("%1: fit to '%2'", name(), curCurve->name()) );
		curve->setName( i18n("Fit to '%1'", curCurve->name()) );
		curve->setDataSourceType(XYAnalysisCurve::DataSourceType::Curve);
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
	XYFourierFilterCurve* curve = new XYFourierFilterCurve("Fourier filter");
	const XYCurve* curCurve = currentCurve();
	if (curCurve) {
		beginMacro( i18n("%1: Fourier filtering of '%2'", name(), curCurve->name()) );
		curve->setName( i18n("Fourier filtering of '%1'", curCurve->name()) );
		curve->setDataSourceType(XYAnalysisCurve::DataSourceType::Curve);
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

void CartesianPlot::addImage() {
	Image* image = new Image("image");
	this->addChild(image);
}

void CartesianPlot::addCustomPoint() {
	CustomPoint* point = new CustomPoint(this, "custom point");
	this->addChild(point);
	point->retransform();
}

void CartesianPlot::addReferenceLine() {
	ReferenceLine* line = new ReferenceLine(this, "reference line");
	this->addChild(line);
	line->retransform();
}

int CartesianPlot::curveCount(){
	return children<XYCurve>().length();
}

const XYCurve* CartesianPlot::getCurve(int index){
	return children<XYCurve>().at(index);
}

double CartesianPlot::cursorPos(int cursorNumber) {
	Q_D(CartesianPlot);
	if (cursorNumber == 0)
		return d->cursor0Pos.x();
	else
		return d->cursor1Pos.x();
}

void CartesianPlot::childAdded(const AbstractAspect* child) {
	Q_D(CartesianPlot);
	const auto* curve = qobject_cast<const XYCurve*>(child);
	if (curve) {
		connect(curve, &XYCurve::dataChanged, this, &CartesianPlot::dataChanged);
		connect(curve, &XYCurve::xDataChanged, this, &CartesianPlot::xDataChanged);
		connect(curve, &XYCurve::xErrorTypeChanged, this, &CartesianPlot::dataChanged);
		connect(curve, &XYCurve::xErrorPlusColumnChanged, this, &CartesianPlot::dataChanged);
		connect(curve, &XYCurve::xErrorMinusColumnChanged, this, &CartesianPlot::dataChanged);
		connect(curve, &XYCurve::yDataChanged, this, &CartesianPlot::yDataChanged);
		connect(curve, &XYCurve::yErrorTypeChanged, this, &CartesianPlot::dataChanged);
		connect(curve, &XYCurve::yErrorPlusColumnChanged, this, &CartesianPlot::dataChanged);
		connect(curve, &XYCurve::yErrorMinusColumnChanged, this, &CartesianPlot::dataChanged);
		connect(curve, static_cast<void (XYCurve::*)(bool)>(&XYCurve::visibilityChanged),
				this, &CartesianPlot::curveVisibilityChanged);

		//update the legend on changes of the name, line and symbol styles
		connect(curve, &XYCurve::aspectDescriptionChanged, this, &CartesianPlot::updateLegend);
		connect(curve, &XYCurve::aspectDescriptionChanged, this, &CartesianPlot::curveNameChanged);
		connect(curve, &XYCurve::lineTypeChanged, this, &CartesianPlot::updateLegend);
		connect(curve, &XYCurve::linePenChanged, this, &CartesianPlot::updateLegend);
		connect(curve, &XYCurve::linePenChanged, this, static_cast<void (CartesianPlot::*)(QPen)>(&CartesianPlot::curveLinePenChanged));
		connect(curve, &XYCurve::lineOpacityChanged, this, &CartesianPlot::updateLegend);
		connect(curve, &XYCurve::symbolsStyleChanged, this, &CartesianPlot::updateLegend);
		connect(curve, &XYCurve::symbolsSizeChanged, this, &CartesianPlot::updateLegend);
		connect(curve, &XYCurve::symbolsRotationAngleChanged, this, &CartesianPlot::updateLegend);
		connect(curve, &XYCurve::symbolsOpacityChanged, this, &CartesianPlot::updateLegend);
		connect(curve, &XYCurve::symbolsBrushChanged, this, &CartesianPlot::updateLegend);
		connect(curve, &XYCurve::symbolsPenChanged, this, &CartesianPlot::updateLegend);

		updateLegend();
		d->curvesXMinMaxIsDirty = true;
		d->curvesYMinMaxIsDirty = true;

		//in case the first curve is added, check whether we start plotting datetime data
		if (children<XYCurve>().size() == 1) {
			const auto* col = dynamic_cast<const Column*>(curve->xColumn());
			if (col) {
				if (col->columnMode() == AbstractColumn::ColumnMode::DateTime) {
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
				if (col->columnMode() == AbstractColumn::ColumnMode::DateTime) {
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
		emit curveAdded(curve);

	} else {
		const auto* hist = qobject_cast<const Histogram*>(child);
		if (hist) {
			connect(hist, &Histogram::dataChanged, this, &CartesianPlot::dataChanged);
			connect(hist, &Histogram::visibilityChanged, this, &CartesianPlot::curveVisibilityChanged);

			updateLegend();
		}
		// if an element is hovered, the curves which are handled manually in this class
		// must be unhovered
		const auto* element = static_cast<const WorksheetElement*>(child);
		connect(element, &WorksheetElement::hovered, this, &CartesianPlot::childHovered);
	}

	if (!isLoading()) {
		//if a theme was selected, apply the theme settings for newly added children,
		//load default theme settings otherwise.
		const auto* elem = dynamic_cast<const WorksheetElement*>(child);
		if (elem) {
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

void CartesianPlot::childRemoved(const AbstractAspect* parent, const AbstractAspect* before, const AbstractAspect* child) {
	Q_UNUSED(parent);
	Q_UNUSED(before);
	if (m_legend == child) {
		if (m_menusInitialized)
			addLegendAction->setEnabled(true);
		m_legend = nullptr;
	} else {
		const auto* curve = qobject_cast<const XYCurve*>(child);
		if (curve) {
			updateLegend();
			emit curveRemoved(curve);
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
	bool curveSender = dynamic_cast<XYCurve*>(QObject::sender()) != nullptr;
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

/*!
	called when in one of the curves the data was changed.
	Autoscales the coordinate system and the x-axes, when "auto-scale" is active.
*/
void CartesianPlot::dataChanged() {
	if (project() && project()->isLoading())
		return;

	Q_D(CartesianPlot);
	d->curvesXMinMaxIsDirty = true;
	d->curvesYMinMaxIsDirty = true;
	bool updated = false;
	if (d->autoScaleX && d->autoScaleY)
		updated = scaleAuto();
	else if (d->autoScaleX)
		updated = scaleAutoX();
	else if (d->autoScaleY)
		updated = scaleAutoY();

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
				//no sender available, the function was called directly in the file filter (live data source got new data)
				//or in Project::load() -> retransform all available curves since we don't know which curves are affected.
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
		if (curve) {
			const AbstractColumn* col = curve->xColumn();
			if (col->columnMode() == AbstractColumn::ColumnMode::DateTime && d->xRangeFormat != CartesianPlot::DateTime) {
				setUndoAware(false);
				setXRangeFormat(CartesianPlot::DateTime);
				setUndoAware(true);
			}
		}
	}
	emit curveDataChanged(dynamic_cast<XYCurve*>(QObject::sender()));
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
		updated = this->scaleAutoY();

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
		if (curve) {
			const AbstractColumn* col = curve->yColumn();
			if (col->columnMode() == AbstractColumn::ColumnMode::DateTime && d->xRangeFormat != CartesianPlot::DateTime) {
				setUndoAware(false);
				setYRangeFormat(CartesianPlot::DateTime);
				setUndoAware(true);
			}
		}
	}
	emit curveDataChanged(dynamic_cast<XYCurve*>(QObject::sender()));
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

	emit curveVisibilityChangedSignal();
}

void CartesianPlot::curveLinePenChanged(QPen pen) {
	const auto* curve = qobject_cast<const XYCurve*>(QObject::sender());
	emit curveLinePenChanged(pen, curve->name());
}

void CartesianPlot::setMouseMode(const MouseMode mouseMode) {
	Q_D(CartesianPlot);

	d->mouseMode = mouseMode;
	d->setHandlesChildEvents(mouseMode != MouseMode::Selection);

	QList<QGraphicsItem*> items = d->childItems();
	if (d->mouseMode == MouseMode::Selection) {
		d->setZoomSelectionBandShow(false);
		d->setCursor(Qt::ArrowCursor);
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

void CartesianPlot::setLocked(bool locked) {
	Q_D(CartesianPlot);
	d->locked = locked;
}

bool CartesianPlot::scaleAutoX() {
	Q_D(CartesianPlot);
	if (d->curvesXMinMaxIsDirty) {
		calculateCurvesXMinMax(false);

		//loop over all histograms and determine the maximum and minimum x-values
		for (const auto* curve : this->children<const Histogram>()) {
			if (!curve->isVisible())
				continue;
			if (!curve->dataColumn())
				continue;

			const double min = curve->getXMinimum();
			if (min < d->curvesXMin)
				d->curvesXMin = min;

			const double max = curve->getXMaximum();
			if (max > d->curvesXMax)
				d->curvesXMax = max;
		}

		// do it at the end, because it must be from the real min/max values
		double errorBarsCapSize = -1;
		for (auto* curve : this->children<const XYCurve>()) {
			if (curve->yErrorType() != XYCurve::ErrorType::NoError) {
				errorBarsCapSize = qMax(errorBarsCapSize, curve->errorBarsCapSize());
			}
		}

		if (errorBarsCapSize > 0) {
			// must be done, because retransformScales uses xMin/xMax
			if (d->curvesXMin != d->xMin && d->curvesXMin != INFINITY)
				d->xMin = d->curvesXMin;

			if (d->curvesXMax != d->xMax && d->curvesXMax != -INFINITY)
				d->xMax = d->curvesXMax;
			// When the previous scale is completely different. The mapTo functions scale with wrong values. To prevent
			// this a rescale must be done.
			// The errorBarsCapSize is in Scene coordinates. So this value must be transformed into a logical value. Due
			// to nonlinear scalings it cannot only be multiplied with a scaling factor and depends on the position of the
			// column value
			// dirty hack: call setIsLoading(true) to suppress the call of retransform() in retransformScales() since a
			// retransform is already done at the end of this function
			setIsLoading(true);
			d->retransformScales();
			setIsLoading(false);
			QPointF point = coordinateSystem()->mapLogicalToScene(QPointF(d->curvesXMin, 0), AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);
			point.setX(point.x() - errorBarsCapSize);
			point = coordinateSystem()->mapSceneToLogical(point, AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);
			// Problem is, when the scaling is not linear (for example log(x)) and the minimum is 0. In this
			// case mapLogicalToScene returns (0,0) which is smaller than the curves minimum
			if (point.x() < d->curvesXMin)
				d->curvesXMin = point.x();

			point = coordinateSystem()->mapLogicalToScene(QPointF(d->curvesXMax, 0), AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);
			point.setX(point.x() + errorBarsCapSize);
			point = coordinateSystem()->mapSceneToLogical(point, AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);
			if (point.x() > d->curvesXMax)
				d->curvesXMax = point.x();
		}
		d->curvesYMinMaxIsDirty = true;
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
		d->retransformScales();
	}

	return update;
}

bool CartesianPlot::scaleAutoY() {
	Q_D(CartesianPlot);

	if (d->curvesYMinMaxIsDirty) {
		calculateCurvesYMinMax(false); // loop over all curves

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

		// do it at the end, because it must be from the real min/max values
		double errorBarsCapSize = -1;
		for (auto* curve : this->children<const XYCurve>()) {
			if (curve->xErrorType() != XYCurve::ErrorType::NoError) {
				errorBarsCapSize = qMax(errorBarsCapSize, curve->errorBarsCapSize());
			}
		}

		if (errorBarsCapSize > 0) {
			if (d->curvesYMin != d->yMin && d->curvesYMin != INFINITY)
				d->yMin = d->curvesYMin;

			if (d->curvesYMax != d->yMax && d->curvesYMax != -INFINITY)
				d->yMax = d->curvesYMax;
			setIsLoading(true);
			d->retransformScales();
			setIsLoading(false);
			QPointF point = coordinateSystem()->mapLogicalToScene(QPointF(0, d->curvesYMin), AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);
			point.setY(point.y() + errorBarsCapSize);
			point = coordinateSystem()->mapSceneToLogical(point, AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);
			if (point.y() < d->curvesYMin)
				d->curvesYMin = point.y();

			point = coordinateSystem()->mapLogicalToScene(QPointF(0, d->curvesYMax), AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);
			point.setY(point.y() - errorBarsCapSize);
			point = coordinateSystem()->mapSceneToLogical(point, AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);
			if (point.y() > d->curvesYMax)
				d->curvesYMax = point.y();
		}

		d->curvesXMinMaxIsDirty = true;
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
		d->retransformScales();
	}

	return update;
}

void CartesianPlot::scaleAutoTriggered() {
	QAction* action = dynamic_cast<QAction*>(QObject::sender());
	if (!action)
		return;

	if (action == scaleAutoAction)
		scaleAuto();
	else if (action == scaleAutoXAction)
		setAutoScaleX(true);
	else if (action == scaleAutoYAction)
		setAutoScaleY(true);
}

bool CartesianPlot::scaleAuto() {
	DEBUG("CartesianPlot::scaleAuto()");
	Q_D(CartesianPlot);

	if (d->curvesXMinMaxIsDirty) {
		calculateCurvesXMinMax();
		double errorBarsCapSize = -1;
		for (auto* curve : this->children<const XYCurve>()) {
			if (curve->yErrorType() != XYCurve::ErrorType::NoError) {
				errorBarsCapSize = qMax(errorBarsCapSize, curve->errorBarsCapSize());
			}
		}

		if (errorBarsCapSize > 0) {
			if (d->curvesXMin != d->xMin && d->curvesXMin != INFINITY)
				d->xMin = d->curvesXMin;

			if (d->curvesXMax != d->xMax && d->curvesXMax != -INFINITY)
				d->xMax = d->curvesXMax;
			setIsLoading(true);
			d->retransformScales();
			setIsLoading(false);
			QPointF point = coordinateSystem()->mapLogicalToScene(QPointF(d->curvesXMin, 0), AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);
			point.setX(point.x() - errorBarsCapSize);
			point = coordinateSystem()->mapSceneToLogical(point, AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);
			if (point.x() < d->curvesXMin)
				d->curvesXMin = point.x();

			point = coordinateSystem()->mapLogicalToScene(QPointF(d->curvesXMax, 0), AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);
			point.setX(point.x() + errorBarsCapSize);
			point = coordinateSystem()->mapSceneToLogical(point, AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);
			if (point.x() > d->curvesXMax)
				d->curvesXMax = point.x();
		}
		d->curvesXMinMaxIsDirty = false;
	}

	if (d->curvesYMinMaxIsDirty) {
		calculateCurvesYMinMax();
		double errorBarsCapSize = -1;
		for (auto* curve : this->children<const XYCurve>()) {
			if (curve->xErrorType() != XYCurve::ErrorType::NoError) {
				errorBarsCapSize = qMax(errorBarsCapSize, curve->errorBarsCapSize());
			}
		}

		if (errorBarsCapSize > 0) {
			if (d->curvesYMin != d->yMin && d->curvesYMin != INFINITY)
				d->yMin = d->curvesYMin;

			if (d->curvesYMax != d->yMax && d->curvesYMax != -INFINITY)
				d->yMax = d->curvesYMax;
			setIsLoading(true);
			d->retransformScales();
			setIsLoading(false);
			QPointF point = coordinateSystem()->mapLogicalToScene(QPointF(0, d->curvesYMin), AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);
			point.setY(point.y() + errorBarsCapSize);
			point = coordinateSystem()->mapSceneToLogical(point, AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);
			if (point.y() < d->curvesYMin)
				d->curvesYMin = point.y();

			point = coordinateSystem()->mapLogicalToScene(QPointF(0, d->curvesYMax), AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);
			point.setY(point.y() - errorBarsCapSize);
			point = coordinateSystem()->mapSceneToLogical(point, AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);
			if (point.y() > d->curvesYMax)
				d->curvesYMax = point.y();
		}
		d->curvesYMinMaxIsDirty = false;
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

/*!
 * Calculates and sets curves y min and max. This function does not respect the range
 * of the y axis
 */
void CartesianPlot::calculateCurvesXMinMax(bool completeRange) {
	Q_D(CartesianPlot);

	d->curvesXMin = INFINITY;
	d->curvesXMax = -INFINITY;

	//loop over all xy-curves and determine the maximum and minimum x-values
	for (const auto* curve : this->children<const XYCurve>()) {
		if (!curve->isVisible())
			continue;

		auto* xColumn = curve->xColumn();
		if (!xColumn)
			continue;

		double min = d->curvesXMin;
		double max = d->curvesXMax;

		int start =0;
		int end = 0;
		if (d->rangeType == RangeType::Free && curve->yColumn()
				&& !completeRange) {
			curve->yColumn()->indicesMinMax(yMin(), yMax(), start, end);
			if (end < curve->yColumn()->rowCount())
				end ++;
		} else {
			switch (d->rangeType) {
			case RangeType::Free:
				start = 0;
				end = xColumn->rowCount();
				break;
			case RangeType::Last:
				start = xColumn->rowCount() - d->rangeLastValues;
				end = xColumn->rowCount();
				break;
			case RangeType::First:
				start = 0;
				end = d->rangeFirstValues;
				break;
			}
		}

		curve->minMaxX(start, end, min, max, true);
		if (min < d->curvesXMin)
			d->curvesXMin = min;

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
}

/*!
 * Calculates and sets curves y min and max. This function does not respect the range
 * of the x axis
 */
void CartesianPlot::calculateCurvesYMinMax(bool completeRange) {
	Q_D(CartesianPlot);

	d->curvesYMin = INFINITY;
	d->curvesYMax = -INFINITY;

	double min = d->curvesYMin;
	double max = d->curvesYMax;


	//loop over all xy-curves and determine the maximum and minimum y-values
	for (const auto* curve : this->children<const XYCurve>()) {
		if (!curve->isVisible())
			continue;

		auto* yColumn = curve->yColumn();
		if (!yColumn)
			continue;

		int start =0;
		int end = 0;
		if (d->rangeType == RangeType::Free && curve->xColumn() &&
				!completeRange) {
			curve->xColumn()->indicesMinMax(xMin(), xMax(), start, end);
			if (end < curve->xColumn()->rowCount())
				end ++; // because minMaxY excludes indexMax
		} else {
			switch (d->rangeType) {
				case RangeType::Free:
					start = 0;
					end = yColumn->rowCount();
					break;
				case RangeType::Last:
					start = yColumn->rowCount() - d->rangeLastValues;
					end = yColumn->rowCount();
					break;
				case RangeType::First:
					start = 0;
					end = d->rangeFirstValues;
					break;
			}
		}

		curve->minMaxY(start, end, min, max, true);

		if (min < d->curvesYMin)
			d->curvesYMin = min;

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

void CartesianPlot::zoomIn() {
	Q_D(CartesianPlot);

	setUndoAware(false);
	setAutoScaleX(false);
	setAutoScaleY(false);
	setUndoAware(true);
	d->curvesXMinMaxIsDirty = true;
	d->curvesYMinMaxIsDirty = true;
	zoom(true, true); //zoom in x
	zoom(false, true); //zoom in y
	d->retransformScales();
}

void CartesianPlot::zoomOut() {
	Q_D(CartesianPlot);

	setUndoAware(false);
	setAutoScaleX(false);
	setAutoScaleY(false);
	setUndoAware(true);
	d->curvesXMinMaxIsDirty = true;
	d->curvesYMinMaxIsDirty = true;
	zoom(true, false); //zoom out x
	zoom(false, false); //zoom out y
	d->retransformScales();
}

void CartesianPlot::zoomInX() {
	Q_D(CartesianPlot);

	setUndoAware(false);
	setAutoScaleX(false);
	setUndoAware(true);
	d->curvesYMinMaxIsDirty = true;
	zoom(true, true); //zoom in x
	if (d->autoScaleY && autoScaleY())
		return;

	d->retransformScales();
}

void CartesianPlot::zoomOutX() {
	Q_D(CartesianPlot);

	setUndoAware(false);
	setAutoScaleX(false);
	setUndoAware(true);
	d->curvesYMinMaxIsDirty = true;
	zoom(true, false); //zoom out x

	if (d->autoScaleY && autoScaleY())
		return;

	d->retransformScales();
}

void CartesianPlot::zoomInY() {
	Q_D(CartesianPlot);

	setUndoAware(false);
	setAutoScaleY(false);
	setUndoAware(true);
	d->curvesYMinMaxIsDirty = true;
	zoom(false, true); //zoom in y

	if (d->autoScaleX && autoScaleX())
		return;

	d->retransformScales();
}

void CartesianPlot::zoomOutY() {
	Q_D(CartesianPlot);

	setUndoAware(false);
	setAutoScaleY(false);
	setUndoAware(true);
	d->curvesYMinMaxIsDirty = true;
	zoom(false, false); //zoom out y

	if (d->autoScaleX && autoScaleX())
		return;

	d->retransformScales();
}

/*!
 * helper function called in other zoom*() functions
 * and doing the actual change of the data ranges.
 * @param x if set to \true the x-range is modified, the y-range for \c false
 * @param in the "zoom in" is performed if set to \c \true, "zoom out" for \c false
 */
void CartesianPlot::zoom(bool x, bool in) {
	Q_D(CartesianPlot);

	double min;
	double max;
	CartesianPlot::Scale scale;
	if (x) {
		min = d->xMin;
		max = d->xMax;
		scale = d->xScale;
	} else {
		min = d->yMin;
		max = d->yMax;
		scale = d->yScale;
	}

	double factor = m_zoomFactor;
	if (in)
		factor = 1/factor;

	switch (scale) {
	case Scale::Linear: {
		double oldRange = max - min;
		double newRange = (max - min) * factor;
		max = max + (newRange - oldRange) / 2;
		min = min - (newRange - oldRange) / 2;
		break;
	}
	case Scale::Log10:
	case Scale::Log10Abs: {
		double oldRange = log10(max) - log10(min);
		double newRange = (log10(max) - log10(min)) * factor;
		max = max * pow(10, (newRange - oldRange) / 2.);
		min = min / pow(10, (newRange - oldRange) / 2.);
		break;
	}
	case Scale::Log2:
	case Scale::Log2Abs: {
		double oldRange = log2(max) - log2(min);
		double newRange = (log2(max) - log2(min)) * factor;
		max = max * pow(2, (newRange - oldRange) / 2.);
		min = min / pow(2, (newRange - oldRange) / 2.);
		break;
	}
	case Scale::Ln:
	case Scale::LnAbs: {
		double oldRange = log(max) - log(min);
		double newRange = (log(max) - log(min)) * factor;
		max = max * exp((newRange - oldRange) / 2.);
		min = min / exp((newRange - oldRange) / 2.);
		break;
	}
	case Scale::Sqrt:
	case Scale::X2:
		break;
	}

	if (!std::isnan(min) && !std::isnan(max) && std::isfinite(min) && std::isfinite(max)) {
		if (x) {
			d->xMin = min;
			d->xMax = max;
		} else {
			d->yMin = min;
			d->yMax = max;
		}
	}
}

/*!
 * helper function called in other shift*() functions
 * and doing the actual change of the data ranges.
 * @param x if set to \true the x-range is modified, the y-range for \c false
 * @param leftOrDown the "shift left" for x or "shift dows" for y is performed if set to \c \true,
 * "shift right" or "shift up" for \c false
 */
void CartesianPlot::shift(bool x, bool leftOrDown) {
	Q_D(CartesianPlot);

	double min;
	double max;
	CartesianPlot::Scale scale;
	double offset = 0.0;
	double factor = 0.1;
	if (x) {
		min = d->xMin;
		max = d->xMax;
		scale = d->xScale;
	} else {
		min = d->yMin;
		max = d->yMax;
		scale = d->yScale;
	}

	if (leftOrDown)
		factor *= -1.;

	switch (scale) {
	case Scale::Linear: {
		offset = (max - min) * factor;
		min += offset;
		max += offset;
		break;
	}
	case Scale::Log10:
	case Scale::Log10Abs: {
		offset = (log10(max) - log10(min)) * factor;
		min *= pow(10, offset);
		max *= pow(10, offset);
		break;
	}
	case Scale::Log2:
	case Scale::Log2Abs: {
		offset = (log2(max) - log2(min)) * factor;
		min *= pow(2, offset);
		max *= pow(2, offset);
		break;
	}
	case Scale::Ln:
	case Scale::LnAbs: {
		offset = (log10(max) - log10(min)) * factor;
		min *= exp(offset);
		max *= exp(offset);
		break;
	}
	case Scale::Sqrt:
	case Scale::X2:
		break;
	}

	if (!std::isnan(min) && !std::isnan(max) && std::isfinite(min) && std::isfinite(max)) {
		if (x) {
			d->xMin = min;
			d->xMax = max;
		} else {
			d->yMin = min;
			d->yMax = max;
		}
	}
}

void CartesianPlot::shiftLeftX() {
	Q_D(CartesianPlot);

	setUndoAware(false);
	setAutoScaleX(false);
	setUndoAware(true);
	d->curvesYMinMaxIsDirty = true;
	shift(true, true);

	if (d->autoScaleY && scaleAutoY())
		return;

	d->retransformScales();
}

void CartesianPlot::shiftRightX() {
	Q_D(CartesianPlot);

	setUndoAware(false);
	setAutoScaleX(false);
	setUndoAware(true);
	d->curvesYMinMaxIsDirty = true;
	shift(true, false);

	if (d->autoScaleY && scaleAutoY())
		return;

	d->retransformScales();
}

void CartesianPlot::shiftUpY() {
	Q_D(CartesianPlot);

	setUndoAware(false);
	setAutoScaleY(false);
	setUndoAware(true);
	d->curvesXMinMaxIsDirty = true;
	shift(false, false);

	if (d->autoScaleX && scaleAutoX())
		return;

	d->retransformScales();
}

void CartesianPlot::shiftDownY() {
	Q_D(CartesianPlot);

	setUndoAware(false);
	setAutoScaleY(false);
	setUndoAware(true);
	d->curvesXMinMaxIsDirty = true;
	shift(false, true);

	if (d->autoScaleX && scaleAutoX())
		return;

	d->retransformScales();
}

void CartesianPlot::cursor() {
	Q_D(CartesianPlot);
	d->retransformScales();
}

void CartesianPlot::mousePressZoomSelectionMode(QPointF logicPos) {
	Q_D(CartesianPlot);
	d->mousePressZoomSelectionMode(logicPos);
}
void CartesianPlot::mousePressCursorMode(int cursorNumber, QPointF logicPos) {
	Q_D(CartesianPlot);
	d->mousePressCursorMode(cursorNumber, logicPos);
}
void CartesianPlot::mouseMoveZoomSelectionMode(QPointF logicPos) {
	Q_D(CartesianPlot);
	d->mouseMoveZoomSelectionMode(logicPos);
}
void CartesianPlot::mouseMoveCursorMode(int cursorNumber, QPointF logicPos) {
	Q_D(CartesianPlot);
	d->mouseMoveCursorMode(cursorNumber, logicPos);
}

void CartesianPlot::mouseReleaseZoomSelectionMode() {
	Q_D(CartesianPlot);
	d->mouseReleaseZoomSelectionMode();
}

void CartesianPlot::mouseHoverZoomSelectionMode(QPointF logicPos) {
	Q_D(CartesianPlot);
	d->mouseHoverZoomSelectionMode(logicPos);
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
	setData(0, static_cast<int>(WorksheetElement::WorksheetElementName::NameCartesianPlot));
	m_cursor0Text.prepare();
	m_cursor1Text.prepare();
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

	QVector<CartesianScale*> scales;

	//check ranges for log-scales
	if (xScale != CartesianPlot::Scale::Linear)
		checkXRange();

	//check whether we have x-range breaks - the first break, if available, should be valid
	bool hasValidBreak = (xRangeBreakingEnabled && !xRangeBreaks.list.isEmpty() && xRangeBreaks.list.first().isValid());

	static const int breakGap = 20;
	double sceneStart, sceneEnd, logicalStart, logicalEnd;

	//create x-scales
	int plotSceneStart = dataRect.x();
	int plotSceneEnd = dataRect.x() + dataRect.width();
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
	if (yScale != CartesianPlot::Scale::Linear)
		checkYRange();

	//check whether we have y-range breaks - the first break, if available, should be valid
	hasValidBreak = (yRangeBreakingEnabled && !yRangeBreaks.list.isEmpty() && yRangeBreaks.list.first().isValid());

	//create y-scales
	scales.clear();
	plotSceneStart = dataRect.y() + dataRect.height();
	plotSceneEnd = dataRect.y();
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
		emit q->xMinChanged(xMin);
	}

	if (xMax != xMaxPrev) {
		deltaXMax = xMax - xMaxPrev;
		emit q->xMaxChanged(xMax);
	}

	if (yMin != yMinPrev) {
		deltaYMin = yMin - yMinPrev;
		emit q->yMinChanged(yMin);
	}

	if (yMax != yMaxPrev) {
		deltaYMax = yMax - yMaxPrev;
		emit q->yMaxChanged(yMax);
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
	if (type == CartesianPlot::Scale::Linear)
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
void CartesianPlotPrivate::mousePressEvent(QGraphicsSceneMouseEvent *event) {

	if (mouseMode == CartesianPlot::MouseMode::ZoomSelection || mouseMode == CartesianPlot::MouseMode::ZoomXSelection || mouseMode == CartesianPlot::MouseMode::ZoomYSelection)
		emit q->mousePressZoomSelectionModeSignal(cSystem->mapSceneToLogical(event->pos(), AbstractCoordinateSystem::MappingFlag::Limit));
	else if (mouseMode == CartesianPlot::MouseMode::Cursor) {
		setCursor(Qt::SizeHorCursor);
		QPointF logicalPos = cSystem->mapSceneToLogical(event->pos(), AbstractCoordinateSystem::MappingFlag::Limit);
		double cursorPenWidth2 = cursorPen.width()/2.;
		if (cursorPenWidth2 < 10.)
			cursorPenWidth2 = 10.;
		if (cursor0Enable && qAbs(event->pos().x()-cSystem->mapLogicalToScene(QPointF(cursor0Pos.x(),yMin)).x()) < cursorPenWidth2) {
			selectedCursor = 0;
		} else if (cursor1Enable && qAbs(event->pos().x()-cSystem->mapLogicalToScene(QPointF(cursor1Pos.x(),yMin)).x()) < cursorPenWidth2) {
			selectedCursor = 1;
		} else if (QApplication::keyboardModifiers() & Qt::ControlModifier){
			cursor1Enable = true;
			selectedCursor = 1;
			emit q->cursor1EnableChanged(cursor1Enable);
		} else {
			cursor0Enable = true;
			selectedCursor = 0;
			emit q->cursor0EnableChanged(cursor0Enable);
		}
		emit q->mousePressCursorModeSignal(selectedCursor, logicalPos);

	} else {
		if (!locked && dataRect.contains(event->pos())) {
			panningStarted = true;
			m_panningStart = event->pos();
			setCursor(Qt::ClosedHandCursor);
		}
	}
	QGraphicsItem::mousePressEvent(event);
}

void CartesianPlotPrivate::mousePressZoomSelectionMode(QPointF logicalPos) {
	if (mouseMode == CartesianPlot::MouseMode::ZoomSelection) {

		if (logicalPos.x() < xMin)
			logicalPos.setX(xMin);

		if (logicalPos.x() > xMax)
			logicalPos.setX(xMax);

		if (logicalPos.y() < yMin)
			logicalPos.setY(yMin);

		if (logicalPos.y() > yMax)
			logicalPos.setY(yMax);

		m_selectionStart = cSystem->mapLogicalToScene(logicalPos, AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);

	} else if (mouseMode == CartesianPlot::MouseMode::ZoomXSelection) {
		logicalPos.setY(yMin); // must be done, because the other plots can have other ranges, value must be in the scenes
		m_selectionStart.setX(cSystem->mapLogicalToScene(logicalPos, AbstractCoordinateSystem::MappingFlag::SuppressPageClipping).x());
		m_selectionStart.setY(dataRect.y());
	} else if (mouseMode == CartesianPlot::MouseMode::ZoomYSelection) {
		logicalPos.setX(xMin); // must be done, because the other plots can have other ranges, value must be in the scenes
		m_selectionStart.setX(dataRect.x());
		m_selectionStart.setY(cSystem->mapLogicalToScene(logicalPos, AbstractCoordinateSystem::MappingFlag::SuppressPageClipping).y());
	}
	m_selectionEnd = m_selectionStart;
	m_selectionBandIsShown = true;
}

void CartesianPlotPrivate::mousePressCursorMode(int cursorNumber, QPointF logicalPos) {

	cursorNumber == 0 ? cursor0Enable = true : cursor1Enable = true;

	QPointF p1(logicalPos.x(), yMin);
	QPointF p2(logicalPos.x(), yMax);

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
	if (mouseMode == CartesianPlot::MouseMode::Selection) {
		if (panningStarted && dataRect.contains(event->pos()) ) {
			//don't retransform on small mouse movement deltas
			const int deltaXScene = (m_panningStart.x() - event->pos().x());
			const int deltaYScene = (m_panningStart.y() - event->pos().y());
			if (abs(deltaXScene) < 5 && abs(deltaYScene) < 5)
				return;

			const QPointF logicalEnd = cSystem->mapSceneToLogical(event->pos());
			const QPointF logicalStart = cSystem->mapSceneToLogical(m_panningStart);

			//handle the change in x
			switch (xScale) {
			case CartesianPlot::Scale::Linear: {
				const float deltaX = (logicalStart.x() - logicalEnd.x());
				xMax += deltaX;
				xMin += deltaX;
				break;
			}
			case CartesianPlot::Scale::Log10:
			case CartesianPlot::Scale::Log10Abs: {
				const float deltaX = log10(logicalStart.x()) - log10(logicalEnd.x());
				xMin *= pow(10, deltaX);
				xMax *= pow(10, deltaX);
				break;
			}
			case CartesianPlot::Scale::Log2:
			case CartesianPlot::Scale::Log2Abs: {
				const float deltaX = log2(logicalStart.x()) - log2(logicalEnd.x());
				xMin *= pow(2, deltaX);
				xMax *= pow(2, deltaX);
				break;
			}
			case CartesianPlot::Scale::Ln:
			case CartesianPlot::Scale::LnAbs: {
				const float deltaX = log(logicalStart.x()) - log(logicalEnd.x());
				xMin *= exp(deltaX);
				xMax *= exp(deltaX);
				break;
			}
			case CartesianPlot::Scale::Sqrt:
			case CartesianPlot::Scale::X2:
				break;
			}

			//handle the change in y
			switch (yScale) {
			case CartesianPlot::Scale::Linear: {
				const float deltaY = (logicalStart.y() - logicalEnd.y());
				yMax += deltaY;
				yMin += deltaY;
				break;
			}
			case CartesianPlot::Scale::Log10:
			case CartesianPlot::Scale::Log10Abs: {
				const float deltaY = log10(logicalStart.y()) - log10(logicalEnd.y());
				yMin *= pow(10, deltaY);
				yMax *= pow(10, deltaY);
				break;
			}
			case CartesianPlot::Scale::Log2:
			case CartesianPlot::Scale::Log2Abs: {
				const float deltaY = log2(logicalStart.y()) - log2(logicalEnd.y());
				yMin *= pow(2, deltaY);
				yMax *= pow(2, deltaY);
				break;
			}
			case CartesianPlot::Scale::Ln:
			case CartesianPlot::Scale::LnAbs: {
				const float deltaY = log(logicalStart.y()) - log(logicalEnd.y());
				yMin *= exp(deltaY);
				yMax *= exp(deltaY);
				break;
			}
			case CartesianPlot::Scale::Sqrt:
			case CartesianPlot::Scale::X2:
				break;
			}

			q->setUndoAware(false);
			q->setAutoScaleX(false);
			q->setAutoScaleY(false);
			q->setUndoAware(true);

			retransformScales();
			m_panningStart = event->pos();
		} else
			QGraphicsItem::mouseMoveEvent(event);
	} else if (mouseMode == CartesianPlot::MouseMode::ZoomSelection || mouseMode == CartesianPlot::MouseMode::ZoomXSelection || mouseMode == CartesianPlot::MouseMode::ZoomYSelection) {
		QGraphicsItem::mouseMoveEvent(event);
		if ( !boundingRect().contains(event->pos()) ) {
			q->info(QString());
			return;
		}
		emit q->mouseMoveZoomSelectionModeSignal(cSystem->mapSceneToLogical(event->pos(), AbstractCoordinateSystem::MappingFlag::Limit));

	} else if (mouseMode == CartesianPlot::MouseMode::Cursor) {
		QGraphicsItem::mouseMoveEvent(event);
		if (!boundingRect().contains(event->pos())) {
			q->info(i18n("Not inside of the bounding rect"));
			return;
		}
		QPointF logicalPos = cSystem->mapSceneToLogical(event->pos(), AbstractCoordinateSystem::MappingFlag::Limit);

		// updating treeview data and cursor position
		// updatign cursor position is done in Worksheet, because
		// multiple plots must be updated
		emit q->mouseMoveCursorModeSignal(selectedCursor, logicalPos);
	}
}

void CartesianPlotPrivate::mouseMoveZoomSelectionMode(QPointF logicalPos) {
	QString info;
	QPointF logicalStart = cSystem->mapSceneToLogical(m_selectionStart, AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);
	if (mouseMode == CartesianPlot::MouseMode::ZoomSelection) {
		m_selectionEnd = cSystem->mapLogicalToScene(logicalPos, AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);
		QPointF logicalEnd = logicalPos;
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
	} else if (mouseMode == CartesianPlot::MouseMode::ZoomXSelection) {
		logicalPos.setY(yMin); // must be done, because the other plots can have other ranges, value must be in the scenes
		m_selectionEnd.setX(cSystem->mapLogicalToScene(logicalPos, CartesianCoordinateSystem::MappingFlag::SuppressPageClipping).x());//event->pos().x());
		m_selectionEnd.setY(dataRect.bottom());
		QPointF logicalEnd = logicalPos;
		if (xRangeFormat == CartesianPlot::Numeric)
			info = QString::fromUtf8("Δx=") + QString::number(logicalEnd.x()-logicalStart.x());
		else
			info = i18n("from x=%1 to x=%2", QDateTime::fromMSecsSinceEpoch(logicalStart.x()).toString(xRangeDateTimeFormat),
						QDateTime::fromMSecsSinceEpoch(logicalEnd.x()).toString(xRangeDateTimeFormat));
	} else if (mouseMode == CartesianPlot::MouseMode::ZoomYSelection) {
		m_selectionEnd.setX(dataRect.right());
		logicalPos.setX(xMin); // must be done, because the other plots can have other ranges, value must be in the scenes
		m_selectionEnd.setY(cSystem->mapLogicalToScene(logicalPos, CartesianCoordinateSystem::MappingFlag::SuppressPageClipping).y());//event->pos().y());
		QPointF logicalEnd = logicalPos;
		if (yRangeFormat == CartesianPlot::Numeric)
			info = QString::fromUtf8("Δy=") + QString::number(logicalEnd.y()-logicalStart.y());
		else
			info = i18n("from y=%1 to y=%2", QDateTime::fromMSecsSinceEpoch(logicalStart.y()).toString(xRangeDateTimeFormat),
						QDateTime::fromMSecsSinceEpoch(logicalEnd.y()).toString(xRangeDateTimeFormat));
	}
	q->info(info);
	update();
}

void CartesianPlotPrivate::mouseMoveCursorMode(int cursorNumber, QPointF logicalPos) {

	QPointF p1(logicalPos.x(), 0);
	cursorNumber == 0 ? cursor0Pos = p1 : cursor1Pos = p1;

	QString info;
	if (xRangeFormat == CartesianPlot::Numeric)
		info = QString::fromUtf8("x=") + QString::number(logicalPos.x());
	else
		info = i18n("x=%1", QDateTime::fromMSecsSinceEpoch(logicalPos.x()).toString(xRangeDateTimeFormat));
	q->info(info);
	update();
}

void CartesianPlotPrivate::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
	setCursor(Qt::ArrowCursor);
	if (mouseMode == CartesianPlot::MouseMode::Selection) {
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
	} else if (mouseMode == CartesianPlot::MouseMode::ZoomSelection || mouseMode == CartesianPlot::MouseMode::ZoomXSelection || mouseMode == CartesianPlot::MouseMode::ZoomYSelection) {
		emit q->mouseReleaseZoomSelectionModeSignal();
	}
}

void CartesianPlotPrivate::mouseReleaseZoomSelectionMode() {
	//don't zoom if very small region was selected, avoid occasional/unwanted zooming
	if ( qAbs(m_selectionEnd.x()-m_selectionStart.x()) < 20 || qAbs(m_selectionEnd.y()-m_selectionStart.y()) < 20 ) {
		m_selectionBandIsShown = false;
		return;
	}
	bool retransformPlot = true;

	//determine the new plot ranges
	QPointF logicalZoomStart = cSystem->mapSceneToLogical(m_selectionStart, AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);
	QPointF logicalZoomEnd = cSystem->mapSceneToLogical(m_selectionEnd, AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);
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

	if (mouseMode == CartesianPlot::MouseMode::ZoomSelection) {
		curvesXMinMaxIsDirty = true;
		curvesYMinMaxIsDirty = true;
		q->setAutoScaleX(false);
		q->setAutoScaleY(false);
	} else if (mouseMode == CartesianPlot::MouseMode::ZoomXSelection) {
		curvesYMinMaxIsDirty = true;
		q->setAutoScaleX(false);
		if (q->autoScaleY() && q->scaleAutoY())
			retransformPlot = false;
	} else if (mouseMode == CartesianPlot::MouseMode::ZoomYSelection) {
		curvesXMinMaxIsDirty = true;
		q->setAutoScaleY(false);
		if (q->autoScaleX() && q->scaleAutoX())
			retransformPlot = false;
	}

	if (retransformPlot)
		retransformScales();

	m_selectionBandIsShown = false;
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

void CartesianPlotPrivate::keyPressEvent(QKeyEvent* event) {
	if (event->key() == Qt::Key_Escape) {
		setCursor(Qt::ArrowCursor);
		q->setMouseMode(CartesianPlot::MouseMode::Selection);
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
	if (dataRect.contains(point)) {
		QPointF logicalPoint = cSystem->mapSceneToLogical(point);

		if ((mouseMode == CartesianPlot::MouseMode::ZoomSelection) ||
			mouseMode == CartesianPlot::MouseMode::Selection) {
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
		}

		if (mouseMode == CartesianPlot::MouseMode::ZoomSelection && !m_selectionBandIsShown) {
			emit q->mouseHoverZoomSelectionModeSignal(logicalPoint);
		} else if (mouseMode == CartesianPlot::MouseMode::ZoomXSelection && !m_selectionBandIsShown) {
			info = "x=";
			if (xRangeFormat == CartesianPlot::Numeric)
				 info += QString::number(logicalPoint.x());
			else
				info += QDateTime::fromMSecsSinceEpoch(logicalPoint.x()).toString(xRangeDateTimeFormat);
			emit q->mouseHoverZoomSelectionModeSignal(logicalPoint);
		} else if (mouseMode == CartesianPlot::MouseMode::ZoomYSelection && !m_selectionBandIsShown) {
			info = "y=";
			if (yRangeFormat == CartesianPlot::Numeric)
				info += QString::number(logicalPoint.y());
			else
				info += QDateTime::fromMSecsSinceEpoch(logicalPoint.y()).toString(yRangeDateTimeFormat);
			emit q->mouseHoverZoomSelectionModeSignal(logicalPoint);
		} else if (mouseMode == CartesianPlot::MouseMode::Selection) {
			// hover the nearest curve to the mousepointer
			// hovering curves is implemented in the parent, because no ignoreEvent() exists
			// for it. Checking all curves and hover the first
			bool curve_hovered = false;
			QVector<XYCurve*> curves = q->children<XYCurve>();
			for (int i=curves.count() - 1; i >= 0; i--){ // because the last curve is above the other curves
				if (curve_hovered){ // if a curve is already hovered, disable hover for the rest
					curves[i]->setHover(false);
					continue;
				}
				if (curves[i]->activateCurve(event->pos())) {
					curves[i]->setHover(true);
					curve_hovered = true;
					continue;
				}
				curves[i]->setHover(false);
			}
		} else if (mouseMode == CartesianPlot::MouseMode::Cursor) {
			info = "x=";
			if (yRangeFormat == CartesianPlot::Numeric)
				info += QString::number(logicalPoint.x());
			else
				info += QDateTime::fromMSecsSinceEpoch(logicalPoint.x()).toString(xRangeDateTimeFormat);

			double cursorPenWidth2 = cursorPen.width()/2.;
			if (cursorPenWidth2 < 10.)
				cursorPenWidth2 = 10.;
			if ((cursor0Enable && qAbs(point.x()-cSystem->mapLogicalToScene(QPointF(cursor0Pos.x(),yMin)).x()) < cursorPenWidth2) ||
					(cursor1Enable && qAbs(point.x()-cSystem->mapLogicalToScene(QPointF(cursor1Pos.x(),yMin)).x()) < cursorPenWidth2))
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
	QVector<XYCurve*> curves = q->children<XYCurve>();
	for (auto* curve : curves)
		curve->setHover(false);

	m_hovered = false;
	QGraphicsItem::hoverLeaveEvent(event);
}

void CartesianPlotPrivate::mouseHoverZoomSelectionMode(QPointF logicPos) {
	m_insideDataRect = true;

	if (mouseMode == CartesianPlot::MouseMode::ZoomSelection && !m_selectionBandIsShown) {

	} else if (mouseMode == CartesianPlot::MouseMode::ZoomXSelection && !m_selectionBandIsShown) {
		QPointF p1(logicPos.x(), yMin);
		QPointF p2(logicPos.x(), yMax);
		m_selectionStartLine.setP1(cSystem->mapLogicalToScene(p1, CartesianCoordinateSystem::MappingFlag::Limit));
		m_selectionStartLine.setP2(cSystem->mapLogicalToScene(p2, CartesianCoordinateSystem::MappingFlag::Limit));
	} else if (mouseMode == CartesianPlot::MouseMode::ZoomYSelection && !m_selectionBandIsShown) {
		QPointF p1(xMin, logicPos.y());
		QPointF p2(xMax, logicPos.y());
		m_selectionStartLine.setP1(cSystem->mapLogicalToScene(p1, CartesianCoordinateSystem::MappingFlag::Limit));
		m_selectionStartLine.setP2(cSystem->mapLogicalToScene(p2, CartesianCoordinateSystem::MappingFlag::Limit));
	}
	update(); // because if previous another selection mode was selected, the lines must be deleted
}

void CartesianPlotPrivate::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
	Q_UNUSED(option)
	Q_UNUSED(widget)

	if (!isVisible())
		return;

	if (!m_printing) {
		painter->save();

		painter->setPen(cursorPen);
		QFont font = painter->font();
		font.setPointSize(font.pointSize() * 4);
		painter->setFont(font);

		QPointF p1 = cSystem->mapLogicalToScene(QPointF(cursor0Pos.x(),yMin));
		if (cursor0Enable && p1 != QPointF(0,0)){
			QPointF p2 = cSystem->mapLogicalToScene(QPointF(cursor0Pos.x(),yMax));
			painter->drawLine(p1,p2);
			QPointF textPos = p2;
			textPos.setX(p2.x() - m_cursor0Text.size().width()/2);
			textPos.setY(p2.y() - m_cursor0Text.size().height());
			if (textPos.y() < boundingRect().y())
				textPos.setY(boundingRect().y());
			painter->drawStaticText(textPos, m_cursor0Text);
		}

		p1 = cSystem->mapLogicalToScene(QPointF(cursor1Pos.x(),yMin));
		if (cursor1Enable && p1 != QPointF(0,0)){
			QPointF p2 = cSystem->mapLogicalToScene(QPointF(cursor1Pos.x(),yMax));
			painter->drawLine(p1,p2);
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

	painter->setPen(QPen(Qt::black, 3));
	if ((mouseMode == CartesianPlot::MouseMode::ZoomXSelection || mouseMode == CartesianPlot::MouseMode::ZoomYSelection)
			&& (!m_selectionBandIsShown) && m_insideDataRect)
		painter->drawLine(m_selectionStartLine);

	if (m_selectionBandIsShown) {
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
		painter->setPen(QPen(Qt::black, 5));
		painter->drawRect(QRectF(selectionStart, selectionEnd));
		painter->setBrush(Qt::blue);
		painter->setOpacity(0.2);
		painter->drawRect(QRectF(selectionStart, selectionEnd));
		painter->restore();
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
	writer->writeStartElement( "coordinateSystem" );
	writer->writeAttribute( "autoScaleX", QString::number(d->autoScaleX) );
	writer->writeAttribute( "autoScaleY", QString::number(d->autoScaleY) );
	writer->writeAttribute( "xMin", QString::number(d->xMin, 'g', 16));
	writer->writeAttribute( "xMax", QString::number(d->xMax, 'g', 16) );
	writer->writeAttribute( "yMin", QString::number(d->yMin, 'g', 16) );
	writer->writeAttribute( "yMax", QString::number(d->yMax, 'g', 16) );
	writer->writeAttribute( "xScale", QString::number(static_cast<int>(d->xScale)) );
	writer->writeAttribute( "yScale", QString::number(static_cast<int>(d->yScale)) );
	writer->writeAttribute( "xRangeFormat", QString::number(d->xRangeFormat) );
	writer->writeAttribute( "yRangeFormat", QString::number(d->yRangeFormat) );
	writer->writeAttribute( "horizontalPadding", QString::number(d->horizontalPadding) );
	writer->writeAttribute( "verticalPadding", QString::number(d->verticalPadding) );
	writer->writeAttribute( "rightPadding", QString::number(d->rightPadding) );
	writer->writeAttribute( "bottomPadding", QString::number(d->bottomPadding) );
	writer->writeAttribute( "symmetricPadding", QString::number(d->symmetricPadding));
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
			writer->writeAttribute("start", QString::number(rb.start));
			writer->writeAttribute("end", QString::number(rb.end));
			writer->writeAttribute("position", QString::number(rb.position));
			writer->writeAttribute("style", QString::number(static_cast<int>(rb.style)));
			writer->writeEndElement();
		}
		writer->writeEndElement();
	}

	//serialize all children (plot area, title text label, axes and curves)
	for (auto* elem : children<WorksheetElement>(ChildIndexFlag::IncludeHidden))
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
			READ_DOUBLE_VALUE("rightPadding", rightPadding);
			READ_DOUBLE_VALUE("bottomPadding", bottomPadding);
			READ_INT_VALUE("symmetricPadding", symmetricPadding, bool);
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
		} else if (reader->name() == "image") {
			Image* image = new Image(QString());
			if (!image->load(reader, preview)) {
				delete image;
				return false;
			} else
				addChildFast(image);
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
		} else if (reader->name() == "referenceLine") {
			ReferenceLine* line = new ReferenceLine(this, QString());
			if (line->load(reader, preview))
				addChildFast(line);
			else {
				delete line;
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
		DEBUG("	set theme to " << STDSTRING(theme));
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
	for (auto* child : children<WorksheetElement>(ChildIndexFlag::IncludeHidden))
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

	for (auto *child : children<XYCurve>(ChildIndexFlag::IncludeHidden))
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
}

const QList<QColor>& CartesianPlot::themeColorPalette() const {
	return m_themeColorPalette;
}
