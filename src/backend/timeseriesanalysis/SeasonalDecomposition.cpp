/*
 File                 : SeasonalDecomposition.cpp
 Project              : LabPlot
 Description          : Seasonal Decomposition of Time Series
 --------------------------------------------------------------------
 SPDX-FileCopyrightText: 2025 Alexander Semke <alexander.semke@web.de>

 SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "SeasonalDecomposition.h"
#include "backend/core/Project.h"
#include "backend/core/Settings.h"
#include "backend/core/column/Column.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/macrosCurve.h"
#include "backend/lib/trace.h"
#include "backend/nsl/stl.hpp"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/timeseriesanalysis/SeasonalDecompositionPrivate.h"
#include "backend/worksheet/TextLabel.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"
#ifndef SDK
#include "src/frontend/core/ContentDockWidget.h"
#endif

CURVE_COLUMN_CONNECT(SeasonalDecomposition, X, x, recalc)
CURVE_COLUMN_CONNECT(SeasonalDecomposition, Y, y, recalc)

/**
 * \class SeasonalDecomposition
 * \brief Seasonal Decomposition of Time Series
 * \ingroup backend
 */
SeasonalDecomposition::SeasonalDecomposition(const QString& name, const bool loading)
	: AbstractPart(name, AspectType::SeasonalDecomposition)
	, d_ptr(new SeasonalDecompositionPrivate(this)) {
	if (!loading)
		init();
}

SeasonalDecomposition::~SeasonalDecomposition() {
	delete d_ptr;
}

void SeasonalDecomposition::init() {
	Q_D(SeasonalDecomposition);

	// init the properties
	KConfig config;
	KConfigGroup group = config.group(QStringLiteral("SeasonalDecomposition"));

	d->method = static_cast<Method>(group.readEntry(QStringLiteral("Method"), static_cast<int>(Method::STL)));

	// STL parameters
	d->stlPeriod = group.readEntry(QStringLiteral("STLPeriod"), 12);
	d->stlRobust = group.readEntry(QStringLiteral("STLRobust"), false);
	d->stlSeasonalLength = group.readEntry(QStringLiteral("STLSeasonalLength"), 7);
	d->stlTrendLength = group.readEntry(QStringLiteral("STLTrendLength"), 13);
	d->stlTrendLengthAuto = group.readEntry(QStringLiteral("STLTrendLengthAuto"), true);
	d->stlLowPassLength = group.readEntry(QStringLiteral("STLLowPassLength"), 15);
	d->stlLowPassLengthAuto = group.readEntry(QStringLiteral("STLLowPassLengthAuto"), true);
	d->stlSeasonalDegree = group.readEntry(QStringLiteral("STLSeasonalDegree"), 1);
	d->stlTrendDegree = group.readEntry(QStringLiteral("STLTrendDegree"), 1);
	d->stlLowPassDegree = group.readEntry(QStringLiteral("STLLowPassDegree"), 1);
	d->stlSeasonalJump = group.readEntry(QStringLiteral("STLSeasonalJump"), 0);
	d->stlTrendJump = group.readEntry(QStringLiteral("STLTrendJump"), 0);
	d->stlLowPassJump = group.readEntry(QStringLiteral("STLLowPassJump"), 0);

	// spreadsheet with columns for the result y-data
	d->resultSpreadsheet = new Spreadsheet(i18n("Result"));
	d->resultSpreadsheet->setFixed(true);
	d->resultSpreadsheet->setUndoAware(false);
	d->resultSpreadsheet->setReadOnly(true);
	d->resultSpreadsheet->setColumnCount(3); // 3 columns initially: Trend, Seasonal, Residual
	addChildFast(d->resultSpreadsheet);

	d->columnTrend = d->resultSpreadsheet->column(0);
	d->columnTrend->setUndoAware(false);
	d->columnTrend->setName(i18n("Trend"));
	d->columnTrend->setFixed(true);
	d->columnTrend->setPlotDesignation(AbstractColumn::PlotDesignation::Y);

	d->columnSeasonal = d->resultSpreadsheet->column(1);
	d->columnSeasonal->setUndoAware(false);
	d->columnSeasonal->setName(i18n("Seasonal"));
	d->columnSeasonal->setFixed(true);
	d->columnSeasonal->setPlotDesignation(AbstractColumn::PlotDesignation::Y);

	d->columnResidual = d->resultSpreadsheet->column(2);
	d->columnResidual->setUndoAware(false);
	d->columnResidual->setName(i18n("Residual"));
	d->columnResidual->setFixed(true);
	d->columnResidual->setPlotDesignation(AbstractColumn::PlotDesignation::Y);

	// worksheet
	d->worksheet = new Worksheet(i18n("Worksheet"));
	d->worksheet->setFixed(true);
	QRectF newRect = d->worksheet->pageRect();
	double newHeight = Worksheet::convertToSceneUnits(20, Worksheet::Unit::Centimeter);
	double newWidth = Worksheet::convertToSceneUnits(15, Worksheet::Unit::Centimeter);
	newRect.setHeight(round(newHeight));
	newRect.setWidth(round(newWidth));
	d->worksheet->setPageRect(newRect);
	addChild(d->worksheet);

	// plot areas
	d->plotAreaOriginal = new CartesianPlot(i18n("Original"));
	d->plotAreaOriginal->setFixed(true);
	d->plotAreaOriginal->setType(CartesianPlot::Type::FourAxes);
	d->plotAreaOriginal->horizontalAxis()->title()->setText(QString());
	d->worksheet->addChild(d->plotAreaOriginal);

	d->plotAreaTrend = new CartesianPlot(i18n("Trend"));
	d->plotAreaTrend->setFixed(true);
	d->plotAreaTrend->setType(CartesianPlot::Type::FourAxes);
	d->plotAreaTrend->title()->setText(i18n("Trend Component"));
	d->plotAreaTrend->horizontalAxis()->title()->setText(QString());
	d->worksheet->addChild(d->plotAreaTrend);

	d->plotAreaSeasonal = new CartesianPlot(i18n("Seasonal"));
	d->plotAreaSeasonal->setFixed(true);
	d->plotAreaSeasonal->setType(CartesianPlot::Type::FourAxes);
	d->plotAreaSeasonal->title()->setText(i18n("Seasonal Component"));
	d->plotAreaSeasonal->horizontalAxis()->title()->setText(QString());
	d->worksheet->addChild(d->plotAreaSeasonal);

	d->plotAreaResidual = new CartesianPlot(i18n("Residual"));
	d->plotAreaResidual->setFixed(true);
	d->plotAreaResidual->setType(CartesianPlot::Type::FourAxes);
	d->plotAreaResidual->title()->setText(i18n("Residual Component"));
	d->worksheet->addChild(d->plotAreaResidual);

	// curves
	d->curveOriginal = new XYCurve(i18n("Original"));
	d->curveOriginal->setFixed(true);
	d->plotAreaOriginal->addChild(d->curveOriginal);

	d->curveTrend = new XYCurve(i18n("Trend"));
	d->curveTrend->setFixed(true);
	d->curveTrend->setYColumn(d->columnTrend);
	d->plotAreaTrend->addChild(d->curveTrend);

	d->curveSeasonal = new XYCurve(i18n("Seasonal"));
	d->curveSeasonal->setFixed(true);
	d->curveSeasonal->setYColumn(d->columnSeasonal);
	d->plotAreaSeasonal->addChild(d->curveSeasonal);

	d->curveResidual = new XYCurve(i18n("Residual"));
	d->curveResidual->setFixed(true);
	d->curveResidual->setYColumn(d->columnResidual);
	d->plotAreaResidual->addChild(d->curveResidual);

	// for the seasonal component we create only one plot area and curve by default,
	// for other algorithms that support multiple seasonal components like MSTL,
	// additional plot areas, etc. will be created when needed.
	d->plotAreasSeasonal << d->plotAreaSeasonal;
	d->curvesSeasonal << d->curveSeasonal;
	d->columnsSeasonal << d->columnSeasonal;
}

/*!
	Returns an icon to be used in the project explorer.
*/
QIcon SeasonalDecomposition::icon() const {
	return QIcon::fromTheme(QLatin1String("preferences-system-time"));
}

QWidget* SeasonalDecomposition::view() const {
	Q_D(const SeasonalDecomposition);
	return d->worksheet->view();
}

#ifndef SDK
ContentDockWidget* SeasonalDecomposition::dockWidget() const {
	Q_D(const SeasonalDecomposition);
	return d->worksheet->dockWidget();
}
#endif

bool SeasonalDecomposition::exportView() const {
	Q_D(const SeasonalDecomposition);
	return d->worksheet->exportView();
}

bool SeasonalDecomposition::printView() {
	Q_D(const SeasonalDecomposition);
	return d->worksheet->printView();
}

bool SeasonalDecomposition::printPreview() const {
	Q_D(const SeasonalDecomposition);
	return d->worksheet->printPreview();
}

// ##############################################################################
// ##########################  getter methods  ##################################
// ##############################################################################
BASIC_SHARED_D_READER_IMPL(SeasonalDecomposition, const AbstractColumn*, xColumn, xColumn)
BASIC_SHARED_D_READER_IMPL(SeasonalDecomposition, const AbstractColumn*, yColumn, yColumn)
BASIC_SHARED_D_READER_IMPL(SeasonalDecomposition, QString, xColumnPath, xColumnPath)
BASIC_SHARED_D_READER_IMPL(SeasonalDecomposition, QString, yColumnPath, yColumnPath)
BASIC_SHARED_D_READER_IMPL(SeasonalDecomposition, SeasonalDecomposition::Method, method, method)

// STL parameters
BASIC_SHARED_D_READER_IMPL(SeasonalDecomposition, int, stlPeriod, stlPeriod)
BASIC_SHARED_D_READER_IMPL(SeasonalDecomposition, bool, stlRobust, stlRobust)
BASIC_SHARED_D_READER_IMPL(SeasonalDecomposition, int, stlSeasonalLength, stlSeasonalLength)
BASIC_SHARED_D_READER_IMPL(SeasonalDecomposition, int, stlTrendLength, stlTrendLength)
BASIC_SHARED_D_READER_IMPL(SeasonalDecomposition, bool, stlTrendLengthAuto, stlTrendLengthAuto)
BASIC_SHARED_D_READER_IMPL(SeasonalDecomposition, int, stlLowPassLength, stlLowPassLength)
BASIC_SHARED_D_READER_IMPL(SeasonalDecomposition, bool, stlLowPassLengthAuto, stlLowPassLengthAuto)
BASIC_SHARED_D_READER_IMPL(SeasonalDecomposition, int, stlSeasonalDegree, stlSeasonalDegree)
BASIC_SHARED_D_READER_IMPL(SeasonalDecomposition, int, stlTrendDegree, stlTrendDegree)
BASIC_SHARED_D_READER_IMPL(SeasonalDecomposition, int, stlLowPassDegree, stlLowPassDegree)
BASIC_SHARED_D_READER_IMPL(SeasonalDecomposition, int, stlSeasonalJump, stlSeasonalJump)
BASIC_SHARED_D_READER_IMPL(SeasonalDecomposition, bool, stlSeasonalJumpAuto, stlSeasonalJumpAuto)
BASIC_SHARED_D_READER_IMPL(SeasonalDecomposition, int, stlTrendJump, stlTrendJump)
BASIC_SHARED_D_READER_IMPL(SeasonalDecomposition, bool, stlTrendJumpAuto, stlTrendJumpAuto)
BASIC_SHARED_D_READER_IMPL(SeasonalDecomposition, int, stlLowPassJump, stlLowPassJump)
BASIC_SHARED_D_READER_IMPL(SeasonalDecomposition, bool, stlLowPassJumpAuto, stlLowPassJumpAuto)

// MSTL parameters
BASIC_SHARED_D_READER_IMPL(SeasonalDecomposition, std::vector<size_t>, mstlPeriods, mstlPeriods)
BASIC_SHARED_D_READER_IMPL(SeasonalDecomposition, double, mstlLambda, mstlLambda)
BASIC_SHARED_D_READER_IMPL(SeasonalDecomposition, int, mstlIterations, mstlIterations)

// ##############################################################################
// #################  setter methods and undo commands ##########################
// ##############################################################################
CURVE_COLUMN_SETTER_CMD_IMPL_F_S(SeasonalDecomposition, X, x, recalc)
void SeasonalDecomposition::setXColumn(const AbstractColumn* column) {
	Q_D(SeasonalDecomposition);
	if (column != d->xColumn)
		exec(new SeasonalDecompositionSetXColumnCmd(d, column, ki18n("%1: x-data source changed")));
}

CURVE_COLUMN_SETTER_CMD_IMPL_F_S(SeasonalDecomposition, Y, y, recalc)
void SeasonalDecomposition::setYColumn(const AbstractColumn* column) {
	Q_D(SeasonalDecomposition);
	if (column != d->yColumn)
		exec(new SeasonalDecompositionSetYColumnCmd(d, column, ki18n("%1: y-data source changed")));
}

void SeasonalDecomposition::setXColumnPath(const QString& path) {
	Q_D(SeasonalDecomposition);
	d->xColumnPath = path;
}

void SeasonalDecomposition::setYColumnPath(const QString& path) {
	Q_D(SeasonalDecomposition);
	d->yColumnPath = path;
}

STD_SETTER_CMD_IMPL_F_S(SeasonalDecomposition, SetMethod, SeasonalDecomposition::Method, method, recalcDecomposition)
void SeasonalDecomposition::setMethod(Method method) {
	Q_D(SeasonalDecomposition);
	if (method != d->method)
		exec(new SeasonalDecompositionSetMethodCmd(d, method, ki18n("%1: set method")));
}

// STL parameters
STD_SETTER_CMD_IMPL_F_S(SeasonalDecomposition, SetSTLPeriod, int, stlPeriod, recalcDecomposition)
void SeasonalDecomposition::setSTLPeriod(int value) {
	Q_D(SeasonalDecomposition);
	if (value != d->stlPeriod)
		exec(new SeasonalDecompositionSetSTLPeriodCmd(d, value, ki18n("%1: set period for STL")));
}
STD_SETTER_CMD_IMPL_F_S(SeasonalDecomposition, SetSTLRobust, bool, stlRobust, recalcDecomposition)
void SeasonalDecomposition::setSTLRobust(bool value) {
	Q_D(SeasonalDecomposition);
	if (value != d->stlRobust)
		exec(new SeasonalDecompositionSetSTLRobustCmd(d, value, ki18n("%1: set robust parameter for STL")));
}
STD_SETTER_CMD_IMPL_F_S(SeasonalDecomposition, SetSTLSeasonalLength, int, stlSeasonalLength, recalcDecomposition)
void SeasonalDecomposition::setSTLSeasonalLength(int value) {
	Q_D(SeasonalDecomposition);
	if (value != d->stlSeasonalLength)
		exec(new SeasonalDecompositionSetSTLSeasonalLengthCmd(d, value, ki18n("%1: set seasonal length for STL")));
}
STD_SETTER_CMD_IMPL_F_S(SeasonalDecomposition, SetSTLTrendLength, int, stlTrendLength, recalcDecomposition)
void SeasonalDecomposition::setSTLTrendLength(int value) {
	Q_D(SeasonalDecomposition);
	if (value != d->stlTrendLength)
		exec(new SeasonalDecompositionSetSTLTrendLengthCmd(d, value, ki18n("%1: set trend length for STL")));
}
STD_SETTER_CMD_IMPL_F_S(SeasonalDecomposition, SetSTLTrendLengthAuto, bool, stlTrendLengthAuto, recalcDecomposition)
void SeasonalDecomposition::setSTLTrendLengthAuto(bool value) {
	Q_D(SeasonalDecomposition);
	if (value != d->stlTrendLengthAuto)
		exec(new SeasonalDecompositionSetSTLTrendLengthAutoCmd(d, value, ki18n("%1: set trend auto-length for STL")));
}
STD_SETTER_CMD_IMPL_F_S(SeasonalDecomposition, SetSTLLowPassLength, int, stlLowPassLength, recalcDecomposition)
void SeasonalDecomposition::setSTLLowPassLength(int value) {
	Q_D(SeasonalDecomposition);
	if (value != d->stlLowPassLength)
		exec(new SeasonalDecompositionSetSTLLowPassLengthCmd(d, value, ki18n("%1: set low pass length for STL")));
}
STD_SETTER_CMD_IMPL_F_S(SeasonalDecomposition, SetSTLLowPassLengthAuto, bool, stlLowPassLengthAuto, recalcDecomposition)
void SeasonalDecomposition::setSTLLowPassLengthAuto(bool value) {
	Q_D(SeasonalDecomposition);
	if (value != d->stlLowPassLengthAuto)
		exec(new SeasonalDecompositionSetSTLLowPassLengthAutoCmd(d, value, ki18n("%1: set low pass auto-length for STL")));
}
STD_SETTER_CMD_IMPL_F_S(SeasonalDecomposition, SetSTLSeasonalDegree, int, stlSeasonalDegree, recalcDecomposition)
void SeasonalDecomposition::setSTLSeasonalDegree(int value) {
	Q_D(SeasonalDecomposition);
	if (value != d->stlSeasonalDegree)
		exec(new SeasonalDecompositionSetSTLSeasonalDegreeCmd(d, value, ki18n("%1: set seasonal degree for STL")));
}
STD_SETTER_CMD_IMPL_F_S(SeasonalDecomposition, SetSTLTrendDegree, int, stlTrendDegree, recalcDecomposition)
void SeasonalDecomposition::setSTLTrendDegree(int value) {
	Q_D(SeasonalDecomposition);
	if (value != d->stlTrendDegree)
		exec(new SeasonalDecompositionSetSTLTrendDegreeCmd(d, value, ki18n("%1: set trend degree for STL")));
}
STD_SETTER_CMD_IMPL_F_S(SeasonalDecomposition, SetSTLLowPassDegree, int, stlLowPassDegree, recalcDecomposition)
void SeasonalDecomposition::setSTLLowPassDegree(int value) {
	Q_D(SeasonalDecomposition);
	if (value != d->stlLowPassDegree)
		exec(new SeasonalDecompositionSetSTLLowPassDegreeCmd(d, value, ki18n("%1: set low pass degree for STL")));
}
STD_SETTER_CMD_IMPL_F_S(SeasonalDecomposition, SetSTLSeasonalJump, int, stlSeasonalJump, recalcDecomposition)
void SeasonalDecomposition::setSTLSeasonalJump(int value) {
	Q_D(SeasonalDecomposition);
	if (value != d->stlSeasonalJump)
		exec(new SeasonalDecompositionSetSTLSeasonalJumpCmd(d, value, ki18n("%1: set seasonal jump for STL")));
}
STD_SETTER_CMD_IMPL_F_S(SeasonalDecomposition, SetSTLSeasonalJumpAuto, bool, stlSeasonalJumpAuto, recalcDecomposition)
void SeasonalDecomposition::setSTLSeasonalJumpAuto(bool value) {
	Q_D(SeasonalDecomposition);
	if (value != d->stlSeasonalJumpAuto)
		exec(new SeasonalDecompositionSetSTLSeasonalJumpAutoCmd(d, value, ki18n("%1: set seasonal auto-jump for STL")));
}
STD_SETTER_CMD_IMPL_F_S(SeasonalDecomposition, SetSTLTrendJump, int, stlTrendJump, recalcDecomposition)
void SeasonalDecomposition::setSTLTrendJump(int value) {
	Q_D(SeasonalDecomposition);
	if (value != d->stlTrendJump)
		exec(new SeasonalDecompositionSetSTLTrendJumpCmd(d, value, ki18n("%1: set trend jump for STL")));
}
STD_SETTER_CMD_IMPL_F_S(SeasonalDecomposition, SetSTLTrendJumpAuto, bool, stlTrendJumpAuto, recalcDecomposition)
void SeasonalDecomposition::setSTLTrendJumpAuto(bool value) {
	Q_D(SeasonalDecomposition);
	if (value != d->stlTrendJumpAuto)
		exec(new SeasonalDecompositionSetSTLTrendJumpAutoCmd(d, value, ki18n("%1: set trend auto-jump for STL")));
}
STD_SETTER_CMD_IMPL_F_S(SeasonalDecomposition, SetSTLLowPassJump, int, stlLowPassJump, recalcDecomposition)
void SeasonalDecomposition::setSTLLowPassJump(int value) {
	Q_D(SeasonalDecomposition);
	if (value != d->stlLowPassJump)
		exec(new SeasonalDecompositionSetSTLLowPassJumpCmd(d, value, ki18n("%1: set low pass jump for STL")));
}
STD_SETTER_CMD_IMPL_F_S(SeasonalDecomposition, SetSTLLowPassJumpAuto, bool, stlLowPassJumpAuto, recalcDecomposition)
void SeasonalDecomposition::setSTLLowPassJumpAuto(bool value) {
	Q_D(SeasonalDecomposition);
	if (value != d->stlLowPassJumpAuto)
		exec(new SeasonalDecompositionSetSTLLowPassJumpAutoCmd(d, value, ki18n("%1: set low pass auto-jump for STL")));
}

// MSTL parameters
STD_SETTER_CMD_IMPL_F_S(SeasonalDecomposition, SetMSTLPeriods, std::vector<size_t>, mstlPeriods, recalcDecomposition)
void SeasonalDecomposition::setMSTLPeriods(const std::vector<size_t>& periods) {
	Q_D(SeasonalDecomposition);
	if (periods != d->mstlPeriods)
		exec(new SeasonalDecompositionSetMSTLPeriodsCmd(d, periods, ki18n("%1: set periods for MSTL")));
}
STD_SETTER_CMD_IMPL_F_S(SeasonalDecomposition, SetMSTLLambda, double, mstlLambda, recalcDecomposition)
void SeasonalDecomposition::setMSTLLambda(double lambda) {
	Q_D(SeasonalDecomposition);
	if (lambda != d->mstlLambda)
		exec(new SeasonalDecompositionSetMSTLLambdaCmd(d, lambda, ki18n("%1: set Lambda for MSTL")));
}
STD_SETTER_CMD_IMPL_F_S(SeasonalDecomposition, SetMSTLIterations, int, mstlIterations, recalcDecomposition)
void SeasonalDecomposition::setMSTLIterations(int iterations) {
	Q_D(SeasonalDecomposition);
	if (iterations != d->mstlIterations)
		exec(new SeasonalDecompositionSetMSTLIterationsCmd(d, iterations, ki18n("%1: set iterations for MSTL")));
}

// ##############################################################################
// #################################  SLOTS  ####################################
// ##############################################################################
void SeasonalDecomposition::recalc() {
	Q_D(SeasonalDecomposition);
	d->recalc();
}

void SeasonalDecomposition::xColumnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(SeasonalDecomposition);
	if (aspect == d->xColumn) {
		d->xColumn = nullptr;
		d->recalc();
	}
}

void SeasonalDecomposition::yColumnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(SeasonalDecomposition);
	if (aspect == d->yColumn) {
		d->yColumn = nullptr;
		d->recalc();
	}
}

// ##############################################################################
// ######################  Private implementation ###############################
// ##############################################################################
SeasonalDecompositionPrivate::SeasonalDecompositionPrivate(SeasonalDecomposition* owner)
	: q(owner) {
}

QString SeasonalDecompositionPrivate::name() const {
	return q->name();
}

/*!
 * called when the source columns were changed. adjusts the title of axes and plots accordingly
 * and copies the valid values from the source column into the internal vector used for the
 * calculation of the decomposition.
 */
void SeasonalDecompositionPrivate::recalc() {
	PERFTRACE(name() + QLatin1String(Q_FUNC_INFO));

	// set the columns in the curves, no need to put this onto the undo stack
	curveOriginal->setUndoAware(false);
	curveTrend->setUndoAware(false);
	for (auto* curve : curvesSeasonal)
		curve->setUndoAware(false);
	curveResidual->setUndoAware(false);

	curveOriginal->setXColumn(xColumn);
	curveOriginal->setYColumn(yColumn);
	curveTrend->setXColumn(xColumn);
	for (auto* curve : curvesSeasonal)
		curve->setXColumn(xColumn);
	curveResidual->setXColumn(xColumn);

	curveOriginal->setUndoAware(true);
	curveTrend->setUndoAware(true);
	for (auto* curve : curvesSeasonal)
		curve->setUndoAware(true);
	curveResidual->setUndoAware(true);

	if (!xColumn || !yColumn)
		return;

	// in case the y-column was changed, adjust the title of the plots and of the y-axes
	const auto& name = yColumn->name();
	plotAreaOriginal->title()->setText(name);
	plotAreaOriginal->verticalAxis()->title()->setText(name);
	plotAreaTrend->verticalAxis()->title()->setText(name);
	for (auto* plotArea : plotAreasSeasonal)
		plotArea->verticalAxis()->title()->setText(name);
	plotAreaResidual->verticalAxis()->title()->setText(name);

	// copy valid data into a temp vector
	yDataVector.clear();
	const int rowCount = std::min(xColumn->rowCount(), yColumn->rowCount());
	for (int row = 0; row < rowCount; ++row) {
		if (!yColumn->isValid(row) || yColumn->isMasked(row))
			continue;

		yDataVector.push_back(yColumn->valueAt(row));
	}

	if (yDataVector.size() == 0) {
		// no input data and no result available, clear the previous data shown in the result spreadsheet
		resultSpreadsheet->setRowCount(0);
		Q_EMIT q->statusError(i18n("No valid data provided."));
		return;
	}

	recalcDecomposition();
}

/*!
 * called when one of the parameters influencing the result for the current decomposition method
 * was changed, recalculates the decomposition with the new set of parameters.
 */
void SeasonalDecompositionPrivate::recalcDecomposition() {
	Q_EMIT q->statusError(QString());
	PERFTRACE(name() + QLatin1String(Q_FUNC_INFO));

	QVector<double> trendData;
	QVector<QVector<double>> seasonalData; // vector of vectors for multiple seasonal components, one element for methods supporting one single component only
	QVector<double> residualData;

	switch (method) {
	case (SeasonalDecomposition::Method::STL):
	case (SeasonalDecomposition::Method::MSTL): {
		// check if we have valid parameters first
		if (stlSeasonalLength < 3 || stlSeasonalLength % 2 != 1) {
			reset(i18n("Seasonal Length must be odd and at least 3."));
			return;
		}

		if (stlTrendLength < 3 || stlTrendLength % 2 != 1) {
			reset(i18n("Trend Length must be odd and at least 3."));
			return;
		}

		if (stlLowPassLength < 3 || stlLowPassLength % 2 != 1) {
			reset(i18n("Low-Pass Length must be odd and at least 3."));
			return;
		}

		// set the STL parameters
		auto stlParameters = stl::params().robust(stlRobust);

		// length
		stlParameters = stl::params().seasonal_length(stlSeasonalLength);
		if (!stlTrendLengthAuto)
			stlParameters = stlParameters.trend_length(stlTrendLength);
		if (!stlLowPassLengthAuto)
			stlParameters = stlParameters.low_pass_length(stlLowPassLength);

		// degrees
		stlParameters = stlParameters.seasonal_degree(stlSeasonalDegree).trend_degree(stlTrendDegree).low_pass_degree(stlLowPassDegree);

		// jumps
		if (!stlSeasonalJumpAuto)
			stlParameters = stlParameters.seasonal_jump(stlSeasonalJump);
		if (!stlTrendJumpAuto)
			stlParameters = stlParameters.trend_jump(stlTrendJump);
		if (!stlLowPassJumpAuto)
			stlParameters = stlParameters.low_pass_jump(stlLowPassJump);

		if (method == SeasonalDecomposition::Method::STL) {
			WAIT_CURSOR_AUTO_RESET
			adjustSeasonalComponents({static_cast<size_t>(stlPeriod)});

			// perform the decomposition
			auto result = stlParameters.fit(yDataVector, stlPeriod);

			// copy the result data into the internal column vectors
			const auto size = result.trend.size();
			QVector<double> seasonalDataSingle;
			trendData.resize(size);
			seasonalDataSingle.resize(size);
			residualData.resize(size);
			for (size_t i = 0; i < size; ++i) {
				trendData[i] = result.trend[i];
				seasonalDataSingle[i] = result.seasonal[i];
				residualData[i] = result.remainder[i];
			}

			seasonalData << seasonalDataSingle;
		} else {
			// check if we have valid MSTL parameters
			for (size_t i = 0; i < mstlPeriods.size(); i++) {
				if (yDataVector.size() < mstlPeriods.at(i) * 2) {
					reset(i18n("Time-series has less than two periods."));
					return;
				}
			}

			if (mstlLambda < 0 || mstlLambda > 1) {
				reset(i18n("Lambda must be between 0 and 1."));
				return;
			}

			WAIT_CURSOR_AUTO_RESET
			adjustSeasonalComponents(mstlPeriods);

			// perform the decomposition
			auto mstlParameters = stl::mstl_params().stl_params(stlParameters).iterations(mstlIterations).lambda(mstlLambda);
			auto result = stl::mstl_params().fit(yDataVector, mstlPeriods);

			// copy the result data into the internal column vectors
			const auto size = result.trend.size();
			trendData.resize(size);
			residualData.resize(size);
			seasonalData.resize((int)columnsSeasonal.size());
			for (int i = 0; i < (int)columnsSeasonal.size(); ++i)
				seasonalData[i].resize(size);

			for (size_t i = 0; i < size; ++i) {
				trendData[i] = result.trend[i];
				residualData[i] = result.remainder[i];
				for (int j = 0; j < (int)columnsSeasonal.size(); ++j)
					seasonalData[j][i] = result.seasonal.at(j)[i];
			}
		}

		break;
	}
	}

	columnTrend->setValues(trendData);
	columnResidual->setValues(residualData);
	for (int i = 0; i < (int)columnsSeasonal.size(); ++i)
		columnsSeasonal.at(i)->setValues(seasonalData.at(i));
}

/*!
 * adjusts the size of the result spreadsheet and adds/removes new plot areas and curves for seasonal components.
 * has to be called when the number of the seasonal components was changed.
 */
void SeasonalDecompositionPrivate::adjustSeasonalComponents(const std::vector<size_t>& periods) {
	if (periods.size() == 1) { // one single period, STL is being used
		if (plotAreasSeasonal.size() > 1) {
			// remove extra plot areas and curves
			while (plotAreasSeasonal.size() > 1) {
				auto plotArea = std::unique_ptr<CartesianPlot>(plotAreasSeasonal.takeLast());
				curvesSeasonal.takeLast(); // Curve is a child of the plot area, so we don't have to explicitly delete it
				columnsSeasonal.takeLast(); // Column is a child of the spreadsheet, so we don't have to explicitly delete it
				AutoRestore cleanup(false, [this](bool value) {
					worksheet->setUndoAware(value);
				});
				worksheet->removeChild(plotArea.get());
			}

			// adjust the result spreadsheet
			resultSpreadsheet->setColumnCount(3); // 3 columns: trend, seasonal, residual
			columnSeasonal->setName(i18n("Seasonal"));
		}
	} else { // multiple periods, MSTL is being used
		// adjust the result spreadsheet
		const int columnCount = 2 + mstlPeriods.size(); // trend, residual + seasonal components
		if (resultSpreadsheet->columnCount() != columnCount) {
			resultSpreadsheet->setColumnCount(2 + mstlPeriods.size());
			columnsSeasonal.clear();
			for (int i = 1; i < resultSpreadsheet->columnCount() - 1; i++) {
				auto* column = resultSpreadsheet->column(i);
				column->setUndoAware(false); // No undo required because everything is autogenerated and write locked
				column->setFixed(true);
				column->setPlotDesignation(AbstractColumn::PlotDesignation::Y);
				columnsSeasonal << column;
			}

			// add plot areas and curves if needed
			if (plotAreasSeasonal.size() < (int)mstlPeriods.size()) {
				AutoRestore cleanup(true, [this](bool value) {
					q->project()->setSuppressAspectAddedSignal(value);
				}); // don't change the selection in the project explorer when adding new children
				while (plotAreasSeasonal.size() < (int)mstlPeriods.size()) {
					auto* plotArea = new CartesianPlot(i18n("Seasonal"));
					plotArea->setFixed(true);
					plotArea->setType(CartesianPlot::Type::FourAxes);
					plotArea->horizontalAxis()->title()->setText(QString());
					plotArea->verticalAxis()->title()->setText(yColumn->name());
					worksheet->insertChildBeforeFast(plotArea, plotAreaResidual);
					plotAreasSeasonal << plotArea;

					auto* curveSeasonal = new XYCurve(i18n("Seasonal"));
					curveSeasonal->setFixed(true);
					plotArea->addChild(curveSeasonal);
					curvesSeasonal << curveSeasonal;
				}
			} else {
				// remove extra plot areas and curves
				while (plotAreasSeasonal.size() > (int)mstlPeriods.size()) {
					auto plotArea = std::unique_ptr<CartesianPlot>(plotAreasSeasonal.takeLast());
					curvesSeasonal.takeLast(); // Curve is a child of the plot area, so we don't have to explicitly delete it
					AutoRestore undo(false, [this](bool value) {
						worksheet->setUndoAware(value);
					});
					worksheet->removeChild(plotArea.get());
				}
			}
		}

		// adjust the plot titles for seasonal components to reflect new periods
		for (size_t i = 0; i < mstlPeriods.size(); i++) {
			AutoRestore undo(false, [this, i](bool value) {
				plotAreasSeasonal.at(i)->setUndoAware(value);
			});
			plotAreasSeasonal.at(i)->title()->setText(i18n("Seasonal Component (Period: %1)", mstlPeriods.at(i)));
			columnsSeasonal.at(i)->setName(i18n("Seasonal, Period: %1", mstlPeriods.at(i)));
			curvesSeasonal.at(i)->setYColumn(columnsSeasonal.at(i));
			curvesSeasonal.at(i)->setXColumn(xColumn);
		}
	}

	// the column with the residuals is always the last one, repoint it after spreadsheet size changes
	columnResidual = resultSpreadsheet->column(resultSpreadsheet->columnCount() - 1);
	columnResidual->setName(i18n("Residual"));
	curveResidual->setYColumn(columnResidual);
}

void SeasonalDecompositionPrivate::reset(const QString& info) const {
	Q_EMIT q->statusError(info);
	columnTrend->clear();
	columnSeasonal->clear();
	columnResidual->clear();
}

// ##############################################################################
// ##################  Serialization/Deserialization  ###########################
// ##############################################################################
namespace {
QString periodsToString(const std::vector<size_t>& periods) {
	QString str;
	for (const auto period : periods) {
		if (!str.isEmpty())
			str += QLatin1Char(',');
		str += QString::number(period);
	}
	return str;
}

std::vector<size_t> stringToPeriods(const QString& str) {
	std::vector<size_t> periods;
	const auto list = str.split(QLatin1Char(','));
	for (const auto& s : list) {
		bool ok;
		const auto period = s.trimmed().toULongLong(&ok);
		if (ok)
			periods.push_back(period);
	}
	return periods;
}
}

//! Save as XML
void SeasonalDecomposition::save(QXmlStreamWriter* writer) const {
	Q_D(const SeasonalDecomposition);
	writer->writeStartElement(QStringLiteral("seasonalDecomposition"));
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	// general
	writer->writeStartElement(QStringLiteral("general"));
	WRITE_COLUMN(d->xColumn, xColumn);
	WRITE_COLUMN(d->yColumn, yColumn);
	writer->writeAttribute(QStringLiteral("method"), QString::number(static_cast<int>(d->method)));

	// STL parameters
	writer->writeAttribute(QStringLiteral("stlPeriod"), QString::number(d->stlPeriod));
	writer->writeAttribute(QStringLiteral("stlRobust"), QString::number(d->stlRobust));

	writer->writeAttribute(QStringLiteral("stlSeasonalLength"), QString::number(d->stlSeasonalLength));
	writer->writeAttribute(QStringLiteral("stlTrendLength"), QString::number(d->stlTrendLength));
	writer->writeAttribute(QStringLiteral("stlTrendLengthAuto"), QString::number(d->stlTrendLengthAuto));
	writer->writeAttribute(QStringLiteral("stlLowPassLength"), QString::number(d->stlLowPassLength));
	writer->writeAttribute(QStringLiteral("stlLowPassLengthAuto"), QString::number(d->stlLowPassLengthAuto));

	writer->writeAttribute(QStringLiteral("stlSeasonalDegree"), QString::number(d->stlSeasonalDegree));
	writer->writeAttribute(QStringLiteral("stlTrendDegree"), QString::number(d->stlTrendDegree));
	writer->writeAttribute(QStringLiteral("stlLowPassDegree"), QString::number(d->stlLowPassDegree));

	writer->writeAttribute(QStringLiteral("stlSeasonalJump"), QString::number(d->stlSeasonalJump));
	writer->writeAttribute(QStringLiteral("stlSeasonalJumpAuto"), QString::number(d->stlSeasonalJumpAuto));
	writer->writeAttribute(QStringLiteral("stlTrendJump"), QString::number(d->stlTrendJump));
	writer->writeAttribute(QStringLiteral("stlTrendJumpAuto"), QString::number(d->stlTrendJumpAuto));
	writer->writeAttribute(QStringLiteral("stlLowPassJump"), QString::number(d->stlLowPassJump));
	writer->writeAttribute(QStringLiteral("stlLowPassJumpAuto"), QString::number(d->stlLowPassJumpAuto));

	// MSTL parameters
	writer->writeAttribute(QStringLiteral("mstlLambda"), QString::number(d->mstlLambda));
	writer->writeAttribute(QStringLiteral("mstlIterations"), QString::number(d->mstlIterations));
	writer->writeAttribute(QStringLiteral("mstlPeriods"), periodsToString(d->mstlPeriods));

	writer->writeEndElement();

	// serialize all children
	for (auto* child : children<AbstractAspect>())
		child->save(writer);

	writer->writeEndElement(); // close "seasonalDecomposition" section
}

//! Load from XML
bool SeasonalDecomposition::load(XmlStreamReader* reader, bool preview) {
	Q_D(SeasonalDecomposition);

	if (!readBasicAttributes(reader))
		return false;

	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == QLatin1String("seasonalDecomposition"))
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == QLatin1String("comment")) {
			if (!readCommentElement(reader))
				return false;
		} else if (reader->name() == QLatin1String("general")) {
			attribs = reader->attributes();
			READ_COLUMN(xColumn);
			READ_COLUMN(yColumn);
			READ_INT_VALUE("method", method, Method);

			// STL parameters
			READ_INT_VALUE("stlPeriod", stlPeriod, int);
			READ_INT_VALUE("stlRobust", stlRobust, bool);

			READ_INT_VALUE("stlSeasonalLength", stlSeasonalLength, int);
			READ_INT_VALUE("stlTrendLength", stlTrendLength, int);
			READ_INT_VALUE("stlTrendLengthAuto", stlTrendLengthAuto, bool);
			READ_INT_VALUE("stlLowPassLength", stlLowPassLength, int);
			READ_INT_VALUE("stlLowPassLengthAuto", stlLowPassLengthAuto, int);

			READ_INT_VALUE("stlSeasonalDegree", stlSeasonalDegree, int);
			READ_INT_VALUE("stlTrendDegree", stlTrendDegree, int);
			READ_INT_VALUE("stlLowPassDegree", stlLowPassDegree, int);

			READ_INT_VALUE("stlSeasonalJump", stlSeasonalJump, int);
			READ_INT_VALUE("stlSeasonalJumpAuto", stlSeasonalJumpAuto, bool);
			READ_INT_VALUE("stlTrendJump", stlTrendJump, int);
			READ_INT_VALUE("stlTrendJumpAuto", stlTrendJumpAuto, bool);
			READ_INT_VALUE("stlLowPassJump", stlLowPassJump, int);
			READ_INT_VALUE("stlLowPassJumpAuto", stlLowPassJumpAuto, bool);

			// MSTL parameters
			READ_DOUBLE_VALUE("mstlLambda", mstlLambda);
			READ_INT_VALUE("mstlIterations", mstlIterations, int);
			d->mstlPeriods = stringToPeriods(attribs.value("mstlPeriods").toString());
		} else if (reader->name() == QLatin1String("spreadsheet")) {
			d->resultSpreadsheet = new Spreadsheet(i18n("Result"), true);
			if (!d->resultSpreadsheet->load(reader, preview))
				return false;
			addChildFast(d->resultSpreadsheet);
		} else if (reader->name() == QLatin1String("worksheet")) {
			d->worksheet = new Worksheet(i18n("Worksheet"), true);
			if (!d->worksheet->load(reader, preview))
				return false;
			addChildFast(d->worksheet);
		} else { // unknown element
			reader->raiseWarning(i18n("unknown element '%1'", reader->name().toString()));
			if (!reader->skipToEndElement())
				return false;
		}
	}

	if (preview)
		return true;

	// after all children were read, set the internal pointers for columns, plot areas and curves

	// columns
	if (d->resultSpreadsheet->columnCount() < 3)
		return false;
	d->columnTrend = d->resultSpreadsheet->column(0); // first column for trend
	d->columnSeasonal = d->resultSpreadsheet->column(1); // second column for seasonal
	for (int i = 2; i < d->resultSpreadsheet->columnCount() - 1; ++i) // more column for seasonal, if present
		d->columnsSeasonal << d->resultSpreadsheet->column(i);
	d->columnResidual = d->resultSpreadsheet->column(d->resultSpreadsheet->columnCount() - 1); // last column for residuals

	// plot areas
	auto plotAreas = d->worksheet->children<CartesianPlot>();
	if (plotAreas.count() < 4)
		return false;
	d->plotAreaOriginal = plotAreas.at(0); // first plot area for original
	d->plotAreaTrend = plotAreas.at(1); // second plot area for trend
	d->plotAreaSeasonal = plotAreas.at(2); // third plot area for seasonal
	d->plotAreasSeasonal << d->plotAreaSeasonal;
	for (int i = 3; i < plotAreas.count() - 1; ++i) // more plot areas for seasonal, if present
		d->plotAreasSeasonal << plotAreas.at(i);
	d->plotAreaResidual = plotAreas.at(plotAreas.count() - 1); // last plot area for residual

	// curves

	// original
	auto curves = d->plotAreaOriginal->children<XYCurve>();
	if (curves.isEmpty())
		return false;
	d->curveOriginal = curves.constFirst();

	// trend
	curves = d->plotAreaTrend->children<XYCurve>();
	if (curves.isEmpty())
		return false;
	d->curveTrend = curves.constFirst();

	// seasonal
	for (auto* plotArea : d->plotAreasSeasonal) {
		curves = plotArea->children<XYCurve>();
		if (curves.isEmpty())
			return false;
		d->curvesSeasonal << curves.constFirst();
	}
	d->curveSeasonal = d->curvesSeasonal.constFirst();

	// residual
	curves = d->plotAreaResidual->children<XYCurve>();
	if (curves.isEmpty())
		return false;
	d->curveResidual = curves.constFirst();

	return true;
}
