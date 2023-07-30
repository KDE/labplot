/*
	File                 : QQPlot.cpp
	Project              : LabPlot
	Description          : QQPlot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

/*!
  \class QQPlot
  \brief

  \ingroup worksheet
  */
#include "QQPlot.h"
#include "QQPlotPrivate.h"
#include "backend/core/column/Column.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/macrosCurve.h"
#include "backend/lib/trace.h"
#include "backend/worksheet/Background.h"
#include "backend/worksheet/Line.h"
#include "backend/worksheet/plots/cartesian/Symbol.h"

extern "C" {
#include <gsl/gsl_cdf.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_statistics.h>
}

#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QPainter>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

CURVE_COLUMN_CONNECT(QQPlot, Data, data, recalc)

QQPlot::QQPlot(const QString& name)
	: Plot(name, new QQPlotPrivate(this), AspectType::QQPlot) {
	init();
}

QQPlot::QQPlot(const QString& name, QQPlotPrivate* dd)
	: Plot(name, dd, AspectType::QQPlot) {
	init();
}

// no need to delete the d-pointer here - it inherits from QGraphicsItem
// and is deleted during the cleanup in QGraphicsScene
QQPlot::~QQPlot() = default;

void QQPlot::init() {
	Q_D(QQPlot);

	KConfig config;
	KConfigGroup group = config.group("QQPlot");

	// reference curve - line conneting two central quantiles Q1 and Q3
	d->referenceCurve = new XYCurve(QStringLiteral("reference"));
	d->referenceCurve->setUndoAware(false);
	d->referenceCurve->setHidden(true);
	d->referenceCurve->graphicsItem()->setParentItem(d);

	d->referenceCurve->line()->init(group);
	d->referenceCurve->line()->setStyle(Qt::SolidLine);
	d->referenceCurve->symbol()->setStyle(Symbol::Style::NoSymbols);
	d->referenceCurve->background()->setPosition(Background::Position::No);

	d->referenceCurve->setUndoAware(true);

	// columns holding the data for the reference curve
	d->xReferenceColumn = new Column(QStringLiteral("xReference"));
	d->xReferenceColumn->setHidden(true);
	addChildFast(d->xReferenceColumn);
	d->referenceCurve->setXColumn(d->xReferenceColumn);

	d->yReferenceColumn = new Column(QStringLiteral("yReference"));
	d->yReferenceColumn->setHidden(true);
	addChildFast(d->yReferenceColumn);
	d->referenceCurve->setYColumn(d->yReferenceColumn);

	// percentiles curve
	d->percentilesCurve = new XYCurve(QStringLiteral("percentiles"));
	d->percentilesCurve->setUndoAware(false);
	d->percentilesCurve->setHidden(true);
	d->percentilesCurve->graphicsItem()->setParentItem(d);

	d->percentilesCurve->symbol()->init(group);
	d->percentilesCurve->symbol()->setStyle(Symbol::Style::Circle);
	d->percentilesCurve->line()->setStyle(Qt::NoPen);
	d->percentilesCurve->background()->setPosition(Background::Position::No);
	d->percentilesCurve->setUndoAware(true);

	// columns holding the data for the percentiles curve
	d->xPercentilesColumn = new Column(QStringLiteral("xPercentiles"));
	d->xPercentilesColumn->setHidden(true);
	addChildFast(d->xPercentilesColumn);
	d->percentilesCurve->setXColumn(d->xPercentilesColumn);

	d->yPercentilesColumn = new Column(QStringLiteral("yPercentiles"));
	d->yPercentilesColumn->setHidden(true);
	addChildFast(d->yPercentilesColumn);
	d->percentilesCurve->setYColumn(d->yPercentilesColumn);

	d->updateDistribution();
}

void QQPlot::finalizeAdd() {
	Q_D(QQPlot);
	WorksheetElement::finalizeAdd();
	addChild(d->referenceCurve);
	addChild(d->percentilesCurve);
}

void QQPlot::initActions() {
	visibilityAction = new QAction(QIcon::fromTheme(QStringLiteral("view-visible")), i18n("Visible"), this);
	visibilityAction->setCheckable(true);
	connect(visibilityAction, &QAction::triggered, this, &QQPlot::changeVisibility);
}

QMenu* QQPlot::createContextMenu() {
	if (!visibilityAction)
		initActions();

	QMenu* menu = WorksheetElement::createContextMenu();
	QAction* firstAction = menu->actions().at(1); // skip the first action because of the "title-action"
	visibilityAction->setChecked(isVisible());
	menu->insertAction(firstAction, visibilityAction);

	menu->insertSeparator(visibilityAction);
	menu->insertSeparator(firstAction);

	return menu;
}

/*!
  Returns an icon to be used in the project explorer.
  */
QIcon QQPlot::icon() const {
	return QIcon::fromTheme(QStringLiteral("view-object-histogram-linear"));
}

QGraphicsItem* QQPlot::graphicsItem() const {
	return d_ptr;
}

bool QQPlot::activatePlot(QPointF mouseScenePos, double maxDist) {
	Q_D(QQPlot);
	return d->activateCurve(mouseScenePos, maxDist);
}

void QQPlot::setHover(bool on) {
	Q_D(QQPlot);
	d->setHover(on);
}

void QQPlot::handleResize(double /*horizontalRatio*/, double /*verticalRatio*/, bool /*pageResize*/) {
	// TODO
}

void QQPlot::setVisible(bool on) {
	Q_D(QQPlot);
	beginMacro(on ? i18n("%1: set visible", name()) : i18n("%1: set invisible", name()));
	d->referenceCurve->setVisible(on);
	d->percentilesCurve->setVisible(on);
	WorksheetElement::setVisible(on);
	endMacro();
}

//##############################################################################
//##########################  getter methods  ##################################
//##############################################################################
// general
BASIC_SHARED_D_READER_IMPL(QQPlot, const AbstractColumn*, dataColumn, dataColumn)
BASIC_SHARED_D_READER_IMPL(QQPlot, QString, dataColumnPath, dataColumnPath)
BASIC_SHARED_D_READER_IMPL(QQPlot, nsl_sf_stats_distribution, distribution, distribution)

// line
Line* QQPlot::line() const {
	Q_D(const QQPlot);
	return d->referenceCurve->line();
}

// symbols
Symbol* QQPlot::symbol() const {
	Q_D(const QQPlot);
	return d->percentilesCurve->symbol();
}

bool QQPlot::minMax(const Dimension dim, const Range<int>& indexRange, Range<double>& r, bool /* includeErrorBars */) const {
	Q_D(const QQPlot);

	switch (dim) {
	case Dimension::X:
		return d->referenceCurve->minMax(dim, indexRange, r, false);
	case Dimension::Y: {
		Range referenceRange(r);
		Range percentilesRange(r);
		bool rc = true;
		rc = d->referenceCurve->minMax(dim, indexRange, referenceRange, false);
		if (!rc)
			return false;

		rc = d->percentilesCurve->minMax(dim, indexRange, percentilesRange, false);
		if (!rc)
			return false;

		r.setStart(std::min(referenceRange.start(), percentilesRange.start()));
		r.setEnd(std::max(referenceRange.end(), percentilesRange.end()));
		return true;
	}
	}
	return false;
}

double QQPlot::minimum(const Dimension dim) const {
	Q_D(const QQPlot);
	switch (dim) {
	case Dimension::X:
		return d->referenceCurve->minimum(dim);
	case Dimension::Y:
		return std::min(d->referenceCurve->minimum(dim), d->percentilesCurve->minimum(dim));
	}
	return NAN;
}

double QQPlot::maximum(const Dimension dim) const {
	Q_D(const QQPlot);
	switch (dim) {
	case Dimension::X:
		return d->referenceCurve->maximum(dim);
	case Dimension::Y:
		return std::max(d->referenceCurve->maximum(dim), d->percentilesCurve->maximum(dim));
	}
	return NAN;
}

bool QQPlot::hasData() const {
	Q_D(const QQPlot);
	return (d->dataColumn != nullptr);
}

//##############################################################################
//#################  setter methods and undo commands ##########################
//##############################################################################

// General
CURVE_COLUMN_SETTER_CMD_IMPL_F_S(QQPlot, Data, data, recalc)
void QQPlot::setDataColumn(const AbstractColumn* column) {
	Q_D(QQPlot);
	if (column != d->dataColumn)
		exec(new QQPlotSetDataColumnCmd(d, column, ki18n("%1: set data column")));
}

void QQPlot::setDataColumnPath(const QString& path) {
	Q_D(QQPlot);
	d->dataColumnPath = path;
}

STD_SETTER_CMD_IMPL_F_S(QQPlot, SetDistribution, nsl_sf_stats_distribution, distribution, updateDistribution)
void QQPlot::setDistribution(nsl_sf_stats_distribution distribution) {
	Q_D(QQPlot);
	if (distribution != d->distribution)
		exec(new QQPlotSetDistributionCmd(d, distribution, ki18n("%1: set distribution")));
}

//##############################################################################
//#################################  SLOTS  ####################################
//##############################################################################
void QQPlot::retransform() {
	d_ptr->retransform();
}

void QQPlot::recalc() {
	D(QQPlot);
	d->recalc();
}

void QQPlot::dataColumnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(QQPlot);
	if (aspect == d->dataColumn) {
		d->dataColumn = nullptr;
		d->retransform();
	}
}

void QQPlot::dataColumnNameChanged() {
	Q_D(QQPlot);
	setDataColumnPath(d->dataColumn->path());
}

//##############################################################################
//######################### Private implementation #############################
//##############################################################################
QQPlotPrivate::QQPlotPrivate(QQPlot* owner)
	: PlotPrivate(owner)
	, q(owner) {
	setFlag(QGraphicsItem::ItemIsSelectable, true);
	setAcceptHoverEvents(false);
}

QQPlotPrivate::~QQPlotPrivate() {
}

QRectF QQPlotPrivate::boundingRect() const {
	return boundingRectangle;
}

/*!
  Returns the shape of the QQPlot as a QPainterPath in local coordinates
  */
QPainterPath QQPlotPrivate::shape() const {
	return curveShape;
}

/*!
  called when the size of the plot or its data ranges (manual changes, zooming, etc.) were changed.
  recalculates the position of the scene points to be drawn.
  triggers the update of lines, drop lines, symbols etc.
*/
void QQPlotPrivate::retransform() {
	const bool suppressed = suppressRetransform || q->isLoading();
	if (suppressed)
		return;

	if (!isVisible())
		return;

	PERFTRACE(name() + QLatin1String(Q_FUNC_INFO));
	referenceCurve->retransform();
	percentilesCurve->retransform();
	recalcShapeAndBoundingRect();
}

/*!
 * called when the source data was changed, recalculates the plot.
 */
void QQPlotPrivate::recalc() {
	PERFTRACE(name() + QLatin1String(Q_FUNC_INFO));

	if (!dataColumn) {
		yPercentilesColumn->clear();
		Q_EMIT q->dataChanged();
		return;
	}

	// copy the non-nan and not masked values into a new vector
	QVector<double> rawData;
	copyValidData(rawData);
	size_t n = rawData.count();

	// sort the data to calculate the percentiles
	std::sort(rawData.begin(), rawData.end());

	// calculate y-values - the percentiles for the column data
	QVector<double> yData;
	for (int i = 1; i < 100; ++i)
		yData << gsl_stats_quantile_from_sorted_data(rawData.data(), 1, n, double(i) / 100.);

	yPercentilesColumn->replaceValues(0, yData);

	double y1 = gsl_stats_quantile_from_sorted_data(rawData.data(), 1, n, 0.01);
	double y2 = gsl_stats_quantile_from_sorted_data(rawData.data(), 1, n, 0.99);
	yReferenceColumn->setValueAt(0, y1);
	yReferenceColumn->setValueAt(1, y2);

	// Q_EMIT dataChanged() in order to retransform everything with the new size/shape of the plot
	Q_EMIT q->dataChanged();
}

/*!
 * called when the distribution was changed, recalculates everything that depends on
 * the distribution only and doesn't dependent on the source data
 */
void QQPlotPrivate::updateDistribution() {
	QVector<double> xData;
	double x1 = 0.;
	double x2 = 0.;

	// handle all distributions where the inverse of the CDF is available in gsl_cdf.h
	switch (distribution) {
	case nsl_sf_stats_gaussian: {
		x1 = gsl_cdf_gaussian_Pinv(0.01, 1.0);
		x2 = gsl_cdf_gaussian_Pinv(0.99, 1.0);
		for (int i = 1; i < 100; ++i)
			xData << gsl_cdf_gaussian_Pinv(double(i) / 100., 1.0);
		break;
	}
	case nsl_sf_stats_exponential: {
		x1 = gsl_cdf_exponential_Pinv(0.01, 1.0);
		x2 = gsl_cdf_exponential_Pinv(0.99, 1.0);
		for (int i = 1; i < 100; ++i)
			xData << gsl_cdf_exponential_Pinv(double(i) / 100., 1.0);
		break;
	}
	case nsl_sf_stats_laplace: {
		x1 = gsl_cdf_laplace_Pinv(0.01, 1.0);
		x2 = gsl_cdf_laplace_Pinv(0.99, 1.0);
		for (int i = 1; i < 100; ++i)
			xData << gsl_cdf_laplace_Pinv(double(i) / 100., 1.0);
		break;
	}
	case nsl_sf_stats_cauchy_lorentz: {
		x1 = gsl_cdf_cauchy_Pinv(0.01, 1.0);
		x2 = gsl_cdf_cauchy_Pinv(0.99, 1.0);
		for (int i = 1; i < 100; ++i)
			xData << gsl_cdf_cauchy_Pinv(double(i) / 100., 1.0);
		break;
	}
	case nsl_sf_stats_rayleigh: {
		x1 = gsl_cdf_rayleigh_Pinv(0.01, 1.0);
		x2 = gsl_cdf_rayleigh_Pinv(0.99, 1.0);
		for (int i = 1; i < 100; ++i)
			xData << gsl_cdf_rayleigh_Pinv(double(i) / 100., 1.0);
		break;
	}
	case nsl_sf_stats_gamma: {
		x1 = gsl_cdf_gamma_Pinv(0.01, 1.0, 1.0);
		x2 = gsl_cdf_gamma_Pinv(0.99, 1.0, 1.0);
		for (int i = 1; i < 100; ++i)
			xData << gsl_cdf_gamma_Pinv(double(i) / 100., 1.0, 1.0);
		break;
	}
	case nsl_sf_stats_flat: {
		x1 = gsl_cdf_flat_Pinv(0.01, 0.0, 1.0);
		x2 = gsl_cdf_flat_Pinv(0.99, 0.0, 1.0);
		for (int i = 1; i < 100; ++i)
			xData << gsl_cdf_flat_Pinv(double(i) / 100., 0.0, 1.0);
		break;
	}
	case nsl_sf_stats_lognormal: {
		x1 = gsl_cdf_lognormal_Pinv(0.01, 1.0, 1.0);
		x2 = gsl_cdf_lognormal_Pinv(0.99, 1.0, 1.0);
		for (int i = 1; i < 100; ++i)
			xData << gsl_cdf_lognormal_Pinv(double(i) / 100., 1.0, 1.0);
		break;
	}
	case nsl_sf_stats_chi_squared: {
		x1 = gsl_cdf_chisq_Pinv(0.01, 1.0);
		x2 = gsl_cdf_chisq_Pinv(0.99, 1.0);
		for (int i = 1; i < 100; ++i)
			xData << gsl_cdf_chisq_Pinv(double(i) / 100., 1.0);
		break;
	}
	case nsl_sf_stats_fdist: {
		x1 = gsl_cdf_fdist_Pinv(0.01, 1.0, 1.0);
		x2 = gsl_cdf_fdist_Pinv(0.99, 1.0, 1.0);
		for (int i = 1; i < 100; ++i)
			xData << gsl_cdf_fdist_Pinv(double(i) / 100., 1.0, 1.0);
		break;
	}
	case nsl_sf_stats_tdist: {
		x1 = gsl_cdf_tdist_Pinv(0.01, 1.0);
		x2 = gsl_cdf_tdist_Pinv(0.99, 1.0);
		for (int i = 1; i < 100; ++i)
			xData << gsl_cdf_tdist_Pinv(double(i) / 100., 1.0);
		break;
	}
	case nsl_sf_stats_beta: {
		x1 = gsl_cdf_beta_Pinv(0.01, 1.0, 1.0);
		x2 = gsl_cdf_beta_Pinv(0.99, 1.0, 1.0);
		for (int i = 1; i < 100; ++i)
			xData << gsl_cdf_beta_Pinv(double(i) / 100., 1.0, 1.0);
		break;
	}
	case nsl_sf_stats_logistic: {
		x1 = gsl_cdf_logistic_Pinv(0.01, 1.0);
		x2 = gsl_cdf_logistic_Pinv(0.99, 1.0);
		for (int i = 1; i < 100; ++i)
			xData << gsl_cdf_logistic_Pinv(double(i) / 100., 1.0);
		break;
	}
	case nsl_sf_stats_pareto: {
		x1 = gsl_cdf_pareto_Pinv(0.01, 1.0, 1.0);
		x2 = gsl_cdf_pareto_Pinv(0.99, 1.0, 1.0);
		for (int i = 1; i < 100; ++i)
			xData << gsl_cdf_pareto_Pinv(double(i) / 100., 1.0, 1.0);
		break;
	}
	case nsl_sf_stats_weibull: {
		x1 = gsl_cdf_weibull_Pinv(0.01, 1.0, 1.0);
		x2 = gsl_cdf_weibull_Pinv(0.99, 1.0, 1.0);
		for (int i = 1; i < 100; ++i)
			xData << gsl_cdf_weibull_Pinv(double(i) / 100., 1.0, 1.0);
		break;
	}
	case nsl_sf_stats_gumbel1: {
		x1 = gsl_cdf_gumbel1_Pinv(0.01, 1.0, 1.0);
		x2 = gsl_cdf_gumbel1_Pinv(0.99, 1.0, 1.0);
		for (int i = 1; i < 100; ++i)
			xData << gsl_cdf_gumbel1_Pinv(double(i) / 100., 1.0, 1.0);
		break;
	}
	case nsl_sf_stats_gumbel2: {
		x1 = gsl_cdf_gumbel2_Pinv(0.01, 1.0, 1.0);
		x2 = gsl_cdf_gumbel2_Pinv(0.99, 1.0, 1.0);
		for (int i = 1; i < 100; ++i)
			xData << gsl_cdf_gumbel2_Pinv(double(i) / 100., 1.0, 1.0);
		break;
	}
	case nsl_sf_stats_gaussian_tail:
	case nsl_sf_stats_exponential_power:
	case nsl_sf_stats_rayleigh_tail:
	case nsl_sf_stats_landau:
	case nsl_sf_stats_levy_alpha_stable:
	case nsl_sf_stats_levy_skew_alpha_stable:
	case nsl_sf_stats_poisson:
	case nsl_sf_stats_bernoulli:
	case nsl_sf_stats_binomial:
	case nsl_sf_stats_negative_binomial:
	case nsl_sf_stats_pascal:
	case nsl_sf_stats_geometric:
	case nsl_sf_stats_hypergeometric:
	case nsl_sf_stats_logarithmic:
	case nsl_sf_stats_maxwell_boltzmann:
	case nsl_sf_stats_sech:
	case nsl_sf_stats_levy:
	case nsl_sf_stats_frechet:
		break;
	}

	xReferenceColumn->setValueAt(0, x1);
	xReferenceColumn->setValueAt(1, x2);

	xPercentilesColumn->replaceValues(0, xData);

	// Q_EMIT dataChanged() in order to retransform everything with the new size/shape of the plot
	Q_EMIT q->dataChanged();
}

/*!
 * copy the non-nan and not masked values of the current column
 * into the vector \c data.
 */
void QQPlotPrivate::copyValidData(QVector<double>& data) const {
	const int rowCount = dataColumn->rowCount();
	data.reserve(rowCount);
	double val;
	if (dataColumn->columnMode() == AbstractColumn::ColumnMode::Double) {
		auto* rowValues = reinterpret_cast<QVector<double>*>(reinterpret_cast<const Column*>(dataColumn)->data());
		for (int row = 0; row < rowCount; ++row) {
			val = rowValues->value(row);
			if (std::isnan(val) || dataColumn->isMasked(row))
				continue;

			data.push_back(val);
		}
	} else if (dataColumn->columnMode() == AbstractColumn::ColumnMode::Integer) {
		auto* rowValues = reinterpret_cast<QVector<int>*>(reinterpret_cast<const Column*>(dataColumn)->data());
		for (int row = 0; row < rowCount; ++row) {
			val = rowValues->value(row);
			if (std::isnan(val) || dataColumn->isMasked(row))
				continue;

			data.push_back(val);
		}
	} else if (dataColumn->columnMode() == AbstractColumn::ColumnMode::BigInt) {
		auto* rowValues = reinterpret_cast<QVector<qint64>*>(reinterpret_cast<const Column*>(dataColumn)->data());
		for (int row = 0; row < rowCount; ++row) {
			val = rowValues->value(row);
			if (std::isnan(val) || dataColumn->isMasked(row))
				continue;

			data.push_back(val);
		}
	}

	if (data.size() < rowCount)
		data.squeeze();
}

/*!
  recalculates the outer bounds and the shape of the curve.
  */
void QQPlotPrivate::recalcShapeAndBoundingRect() {
	if (m_suppressRecalc)
		return;

	prepareGeometryChange();
	curveShape = QPainterPath();
	curveShape.addPath(referenceCurve->graphicsItem()->shape());
	curveShape.addPath(percentilesCurve->graphicsItem()->shape());

	boundingRectangle = curveShape.boundingRect();
}

bool QQPlotPrivate::activateCurve(QPointF mouseScenePos, double /*maxDist*/) {
	if (!isVisible())
		return false;

	return curveShape.contains(mouseScenePos);
}

/*!
 * Is called in CartesianPlot::hoverMoveEvent where it is determined which curve to hover.
 * \p on
 */
void QQPlotPrivate::setHover(bool on) {
	if (on == m_hovered)
		return; // don't update if state not changed

	m_hovered = on;
	on ? Q_EMIT q->hovered() : emit q->unhovered();
	update();
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
//! Save as XML
void QQPlot::save(QXmlStreamWriter* writer) const {
	Q_D(const QQPlot);

	writer->writeStartElement(QStringLiteral("QQPlot"));
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	// general
	writer->writeStartElement(QStringLiteral("general"));
	WRITE_COLUMN(d->dataColumn, dataColumn);
	WRITE_COLUMN(d->xReferenceColumn, xReferenceColumn);
	WRITE_COLUMN(d->yReferenceColumn, yReferenceColumn);
	WRITE_COLUMN(d->xPercentilesColumn, xPercentilesColumn);
	WRITE_COLUMN(d->yPercentilesColumn, yPercentilesColumn);
	writer->writeAttribute(QStringLiteral("distribution"), QString::number(static_cast<int>(d->distribution)));
	writer->writeAttribute(QStringLiteral("visible"), QString::number(d->isVisible()));
	writer->writeEndElement();

	// save the internal columns, above only the references to them were saved
	d->xReferenceColumn->save(writer);
	d->yReferenceColumn->save(writer);
	d->xPercentilesColumn->save(writer);
	d->yPercentilesColumn->save(writer);

	// save the internal curves
	d->referenceCurve->save(writer);
	d->percentilesCurve->save(writer);

	writer->writeEndElement(); // close "QQPlot" section
}

//! Load from XML
bool QQPlot::load(XmlStreamReader* reader, bool preview) {
	Q_D(QQPlot);

	if (!readBasicAttributes(reader))
		return false;

	KLocalizedString attributeWarning = ki18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == QLatin1String("QQPlot"))
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == QLatin1String("comment")) {
			if (!readCommentElement(reader))
				return false;
		} else if (!preview && reader->name() == QLatin1String("general")) {
			attribs = reader->attributes();
			READ_COLUMN(dataColumn);
			READ_COLUMN(xReferenceColumn);
			READ_COLUMN(yReferenceColumn);
			READ_COLUMN(xPercentilesColumn);
			READ_COLUMN(yPercentilesColumn);
			READ_INT_VALUE("distribution", distribution, nsl_sf_stats_distribution);

			str = attribs.value(QStringLiteral("visible")).toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs(QStringLiteral("visible")).toString());
			else
				d->setVisible(str.toInt());
		} else if (reader->name() == QLatin1String("column")) {
			attribs = reader->attributes();
			bool rc = true;
			const auto& name = attribs.value(QStringLiteral("name"));
			if (name == QLatin1String("xReference"))
				rc = d->xReferenceColumn->load(reader, preview);
			else if (name == QLatin1String("yReference"))
				rc = d->yReferenceColumn->load(reader, preview);
			else if (name == QLatin1String("xPercentiles"))
				rc = d->xPercentilesColumn->load(reader, preview);
			else if (name == QLatin1String("yPercentiles"))
				rc = d->yPercentilesColumn->load(reader, preview);

			if (!rc)
				return false;
		} else if (reader->name() == QLatin1String("xyCurve")) {
			attribs = reader->attributes();
			bool rc = true;
			if (attribs.value(QStringLiteral("name")) == QLatin1String("reference"))
				rc = d->referenceCurve->load(reader, preview);
			else
				rc = d->percentilesCurve->load(reader, preview);

			if (!rc)
				return false;
		}
	}
	return true;
}

//##############################################################################
//#########################  Theme management ##################################
//##############################################################################
void QQPlot::loadThemeConfig(const KConfig& config) {
	KConfigGroup group;
	if (config.hasGroup(QLatin1String("Theme")))
		group = config.group("XYCurve"); // when loading from the theme config, use the same properties as for XYCurve
	else
		group = config.group("QQPlot");

	const auto* plot = static_cast<const CartesianPlot*>(parentAspect());
	int index = plot->curveChildIndex(this);
	const QColor themeColor = plot->themeColorPalette(index);

	QPen p;

	Q_D(QQPlot);
	d->m_suppressRecalc = true;

	d->referenceCurve->line()->loadThemeConfig(group, themeColor);
	d->percentilesCurve->line()->setStyle(Qt::NoPen);
	d->percentilesCurve->symbol()->loadThemeConfig(group, themeColor);

	d->m_suppressRecalc = false;
	d->recalcShapeAndBoundingRect();
}

void QQPlot::saveThemeConfig(const KConfig& config) {
	Q_D(const QQPlot);
	KConfigGroup group = config.group("QQPlot");
	d->referenceCurve->line()->saveThemeConfig(group);
	d->percentilesCurve->symbol()->saveThemeConfig(group);
}
