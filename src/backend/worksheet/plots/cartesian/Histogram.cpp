/*
	File                 : Histogram.cpp
	Project              : LabPlot
	Description          : Histogram
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2016 Anu Mittal <anu22mittal@gmail.com>
	SPDX-FileCopyrightText: 2016-2024 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2017-2018 Garvit Khatri <garvitdelhi@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

/*!
  \class Histogram
  \brief A 2D-curve, provides an interface for editing many properties of the curve.

  \ingroup worksheet
  */
#include "Histogram.h"
#include "HistogramPrivate.h"
#include "backend/core/AbstractColumn.h"
#include "backend/core/Folder.h"
#include "backend/core/Project.h"
#include "backend/core/Settings.h"
#include "backend/core/column/Column.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/macrosCurve.h"
#include "backend/lib/trace.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/worksheet/Background.h"
#include "backend/worksheet/Line.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/ErrorBar.h"
#include "backend/worksheet/plots/cartesian/Symbol.h"
#include "backend/worksheet/plots/cartesian/Value.h"
#include "tools/ImageTools.h"

#include <QMenu>
#include <QPainter>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <QGraphicsSceneMouseEvent>

#include <backend/worksheet/WorksheetElement.h>

CURVE_COLUMN_CONNECT(Histogram, Data, data, recalc)
static constexpr double zero = std::numeric_limits<double>::epsilon(); // zero baseline, don't use the exact 0.0 since it breaks the histrogram with log-scaling

Histogram::Histogram(const QString& name, bool loading)
	: Plot(name, new HistogramPrivate(this), AspectType::Histogram) {
	init(loading);
}

Histogram::Histogram(const QString& name, HistogramPrivate* dd)
	: Plot(name, dd, AspectType::Histogram) {
	init();
}

// no need to delete the d-pointer here - it inherits from QGraphicsItem
// and is deleted during the cleanup in QGraphicsScene
Histogram::~Histogram() = default;

void Histogram::init(bool loading) {
	Q_D(Histogram);

	// line
	d->line = new Line(QStringLiteral("line"));
	d->line->setHistogramLineTypeAvailable(true);
	d->line->setHidden(true);
	addChild(d->line);
	connect(d->line, &Line::histogramLineTypeChanged, [=] {
		d->updateLines();
	});
	connect(d->line, &Line::updatePixmapRequested, [=] {
		d->updatePixmap();
	});
	connect(d->line, &Line::updateRequested, [=] {
		d->recalcShapeAndBoundingRect();
	});

	// symbol
	d->symbol = new Symbol(QStringLiteral("symbol"));
	addChild(d->symbol);
	d->symbol->setHidden(true);
	connect(d->symbol, &Symbol::updateRequested, [=] {
		d->updateSymbols();
	});
	connect(d->symbol, &Symbol::updatePixmapRequested, [=] {
		d->updatePixmap();
	});

	// values
	d->value = new Value(QString());
	addChild(d->value);
	d->value->setHidden(true);
	d->value->setcenterPositionAvailable(true);
	connect(d->value, &Value::updatePixmapRequested, [=] {
		d->updatePixmap();
	});
	connect(d->value, &Value::updateRequested, [=] {
		d->updateValues();
	});

	// Background/Filling
	d->background = new Background(QStringLiteral("background"));
	d->background->setPrefix(QStringLiteral("Filling"));
	d->background->setEnabledAvailable(true);
	addChild(d->background);
	d->background->setHidden(true);
	connect(d->background, &Background::updateRequested, [=] {
		d->updatePixmap();
	});
	connect(d->background, &Background::updatePositionRequested, [=] {
		d->updateFilling();
	});

	// error bars
	d->errorBar = new ErrorBar(QStringLiteral("errorBar"), ErrorBar::Dimension::Y);
	addChild(d->errorBar);
	d->errorBar->setHidden(true);
	connect(d->errorBar, &ErrorBar::updatePixmapRequested, [=] {
		d->updatePixmap();
	});
	connect(d->errorBar, &ErrorBar::updateRequested, [=] {
		d->updateErrorBars();
	});

	if (loading)
		return;

	// init the properties
	KConfig config;
	KConfigGroup group = config.group(QStringLiteral("Histogram"));

	d->type = (Histogram::Type)group.readEntry(QStringLiteral("Type"), (int)Histogram::Ordinary);
	d->orientation = (Histogram::Orientation)group.readEntry(QStringLiteral("Orientation"), (int)Histogram::Orientation::Vertical);
	d->normalization = (Histogram::Normalization)group.readEntry(QStringLiteral("Normalization"), (int)Histogram::Count);
	d->binningMethod = (Histogram::BinningMethod)group.readEntry(QStringLiteral("BinningMethod"), (int)Histogram::SquareRoot);
	d->binCount = group.readEntry(QStringLiteral("BinCount"), 10);
	d->binWidth = group.readEntry(QStringLiteral("BinWidth"), 1.0);
	d->autoBinRanges = group.readEntry(QStringLiteral("AutoBinRanges"), true);

	d->line->init(group);
	d->symbol->init(group);
	d->value->init(group);
	d->background->init(group);
	d->errorBar->init(group);

	// marginal plots (rug, histogram, boxplot)
	d->rugEnabled = group.readEntry(QStringLiteral("RugEnabled"), false);
	d->rugLength = group.readEntry(QStringLiteral("RugLength"), Worksheet::convertToSceneUnits(5, Worksheet::Unit::Point));
	d->rugWidth = group.readEntry(QStringLiteral("RugWidth"), 0.0);
	d->rugOffset = group.readEntry(QStringLiteral("RugOffset"), 0.0);
	this->initActions();
}

void Histogram::initActions() {
}

/*!
 * creates a new spreadsheet having the data with the positions and the values of the bins.
 * the new spreadsheet is added to the current folder.
 */
void Histogram::createDataSpreadsheet() {
	if (!bins() || !binValues())
		return;

	auto* spreadsheet = new Spreadsheet(i18n("%1 - Data", name()));
	spreadsheet->removeColumns(0, spreadsheet->columnCount()); // remove default columns
	spreadsheet->setRowCount(bins()->rowCount());

	// bin positions
	auto* data = static_cast<const Column*>(bins())->data();
	auto* xColumn = new Column(i18n("bin positions"), *static_cast<QVector<double>*>(data));
	xColumn->setPlotDesignation(AbstractColumn::PlotDesignation::X);
	spreadsheet->addChild(xColumn);

	// y values
	data = static_cast<const Column*>(binValues())->data();
	auto* yColumn = new Column(i18n("bin values"), *static_cast<QVector<double>*>(data));
	yColumn->setPlotDesignation(AbstractColumn::PlotDesignation::Y);
	spreadsheet->addChild(yColumn);

	// add the new spreadsheet to the current folder
	folder()->addChild(spreadsheet);
}

QMenu* Histogram::createContextMenu() {
	Q_D(const Histogram);
	QMenu* menu = WorksheetElement::createContextMenu();
	QAction* visibilityAction = this->visibilityAction();

	//"data analysis" menu
	auto* analysisMenu = new QMenu(i18n("Analysis"));

	// TODO: if there are more actions, add a group for all fit types
	auto* fitGaussianAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-fit-curve")), i18n("Fit Gaussian (Normal) Distribution"));
	analysisMenu->addAction(fitGaussianAction);
	connect(fitGaussianAction, &QAction::triggered, this, [=]() {
		d->m_plot->addHistogramFit(this, nsl_sf_stats_gaussian);
	});

	auto* fitExponentialAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-fit-curve")), i18n("Fit Exponential Distribution"));
	analysisMenu->addAction(fitExponentialAction);
	connect(fitExponentialAction, &QAction::triggered, this, [=]() {
		d->m_plot->addHistogramFit(this, nsl_sf_stats_exponential);
	});

	auto* fitLaplaceAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-fit-curve")), i18n("Fit Laplace Distribution"));
	analysisMenu->addAction(fitLaplaceAction);
	connect(fitLaplaceAction, &QAction::triggered, this, [=]() {
		d->m_plot->addHistogramFit(this, nsl_sf_stats_laplace);
	});

	auto* fitCauchyAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-fit-curve")), i18n("Fit Cauchy-Lorentz Distribution"));
	analysisMenu->addAction(fitCauchyAction);
	connect(fitCauchyAction, &QAction::triggered, this, [=]() {
		d->m_plot->addHistogramFit(this, nsl_sf_stats_cauchy_lorentz);
	});

	auto* fitLognormalAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-fit-curve")), i18n("Fit Log-normal Distribution"));
	analysisMenu->addAction(fitLognormalAction);
	connect(fitLognormalAction, &QAction::triggered, this, [=]() {
		d->m_plot->addHistogramFit(this, nsl_sf_stats_lognormal);
	});

	auto* fitPoissonAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-fit-curve")), i18n("Fit Poisson Distribution"));
	analysisMenu->addAction(fitPoissonAction);
	connect(fitPoissonAction, &QAction::triggered, this, [=]() {
		d->m_plot->addHistogramFit(this, nsl_sf_stats_poisson);
	});

	auto* fitBinomialAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-fit-curve")), i18n("Fit Binomial Distribution"));
	analysisMenu->addAction(fitBinomialAction);
	connect(fitBinomialAction, &QAction::triggered, this, [=]() {
		d->m_plot->addHistogramFit(this, nsl_sf_stats_binomial);
	});

	menu->insertMenu(visibilityAction, analysisMenu);
	menu->insertSeparator(visibilityAction);

	return menu;
}

/*!
  Returns an icon to be used in the project explorer.
  */
QIcon Histogram::icon() const {
	return QIcon::fromTheme(QStringLiteral("view-object-histogram-linear"));
}

// ##############################################################################
// ##########################  getter methods  ##################################
// ##############################################################################
//  general
BASIC_SHARED_D_READER_IMPL(Histogram, Histogram::Type, type, type)
BASIC_SHARED_D_READER_IMPL(Histogram, Histogram::Orientation, orientation, orientation)
BASIC_SHARED_D_READER_IMPL(Histogram, Histogram::Normalization, normalization, normalization)
BASIC_SHARED_D_READER_IMPL(Histogram, Histogram::BinningMethod, binningMethod, binningMethod)
BASIC_SHARED_D_READER_IMPL(Histogram, int, binCount, binCount)
BASIC_SHARED_D_READER_IMPL(Histogram, double, binWidth, binWidth)
BASIC_SHARED_D_READER_IMPL(Histogram, bool, autoBinRanges, autoBinRanges)
BASIC_SHARED_D_READER_IMPL(Histogram, double, binRangesMin, binRangesMin)
BASIC_SHARED_D_READER_IMPL(Histogram, double, binRangesMax, binRangesMax)
BASIC_SHARED_D_READER_IMPL(Histogram, const AbstractColumn*, dataColumn, dataColumn)
BASIC_SHARED_D_READER_IMPL(Histogram, QString, dataColumnPath, dataColumnPath)

// line
Line* Histogram::line() const {
	Q_D(const Histogram);
	return d->line;
}

// symbols
Symbol* Histogram::symbol() const {
	Q_D(const Histogram);
	return d->symbol;
}

// values
Value* Histogram::value() const {
	Q_D(const Histogram);
	return d->value;
}

// filling
Background* Histogram::background() const {
	Q_D(const Histogram);
	return d->background;
}

// error bars
ErrorBar* Histogram::errorBar() const {
	Q_D(const Histogram);
	return d->errorBar;
}

// margin plots
BASIC_SHARED_D_READER_IMPL(Histogram, bool, rugEnabled, rugEnabled)
BASIC_SHARED_D_READER_IMPL(Histogram, double, rugLength, rugLength)
BASIC_SHARED_D_READER_IMPL(Histogram, double, rugWidth, rugWidth)
BASIC_SHARED_D_READER_IMPL(Histogram, double, rugOffset, rugOffset)

double Histogram::minimum(const Dimension dim) const {
	Q_D(const Histogram);
	switch (dim) {
	case Dimension::X:
		return d->xMinimum();
	case Dimension::Y:
		return d->yMinimum();
	}
	return NAN;
}

double Histogram::maximum(const Dimension dim) const {
	Q_D(const Histogram);
	switch (dim) {
	case Dimension::X:
		return d->xMaximum();
	case Dimension::Y:
		return d->yMaximum();
	}
	return NAN;
}

bool Histogram::hasData() const {
	Q_D(const Histogram);
	return (d->dataColumn != nullptr);
}

bool Histogram::usingColumn(const AbstractColumn* column, bool) const {
	Q_D(const Histogram);
	return (
		d->dataColumn == column || (d->errorBar->yErrorType() == ErrorBar::ErrorType::Symmetric && d->errorBar->yPlusColumn() == column)
		|| (d->errorBar->yErrorType() == ErrorBar::ErrorType::Asymmetric && (d->errorBar->yPlusColumn() == column || d->errorBar->yMinusColumn() == column)));
}

void Histogram::handleAspectUpdated(const QString& aspectPath, const AbstractAspect* aspect) {
	Q_D(Histogram);
	const auto column = dynamic_cast<const AbstractColumn*>(aspect);
	if (!column)
		return;

	setUndoAware(false);

	if (d->dataColumn == column) // the column is the same and was just renamed -> update the column path
		d->dataColumnPath = aspectPath;
	else if (d->dataColumnPath == aspectPath) // another column was renamed to the current path -> set and connect to the new column
		setDataColumn(column);

	if (d->value->column() == column)
		d->value->setColumnPath(aspectPath);
	else if (d->value->columnPath() == aspectPath)
		d->value->setColumn(column);

	if (d->errorBar->yPlusColumn() == column)
		d->errorBar->setYPlusColumnPath(aspectPath);
	else if (d->errorBar->yPlusColumnPath() == aspectPath)
		d->errorBar->setYPlusColumn(column);

	if (d->errorBar->yMinusColumn() == column)
		d->errorBar->setYMinusColumnPath(aspectPath);
	else if (d->errorBar->yMinusColumnPath() == aspectPath)
		d->errorBar->setYMinusColumn(column);

	setUndoAware(true);
}

QColor Histogram::color() const {
	Q_D(const Histogram);
	if (d->background->enabled())
		return d->background->firstColor();
	else if (d->line->style() != Qt::PenStyle::NoPen)
		return d->line->pen().color();
	return QColor();
}

const AbstractColumn* Histogram::bins() const {
	D(Histogram);
	return d->bins();
}

const AbstractColumn* Histogram::binValues() const {
	D(Histogram);
	return d->binValues();
}

const AbstractColumn* Histogram::binPDValues() const {
	D(Histogram);
	return d->binPDValues();
}

// ##############################################################################
// #################  setter methods and undo commands ##########################
// ##############################################################################

// General
CURVE_COLUMN_SETTER_CMD_IMPL_F_S(Histogram, Data, data, recalc)
void Histogram::setDataColumn(const AbstractColumn* column) {
	Q_D(Histogram);
	if (column != d->dataColumn)
		exec(new HistogramSetDataColumnCmd(d, column, ki18n("%1: set data column")));
}

void Histogram::setDataColumnPath(const QString& path) {
	Q_D(Histogram);
	d->dataColumnPath = path;
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetType, Histogram::Type, type, updateType)
void Histogram::setType(Histogram::Type type) {
	Q_D(Histogram);
	if (type != d->type)
		exec(new HistogramSetTypeCmd(d, type, ki18n("%1: set histogram type")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetOrientation, Histogram::Orientation, orientation, updateOrientation)
void Histogram::setOrientation(Histogram::Orientation orientation) {
	Q_D(Histogram);
	if (orientation != d->orientation)
		exec(new HistogramSetOrientationCmd(d, orientation, ki18n("%1: set histogram orientation")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetNormalization, Histogram::Normalization, normalization, updateOrientation)
void Histogram::setNormalization(Histogram::Normalization normalization) {
	Q_D(Histogram);
	if (normalization != d->normalization)
		exec(new HistogramSetNormalizationCmd(d, normalization, ki18n("%1: set histogram normalization")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetBinningMethod, Histogram::BinningMethod, binningMethod, recalc)
void Histogram::setBinningMethod(Histogram::BinningMethod method) {
	Q_D(Histogram);
	if (method != d->binningMethod)
		exec(new HistogramSetBinningMethodCmd(d, method, ki18n("%1: set binning method")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetBinCount, int, binCount, recalc)
void Histogram::setBinCount(int count) {
	Q_D(Histogram);
	if (count != d->binCount)
		exec(new HistogramSetBinCountCmd(d, count, ki18n("%1: set bin count")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetBinWidth, double, binWidth, recalc)
void Histogram::setBinWidth(double width) {
	Q_D(Histogram);
	if (width != d->binWidth)
		exec(new HistogramSetBinWidthCmd(d, width, ki18n("%1: set bin width")));
}

class HistogramSetAutoBinRangesCmd : public QUndoCommand {
public:
	HistogramSetAutoBinRangesCmd(HistogramPrivate* private_obj, bool autoBinRanges)
		: m_private(private_obj)
		, m_autoBinRanges(autoBinRanges) {
		setText(i18n("%1: change auto bin ranges", m_private->name()));
	}

	void redo() override {
		m_autoBinRangesOld = m_private->autoBinRanges;
		m_private->autoBinRanges = m_autoBinRanges;
		if (m_autoBinRanges) {
			m_binRangesMinOld = m_private->binRangesMin;
			m_binRangesMaxOld = m_private->binRangesMax;
			m_private->q->recalc();
		}
		Q_EMIT m_private->q->autoBinRangesChanged(m_autoBinRanges);
	}

	void undo() override {
		m_private->autoBinRanges = m_autoBinRangesOld;
		if (!m_autoBinRangesOld) {
			if (m_private->binRangesMin != m_binRangesMinOld) {
				m_private->binRangesMin = m_binRangesMinOld;
				Q_EMIT m_private->q->binRangesMinChanged(m_private->binRangesMin);
			}
			if (m_private->binRangesMax != m_binRangesMaxOld) {
				m_private->binRangesMax = m_binRangesMaxOld;
				Q_EMIT m_private->q->binRangesMaxChanged(m_private->binRangesMax);
			}
			m_private->recalc();
		}
		Q_EMIT m_private->q->autoBinRangesChanged(m_autoBinRangesOld);
	}

private:
	HistogramPrivate* m_private;
	double m_binRangesMinOld{0.0};
	double m_binRangesMaxOld{0.0};
	bool m_autoBinRanges;
	bool m_autoBinRangesOld{false};
};

void Histogram::setAutoBinRanges(bool autoBinRanges) {
	Q_D(Histogram);
	if (autoBinRanges != d->autoBinRanges)
		exec(new HistogramSetAutoBinRangesCmd(d, autoBinRanges));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetBinRangesMin, double, binRangesMin, recalc)
void Histogram::setBinRangesMin(double binRangesMin) {
	Q_D(Histogram);
	if (binRangesMin != d->binRangesMin)
		exec(new HistogramSetBinRangesMinCmd(d, binRangesMin, ki18n("%1: set bin ranges start")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetBinRangesMax, double, binRangesMax, recalc)
void Histogram::setBinRangesMax(double binRangesMax) {
	Q_D(Histogram);
	if (binRangesMax != d->binRangesMax)
		exec(new HistogramSetBinRangesMaxCmd(d, binRangesMax, ki18n("%1: set bin ranges end")));
}

// margin plots
STD_SETTER_CMD_IMPL_F_S(Histogram, SetRugEnabled, bool, rugEnabled, updateRug)
void Histogram::setRugEnabled(bool enabled) {
	Q_D(Histogram);
	if (enabled != d->rugEnabled)
		exec(new HistogramSetRugEnabledCmd(d, enabled, ki18n("%1: change rug enabled")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetRugWidth, double, rugWidth, updatePixmap)
void Histogram::setRugWidth(double width) {
	Q_D(Histogram);
	if (width != d->rugWidth)
		exec(new HistogramSetRugWidthCmd(d, width, ki18n("%1: change rug width")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetRugLength, double, rugLength, updateRug)
void Histogram::setRugLength(double length) {
	Q_D(Histogram);
	if (length != d->rugLength)
		exec(new HistogramSetRugLengthCmd(d, length, ki18n("%1: change rug length")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetRugOffset, double, rugOffset, updateRug)
void Histogram::setRugOffset(double offset) {
	Q_D(Histogram);
	if (offset != d->rugOffset)
		exec(new HistogramSetRugOffsetCmd(d, offset, ki18n("%1: change rug offset")));
}

// ##############################################################################
// #################################  SLOTS  ####################################
// ##############################################################################
void Histogram::retransform() {
	d_ptr->retransform();
}

void Histogram::recalc() {
	D(Histogram);
	d->recalc();
}

// TODO
void Histogram::handleResize(double horizontalRatio, double /*verticalRatio*/, bool /*pageResize*/) {
	Q_D(const Histogram);

	// setValuesDistance(d->distance*);
	QFont font = d->value->font();
	font.setPointSizeF(font.pointSizeF() * horizontalRatio);
	d->value->setFont(font);

	retransform();
}

void Histogram::updateValues() {
	D(Histogram);
	d->updateValues();
}

void Histogram::dataColumnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(Histogram);
	if (aspect == d->dataColumn) {
		d->dataColumn = nullptr;
		d->retransform();
		Q_EMIT dataChanged();
		Q_EMIT changed();
	}
}

void Histogram::updateErrorBars() {
	Q_D(Histogram);
	d->updateErrorBars();
}

// ##############################################################################
// ######################### Private implementation #############################
// ##############################################################################
HistogramPrivate::HistogramPrivate(Histogram* owner)
	: PlotPrivate(owner)
	, q(owner) {
	setFlag(QGraphicsItem::ItemIsSelectable, true);
	setAcceptHoverEvents(false);
}

HistogramPrivate::~HistogramPrivate() {
	if (m_histogram)
		gsl_histogram_free(m_histogram);
}

double HistogramPrivate::getMaximumOccuranceofHistogram() const {
	if (m_histogram) {
		double yMaxRange = -INFINITY;
		switch (type) {
		case Histogram::Ordinary: {
			size_t maxYAddes = gsl_histogram_max_bin(m_histogram);
			yMaxRange = gsl_histogram_get(m_histogram, maxYAddes);
			break;
		}
		case Histogram::Cumulative: {
			size_t maxYAddes = gsl_histogram_max_bin(m_histogram);
			yMaxRange = gsl_histogram_get(m_histogram, maxYAddes);
			double point = 0.0;
			for (size_t i = 0; i < m_bins; ++i) {
				point += gsl_histogram_get(m_histogram, i);
				if (point > yMaxRange) {
					yMaxRange = point;
				}
			}
			// yMaxRange = dataColumn->rowCount();
			break;
		}
		case Histogram::AvgShift: {
			// TODO
		}
		}

		switch (normalization) {
		case Histogram::Count:
			break;
		case Histogram::Probability:
			yMaxRange = yMaxRange / totalCount;
			break;
		case Histogram::CountDensity: {
			const double width = (binRangesMax - binRangesMin) / m_bins;
			yMaxRange = yMaxRange / width;
			break;
		}
		case Histogram::ProbabilityDensity: {
			const double width = (binRangesMax - binRangesMin) / m_bins;
			yMaxRange = yMaxRange / totalCount / width;
			break;
		}
		}

		return yMaxRange;
	}

	return -INFINITY;
}

double HistogramPrivate::xMinimum() const {
	switch (orientation) {
	case Histogram::Orientation::Vertical:
		return autoBinRanges ? dataColumn->minimum() : binRangesMin;
	case Histogram::Orientation::Horizontal:
		return 0.;
	case Histogram::Orientation::Both:
		break;
	}

	return INFINITY;
}

double HistogramPrivate::xMaximum() const {
	switch (orientation) {
	case Histogram::Orientation::Vertical:
		return autoBinRanges ? dataColumn->maximum() : binRangesMax;
	case Histogram::Orientation::Horizontal:
		return getMaximumOccuranceofHistogram();
	case Histogram::Orientation::Both:
		break;
	}

	return -INFINITY;
}

double HistogramPrivate::yMinimum() const {
	switch (orientation) {
	case Histogram::Orientation::Vertical:
		return 0.;
	case Histogram::Orientation::Horizontal:
		return autoBinRanges ? dataColumn->minimum() : binRangesMin;
	case Histogram::Orientation::Both:
		break;
	}

	return INFINITY;
}

double HistogramPrivate::yMaximum() const {
	switch (orientation) {
	case Histogram::Orientation::Vertical:
		return getMaximumOccuranceofHistogram();
	case Histogram::Orientation::Horizontal:
		return autoBinRanges ? dataColumn->maximum() : binRangesMax;
	case Histogram::Orientation::Both:
		break;
	}

	return -INFINITY;
}

const AbstractColumn* HistogramPrivate::bins() {
	if (!m_binsColumn) {
		m_binsColumn = new Column(QStringLiteral("bins"));

		const double width = (binRangesMax - binRangesMin) / m_bins;
		m_binsColumn->resizeTo(m_bins);
		for (size_t i = 0; i < m_bins; ++i) {
			const double x = binRangesMin + i * width;
			m_binsColumn->setValueAt(i, x);
		}
	}

	return m_binsColumn;
}

const AbstractColumn* HistogramPrivate::binValues() {
	if (!m_binValuesColumn) {
		m_binValuesColumn = new Column(QStringLiteral("values"));

		m_binValuesColumn->resizeTo(m_bins);
		double value = 0.;
		for (size_t i = 0; i < m_bins; ++i) {
			histogramValue(value, i);
			m_binValuesColumn->setValueAt(i, value);
		}
	}

	return m_binValuesColumn;
}

/*!
 * returns a column with the bin values in the probability density normalization
 * \return
 */
const AbstractColumn* HistogramPrivate::binPDValues() {
	if (!m_binPDValuesColumn) {
		m_binPDValuesColumn = new Column(QStringLiteral("values"));

		m_binPDValuesColumn->resizeTo(m_bins);
		const double width = (binRangesMax - binRangesMin) / m_bins;
		for (size_t i = 0; i < m_bins; ++i)
			m_binPDValuesColumn->setValueAt(i, gsl_histogram_get(m_histogram, i) / totalCount / width); // probability density normalization
	}

	return m_binPDValuesColumn;
}

/*!
  called when the size of the plot or its data ranges (manual changes, zooming, etc.) were changed.
  recalculates the position of the scene points to be drawn.
  triggers the update of lines, drop lines, symbols etc.
*/
void HistogramPrivate::retransform() {
	const bool suppressed = suppressRetransform || q->isLoading();
	Q_EMIT trackRetransformCalled(suppressed);
	if (suppressed)
		return;

	if (!isVisible())
		return;

	PERFTRACE(name() + QLatin1String(Q_FUNC_INFO));

	if (!dataColumn) {
		linePath = QPainterPath();
		symbolsPath = QPainterPath();
		valuesPath = QPainterPath();
		errorBarsPath = QPainterPath();
		rugPath = QPainterPath();
		m_shape = QPainterPath();
		lines.clear();
		linesUnclipped.clear();
		pointsLogical.clear();
		pointsScene.clear();
		visiblePoints.clear();
		valuesPoints.clear();
		valuesStrings.clear();
		fillPolygon.clear();
		recalcShapeAndBoundingRect();
		return;
	}

	suppressRecalc = true;
	updateLines();
	updateSymbols();
	updateErrorBars();
	updateRug();
	suppressRecalc = false;
	updateValues();
}

/*!
 * called when the data was changed. recalculates the histogram.
 */
void HistogramPrivate::recalc() {
	PERFTRACE(name() + QLatin1String(Q_FUNC_INFO));

	if (m_histogram) {
		gsl_histogram_free(m_histogram);
		m_histogram = nullptr;
	}

	if (!dataColumn)
		return;

	// in case wrong bin range was specified, call retransform() to reset
	// all internal containers and paths and exit this function
	if (binRangesMax <= binRangesMin) {
		retransform();
		return;
	}

	const double xMinOld = xMinimum();
	const double xMaxOld = xMaximum();
	const double yMinOld = yMinimum();
	const double yMaxOld = yMaximum();

	// calculate the number of valid data points
	int count = 0;
	for (int row = 0; row < dataColumn->rowCount(); ++row) {
		if (dataColumn->isValid(row) && !dataColumn->isMasked(row))
			++count;
	}

	// calculate the number of bins
	if (count > 0) {
		if (autoBinRanges) {
			if (binRangesMin != dataColumn->minimum()) {
				binRangesMin = dataColumn->minimum();
				Q_EMIT q->binRangesMinChanged(binRangesMin);
			}

			if (binRangesMax != dataColumn->maximum()) {
				binRangesMax = dataColumn->maximum();
				Q_EMIT q->binRangesMaxChanged(binRangesMax);
			}
		}

		if (binRangesMin >= binRangesMax) {
			Q_EMIT q->dataChanged();
			Q_EMIT q->info(i18n("Calculation of the histogram not possible. The max value must be bigger than the min value."));
			return;
		}

		switch (binningMethod) {
		case Histogram::ByNumber:
			m_bins = (size_t)binCount;
			break;
		case Histogram::ByWidth:
			m_bins = (size_t)(binRangesMax - binRangesMin) / binWidth;
			break;
		case Histogram::SquareRoot:
			m_bins = (size_t)sqrt(count);
			break;
		case Histogram::Rice:
			m_bins = (size_t)2 * cbrt(count);
			break;
		case Histogram::Sturges:
			m_bins = (size_t)1 + log2(count);
			break;
		case Histogram::Doane: {
			const double skewness = static_cast<const Column*>(dataColumn)->statistics().skewness;
			m_bins = (size_t)(1 + log2(count) + log2(1 + abs(skewness) / sqrt((double)6 * (count - 2) / (count + 1) / (count + 3))));
			break;
		}
		case Histogram::Scott: {
			const double sigma = static_cast<const Column*>(dataColumn)->statistics().standardDeviation;
			const double width = 3.5 * sigma / cbrt(count);
			m_bins = (size_t)(binRangesMax - binRangesMin) / width;
			break;
		}
		}

		DEBUG("min " << binRangesMin)
		DEBUG("max " << binRangesMax)
		DEBUG("number of bins " << m_bins)

		// calculate the histogram
		if (m_bins > 0) {
			m_histogram = gsl_histogram_alloc(m_bins);
			gsl_histogram_set_ranges_uniform(m_histogram, binRangesMin, binRangesMax);

			switch (dataColumn->columnMode()) {
			case AbstractColumn::ColumnMode::Double:
			case AbstractColumn::ColumnMode::Integer:
			case AbstractColumn::ColumnMode::BigInt:
				for (int row = 0; row < dataColumn->rowCount(); ++row) {
					if (dataColumn->isValid(row) && !dataColumn->isMasked(row))
						gsl_histogram_increment(m_histogram, dataColumn->valueAt(row));
				}
				break;
			case AbstractColumn::ColumnMode::DateTime:
				for (int row = 0; row < dataColumn->rowCount(); ++row) {
					if (dataColumn->isValid(row) && !dataColumn->isMasked(row))
						gsl_histogram_increment(m_histogram, dataColumn->dateTimeAt(row).toMSecsSinceEpoch());
				}
				break;
			case AbstractColumn::ColumnMode::Text:
			case AbstractColumn::ColumnMode::Month:
			case AbstractColumn::ColumnMode::Day:
				break;
			}

			totalCount = 0;
			for (size_t i = 0; i < m_bins; ++i)
				totalCount += gsl_histogram_get(m_histogram, i);

			// fill the columns for the positions and values of the bins
			if (m_binsColumn) {
				m_binsColumn->resizeTo(m_bins);
				const double width = (binRangesMax - binRangesMin) / m_bins;
				for (size_t i = 0; i < m_bins; ++i)
					m_binsColumn->setValueAt(i, binRangesMin + i * width);
			}

			if (m_binValuesColumn) {
				m_binValuesColumn->resizeTo(m_bins);
				double value = 0.;
				for (size_t i = 0; i < m_bins; ++i) {
					histogramValue(value, i);
					m_binValuesColumn->setValueAt(i, value);
				}
			}

			if (m_binPDValuesColumn) {
				m_binPDValuesColumn->resizeTo(m_bins);
				const double width = (binRangesMax - binRangesMin) / m_bins;
				for (size_t i = 0; i < m_bins; ++i)
					m_binPDValuesColumn->setValueAt(i, gsl_histogram_get(m_histogram, i) / totalCount / width); // probability density normalization
			}
		} else
			DEBUG("Number of bins must be positive integer")
	}

	const double xMin = xMinimum();
	const double xMax = xMaximum();
	const double yMin = yMinimum();
	const double yMax = yMaximum();

	// if the size of the plot has changed because of the actual
	// data changes or because of new plot settings, emit dataChanged()
	// in order to recalculate the data ranges in the parent plot area
	// and to retransform all its children.
	// Just call retransform() to update the plot only if the ranges didn't change.
	if (xMin != xMinOld || xMax != xMaxOld || yMin != yMinOld || yMax != yMaxOld)
		Q_EMIT q->dataChanged();
	else
		retransform();
}

void HistogramPrivate::updateType() {
	// type (ordinary or cumulative) changed,
	// Q_EMIT dataChanged() in order to recalculate everything with the new size/shape of the histogram
	Q_EMIT q->dataChanged();
}

void HistogramPrivate::updateOrientation() {
	// orientation (horizontal or vertical) changed
	// Q_EMIT dataChanged() in order to recalculate everything with the new size/shape of the histogram
	Q_EMIT q->dataChanged();
}

/*!
  recalculates the painter path for the lines connecting the data points.
  Called each time when the type of this connection is changed.
  */
void HistogramPrivate::updateLines() {
	PERFTRACE(name() + QLatin1String(Q_FUNC_INFO));

	linePath = QPainterPath();
	lines.clear();
	linesUnclipped.clear();
	pointsLogical.clear();
	pointsScene.clear();

	if (orientation == Histogram::Orientation::Vertical)
		verticalHistogram();
	else
		horizontalHistogram();

	// map the lines and the symbol positions to the scene coordinates
	linesUnclipped = q->cSystem->mapLogicalToScene(lines, AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);
	lines = q->cSystem->mapLogicalToScene(lines);
	visiblePoints = std::vector<bool>(pointsLogical.count(), false);
	q->cSystem->mapLogicalToScene(pointsLogical, pointsScene, visiblePoints);

	// new line path
	for (const auto& line : lines) {
		linePath.moveTo(line.p1());
		linePath.lineTo(line.p2());
	}

	updateFilling();
	recalcShapeAndBoundingRect();
}

void HistogramPrivate::histogramValue(double& value, int bin) const {
	switch (normalization) {
	case Histogram::Count:
		if (type == Histogram::Ordinary)
			value = gsl_histogram_get(m_histogram, bin);
		else
			value += gsl_histogram_get(m_histogram, bin);
		break;
	case Histogram::Probability:
		if (type == Histogram::Ordinary)
			value = gsl_histogram_get(m_histogram, bin) / totalCount;
		else
			value += gsl_histogram_get(m_histogram, bin) / totalCount;
		break;
	case Histogram::CountDensity: {
		const double width = (binRangesMax - binRangesMin) / m_bins;
		if (type == Histogram::Ordinary)
			value = gsl_histogram_get(m_histogram, bin) / width;
		else
			value += gsl_histogram_get(m_histogram, bin) / width;
		break;
	}
	case Histogram::ProbabilityDensity: {
		const double width = (binRangesMax - binRangesMin) / m_bins;
		if (type == Histogram::Ordinary)
			value = gsl_histogram_get(m_histogram, bin) / totalCount / width;
		else
			value += gsl_histogram_get(m_histogram, bin) / totalCount / width;
		break;
	}
	}

	if (value == 0.0)
		value = zero;
}

void HistogramPrivate::verticalHistogram() {
	if (!m_histogram)
		return;

	const double width = (binRangesMax - binRangesMin) / m_bins;
	double value = 0.;
	const auto lineType = line->histogramLineType();
	switch (lineType) {
	case Histogram::Bars: {
		for (size_t i = 0; i < m_bins; ++i) {
			histogramValue(value, i);
			const double x = binRangesMin + i * width;
			lines.append(QLineF(x, zero, x, value));
			lines.append(QLineF(x, value, x + width, value));
			lines.append(QLineF(x + width, value, x + width, zero));
			pointsLogical.append(QPointF(x + width / 2, value));
		}
		break;
	}
	case Histogram::NoLine:
	case Histogram::Envelope: {
		double prevValue = 0.;
		for (size_t i = 0; i < m_bins; ++i) {
			histogramValue(value, i);
			const double x = binRangesMin + i * width;
			lines.append(QLineF(x, prevValue, x, value));
			lines.append(QLineF(x, value, x + width, value));
			pointsLogical.append(QPointF(x + width / 2, value));

			if (i == m_bins - 1)
				lines.append(QLineF(x + width, value, x + width, zero));

			prevValue = value;
		}
		break;
	}
	case Histogram::DropLines: {
		for (size_t i = 0; i < m_bins; ++i) {
			histogramValue(value, i);
			const double x = binRangesMin + i * width + width / 2;
			lines.append(QLineF(x, zero, x, value));
			pointsLogical.append(QPointF(x, value));
		}
		break;
	}
	case Histogram::HalfBars: {
		for (size_t i = 0; i < m_bins; ++i) {
			histogramValue(value, i);
			const double x = binRangesMin + i * width + width / 2;
			lines.append(QLineF(x, zero, x, value));
			lines.append(QLineF(x, value, x - width / 4, value));
			pointsLogical.append(QPointF(x, value));
		}
		break;
	}
	}

	if (lineType != Histogram::DropLines && lineType != Histogram::HalfBars)
		lines.append(QLineF(binRangesMax, zero, binRangesMin, zero));
}

void HistogramPrivate::horizontalHistogram() {
	if (!m_histogram)
		return;

	const double width = (binRangesMax - binRangesMin) / m_bins;
	double value = 0.;
	const auto lineType = line->histogramLineType();
	switch (lineType) {
	case Histogram::Bars: {
		for (size_t i = 0; i < m_bins; ++i) {
			histogramValue(value, i);
			const double y = binRangesMin + i * width;
			lines.append(QLineF(zero, y, value, y));
			lines.append(QLineF(value, y, value, y + width));
			lines.append(QLineF(value, y + width, zero, y + width));
			pointsLogical.append(QPointF(value, y + width / 2));
		}
		break;
	}
	case Histogram::NoLine:
	case Histogram::Envelope: {
		double prevValue = 0.;
		for (size_t i = 0; i < m_bins; ++i) {
			histogramValue(value, i);
			const double y = binRangesMin + i * width;
			lines.append(QLineF(prevValue, y, value, y));
			lines.append(QLineF(value, y, value, y + width));
			pointsLogical.append(QPointF(value, y + width / 2));

			if (i == m_bins - 1)
				lines.append(QLineF(value, y + width, zero, y + width));

			prevValue = value;
		}
		break;
	}
	case Histogram::DropLines: {
		for (size_t i = 0; i < m_bins; ++i) {
			histogramValue(value, i);
			const double y = binRangesMin + i * width + width / 2;
			lines.append(QLineF(zero, y, value, y));
			pointsLogical.append(QPointF(value, y));
		}
		break;
	}
	case Histogram::HalfBars: {
		for (size_t i = 0; i < m_bins; ++i) {
			histogramValue(value, i);
			const double y = binRangesMin + i * width + width / 2;
			lines.append(QLineF(zero, y, value, y));
			lines.append(QLineF(value, y, value, y + width / 4));
			pointsLogical.append(QPointF(value, y));
		}
		break;
	}
	}

	if (lineType != Histogram::DropLines && lineType != Histogram::HalfBars)
		lines.append(QLineF(zero, binRangesMin, zero, binRangesMax));
}

void HistogramPrivate::updateSymbols() {
	symbolsPath = QPainterPath();
	if (symbol->style() != Symbol::Style::NoSymbols) {
		auto path = WorksheetElement::shapeFromPath(Symbol::stylePath(symbol->style()), symbol->pen());

		QTransform trafo;
		trafo.scale(symbol->size(), symbol->size());
		path = trafo.map(path);
		trafo.reset();

		if (symbol->rotationAngle() != 0.) {
			trafo.rotate(symbol->rotationAngle());
			path = trafo.map(path);
		}

		for (const auto& point : pointsScene) {
			trafo.reset();
			trafo.translate(point.x(), point.y());
			symbolsPath.addPath(trafo.map(path));
		}
	}

	recalcShapeAndBoundingRect();
}

/*!
  recreates the value strings to be shown and recalculates their draw position.
  */
void HistogramPrivate::updateValues() {
	valuesPath = QPainterPath();
	valuesPoints.clear();
	valuesStrings.clear();

	if (value->type() == Value::NoValues || !m_histogram) {
		recalcShapeAndBoundingRect();
		return;
	}

	// determine the value string for all points that are currently visible in the plot
	const auto& valuesPrefix = value->prefix();
	const auto& valuesSuffix = value->suffix();
	if (value->type() == Value::BinEntries) {
		switch (type) {
		case Histogram::Ordinary:
			for (size_t i = 0; i < m_bins; ++i) {
				if (!visiblePoints[i])
					continue;
				valuesStrings << valuesPrefix + QString::number(gsl_histogram_get(m_histogram, i)) + valuesSuffix;
			}
			break;
		case Histogram::Cumulative: {
			int value = 0;
			for (size_t i = 0; i < m_bins; ++i) {
				if (!visiblePoints[i])
					continue;
				value += gsl_histogram_get(m_histogram, i);
				valuesStrings << valuesPrefix + QString::number(value) + valuesSuffix;
			}
			break;
		}
		case Histogram::AvgShift:
			break;
		}
	} else if (value->type() == Value::CustomColumn) {
		const auto* valuesColumn = value->column();
		if (!valuesColumn) {
			recalcShapeAndBoundingRect();
			return;
		}

		const int endRow = std::min(pointsLogical.size(), static_cast<qsizetype>(valuesColumn->rowCount()));
		const auto xColMode = valuesColumn->columnMode();
		for (int i = 0; i < endRow; ++i) {
			if (!visiblePoints.at(i))
				continue;

			if (!valuesColumn->isValid(i) || valuesColumn->isMasked(i))
				continue;

			switch (xColMode) {
			case AbstractColumn::ColumnMode::Double:
				valuesStrings << valuesPrefix + QString::number(valuesColumn->valueAt(i), value->numericFormat(), value->precision()) + valuesSuffix;
				break;
			case AbstractColumn::ColumnMode::Integer:
			case AbstractColumn::ColumnMode::BigInt:
				valuesStrings << valuesPrefix + QString::number(valuesColumn->valueAt(i)) + valuesSuffix;
				break;
			case AbstractColumn::ColumnMode::Text:
				valuesStrings << valuesPrefix + valuesColumn->textAt(i) + valuesSuffix;
				break;
			case AbstractColumn::ColumnMode::DateTime:
			case AbstractColumn::ColumnMode::Month:
			case AbstractColumn::ColumnMode::Day:
				valuesStrings << valuesPrefix + valuesColumn->dateTimeAt(i).toString(value->dateTimeFormat()) + valuesSuffix;
				break;
			}
		}
	}

	// Calculate the coordinates where to paint the value strings.
	// The coordinates depend on the actual size of the string.
	QPointF tempPoint;
	QFontMetrics fm(value->font());
	qreal w;
	const qreal h = fm.ascent();
	int valuesDistance = value->distance();
	switch (value->position()) {
	case Value::Above:
		for (int i = 0; i < valuesStrings.size(); i++) {
			w = fm.boundingRect(valuesStrings.at(i)).width();
			tempPoint.setX(pointsScene.at(i).x() - w / 2);
			tempPoint.setY(pointsScene.at(i).y() - valuesDistance);
			valuesPoints.append(tempPoint);
		}
		break;
	case Value::Under:
		for (int i = 0; i < valuesStrings.size(); i++) {
			w = fm.boundingRect(valuesStrings.at(i)).width();
			tempPoint.setX(pointsScene.at(i).x() - w / 2);
			tempPoint.setY(pointsScene.at(i).y() + valuesDistance + h / 2);
			valuesPoints.append(tempPoint);
		}
		break;
	case Value::Left:
		for (int i = 0; i < valuesStrings.size(); i++) {
			w = fm.boundingRect(valuesStrings.at(i)).width();
			tempPoint.setX(pointsScene.at(i).x() - valuesDistance - w - 1);
			tempPoint.setY(pointsScene.at(i).y());
			valuesPoints.append(tempPoint);
		}
		break;
	case Value::Right:
		for (int i = 0; i < valuesStrings.size(); i++) {
			tempPoint.setX(pointsScene.at(i).x() + valuesDistance - 1);
			tempPoint.setY(pointsScene.at(i).y());
			valuesPoints.append(tempPoint);
		}
		break;
	case Value::Center: {
		QVector<qreal> listBarWidth;
		for (int i = 0, j = 0; i < linesUnclipped.size(); i += 3, j++) {
			auto& columnBarLines = linesUnclipped.at(i);
			if ((int)(visiblePoints.size()) == j)
				break;
			if (visiblePoints.at(j) == true)
				listBarWidth.append(columnBarLines.length());
		}
		if (orientation == Histogram::Orientation::Vertical)
			for (int i = 0; i < valuesStrings.size(); i++) {
				w = fm.boundingRect(valuesStrings.at(i)).width();
				tempPoint.setX(pointsScene.at(i).x() - w / 2);
				tempPoint.setY(pointsScene.at(i).y() + listBarWidth.at(i) / 2 - valuesDistance + h / 2 + w / 2);
				valuesPoints.append(tempPoint);
			}
		else {
			for (int i = 0; i < valuesStrings.size(); i++) {
				w = fm.boundingRect(valuesStrings.at(i)).width();
				tempPoint.setX(pointsScene.at(i).x() - listBarWidth.at(i) / 2 - valuesDistance + h / 2 - w / 2 - 2);
				tempPoint.setY(pointsScene.at(i).y() + w / 2);
				valuesPoints.append(tempPoint);
			}
		}
		break;
	}
	}

	QTransform trafo;
	QPainterPath path;
	double valuesRotationAngle = value->rotationAngle();
	for (int i = 0; i < valuesPoints.size(); i++) {
		path = QPainterPath();
		path.addText(QPoint(0, 0), value->font(), valuesStrings.at(i));

		trafo.reset();
		trafo.translate(valuesPoints.at(i).x(), valuesPoints.at(i).y());
		if (valuesRotationAngle != 0.)
			trafo.rotate(-valuesRotationAngle);

		valuesPath.addPath(trafo.map(path));
	}

	recalcShapeAndBoundingRect();
}

void HistogramPrivate::updateFilling() {
	fillPolygon.clear();

	const auto lineType = line->histogramLineType();
	if (!background->enabled() || lineType == Histogram::DropLines || lineType == Histogram::HalfBars) {
		recalcShapeAndBoundingRect();
		return;
	}

	const auto& lines = linesUnclipped;
	if (lines.isEmpty())
		return;

	// clip the line points to the plot data rect and create a new polygon
	// out of them that will be filled out.
	const QRectF& dataRect = static_cast<CartesianPlot*>(q->parentAspect())->dataRect();
	int i = 0;
	for (const auto& line : lines) {
		// clip the first point of the line
		QPointF p1 = line.p1();
		if (p1.x() < dataRect.left())
			p1.setX(dataRect.left());
		else if (p1.x() > dataRect.right())
			p1.setX(dataRect.right());

		if (p1.y() < dataRect.top())
			p1.setY(dataRect.top());
		else if (p1.y() > dataRect.bottom())
			p1.setY(dataRect.bottom());

		// clip the second point of the line
		QPointF p2 = line.p2();
		if (p2.x() < dataRect.left())
			p2.setX(dataRect.left());
		else if (p2.x() > dataRect.right())
			p2.setX(dataRect.right());

		if (p2.y() < dataRect.top())
			p2.setY(dataRect.top());
		else if (p2.y() > dataRect.bottom())
			p2.setY(dataRect.bottom());

		if (i != lines.size() - 1)
			fillPolygon << p1;
		else {
			// close the polygon for the last line,
			// take care of the different order for different orientations
			if (orientation == Histogram::Orientation::Vertical) {
				fillPolygon << p1;
				fillPolygon << p2;
			} else {
				fillPolygon << p2;
				fillPolygon << p1;
			}
		}

		++i;
	}

	recalcShapeAndBoundingRect();
}

void HistogramPrivate::updateErrorBars() {
	errorBarsPath = errorBar->painterPath(pointsLogical, q->cSystem, orientation);
	recalcShapeAndBoundingRect();
}

void HistogramPrivate::updateRug() {
	rugPath = QPainterPath();

	if (!rugEnabled || !q->plot()) {
		recalcShapeAndBoundingRect();
		return;
	}

	QVector<QPointF> points;
	auto cs = q->plot()->coordinateSystem(q->coordinateSystemIndex());
	const double xMin = q->plot()->range(Dimension::X, cs->index(Dimension::X)).start();
	const double yMin = q->plot()->range(Dimension::Y, cs->index(Dimension::Y)).start();

	if (orientation == Histogram::Orientation::Vertical) {
		for (int row = 0; row < dataColumn->rowCount(); ++row) {
			if (dataColumn->isValid(row) && !dataColumn->isMasked(row))
				points << QPointF(dataColumn->valueAt(row), yMin);
		}

		// map the points to scene coordinates
		points = q->cSystem->mapLogicalToScene(points);

		// path for the vertical rug lines
		for (const auto& point : std::as_const(points)) {
			rugPath.moveTo(point.x(), point.y() - rugOffset);
			rugPath.lineTo(point.x(), point.y() - rugOffset - rugLength);
		}
	} else {
		for (int row = 0; row < dataColumn->rowCount(); ++row) {
			if (dataColumn->isValid(row) && !dataColumn->isMasked(row))
				points << QPointF(xMin, dataColumn->valueAt(row));
		}

		// map the points to scene coordinates
		points = q->cSystem->mapLogicalToScene(points);

		// path for the horizontal rug lines
		for (const auto& point : std::as_const(points)) {
			rugPath.moveTo(point.x() + rugOffset, point.y());
			rugPath.lineTo(point.x() + rugOffset + rugLength, point.y());
		}
	}

	recalcShapeAndBoundingRect();
}

/*!
  recalculates the outer bounds and the shape of the curve.
  */
void HistogramPrivate::recalcShapeAndBoundingRect() {
	if (suppressRecalc)
		return;

	prepareGeometryChange();
	m_shape = QPainterPath();
	if (line->histogramLineType() != Histogram::NoLine)
		m_shape.addPath(WorksheetElement::shapeFromPath(linePath, line->pen()));

	if (symbol->style() != Symbol::Style::NoSymbols)
		m_shape.addPath(symbolsPath);

	if (value->type() != Value::NoValues)
		m_shape.addPath(valuesPath);

	if (errorBar->yErrorType() != ErrorBar::ErrorType::NoError)
		m_shape.addPath(WorksheetElement::shapeFromPath(errorBarsPath, errorBar->line()->pen()));

	m_shape.addPath(rugPath);
	m_shape.addPolygon(fillPolygon);

	m_boundingRectangle = m_shape.boundingRect();
	m_boundingRectangle = m_boundingRectangle.united(fillPolygon.boundingRect());

	// TODO: when the selection is painted, line intersections are visible.
	// simplified() removes those artifacts but is horrible slow for curves with large number of points.
	// search for an alternative.
	// m_shape = m_shape.simplified();

	updatePixmap();
}

void HistogramPrivate::draw(QPainter* painter) {
	PERFTRACE(name() + QLatin1String(Q_FUNC_INFO));

	// drawing line
	if (line->histogramLineType() != Histogram::NoLine) {
		painter->setOpacity(line->opacity());
		painter->setPen(line->pen());
		painter->setBrush(Qt::NoBrush);
		painter->drawPath(linePath);
	}

	// draw filling
	if (background->enabled())
		background->draw(painter, fillPolygon);

	// draw symbols
	symbol->draw(painter, pointsScene);

	// draw values
	value->draw(painter, valuesPoints, valuesStrings);

	// draw error bars
	if (errorBar->yErrorType() != ErrorBar::ErrorType::NoError)
		errorBar->draw(painter, errorBarsPath);

	// draw rug
	if (rugEnabled) {
		QPen pen;
		pen.setColor(line->pen().color());
		pen.setWidthF(rugWidth);
		painter->setPen(pen);
		painter->setOpacity(line->opacity());
		painter->drawPath(rugPath);
	}
}

void HistogramPrivate::updatePixmap() {
	m_pixmap = QPixmap(m_boundingRectangle.width(), m_boundingRectangle.height());
	if (m_boundingRectangle.width() == 0. || m_boundingRectangle.height() == 0.) {
		m_hoverEffectImageIsDirty = true;
		m_selectionEffectImageIsDirty = true;
		return;
	}
	m_pixmap.fill(Qt::transparent);
	QPainter painter(&m_pixmap);
	painter.setRenderHint(QPainter::Antialiasing, true);
	painter.translate(-m_boundingRectangle.topLeft());

	draw(&painter);
	painter.end();

	m_hoverEffectImageIsDirty = true;
	m_selectionEffectImageIsDirty = true;
	Q_EMIT q->changed();
	update();
}

/*!
  Reimplementation of QGraphicsItem::paint(). This function does the actual painting of the curve.
  \sa QGraphicsItem::paint().
  */
void HistogramPrivate::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget*) {
	if (!isVisible())
		return;

	painter->setPen(Qt::NoPen);
	painter->setBrush(Qt::NoBrush);
	painter->setRenderHint(QPainter::SmoothPixmapTransform, true);

	if (!q->isPrinting() && Settings::group(QStringLiteral("Settings_Worksheet")).readEntry<bool>("DoubleBuffering", true))
		painter->drawPixmap(m_boundingRectangle.topLeft(), m_pixmap); // draw the cached pixmap (fast)
	else
		draw(painter); // draw directly again (slow)

	if (m_hovered && !isSelected() && !q->isPrinting()) {
		if (m_hoverEffectImageIsDirty) {
			QPixmap pix = m_pixmap;
			QPainter p(&pix);
			p.setCompositionMode(QPainter::CompositionMode_SourceIn); // source (shadow) pixels merged with the alpha channel of the destination (m_pixmap)
			p.fillRect(pix.rect(), QApplication::palette().color(QPalette::Shadow));
			p.end();

			m_hoverEffectImage = ImageTools::blurred(pix.toImage(), m_pixmap.rect(), 5);
			m_hoverEffectImageIsDirty = false;
		}

		painter->drawImage(m_boundingRectangle.topLeft(), m_hoverEffectImage, m_pixmap.rect());
		return;
	}

	if (isSelected() && !q->isPrinting()) {
		if (m_selectionEffectImageIsDirty) {
			QPixmap pix = m_pixmap;
			QPainter p(&pix);
			p.setCompositionMode(QPainter::CompositionMode_SourceIn);
			p.fillRect(pix.rect(), QApplication::palette().color(QPalette::Highlight));
			p.end();

			m_selectionEffectImage = ImageTools::blurred(pix.toImage(), m_pixmap.rect(), 5);
			m_selectionEffectImageIsDirty = false;
		}

		painter->drawImage(m_boundingRectangle.topLeft(), m_selectionEffectImage, m_pixmap.rect());
		return;
	}
}

/*!
 * checks if the mousePress event was done near the histogram shape
 * and selects the graphics item if it is the case.
 * \p event
 */
void HistogramPrivate::mousePressEvent(QGraphicsSceneMouseEvent* event) {
	if (static_cast<const CartesianPlot*>(q->parentAspect())->mouseMode() != CartesianPlot::MouseMode::Selection) {
		event->ignore();
		return QGraphicsItem::mousePressEvent(event);
	}

	if (q->activatePlot(event->pos())) {
		setSelected(true);
		return;
	}

	event->ignore();
	setSelected(false);
	QGraphicsItem::mousePressEvent(event);
}

// ##############################################################################
// ##################  Serialization/Deserialization  ###########################
// ##############################################################################
//! Save as XML
void Histogram::save(QXmlStreamWriter* writer) const {
	Q_D(const Histogram);

	writer->writeStartElement(QStringLiteral("Histogram"));
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	// general
	writer->writeStartElement(QStringLiteral("general"));
	WRITE_COLUMN(d->dataColumn, dataColumn);
	writer->writeAttribute(QStringLiteral("type"), QString::number(d->type));
	writer->writeAttribute(QStringLiteral("orientation"), QString::number(static_cast<int>(d->orientation)));
	writer->writeAttribute(QStringLiteral("normalization"), QString::number(d->normalization));
	writer->writeAttribute(QStringLiteral("binningMethod"), QString::number(d->binningMethod));
	writer->writeAttribute(QStringLiteral("binCount"), QString::number(d->binCount));
	writer->writeAttribute(QStringLiteral("binWidth"), QString::number(d->binWidth));
	writer->writeAttribute(QStringLiteral("autoBinRanges"), QString::number(d->autoBinRanges));
	writer->writeAttribute(QStringLiteral("binRangesMin"), QString::number(d->binRangesMin));
	writer->writeAttribute(QStringLiteral("binRangesMax"), QString::number(d->binRangesMax));
	writer->writeAttribute(QStringLiteral("plotRangeIndex"), QString::number(m_cSystemIndex));
	writer->writeAttribute(QStringLiteral("legendVisible"), QString::number(d->legendVisible));
	writer->writeAttribute(QStringLiteral("visible"), QString::number(d->isVisible()));
	writer->writeEndElement();

	d->background->save(writer);
	d->line->save(writer);
	d->symbol->save(writer);
	d->value->save(writer);

	// Error bars
	writer->writeStartElement(QStringLiteral("errorBars"));
	d->errorBar->save(writer);
	writer->writeEndElement();

	// margin plots
	writer->writeStartElement(QStringLiteral("margins"));
	writer->writeAttribute(QStringLiteral("rugEnabled"), QString::number(d->rugEnabled));
	writer->writeAttribute(QStringLiteral("rugLength"), QString::number(d->rugLength));
	writer->writeAttribute(QStringLiteral("rugWidth"), QString::number(d->rugWidth));
	writer->writeAttribute(QStringLiteral("rugOffset"), QString::number(d->rugOffset));
	writer->writeEndElement();

	writer->writeEndElement(); // close "Histogram" section
}

//! Load from XML
bool Histogram::load(XmlStreamReader* reader, bool preview) {
	Q_D(Histogram);

	if (!readBasicAttributes(reader))
		return false;

	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == QLatin1String("Histogram"))
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == QLatin1String("comment")) {
			if (!readCommentElement(reader))
				return false;
		} else if (!preview && reader->name() == QLatin1String("general")) {
			attribs = reader->attributes();

			READ_COLUMN(dataColumn);
			READ_INT_VALUE("type", type, Histogram::Type);
			READ_INT_VALUE("orientation", orientation, Histogram::Orientation);
			READ_INT_VALUE("normalization", normalization, Histogram::Normalization);
			READ_INT_VALUE("binningMethod", binningMethod, Histogram::BinningMethod);
			READ_INT_VALUE("binCount", binCount, int);
			READ_DOUBLE_VALUE("binWidth", binWidth);
			READ_INT_VALUE("autoBinRanges", autoBinRanges, bool);
			READ_DOUBLE_VALUE("binRangesMin", binRangesMin);
			READ_DOUBLE_VALUE("binRangesMax", binRangesMax);
			READ_INT_VALUE_DIRECT("plotRangeIndex", m_cSystemIndex, int);
			READ_INT_VALUE("legendVisible", legendVisible, bool);

			str = attribs.value(QStringLiteral("visible")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("visible"));
			else
				d->setVisible(str.toInt());

			// prior to XML version 12, Histogram used its own enum for the orientation instead of the enum in WorksheetElement
			// which had a different order of values ("{Vertical, Horizontal}" in Histogram vs. "{Horizontal, Vertical}" in WorksheetElement)
			// and we need to map from the old to the new values
			if (Project::xmlVersion() < 12) {
				const int orientation = static_cast<int>(d->orientation);
				if (orientation == 0)
					d->orientation = Orientation::Vertical;
				else if (orientation == 1)
					d->orientation = Orientation::Horizontal;
			}
		} else if (!preview && reader->name() == QLatin1String("line")) {
			d->line->load(reader, preview);
		} else if (!preview && reader->name() == QLatin1String("symbols"))
			d->symbol->load(reader, preview);
		else if (!preview && reader->name() == QLatin1String("values"))
			d->value->load(reader, preview);
		else if (!preview && reader->name() == QLatin1String("filling"))
			d->background->load(reader, preview);
		else if (reader->name() == QLatin1String("errorBars")) {
			d->errorBar->load(reader, preview);

			// prior to XML version 11, a different order of enum values for the error type was used in Histogram
			// (old "{ NoError, Poisson, CustomSymmetric, CustomAsymmetric }" instead of
			// the new "{ NoError, Symmetric, Asymmetric, Poisson }")
			// and we need to map from the old to the new values
			if (Project::xmlVersion() < 11) {
				const int errorType = static_cast<int>(d->errorBar->yErrorType());
				if (errorType == 1)
					d->errorBar->setYErrorType(ErrorBar::ErrorType::Poisson);
				else if (errorType == 2)
					d->errorBar->setYErrorType(ErrorBar::ErrorType::Symmetric);
				else if (errorType == 3)
					d->errorBar->setYErrorType(ErrorBar::ErrorType::Asymmetric);
			}
		} else if (!preview && reader->name() == QLatin1String("margins")) {
			attribs = reader->attributes();

			READ_INT_VALUE("rugEnabled", rugEnabled, bool);
			READ_DOUBLE_VALUE("rugLength", rugLength);
			READ_DOUBLE_VALUE("rugWidth", rugWidth);
			READ_DOUBLE_VALUE("rugOffset", rugOffset);
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
void Histogram::loadThemeConfig(const KConfig& config) {
	KConfigGroup group;
	if (config.hasGroup(QStringLiteral("Theme")))
		group = config.group(QStringLiteral("XYCurve")); // when loading from the theme config, use the same properties as for XYCurve
	else
		group = config.group(QStringLiteral("Histogram"));

	Q_D(Histogram);
	const auto* plot = d->m_plot;
	int index = plot->curveChildIndex(this);
	const QColor themeColor = plot->themeColorPalette(index);

	QPen p;
	d->suppressRecalc = true;

	d->line->loadThemeConfig(group, themeColor);
	d->symbol->loadThemeConfig(group, themeColor);
	d->value->loadThemeConfig(group, themeColor);
	d->background->loadThemeConfig(group, themeColor);
	d->errorBar->loadThemeConfig(group, themeColor);

	if (plot->theme() == QLatin1String("Tufte")) {
		d->line->setHistogramLineType(Histogram::LineType::HalfBars);
		if (d->dataColumn && d->dataColumn->rowCount() < 100)
			setRugEnabled(true);
	} else
		setRugEnabled(false);

	d->suppressRecalc = false;
	d->recalcShapeAndBoundingRect();
}

void Histogram::saveThemeConfig(const KConfig& config) {
	Q_D(const Histogram);
	KConfigGroup group = config.group(QStringLiteral("Histogram"));

	d->line->saveThemeConfig(group);
	d->symbol->saveThemeConfig(group);
	d->value->saveThemeConfig(group);
	d->background->saveThemeConfig(group);
	d->errorBar->saveThemeConfig(group);

	int index = parentAspect()->indexOfChild<Histogram>(this);
	if (index < 5) {
		KConfigGroup themeGroup = config.group(QStringLiteral("Theme"));
		for (int i = index; i < 5; i++) {
			QString s = QStringLiteral("ThemePaletteColor") + QString::number(i + 1);
			themeGroup.writeEntry(s, d->line->pen().color());
		}
	}
}
