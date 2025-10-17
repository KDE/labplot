/*
 File                 : SeasonalDecomposition.cpp
 Project              : LabPlot
 Description          : Seasonal Decomposition of Time Series
 --------------------------------------------------------------------
 SPDX-FileCopyrightText: 2025 Alexander Semke <alexander.semke@web.de>

 SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "SeasonalDecomposition.h"
#include "backend/core/column/Column.h"
#include "backend/lib/macrosCurve.h"
#include "backend/lib/XmlStreamReader.h"
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

// line
BASIC_SHARED_D_READER_IMPL(SeasonalDecomposition, SeasonalDecomposition::Method, method, method)

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

STD_SETTER_CMD_IMPL_F_S(SeasonalDecomposition, SetMethod, SeasonalDecomposition::Method, method, recalc)
void SeasonalDecomposition::setMethod(Method method) {
	Q_D(SeasonalDecomposition);
	if (method != d->method)
		exec(new SeasonalDecompositionSetMethodCmd(d, method, ki18n("%1: method changed")));
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

void SeasonalDecompositionPrivate::recalc() {
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
	std::vector<double> yDataVector;
	const int rowCount = std::min(xColumn->rowCount(), yColumn->rowCount());
	for (int row = 0; row < rowCount; ++row) {
		if (!yColumn->isValid(row) || yColumn->isMasked(row))
			continue;

		yDataVector.push_back(yColumn->valueAt(row));
		++row;
	}

	const int period = 13; // TODO
	auto result = stl::params().fit(yDataVector, period);

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

	columnTrend->setValues(trendData);
	columnSeasonal->setValues(seasonalData);
	columnResidual->setValues(residualData);
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
		} else { // unknown element
			reader->raiseWarning(i18n("unknown element '%1'", reader->name().toString()));
			if (!reader->skipToEndElement())
				return false;
		}
	}

	return true;
}
