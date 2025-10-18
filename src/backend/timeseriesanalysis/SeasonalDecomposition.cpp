/*
 File                 : SeasonalDecomposition.cpp
 Project              : LabPlot
 Description          : Seasonal Decomposition of Time Series
 --------------------------------------------------------------------
 SPDX-FileCopyrightText: 2025 Alexander Semke <alexander.semke@web.de>

 SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "SeasonalDecomposition.h"
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
	d->resultSpreadsheet->setColumnCount(3);
	addChildFast(d->resultSpreadsheet);

	d->columnTrend = d->resultSpreadsheet->column(0);
	d->columnTrend->setUndoAware(false);
	d->columnTrend->setName(i18n("Trend"));

	d->columnSeasonal = d->resultSpreadsheet->column(1);
	d->columnSeasonal->setUndoAware(false);
	d->columnSeasonal->setName(i18n("Seasonal"));

	d->columnResidual = d->resultSpreadsheet->column(2);
	d->columnResidual->setUndoAware(false);
	d->columnResidual->setName(i18n("Residual"));

	// worksheet
	d->worksheet = new Worksheet(i18n("Worksheet"));
	d->worksheet->setFixed(true);
	QRectF newRect = d->worksheet->pageRect();
	double newHeight = Worksheet::convertToSceneUnits(20, Worksheet::Unit::Centimeter);
	newRect.setHeight(round(newHeight));
	d->worksheet->setPageRect(newRect);
	addChild(d->worksheet);

	// plot areas
	d->plotAreaOriginal = new CartesianPlot(i18n("Original"));
	d->plotAreaOriginal->setFixed(true);
	d->plotAreaOriginal->setType(CartesianPlot::Type::FourAxes);
	d->worksheet->addChild(d->plotAreaOriginal);

	d->plotAreaTrend = new CartesianPlot(i18n("Trend"));
	d->plotAreaTrend->setFixed(true);
	d->plotAreaTrend->setType(CartesianPlot::Type::FourAxes);
	d->plotAreaTrend->title()->setText(i18n("Trend Component"));
	d->worksheet->addChild(d->plotAreaTrend);

	d->plotAreaSeasonal = new CartesianPlot(i18n("Seasonal"));
	d->plotAreaSeasonal->setFixed(true);
	d->plotAreaSeasonal->setType(CartesianPlot::Type::FourAxes);
	d->plotAreaSeasonal->title()->setText(i18n("Seasonal Component"));
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
}

/*!
	Returns an icon to be used in the project explorer.
*/
QIcon SeasonalDecomposition::icon() const {
	return QIcon::fromTheme(QLatin1String("preferences-system-time"));
}

/*!
 * Returns a new context menu. The caller takes ownership of the menu.
 */
QMenu* SeasonalDecomposition::createContextMenu() {
	QMenu* menu = AbstractPart::createContextMenu();
	// m_image->createContextMenu(menu);
	return menu;
}

QWidget* SeasonalDecomposition::view() const {
	Q_D(const SeasonalDecomposition);
	return d->worksheet->view();
}

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
BASIC_SHARED_D_READER_IMPL(SeasonalDecomposition, int, stlTrendJump, stlTrendJump)
BASIC_SHARED_D_READER_IMPL(SeasonalDecomposition, int, stlLowPassJump, stlLowPassJump)

// MSTL parameters

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
        exec(new SeasonalDecompositionSetSTLPeriodCmd(d, value, ki18n("%1: set STL period")));
}
STD_SETTER_CMD_IMPL_F_S(SeasonalDecomposition, SetSTLRobust, bool, stlRobust, recalcDecomposition)
void SeasonalDecomposition::setSTLRobust(bool value) {
	Q_D(SeasonalDecomposition);
	if (value != d->stlRobust)
		exec(new SeasonalDecompositionSetSTLRobustCmd(d, value, ki18n("%1: set STL robust")));
}
STD_SETTER_CMD_IMPL_F_S(SeasonalDecomposition, SetSTLSeasonalLength, int, stlSeasonalLength, recalcDecomposition)
void SeasonalDecomposition::setSTLSeasonalLength(int value) {
	Q_D(SeasonalDecomposition);
	if (value != d->stlSeasonalLength)
		exec(new SeasonalDecompositionSetSTLSeasonalLengthCmd(d, value, ki18n("%1: set STL seasonal length")));
}
STD_SETTER_CMD_IMPL_F_S(SeasonalDecomposition, SetSTLTrendLength, int, stlTrendLength, recalcDecomposition)
void SeasonalDecomposition::setSTLTrendLength(int value) {
	Q_D(SeasonalDecomposition);
	if (value != d->stlTrendLength)
		exec(new SeasonalDecompositionSetSTLTrendLengthCmd(d, value, ki18n("%1: set STL trend length")));
}
STD_SETTER_CMD_IMPL_F_S(SeasonalDecomposition, SetSTLTrendLengthAuto, bool, stlTrendLengthAuto, recalcDecomposition)
void SeasonalDecomposition::setSTLTrendLengthAuto(bool value) {
	Q_D(SeasonalDecomposition);
	if (value != d->stlTrendLengthAuto)
		exec(new SeasonalDecompositionSetSTLTrendLengthAutoCmd(d, value, ki18n("%1: set STL trend auto-length")));
}
STD_SETTER_CMD_IMPL_F_S(SeasonalDecomposition, SetSTLLowPassLength, int, stlLowPassLength, recalcDecomposition)
void SeasonalDecomposition::setSTLLowPassLength(int value) {
	Q_D(SeasonalDecomposition);
	if (value != d->stlLowPassLength)
		exec(new SeasonalDecompositionSetSTLLowPassLengthCmd(d, value, ki18n("%1: set STL low pass length")));
}
STD_SETTER_CMD_IMPL_F_S(SeasonalDecomposition, SetSTLLowPassLengthAuto, bool, stlLowPassLengthAuto, recalcDecomposition)
void SeasonalDecomposition::setSTLLowPassLengthAuto(bool value) {
	Q_D(SeasonalDecomposition);
	if (value != d->stlLowPassLengthAuto)
		exec(new SeasonalDecompositionSetSTLLowPassLengthAutoCmd(d, value, ki18n("%1: set STL low pass auto-length")));
}
STD_SETTER_CMD_IMPL_F_S(SeasonalDecomposition, SetSTLSeasonalDegree, int, stlSeasonalDegree, recalcDecomposition)
void SeasonalDecomposition::setSTLSeasonalDegree(int value) {
	Q_D(SeasonalDecomposition);
	if (value != d->stlSeasonalDegree)
		exec(new SeasonalDecompositionSetSTLSeasonalDegreeCmd(d, value, ki18n("%1: set STL seasonal degree")));
}
STD_SETTER_CMD_IMPL_F_S(SeasonalDecomposition, SetSTLTrendDegree, int, stlTrendDegree, recalcDecomposition)
void SeasonalDecomposition::setSTLTrendDegree(int value) {
	Q_D(SeasonalDecomposition);
	if (value != d->stlTrendDegree)
		exec(new SeasonalDecompositionSetSTLTrendDegreeCmd(d, value, ki18n("%1: set STL trend degree")));
}
STD_SETTER_CMD_IMPL_F_S(SeasonalDecomposition, SetSTLLowPassDegree, int, stlLowPassDegree, recalcDecomposition)
void SeasonalDecomposition::setSTLLowPassDegree(int value) {
	Q_D(SeasonalDecomposition);
	if (value != d->stlLowPassDegree)
		exec(new SeasonalDecompositionSetSTLLowPassDegreeCmd(d, value, ki18n("%1: set STL low pass degree")));
}
STD_SETTER_CMD_IMPL_F_S(SeasonalDecomposition, SetSTLSeasonalJump, int, stlSeasonalJump, recalcDecomposition)
void SeasonalDecomposition::setSTLSeasonalJump(int value) {
	Q_D(SeasonalDecomposition);
	if (value != d->stlSeasonalJump)
		exec(new SeasonalDecompositionSetSTLSeasonalJumpCmd(d, value, ki18n("%1: set STL seasonal jump")));
}
STD_SETTER_CMD_IMPL_F_S(SeasonalDecomposition, SetSTLTrendJump, int, stlTrendJump, recalcDecomposition)
void SeasonalDecomposition::setSTLTrendJump(int value) {
	Q_D(SeasonalDecomposition);
	if (value != d->stlTrendJump)
		exec(new SeasonalDecompositionSetSTLTrendJumpCmd(d, value, ki18n("%1: set STL trend jump")));
}
STD_SETTER_CMD_IMPL_F_S(SeasonalDecomposition, SetSTLLowPassJump, int, stlLowPassJump, recalcDecomposition)
void SeasonalDecomposition::setSTLLowPassJump(int value) {
	Q_D(SeasonalDecomposition);
	if (value != d->stlLowPassJump)
		exec(new SeasonalDecompositionSetSTLLowPassJumpCmd(d, value, ki18n("%1: set STL low pass jump")));
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
	curveSeasonal->setUndoAware(false);
	curveResidual->setUndoAware(false);

	curveOriginal->setXColumn(xColumn);
	curveOriginal->setYColumn(yColumn);
	curveTrend->setXColumn(xColumn);
	curveSeasonal->setXColumn(xColumn);
	curveResidual->setXColumn(xColumn);

	curveOriginal->setUndoAware(true);
	curveTrend->setUndoAware(true);
	curveSeasonal->setUndoAware(true);
	curveResidual->setUndoAware(true);

	if (!xColumn || !yColumn)
		return;

	// in case the y-column was changed, adjust the title of the plot the y-axes
	const auto& name = yColumn->name();
	plotAreaOriginal->title()->setText(name);
	plotAreaOriginal->verticalAxis()->title()->setText(name);
	plotAreaTrend->verticalAxis()->title()->setText(name);
	plotAreaSeasonal->verticalAxis()->title()->setText(name);
	plotAreaResidual->verticalAxis()->title()->setText(name);

	// copy valid data into a temp vector
	yDataVector.clear();
	const int rowCount = std::min(xColumn->rowCount(), yColumn->rowCount());
	for (int row = 0; row < rowCount; ++row) {
		if (!yColumn->isValid(row) || yColumn->isMasked(row))
			continue;

		yDataVector.push_back(yColumn->valueAt(row));
	}

	recalcDecomposition();
}

/*!
 * called when one of the parameters influencing the result for the current decomposition method
 * was changed, recalculates the decomposition with the new parameters.
 */
void SeasonalDecompositionPrivate::recalcDecomposition() {
	PERFTRACE(name() + QLatin1String(Q_FUNC_INFO));

	if (yDataVector.size() == 0) {
		// no input data and no result available, clear the previous data shown in the result spreadsheet
		resultSpreadsheet->setRowCount(0);
		return;
	}

	switch (method) {
	case (SeasonalDecomposition::Method::STL):
	case (SeasonalDecomposition::Method::MSTL): {
		auto result = stl::params().seasonal_length(stlSeasonalLength).robust(true).fit(yDataVector, stlPeriod);

		const auto size = result.seasonal.size();
		QVector<double> trendData;
		trendData.resize(size);
		QVector<double> seasonalData;
		seasonalData.resize(size);
		QVector<double> residualData;
		residualData.resize(size);

		for (size_t i = 0; i < size; ++i) {
			trendData[i] = result.trend[i];
			seasonalData[i] = result.seasonal[i];
			residualData[i] = result.remainder[i];
		}

		qDebug()<<seasonalData;
		columnTrend->setValues(trendData);
		columnSeasonal->setValues(seasonalData);
		columnResidual->setValues(residualData);

		break;
	}
	}
}

// ##############################################################################
// ##################  Serialization/Deserialization  ###########################
// ##############################################################################

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
	writer->writeAttribute(QStringLiteral("stlTrendJump"), QString::number(d->stlTrendJump));
	writer->writeAttribute(QStringLiteral("stlLowPassJump"), QString::number(d->stlLowPassJump));

	// MSTL parameters
	writer->writeEndElement();

	// serialize all children
	for (auto* child : children<AbstractAspect>())
		child->save(writer);

	writer->writeEndElement(); // close "seasonalDecomposition" section
}

//! Load from XML
bool SeasonalDecomposition::load(XmlStreamReader* reader, bool /* preview */) {
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
			READ_INT_VALUE("stlTrendJump", stlTrendJump, int);
			READ_INT_VALUE("stlLowPassJump", stlLowPassJump, int);

			// MSTL parameters
		} else { // unknown element
			reader->raiseWarning(i18n("unknown element '%1'", reader->name().toString()));
			if (!reader->skipToEndElement())
				return false;
		}
	}

	return true;
}
