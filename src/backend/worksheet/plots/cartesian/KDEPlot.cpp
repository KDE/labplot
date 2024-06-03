/*
	File                 : KDEPlot.cpp
	Project              : LabPlot
	Description          : KDE Plot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

/*!
  \class KDEPlot
  \brief

  \ingroup worksheet
  */
#include "KDEPlot.h"
#include "KDEPlotPrivate.h"
#include "backend/core/column/Column.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/macrosCurve.h"
#include "backend/lib/trace.h"
#include "backend/worksheet/Background.h"
#include "backend/worksheet/Line.h"
#include "backend/worksheet/plots/cartesian/Symbol.h"

#include "backend/nsl/nsl_kde.h"
#include "backend/nsl/nsl_sf_kernel.h"

#include <QMenu>
#include <QPainter>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

CURVE_COLUMN_CONNECT(KDEPlot, Data, data, recalc)

KDEPlot::KDEPlot(const QString& name)
	: Plot(name, new KDEPlotPrivate(this), AspectType::KDEPlot) {
	init();
}

KDEPlot::KDEPlot(const QString& name, KDEPlotPrivate* dd)
	: Plot(name, dd, AspectType::KDEPlot) {
	init();
}

// no need to delete the d-pointer here - it inherits from QGraphicsItem
// and is deleted during the cleanup in QGraphicsScene
KDEPlot::~KDEPlot() = default;

void KDEPlot::init() {
	Q_D(KDEPlot);

	KConfig config;
	KConfigGroup group = config.group(QStringLiteral("KDEPlot"));

	// general
	d->kernelType = static_cast<nsl_kernel_type>(group.readEntry(QStringLiteral("kernelType"), static_cast<int>(nsl_kernel_gauss)));
	d->bandwidthType = static_cast<nsl_kde_bandwidth_type>(group.readEntry(QStringLiteral("bandwidthType"), static_cast<int>(nsl_kde_bandwidth_silverman)));
	d->bandwidth = group.readEntry(QStringLiteral("bandwidth"), 0.1);

	// estimation curve
	d->estimationCurve = new XYCurve(QStringLiteral("estimation"));
	d->estimationCurve->setName(name(), AbstractAspect::NameHandling::UniqueNotRequired);
	d->estimationCurve->setHidden(true);
	d->estimationCurve->graphicsItem()->setParentItem(d);
	d->estimationCurve->line()->init(group);
	d->estimationCurve->line()->setStyle(Qt::SolidLine);
	d->estimationCurve->symbol()->setStyle(Symbol::Style::NoSymbols);
	d->estimationCurve->background()->setPosition(Background::Position::No);

	// columns holding the data for the reference curve
	d->xEstimationColumn = new Column(QStringLiteral("xReference"));
	d->xEstimationColumn->setHidden(true);
	d->xEstimationColumn->setUndoAware(false);
	addChildFast(d->xEstimationColumn);
	d->estimationCurve->setXColumn(d->xEstimationColumn);

	d->yEstimationColumn = new Column(QStringLiteral("yReference"));
	d->yEstimationColumn->setHidden(true);
	d->yEstimationColumn->setUndoAware(false);
	addChildFast(d->yEstimationColumn);
	d->estimationCurve->setYColumn(d->yEstimationColumn);

	// xy-curve for the rug plot
	d->rugCurve = new XYCurve(QStringLiteral("rug"));
	d->rugCurve->setName(name(), AbstractAspect::NameHandling::UniqueNotRequired);
	d->rugCurve->setHidden(true);
	d->rugCurve->graphicsItem()->setParentItem(d);
	d->rugCurve->line()->setStyle(Qt::NoPen);
	d->rugCurve->symbol()->setStyle(Symbol::Style::NoSymbols);
	d->rugCurve->setRugOrientation(WorksheetElement::Orientation::Horizontal);

	// synchronize the names of the internal XYCurves with the name of the current q-q plot
	// so we have the same name shown on the undo stack
	connect(this, &AbstractAspect::aspectDescriptionChanged, [this] {
		Q_D(KDEPlot);
		d->estimationCurve->setName(name(), AbstractAspect::NameHandling::UniqueNotRequired);
		d->rugCurve->setName(name(), AbstractAspect::NameHandling::UniqueNotRequired);
	});
}

void KDEPlot::finalizeAdd() {
	Q_D(KDEPlot);
	WorksheetElement::finalizeAdd();
	addChildFast(d->estimationCurve);
	addChildFast(d->rugCurve);
}

/*!
  Returns an icon to be used in the project explorer.
  */
QIcon KDEPlot::icon() const {
	return QIcon::fromTheme(QStringLiteral("view-object-histogram-linear"));
}

void KDEPlot::handleResize(double /*horizontalRatio*/, double /*verticalRatio*/, bool /*pageResize*/) {
	// TODO
}

void KDEPlot::setVisible(bool on) {
	Q_D(KDEPlot);
	beginMacro(on ? i18n("%1: set visible", name()) : i18n("%1: set invisible", name()));
	d->estimationCurve->setVisible(on);
	d->rugCurve->setVisible(on);
	WorksheetElement::setVisible(on);
	endMacro();
}

// ##############################################################################
// ##########################  getter methods  ##################################
// ##############################################################################
//  general
BASIC_SHARED_D_READER_IMPL(KDEPlot, const AbstractColumn*, dataColumn, dataColumn)
BASIC_SHARED_D_READER_IMPL(KDEPlot, QString, dataColumnPath, dataColumnPath)
BASIC_SHARED_D_READER_IMPL(KDEPlot, nsl_kernel_type, kernelType, kernelType)
BASIC_SHARED_D_READER_IMPL(KDEPlot, nsl_kde_bandwidth_type, bandwidthType, bandwidthType)
BASIC_SHARED_D_READER_IMPL(KDEPlot, double, bandwidth, bandwidth)

XYCurve* KDEPlot::estimationCurve() const {
	Q_D(const KDEPlot);
	return d->estimationCurve;
}

XYCurve* KDEPlot::rugCurve() const {
	Q_D(const KDEPlot);
	return d->rugCurve;
}

bool KDEPlot::minMax(const Dimension dim, const Range<int>& indexRange, Range<double>& r, bool /* includeErrorBars */) const {
	Q_D(const KDEPlot);
	return d->estimationCurve->minMax(dim, indexRange, r);
}

double KDEPlot::minimum(const Dimension dim) const {
	Q_D(const KDEPlot);
	switch (dim) {
	case Dimension::X:
		return d->estimationCurve->minimum(dim);
	case Dimension::Y:
		return d->estimationCurve->minimum(dim);
	}
	return NAN;
}

double KDEPlot::maximum(const Dimension dim) const {
	Q_D(const KDEPlot);
	switch (dim) {
	case Dimension::X:
		return d->estimationCurve->maximum(dim);
	case Dimension::Y:
		return d->estimationCurve->maximum(dim);
	}
	return NAN;
}

bool KDEPlot::hasData() const {
	Q_D(const KDEPlot);
	return (d->dataColumn != nullptr);
}

bool KDEPlot::usingColumn(const Column* column) const {
	Q_D(const KDEPlot);
	return (d->dataColumn == column);
}

void KDEPlot::handleAspectUpdated(const QString& aspectPath, const AbstractAspect* aspect) {
	Q_D(KDEPlot);
	const auto column = dynamic_cast<const AbstractColumn*>(aspect);
	if (!column)
		return;

	if (d->dataColumn == column) // the column is the same and was just renamed -> update the column path
		d->dataColumnPath = aspectPath;
	else if (d->dataColumnPath == aspectPath) { // another column was renamed to the current path -> set and connect to the new column
		setUndoAware(false);
		setDataColumn(column);
		setUndoAware(true);
	}
}

QColor KDEPlot::color() const {
	Q_D(const KDEPlot);
	return d->estimationCurve->color();
}
/*!
 * returns the the number of equaly spaced points at which the density is to be evaluated,
 * which also corresponds to the number of data points in the xy-curve used internally.
 */
int KDEPlot::gridPointsCount() const {
	Q_D(const KDEPlot);
	return d->gridPointsCount;
}

// ##############################################################################
// #################  setter methods and undo commands ##########################
// ##############################################################################

// General
CURVE_COLUMN_SETTER_CMD_IMPL_F_S(KDEPlot, Data, data, recalc)
void KDEPlot::setDataColumn(const AbstractColumn* column) {
	Q_D(KDEPlot);
	if (column != d->dataColumn)
		exec(new KDEPlotSetDataColumnCmd(d, column, ki18n("%1: set data column")));
}

void KDEPlot::setDataColumnPath(const QString& path) {
	Q_D(KDEPlot);
	d->dataColumnPath = path;
}

STD_SETTER_CMD_IMPL_F_S(KDEPlot, SetKernelType, nsl_kernel_type, kernelType, recalc)
void KDEPlot::setKernelType(nsl_kernel_type kernelType) {
	Q_D(KDEPlot);
	if (kernelType != d->kernelType)
		exec(new KDEPlotSetKernelTypeCmd(d, kernelType, ki18n("%1: set kernel type")));
}

STD_SETTER_CMD_IMPL_F_S(KDEPlot, SetBandwidthType, nsl_kde_bandwidth_type, bandwidthType, recalc)
void KDEPlot::setBandwidthType(nsl_kde_bandwidth_type bandwidthType) {
	Q_D(KDEPlot);
	if (bandwidthType != d->bandwidthType)
		exec(new KDEPlotSetBandwidthTypeCmd(d, bandwidthType, ki18n("%1: set bandwidth type")));
}

STD_SETTER_CMD_IMPL_F_S(KDEPlot, SetBandwidth, double, bandwidth, recalc)
void KDEPlot::setBandwidth(double bandwidth) {
	Q_D(KDEPlot);
	if (bandwidth != d->bandwidth)
		exec(new KDEPlotSetBandwidthCmd(d, bandwidth, ki18n("%1: set bandwidth")));
}

// ##############################################################################
// #################################  SLOTS  ####################################
// ##############################################################################
void KDEPlot::retransform() {
	d_ptr->retransform();
}

void KDEPlot::recalc() {
	D(KDEPlot);
	d->recalc();
}

void KDEPlot::dataColumnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(KDEPlot);
	if (aspect == d->dataColumn) {
		d->dataColumn = nullptr;
		d->retransform();
		Q_EMIT dataChanged();
		Q_EMIT changed();
	}
}

// ##############################################################################
// ######################### Private implementation #############################
// ##############################################################################
KDEPlotPrivate::KDEPlotPrivate(KDEPlot* owner)
	: PlotPrivate(owner)
	, q(owner) {
	setFlag(QGraphicsItem::ItemIsSelectable, true);
	setAcceptHoverEvents(false);
}

KDEPlotPrivate::~KDEPlotPrivate() {
}

/*!
  called when the size of the plot or its data ranges (manual changes, zooming, etc.) were changed.
  recalculates the position of the scene points to be drawn.
  triggers the update of lines, drop lines, symbols etc.
*/
void KDEPlotPrivate::retransform() {
	const bool suppressed = suppressRetransform || q->isLoading();
	if (suppressed)
		return;

	if (!isVisible())
		return;

	PERFTRACE(name() + QLatin1String(Q_FUNC_INFO));
	estimationCurve->retransform();
	rugCurve->retransform();
	recalcShapeAndBoundingRect();
}

/*!
 * called when the source data was changed, recalculates the plot.
 */
void KDEPlotPrivate::recalc() {
	PERFTRACE(name() + QLatin1String(Q_FUNC_INFO));

	rugCurve->setXColumn(dataColumn);
	rugCurve->setYColumn(dataColumn);

	if (!dataColumn) {
		xEstimationColumn->clear();
		yEstimationColumn->clear();

		Q_EMIT q->dataChanged();
		return;
	}

	// copy the non-nan and not masked values into a new vector
	QVector<double> data;
	copyValidData(data);

	// calculate the estimation curve for the number
	// of equaly spaced points determined by gridPoints
	QVector<double> xData;
	QVector<double> yData;
	xData.resize(gridPointsCount);
	yData.resize(gridPointsCount);
	int n = data.count();
	const auto& statistics = static_cast<const Column*>(dataColumn)->statistics();
	const double sigma = statistics.standardDeviation;
	const double iqr = statistics.iqr;

	// bandwidth
	double h;
	if (bandwidthType == nsl_kde_bandwidth_custom) {
		if (bandwidth != 0)
			h = bandwidth;
		else {
			// invalid smoothing bandwidth parameter
			xEstimationColumn->setValues(xData);
			yEstimationColumn->setValues(yData);
			Q_EMIT q->dataChanged();
			return;
		}
	} else
		h = nsl_kde_bandwidth(n, sigma, iqr, bandwidthType);

	// calculate KDE for the grid points from min-3*sigma to max+3*sigma
	const double min = statistics.minimum - 3 * h;
	const double max = statistics.maximum + 3 * h;
	const double step = (max - min) / gridPointsCount;

	for (int i = 0; i < gridPointsCount; ++i) {
		double x = min + i * step;
		xData[i] = x;
		yData[i] = nsl_kde(data.data(), x, kernelType, h, n);
	}

	xEstimationColumn->setValues(xData);
	yEstimationColumn->setValues(yData);

	// Q_EMIT dataChanged() in order to retransform everything with the new size/shape of the plot
	Q_EMIT q->dataChanged();
}

/*!
 * copy the non-nan and not masked values of the current column
 * into the vector \c data.
 */
void KDEPlotPrivate::copyValidData(QVector<double>& data) const {
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
void KDEPlotPrivate::recalcShapeAndBoundingRect() {
	if (suppressRecalc)
		return;

	prepareGeometryChange();
	m_shape = QPainterPath();
	m_shape.addPath(estimationCurve->graphicsItem()->shape());
	m_shape.addPath(rugCurve->graphicsItem()->shape());
	m_boundingRectangle = m_shape.boundingRect();
}

// ##############################################################################
// ##################  Serialization/Deserialization  ###########################
// ##############################################################################
//! Save as XML
void KDEPlot::save(QXmlStreamWriter* writer) const {
	Q_D(const KDEPlot);

	writer->writeStartElement(QStringLiteral("KDEPlot"));
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	// general
	writer->writeStartElement(QStringLiteral("general"));
	WRITE_COLUMN(d->dataColumn, dataColumn);
	writer->writeAttribute(QStringLiteral("kernelType"), QString::number(d->kernelType));
	writer->writeAttribute(QStringLiteral("bandwidthType"), QString::number(d->bandwidthType));
	writer->writeAttribute(QStringLiteral("bandwidth"), QString::number(d->bandwidth));
	writer->writeAttribute(QStringLiteral("visible"), QString::number(d->isVisible()));
	writer->writeAttribute(QStringLiteral("legendVisible"), QString::number(d->legendVisible));
	writer->writeEndElement();

	// save the internal columns, above only the references to them were saved
	d->xEstimationColumn->save(writer);
	d->yEstimationColumn->save(writer);

	// save the internal curves
	d->estimationCurve->save(writer);
	d->rugCurve->save(writer);

	writer->writeEndElement(); // close "KDEPlot" section
}

//! Load from XML
bool KDEPlot::load(XmlStreamReader* reader, bool preview) {
	Q_D(KDEPlot);

	if (!readBasicAttributes(reader))
		return false;

	KLocalizedString attributeWarning = ki18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs;
	QString str;
	bool estimationCurveInitialized = false;
	bool rugCurveInitialized = false;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == QLatin1String("KDEPlot"))
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == QLatin1String("comment")) {
			if (!readCommentElement(reader))
				return false;
		} else if (!preview && reader->name() == QLatin1String("general")) {
			attribs = reader->attributes();
			READ_COLUMN(dataColumn);
			READ_INT_VALUE("kernelType", kernelType, nsl_kernel_type);
			READ_INT_VALUE("bandwidthType", bandwidthType, nsl_kde_bandwidth_type);
			READ_DOUBLE_VALUE("bandwidth", bandwidth);
			READ_INT_VALUE("legendVisible", legendVisible, bool);

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
				rc = d->xEstimationColumn->load(reader, preview);
			else if (name == QLatin1String("yReference"))
				rc = d->yEstimationColumn->load(reader, preview);

			if (!rc)
				return false;
		} else if (reader->name() == QLatin1String("xyCurve")) {
			if (!estimationCurveInitialized) {
				if (d->estimationCurve->load(reader, preview)) {
					estimationCurveInitialized = true;
					continue;
				} else
					return false;
			}

			if (!rugCurveInitialized) {
				if (d->rugCurve->load(reader, preview)) {
					rugCurveInitialized = true;
					continue;
				} else
					return false;
			}
		} else { // unknown element
			reader->raiseUnknownElementWarning();
			if (!reader->skipToEndElement())
				return false;
		}
	}
	return true;
}

// ##############################################################################
// #########################  Theme management ##################################
// ##############################################################################
void KDEPlot::loadThemeConfig(const KConfig& config) {
	KConfigGroup group;
	if (config.hasGroup(QStringLiteral("Theme")))
		group = config.group(QStringLiteral("XYCurve")); // when loading from the theme config, use the same properties as for XYCurve
	else
		group = config.group(QStringLiteral("KDEPlot"));

	const auto* plot = static_cast<const CartesianPlot*>(parentAspect());
	int index = plot->curveChildIndex(this);
	const QColor themeColor = plot->themeColorPalette(index);

	Q_D(KDEPlot);
	d->suppressRecalc = true;

	d->estimationCurve->line()->loadThemeConfig(group, themeColor);
	d->estimationCurve->background()->loadThemeConfig(group, themeColor);
	d->rugCurve->symbol()->loadThemeConfig(group, themeColor);

	d->suppressRecalc = false;
	d->recalcShapeAndBoundingRect();
}

void KDEPlot::saveThemeConfig(const KConfig& config) {
	// Q_D(const KDEPlot);
	KConfigGroup group = config.group(QStringLiteral("KDEPlot"));
	// TODO
}
