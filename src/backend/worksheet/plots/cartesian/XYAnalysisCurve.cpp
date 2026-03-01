/*
	File                 : XYAnalysisCurve.h
	Project              : LabPlot
	Description          : Base class for all analysis curves
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2017-2026 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2018-2022 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "XYAnalysisCurve.h"
#include "XYAnalysisCurvePrivate.h"
#include "backend/core/Project.h"
#include "backend/core/column/Column.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/worksheet/plots/cartesian/Symbol.h"
#include "backend/worksheet/plots/cartesian/Value.h"
#include "backend/worksheet/plots/cartesian/XYFitCurve.h"
#include "backend/worksheet/plots/cartesian/XYSmoothCurve.h"

/*!
 * \class XYAnalysisCurve
 * \brief Base class for all analysis curves.
 * \ingroup CartesianAnalysisPlots
 */
XYAnalysisCurve::XYAnalysisCurve(const QString& name, XYAnalysisCurvePrivate* dd, AspectType type)
	: XYCurve(name, dd, type) {
	init();
}

// no need to delete the d-pointer here - it inherits from QGraphicsItem
// and is deleted during the cleanup in QGraphicsScene
XYAnalysisCurve::~XYAnalysisCurve() = default;

void XYAnalysisCurve::init() {
	Q_D(XYAnalysisCurve);
	d->lineType = XYCurve::LineType::Line;
	d->symbol->setStyle(Symbol::Style::NoSymbols);
}

void XYAnalysisCurve::recalculate() {
	Q_D(XYAnalysisCurve);
	d->recalculate();
}

bool XYAnalysisCurve::resultAvailable() const {
	return result().available;
}

bool XYAnalysisCurve::usingColumn(const AbstractColumn* column, bool indirect) const {
	Q_D(const XYAnalysisCurve);

	if (d->dataSourceType == DataSourceType::Spreadsheet)
		return (d->xDataColumn == column || d->yDataColumn == column || d->y2DataColumn == column);
	else if (indirect && d->dataSourceCurve)
		return (d->dataSourceCurve->usingColumn(column, indirect) || d->dataSourceCurve->usingColumn(column, indirect));
	return false;
}

QVector<const Plot*> XYAnalysisCurve::dependingPlots() const {
	Q_D(const XYAnalysisCurve);
	switch (d->dataSourceType) {
	case DataSourceType::Curve: {
		if (d->dataSourceCurve)
			return {d->dataSourceCurve};
		return {};
	}
	case DataSourceType::Spreadsheet:
		return {};
	case DataSourceType::Histogram:
		return {}; // TODO: Histograms not yet implemented
	}
	Q_ASSERT(false);
	return {};
}

// copy valid data from x/y data columns to x/y data vectors
// for analysis functions
// avgUniqueX: average y values for duplicate x values
void XYAnalysisCurve::copyData(QVector<double>& xData,
							   QVector<double>& yData,
							   const AbstractColumn* xDataColumn,
							   const AbstractColumn* yDataColumn,
							   double xMin,
							   double xMax,
							   bool avgUniqueX) {
	if (!xDataColumn || !yDataColumn)
		return;

	const int rowCount = std::min(xDataColumn->rowCount(), yDataColumn->rowCount());
	bool uniqueX = true;
	for (int row = 0; row < rowCount; ++row) {
		if (!xDataColumn->isValid(row) || xDataColumn->isMasked(row) || !yDataColumn->isValid(row) || yDataColumn->isMasked(row))
			continue;

		double x = NAN;
		switch (xDataColumn->columnMode()) {
		case AbstractColumn::ColumnMode::Double:
			x = xDataColumn->valueAt(row);
			break;
		case AbstractColumn::ColumnMode::Integer:
			x = xDataColumn->integerAt(row);
			break;
		case AbstractColumn::ColumnMode::BigInt:
			x = xDataColumn->bigIntAt(row);
			break;
		case AbstractColumn::ColumnMode::Text: // invalid
			break;
		case AbstractColumn::ColumnMode::DateTime:
		case AbstractColumn::ColumnMode::Day:
		case AbstractColumn::ColumnMode::Month:
			x = xDataColumn->dateTimeAt(row).toMSecsSinceEpoch();
		}

		double y = NAN;
		switch (yDataColumn->columnMode()) {
		case AbstractColumn::ColumnMode::Double:
			y = yDataColumn->valueAt(row);
			break;
		case AbstractColumn::ColumnMode::Integer:
			y = yDataColumn->integerAt(row);
			break;
		case AbstractColumn::ColumnMode::BigInt:
			y = yDataColumn->bigIntAt(row);
			break;
		case AbstractColumn::ColumnMode::Text: // invalid
			break;
		case AbstractColumn::ColumnMode::DateTime:
		case AbstractColumn::ColumnMode::Day:
		case AbstractColumn::ColumnMode::Month:
			y = yDataColumn->dateTimeAt(row).toMSecsSinceEpoch();
		}

		// only when inside given range
		if (x >= xMin && x <= xMax) {
			if (xData.contains(x))
				uniqueX = false;
			xData.append(x);
			yData.append(y);
		}
	}

	if (uniqueX || !avgUniqueX)
		return;

	// for (int i = 0; i < xData.size(); i++)
	//	WARN( xData.at(i) << " " << yData.at(i))

	// average values for consecutive same x value
	double oldX = NAN, sum = 0.;
	int count = 1;
	for (int i = 0; i < xData.size(); i++) {
		// WARN(" i = " << i)
		const double x = xData.at(i);
		const double y = yData.at(i);
		// WARN(x << " / " << y << ": " << sum << " " << oldX  << " " << count)
		if (x == oldX) { // same x, but not last
			// DEBUG(" same x")
			sum += y;
			count++;
			if (i < xData.size() - 1) {
				continue;
			}
		}

		// WARN(" next/last x")
		if (count > 1) { // average and remove duplicate
			// WARN("average: " << sum/count)
			const int index = i - count + 1;
			yData[index - 1] = sum / count;
			// WARN("remove at " << index << ", count = " << count-1)
			xData.remove(index, count - 1);
			yData.remove(index, count - 1);

			i -= count - 1;
			count = 1;
		}
		sum = y;
		oldX = x;
	}
}

// ##############################################################################
// ##########################  getter methods  ##################################
// ##############################################################################
BASIC_SHARED_D_READER_IMPL(XYAnalysisCurve, XYAnalysisCurve::DataSourceType, dataSourceType, dataSourceType)
BASIC_SHARED_D_READER_IMPL(XYAnalysisCurve, const XYCurve*, dataSourceCurve, dataSourceCurve)
BASIC_SHARED_D_READER_IMPL(XYAnalysisCurve, QString, dataSourceCurvePath, dataSourceCurvePath)

BASIC_SHARED_D_READER_IMPL(XYAnalysisCurve, const AbstractColumn*, xDataColumn, xDataColumn)
BASIC_SHARED_D_READER_IMPL(XYAnalysisCurve, const AbstractColumn*, yDataColumn, yDataColumn)
BASIC_SHARED_D_READER_IMPL(XYAnalysisCurve, const AbstractColumn*, y2DataColumn, y2DataColumn)
BASIC_SHARED_D_READER_IMPL(XYAnalysisCurve, QString, xDataColumnPath, xDataColumnPath)
BASIC_SHARED_D_READER_IMPL(XYAnalysisCurve, QString, yDataColumnPath, yDataColumnPath)
BASIC_SHARED_D_READER_IMPL(XYAnalysisCurve, QString, y2DataColumnPath, y2DataColumnPath)

bool XYAnalysisCurve::saveCalculations() const {
	return const_cast<XYAnalysisCurve*>(this)->project()->saveCalculations();
}

// ##############################################################################
// #################  setter methods and undo commands ##########################
// ##############################################################################
STD_SETTER_CMD_IMPL_F_S(XYAnalysisCurve, SetDataSourceType, XYAnalysisCurve::DataSourceType, dataSourceType, sourceChanged)
void XYAnalysisCurve::setDataSourceType(DataSourceType type) {
	Q_D(XYAnalysisCurve);
	if (type != d->dataSourceType)
		exec(new XYAnalysisCurveSetDataSourceTypeCmd(d, type, ki18n("%1: data source type changed")));
}

STD_SETTER_CMD_IMPL_F_S(XYAnalysisCurve, SetDataSourceCurve, const XYCurve*, dataSourceCurve, sourceChanged)
void XYAnalysisCurve::setDataSourceCurve(const XYCurve* curve) {
	Q_D(XYAnalysisCurve);
	if (curve != d->dataSourceCurve)
		exec(new XYAnalysisCurveSetDataSourceCurveCmd(d, curve, ki18n("%1: data source curve changed")));
}

STD_SETTER_CMD_IMPL_F_S(XYAnalysisCurve, SetXDataColumn, const AbstractColumn*, xDataColumn, sourceChanged)
void XYAnalysisCurve::setXDataColumn(const AbstractColumn* column) {
	DEBUG(Q_FUNC_INFO);
	Q_D(XYAnalysisCurve);
	if (column != d->xDataColumn)
		exec(new XYAnalysisCurveSetXDataColumnCmd(d, column, ki18n("%1: assign x-data")));
}

STD_SETTER_CMD_IMPL_F_S(XYAnalysisCurve, SetYDataColumn, const AbstractColumn*, yDataColumn, sourceChanged)
void XYAnalysisCurve::setYDataColumn(const AbstractColumn* column) {
	DEBUG(Q_FUNC_INFO);
	Q_D(XYAnalysisCurve);
	if (column != d->yDataColumn)
		exec(new XYAnalysisCurveSetYDataColumnCmd(d, column, ki18n("%1: assign y-data")));
}

STD_SETTER_CMD_IMPL_F_S(XYAnalysisCurve, SetY2DataColumn, const AbstractColumn*, y2DataColumn, sourceChanged)
void XYAnalysisCurve::setY2DataColumn(const AbstractColumn* column) {
	DEBUG(Q_FUNC_INFO);
	Q_D(XYAnalysisCurve);
	if (column != d->y2DataColumn)
		exec(new XYAnalysisCurveSetY2DataColumnCmd(d, column, ki18n("%1: assign second y-data")));
}

void XYAnalysisCurve::setDataSourceCurvePath(const QString& path) {
	Q_D(XYAnalysisCurve);
	d->dataSourceCurvePath = path;
}

void XYAnalysisCurve::setXDataColumnPath(const QString& path) {
	Q_D(XYAnalysisCurve);
	d->xDataColumnPath = path;
}

void XYAnalysisCurve::setYDataColumnPath(const QString& path) {
	Q_D(XYAnalysisCurve);
	d->yDataColumnPath = path;
}

void XYAnalysisCurve::setY2DataColumnPath(const QString& path) {
	Q_D(XYAnalysisCurve);
	d->y2DataColumnPath = path;
}

// ##############################################################################
// #################################  SLOTS  ####################################
// ##############################################################################
/*!
 * called when the data in the data source aspects was changed.
 */
void XYAnalysisCurve::handleSourceDataChanged() {
	if (isLoading())
		return;

	Q_D(XYAnalysisCurve);
	d->sourceDataChangedSinceLastRecalc = true;
	Q_EMIT sourceDataChanged();
}

void XYAnalysisCurve::xDataColumnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(XYAnalysisCurve);
	if (aspect == d->xDataColumn) {
		d->xDataColumn = nullptr;
		d->retransform();
	}
}

void XYAnalysisCurve::yDataColumnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(XYAnalysisCurve);
	if (aspect == d->yDataColumn) {
		d->yDataColumn = nullptr;
		d->retransform();
	}
}

void XYAnalysisCurve::y2DataColumnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(XYAnalysisCurve);
	if (aspect == d->y2DataColumn) {
		d->y2DataColumn = nullptr;
		d->retransform();
	}
}

void XYAnalysisCurve::dataSourceCurveAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(XYAnalysisCurve);
	if (aspect == d->dataSourceCurve) {
		d->dataSourceCurve = nullptr;
		d->retransform();
	}
}

void XYAnalysisCurve::xDataColumnNameChanged() {
	Q_D(XYAnalysisCurve);
	setXDataColumnPath(d->xDataColumn->path());
}

void XYAnalysisCurve::yDataColumnNameChanged() {
	Q_D(XYAnalysisCurve);
	setYDataColumnPath(d->yDataColumn->path());
}

void XYAnalysisCurve::y2DataColumnNameChanged() {
	Q_D(XYAnalysisCurve);
	setYDataColumnPath(d->y2DataColumn->path());
}

void XYAnalysisCurve::dataSourceCurveNameChanged() {
	Q_D(XYAnalysisCurve);
	setDataSourceCurvePath(d->dataSourceCurve->path());
}

/*!
 * creates a new spreadsheet having the data with the results of the calculation.
 * the new spreadsheet is added to the current folder.
 */
void XYAnalysisCurve::createDataSpreadsheet() {
	if (!xColumn() || !yColumn())
		return;

	auto* spreadsheet = new Spreadsheet(i18n("%1 - Data", name()));
	spreadsheet->removeColumns(0, spreadsheet->columnCount()); // remove default columns
	spreadsheet->setRowCount(xColumn()->rowCount());

	// x values
	auto* data = static_cast<const Column*>(xColumn())->data();
	auto* xColumn = new Column(QLatin1String("x"), *static_cast<QVector<double>*>(data));
	xColumn->setPlotDesignation(AbstractColumn::PlotDesignation::X);
	spreadsheet->addChild(xColumn);

	// y values
	data = static_cast<const Column*>(yColumn())->data();
	auto* yColumn = new Column(QLatin1String("y"), *static_cast<QVector<double>*>(data));
	yColumn->setPlotDesignation(AbstractColumn::PlotDesignation::Y);
	spreadsheet->addChild(yColumn);

	// residual values for fit curves
	if (type() == AspectType::XYFitCurve) {
		data = static_cast<const Column*>(static_cast<XYFitCurve*>(this)->residualsColumn())->data();
		auto* residualsColumn = new Column(QLatin1String("residuals"), *static_cast<QVector<double>*>(data));
		residualsColumn->setPlotDesignation(AbstractColumn::PlotDesignation::Y);
		spreadsheet->addChild(residualsColumn);
	} else if (type() == AspectType::XYSmoothCurve) {
		// rough values for smooth curves
		data = static_cast<const Column*>(static_cast<XYSmoothCurve*>(this)->roughsColumn())->data();
		auto* roughsColumn = new Column(QLatin1String("rough values"), *static_cast<QVector<double>*>(data));
		roughsColumn->setPlotDesignation(AbstractColumn::PlotDesignation::Y);
		spreadsheet->addChild(roughsColumn);
	}

	// add the new spreadsheet to the current folder
	folder()->addChild(spreadsheet);
}

void XYAnalysisCurve::handleAspectUpdated(const QString& aspectPath, const AbstractAspect* aspect) {
	const auto column = dynamic_cast<const AbstractColumn*>(aspect);
	const auto curve = dynamic_cast<const XYCurve*>(aspect);

	setUndoAware(false);
	if (column) {
		if (xDataColumnPath() == aspectPath)
			setXDataColumn(column);
		if (yDataColumnPath() == aspectPath)
			setYDataColumn(column);
		if (y2DataColumnPath() == aspectPath)
			setY2DataColumn(column);

		// From XYCurve
		if (value()->columnPath() == aspectPath)
			value()->setColumn(column);
	} else if (curve) {
		if (dataSourceCurvePath() == aspectPath)
			setDataSourceCurve(curve);
	}
	setUndoAware(true);
}

// ##############################################################################
// ######################### Private implementation #############################
// ##############################################################################
XYAnalysisCurvePrivate::XYAnalysisCurvePrivate(XYAnalysisCurve* owner)
	: XYCurvePrivate(owner)
	, q(owner) {
}

// no need to delete xColumn and yColumn, they are deleted
// when the parent aspect is removed
XYAnalysisCurvePrivate::~XYAnalysisCurvePrivate() = default;

void XYAnalysisCurvePrivate::connectCurve(const XYCurve* curve) {
	if (!curve)
		return;

	// avoid circular dependencies - the current curve cannot be part of the variable curve.
	// this shouldn't actually happen because of the checks done when the function is defined,
	// but in case we have bugs somewhere or somebody manipulated the project xml file we add
	// a sanity check to avoid recursive calls here and crash because of the stack overflow.
	if (curve == q || dataSourceType != XYAnalysisCurve::DataSourceType::Curve)
		return;

	m_connections << q->connect(curve, &AbstractAspect::aspectDescriptionChanged, q, &XYAnalysisCurve::dataSourceCurveNameChanged);
	m_connections << q->connect(curve, &XYCurve::dataChanged, q, &XYAnalysisCurve::recalculate);
	m_connections << q->connect(curve, &XYCurve::xDataChanged, q, &XYAnalysisCurve::recalculate);
	m_connections << q->connect(curve, &XYCurve::yDataChanged, q, &XYAnalysisCurve::recalculate);
	m_connections << q->connect(curve, &AbstractAspect::aspectAboutToBeRemoved, q, &XYAnalysisCurve::dataSourceCurveAboutToBeRemoved);
	m_connections << q->connect(curve, &AbstractAspect::aspectAboutToBeRemoved, q, &XYAnalysisCurve::recalculate);

	// handle the changes when different columns were provided for the source curve
	m_connections << q->connect(curve, &XYCurve::xColumnChanged, q, &XYAnalysisCurve::handleSourceDataChanged);
	m_connections << q->connect(curve, &XYCurve::yColumnChanged, q, &XYAnalysisCurve::handleSourceDataChanged);
	// // TODO? connect(curve, SIGNAL(y2ColumnChanged(const AbstractColumn*)), this, SLOT(handleSourceDataChanged()));

	// handle the changes when the data inside of the source curve columns
	m_connections << q->connect(curve, &XYCurve::xDataChanged, q, &XYAnalysisCurve::handleSourceDataChanged);
	m_connections << q->connect(curve, &XYCurve::yDataChanged, q, &XYAnalysisCurve::handleSourceDataChanged);
}

void XYAnalysisCurvePrivate::connectColumn(const AbstractColumn* column, Dimension dim, bool second) {
	if (!column || dataSourceType != XYAnalysisCurve::DataSourceType::Spreadsheet)
		return;

	m_connections << q->connect(column, &AbstractColumn::dataChanged, q, &XYAnalysisCurve::recalculate);
	if (!second) {
		switch (dim) {
		case Dimension::X:
			m_connections << q->connect(column, &AbstractAspect::aspectDescriptionChanged, q, &XYAnalysisCurve::xDataColumnNameChanged);
			m_connections << q->connect(column, &AbstractAspect::aspectAboutToBeRemoved, q, &XYAnalysisCurve::xDataColumnAboutToBeRemoved);
			break;
		case Dimension::Y:
			m_connections << q->connect(column, &AbstractAspect::aspectDescriptionChanged, q, &XYAnalysisCurve::yDataColumnNameChanged);
			m_connections << q->connect(column, &AbstractAspect::aspectAboutToBeRemoved, q, &XYAnalysisCurve::yDataColumnAboutToBeRemoved);
			break;
		}
	} else {
		switch (dim) {
		case Dimension::X:
			break;
		case Dimension::Y:
			m_connections << q->connect(column, &AbstractAspect::aspectDescriptionChanged, q, &XYAnalysisCurve::y2DataColumnNameChanged);
			m_connections << q->connect(column, &AbstractAspect::aspectAboutToBeRemoved, q, &XYAnalysisCurve::y2DataColumnAboutToBeRemoved);
			break;
		}
	}
}

/*!
 * called when one of the source objects was changed,
 * updates the path and triggers the recalculate.
 */
void XYAnalysisCurvePrivate::sourceChanged() {
	updateConnections();

	if (q->isLoading())
		return;

	if (xDataColumn)
		q->setXDataColumnPath(xDataColumn->path());
	else
		q->setXDataColumnPath(QString());

	if (yDataColumn)
		q->setYDataColumnPath(yDataColumn->path());
	else
		q->setYDataColumnPath(QString());

	if (y2DataColumn)
		q->setY2DataColumnPath(y2DataColumn->path());
	else
		q->setY2DataColumnPath(QString());

	if (dataSourceCurve)
		q->setDataSourceCurvePath(dataSourceCurve->path());
	else
		q->setDataSourceCurvePath(QString());

	if (!q->isLoading())
		recalculate();
}

void XYAnalysisCurvePrivate::updateConnections() {
	for (auto c : m_connections)
		q->disconnect(c);

	switch (q->dataSourceType()) {
	case XYAnalysisCurve::DataSourceType::Curve:
		if (dataSourceCurve)
			connectCurve(dataSourceCurve);
		break;
	case XYAnalysisCurve::DataSourceType::Spreadsheet:
		if (xDataColumn)
			connectColumn(xDataColumn, Dimension::X, false);
		if (yDataColumn)
			connectColumn(yDataColumn, Dimension::Y, false);
		if (y2DataColumn)
			connectColumn(y2DataColumn, Dimension::Y, true);
		break;
	case XYAnalysisCurve::DataSourceType::Histogram:
		// TODO
		break;
	}
}

void XYAnalysisCurvePrivate::prepareTmpDataColumn(const AbstractColumn** tmpXDataColumn, const AbstractColumn** tmpYDataColumn) const {
	*tmpXDataColumn = nullptr;
	*tmpYDataColumn = nullptr;
	if (dataSourceType == XYAnalysisCurve::DataSourceType::Spreadsheet) {
		// spreadsheet columns as data source
		*tmpXDataColumn = xDataColumn;
		*tmpYDataColumn = yDataColumn;
	} else {
		if (!dataSourceCurve)
			return;
		// curve columns as data source
		*tmpXDataColumn = dataSourceCurve->xColumn();
		*tmpYDataColumn = dataSourceCurve->yColumn();
	}
}

void XYAnalysisCurvePrivate::recalculate() {
	// process all events first to close the context menu, if the new analysis curve is added via the context menu
	QApplication::processEvents(QEventLoop::AllEvents, 0);
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	// create filter result columns if not available yet, clear them otherwise
	if (!xColumn) {
		xColumn = new Column(QStringLiteral("x"), AbstractColumn::ColumnMode::Double);
		yColumn = new Column(QStringLiteral("y"), AbstractColumn::ColumnMode::Double);
		xVector = static_cast<QVector<double>*>(xColumn->data());
		yVector = static_cast<QVector<double>*>(yColumn->data());

		xColumn->setHidden(true);
		q->addChild(xColumn);
		yColumn->setHidden(true);
		q->addChild(yColumn);

		q->setUndoAware(false);
		q->setXColumn(xColumn); // pass the column to the xycurve
		q->setYColumn(yColumn); // pass the column to the xycurve
		q->setUndoAware(true);
	} else {
		xColumn->invalidateProperties();
		yColumn->invalidateProperties();
		if (xVector)
			xVector->clear();
		if (yVector)
			yVector->clear();
	}

	resetResults();

	const AbstractColumn* tmpXDataColumn = nullptr;
	const AbstractColumn* tmpYDataColumn = nullptr;
	prepareTmpDataColumn(&tmpXDataColumn, &tmpYDataColumn);

	if (!preparationValid(tmpXDataColumn, tmpYDataColumn)) {
		sourceDataChangedSinceLastRecalc = false;
		// recalcLogicalPoints(); TODO: needed?
	} else {
		bool result = recalculateSpecific(tmpXDataColumn, tmpYDataColumn);
		sourceDataChangedSinceLastRecalc = false;

		if (result) {
			// redraw the curve
			recalc();
		}
	}
	Q_EMIT q->dataChanged();
	QApplication::restoreOverrideCursor();
}

bool XYAnalysisCurvePrivate::preparationValid(const AbstractColumn* tmpXDataColumn, const AbstractColumn* tmpYDataColumn) {
	return tmpXDataColumn && tmpYDataColumn;
}

// ##############################################################################
// ##################  Serialization/Deserialization  ###########################
// ##############################################################################
//! Save as XML
void XYAnalysisCurve::save(QXmlStreamWriter* writer) const {
	Q_D(const XYAnalysisCurve);

	writer->writeStartElement(QStringLiteral("xyAnalysisCurve"));

	// write xy-curve information
	XYCurve::save(writer);

	// write data source specific information
	writer->writeStartElement(QStringLiteral("dataSource"));
	writer->writeAttribute(QStringLiteral("type"), QString::number(static_cast<int>(d->dataSourceType)));
	WRITE_PATH(d->dataSourceCurve, dataSourceCurve);
	WRITE_COLUMN(d->xDataColumn, xDataColumn);
	WRITE_COLUMN(d->yDataColumn, yDataColumn);
	WRITE_COLUMN(d->y2DataColumn, y2DataColumn);
	writer->writeEndElement();

	writer->writeEndElement(); //"xyAnalysisCurve"
}

//! Load from XML
bool XYAnalysisCurve::load(XmlStreamReader* reader, bool preview) {
	Q_D(XYAnalysisCurve);

	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == QLatin1String("xyAnalysisCurve"))
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == QLatin1String("xyCurve")) {
			if (!XYCurve::load(reader, preview))
				return false;
		} else if (reader->name() == QLatin1String("dataSource")) {
			attribs = reader->attributes();
			READ_INT_VALUE("type", dataSourceType, XYAnalysisCurve::DataSourceType);
			READ_PATH(dataSourceCurve);
			READ_COLUMN(xDataColumn);
			READ_COLUMN(yDataColumn);
			READ_COLUMN(y2DataColumn);
		} else { // unknown element
			reader->raiseUnknownElementWarning();
			if (!reader->skipToEndElement())
				return false;
		}
	}

	return true;
}
