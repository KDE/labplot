/*
    File                 : Histogram.cpp
    Project              : LabPlot
    Description          : Histogram
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016 Anu Mittal <anu22mittal@gmail.com>
    SPDX-FileCopyrightText: 2016-2021 Alexander Semke <alexander.semke@web.de>
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
#include "backend/core/column/Column.h"
#include "backend/worksheet/plots/cartesian/CartesianCoordinateSystem.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/Symbol.h"
#include "backend/lib/commandtemplates.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/lib/XmlStreamReader.h"
#include "tools/ImageTools.h"
#include "backend/lib/trace.h"

#include <QPainter>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>

#include <KConfig>
#include <KConfigGroup>
#include <KSharedConfig>
#include <KLocalizedString>

extern "C" {
#include <gsl/gsl_histogram.h>
#include <gsl/gsl_spline.h>
#include <gsl/gsl_errno.h>
}

Histogram::Histogram(const QString &name)
	: WorksheetElement(name, new HistogramPrivate(this),
	  AspectType::Histogram), Curve() {

	init();
}

Histogram::Histogram(const QString &name, HistogramPrivate *dd)
	: WorksheetElement(name, dd, AspectType::Histogram), Curve(){

	init();
}

void Histogram::init() {
	Q_D(Histogram);

	KConfig config;
	KConfigGroup group = config.group("Histogram");

	d->dataColumn = nullptr;

	d->type = (Histogram::HistogramType) group.readEntry("Type", (int)Histogram::Ordinary);
	d->orientation = (Histogram::HistogramOrientation) group.readEntry("Orientation", (int)Histogram::Vertical);
	d->normalization = (Histogram::HistogramNormalization) group.readEntry("Normalization", (int)Histogram::Count);
	d->binningMethod = (Histogram::BinningMethod) group.readEntry("BinningMethod", (int)Histogram::SquareRoot);
	d->binCount = group.readEntry("BinCount", 10);
	d->binWidth = group.readEntry("BinWidth", 1.0f);
	d->autoBinRanges = group.readEntry("AutoBinRanges", true);
	d->binRangesMin = 0.0;
	d->binRangesMax = 1.0;

	d->lineType = (Histogram::LineType) group.readEntry("LineType", (int)Histogram::Bars);
	d->linePen.setStyle( (Qt::PenStyle) group.readEntry("LineStyle", (int)Qt::SolidLine) );
	d->linePen.setColor( group.readEntry("LineColor", QColor(Qt::black)) );
	d->linePen.setWidthF( group.readEntry("LineWidth", Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Point)) );
	d->lineOpacity = group.readEntry("LineOpacity", 1.0);

	//initialize the symbol
	d->symbol = new Symbol(QString());
	addChild(d->symbol);
	d->symbol->setHidden(true);
	d->symbol->init(group);
	connect(d->symbol, &Symbol::updateRequested, [=]{d->updateSymbols();});
	connect(d->symbol, &Symbol::updatePixmapRequested, [=]{d->updatePixmap();});

	d->valuesType = (Histogram::ValuesType) group.readEntry("ValuesType", (int)Histogram::NoValues);
	d->valuesColumn = nullptr;
	d->valuesPosition = (Histogram::ValuesPosition) group.readEntry("ValuesPosition", (int)Histogram::ValuesAbove);
	d->valuesDistance = group.readEntry("ValuesDistance", Worksheet::convertToSceneUnits(5, Worksheet::Unit::Point));
	d->valuesRotationAngle = group.readEntry("ValuesRotation", 0.0);
	d->valuesOpacity = group.readEntry("ValuesOpacity", 1.0);
	d->valuesNumericFormat = group.readEntry("ValuesNumericFormat", "f").at(0).toLatin1();
	d->valuesPrecision = group.readEntry("ValuesNumericFormat", 2);
	d->valuesDateTimeFormat = group.readEntry("ValuesDateTimeFormat", "yyyy-MM-dd");
	d->valuesPrefix = group.readEntry("ValuesPrefix", "");
	d->valuesSuffix = group.readEntry("ValuesSuffix", "");
	d->valuesFont = group.readEntry("ValuesFont", QFont());
	d->valuesFont.setPixelSize( Worksheet::convertToSceneUnits( 8, Worksheet::Unit::Point ) );
	d->valuesColor = group.readEntry("ValuesColor", QColor(Qt::black));

	d->fillingEnabled = group.readEntry("FillingEnabled", true);
	d->fillingType = (WorksheetElement::BackgroundType) group.readEntry("FillingType", static_cast<int>(WorksheetElement::BackgroundType::Color));
	d->fillingColorStyle = (WorksheetElement::BackgroundColorStyle) group.readEntry("FillingColorStyle", static_cast<int>(WorksheetElement::BackgroundColorStyle::SingleColor));
	d->fillingImageStyle = (WorksheetElement::BackgroundImageStyle) group.readEntry("FillingImageStyle", static_cast<int>(WorksheetElement::BackgroundImageStyle::Scaled));
	d->fillingBrushStyle = (Qt::BrushStyle) group.readEntry("FillingBrushStyle", static_cast<int>(Qt::SolidPattern));
	d->fillingFileName = group.readEntry("FillingFileName", QString());
	d->fillingFirstColor = group.readEntry("FillingFirstColor", QColor(Qt::white));
	d->fillingSecondColor = group.readEntry("FillingSecondColor", QColor(Qt::black));
	d->fillingOpacity = group.readEntry("FillingOpacity", 0.5);

	d->errorType = (Histogram::ErrorType) group.readEntry("ErrorType", (int)Histogram::NoError);
	d->errorBarsType = (XYCurve::ErrorBarsType) group.readEntry("ErrorBarsType", static_cast<int>(XYCurve::ErrorBarsType::Simple));
	d->errorBarsCapSize = group.readEntry( "ErrorBarsCapSize", Worksheet::convertToSceneUnits(10, Worksheet::Unit::Point) );
	d->errorBarsPen.setStyle( (Qt::PenStyle)group.readEntry("ErrorBarsStyle", (int)Qt::SolidLine) );
	d->errorBarsPen.setColor( group.readEntry("ErrorBarsColor", QColor(Qt::black)) );
	d->errorBarsPen.setWidthF( group.readEntry("ErrorBarsWidth", Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Point)) );
	d->errorBarsOpacity = group.readEntry("ErrorBarsOpacity", 1.0);

	this->initActions();
}

void Histogram::initActions() {
	visibilityAction = new QAction(QIcon::fromTheme("view-visible"), i18n("Visible"), this);
	visibilityAction->setCheckable(true);
	connect(visibilityAction, &QAction::triggered, this, &Histogram::changeVisibility);
}

QMenu* Histogram::createContextMenu() {
	QMenu *menu = WorksheetElement::createContextMenu();
	QAction* firstAction = menu->actions().at(1); //skip the first action because of the "title-action"
	visibilityAction->setChecked(isVisible());
	menu->insertAction(firstAction, visibilityAction);

	//"data analysis" menu
	auto* analysisMenu = new QMenu(i18n("Analysis"));

	//TODO: if there are more actions, add a group for all fit types
	auto* fitGaussianAction = new QAction(QIcon::fromTheme("labplot-xy-fit-curve"), i18n("Fit Gaussian (Normal) Distribution"));
	analysisMenu->addAction(fitGaussianAction);
	connect(fitGaussianAction, &QAction::triggered, this, [=]() { m_plot->addHistogramFit(this, nsl_sf_stats_gaussian); });

	auto* fitExponentialAction = new QAction(QIcon::fromTheme("labplot-xy-fit-curve"), i18n("Fit Exponential Distribution"));
	analysisMenu->addAction(fitGaussianAction);
	connect(fitExponentialAction, &QAction::triggered, this, [=]() { m_plot->addHistogramFit(this, nsl_sf_stats_exponential); });

	auto* fitLaplaceAction = new QAction(QIcon::fromTheme("labplot-xy-fit-curve"), i18n("Fit Laplace Distribution"));
	analysisMenu->addAction(fitLaplaceAction);
	connect(fitLaplaceAction, &QAction::triggered, this, [=]() { m_plot->addHistogramFit(this, nsl_sf_stats_laplace); });

	auto* fitCauchyAction = new QAction(QIcon::fromTheme("labplot-xy-fit-curve"), i18n("Fit Cauchy-Lorentz Distribution"));
	analysisMenu->addAction(fitCauchyAction);
	connect(fitCauchyAction, &QAction::triggered, this, [=]() { m_plot->addHistogramFit(this, nsl_sf_stats_cauchy_lorentz); });

	auto* fitLognormalAction = new QAction(QIcon::fromTheme("labplot-xy-fit-curve"), i18n("Fit Log-normal Distribution"));
	analysisMenu->addAction(fitLognormalAction);
	connect(fitLognormalAction, &QAction::triggered, this, [=]() { m_plot->addHistogramFit(this, nsl_sf_stats_lognormal); });

	auto* fitPoissonAction = new QAction(QIcon::fromTheme("labplot-xy-fit-curve"), i18n("Fit Poisson Distribution"));
	analysisMenu->addAction(fitPoissonAction);
	connect(fitPoissonAction, &QAction::triggered, this, [=]() { m_plot->addHistogramFit(this, nsl_sf_stats_poisson); });

	auto* fitBinomialAction = new QAction(QIcon::fromTheme("labplot-xy-fit-curve"), i18n("Fit Binomial Distribution"));
	analysisMenu->addAction(fitBinomialAction);
	connect(fitBinomialAction, &QAction::triggered, this, [=]() { m_plot->addHistogramFit(this, nsl_sf_stats_binomial); });

	menu->insertMenu(visibilityAction, analysisMenu);
	menu->insertSeparator(visibilityAction);
	menu->insertSeparator(firstAction);

	return menu;
}

/*!
  Returns an icon to be used in the project explorer.
  */
QIcon Histogram::icon() const {
	return QIcon::fromTheme("view-object-histogram-linear");
}

QGraphicsItem* Histogram::graphicsItem() const {
	return d_ptr;
}

bool Histogram::activateCurve(QPointF mouseScenePos, double maxDist) {
	Q_D(Histogram);
	return d->activateCurve(mouseScenePos, maxDist);
}

void Histogram::setHover(bool on) {
	Q_D(Histogram);
	d->setHover(on);
}

//##############################################################################
//##########################  getter methods  ##################################
//##############################################################################
//general
BASIC_SHARED_D_READER_IMPL(Histogram, Histogram::HistogramType, type, type)
BASIC_SHARED_D_READER_IMPL(Histogram, Histogram::HistogramOrientation, orientation, orientation)
BASIC_SHARED_D_READER_IMPL(Histogram, Histogram::HistogramNormalization, normalization, normalization)
BASIC_SHARED_D_READER_IMPL(Histogram, Histogram::BinningMethod, binningMethod, binningMethod)
BASIC_SHARED_D_READER_IMPL(Histogram, int, binCount, binCount)
BASIC_SHARED_D_READER_IMPL(Histogram, float, binWidth, binWidth)
BASIC_SHARED_D_READER_IMPL(Histogram, bool, autoBinRanges, autoBinRanges)
BASIC_SHARED_D_READER_IMPL(Histogram, double, binRangesMin, binRangesMin)
BASIC_SHARED_D_READER_IMPL(Histogram, double, binRangesMax, binRangesMax)
BASIC_SHARED_D_READER_IMPL(Histogram, const AbstractColumn*, dataColumn, dataColumn)

QString& Histogram::dataColumnPath() const {
	D(Histogram);
	return d->dataColumnPath;
}

//line
BASIC_SHARED_D_READER_IMPL(Histogram, Histogram::LineType, lineType, lineType)
BASIC_SHARED_D_READER_IMPL(Histogram, QPen, linePen, linePen)
BASIC_SHARED_D_READER_IMPL(Histogram, qreal, lineOpacity, lineOpacity)

//symbols
Symbol* Histogram::symbol() const {
	Q_D(const Histogram);
	return d->symbol;
}

//values
BASIC_SHARED_D_READER_IMPL(Histogram, Histogram::ValuesType, valuesType, valuesType)
BASIC_SHARED_D_READER_IMPL(Histogram, const AbstractColumn *, valuesColumn, valuesColumn)
QString& Histogram::valuesColumnPath() const {
	D(Histogram);
	return d->valuesColumnPath;
}
BASIC_SHARED_D_READER_IMPL(Histogram, Histogram::ValuesPosition, valuesPosition, valuesPosition)
BASIC_SHARED_D_READER_IMPL(Histogram, qreal, valuesDistance, valuesDistance)
BASIC_SHARED_D_READER_IMPL(Histogram, qreal, valuesRotationAngle, valuesRotationAngle)
BASIC_SHARED_D_READER_IMPL(Histogram, qreal, valuesOpacity, valuesOpacity)
BASIC_SHARED_D_READER_IMPL(Histogram, char, valuesNumericFormat, valuesNumericFormat)
BASIC_SHARED_D_READER_IMPL(Histogram, int, valuesPrecision, valuesPrecision)
BASIC_SHARED_D_READER_IMPL(Histogram, QString, valuesDateTimeFormat, valuesDateTimeFormat)
BASIC_SHARED_D_READER_IMPL(Histogram, QString, valuesPrefix, valuesPrefix)
BASIC_SHARED_D_READER_IMPL(Histogram, QString, valuesSuffix, valuesSuffix)
BASIC_SHARED_D_READER_IMPL(Histogram, QColor, valuesColor, valuesColor)
BASIC_SHARED_D_READER_IMPL(Histogram, QFont, valuesFont, valuesFont)

//filling
BASIC_SHARED_D_READER_IMPL(Histogram, bool, fillingEnabled, fillingEnabled)
BASIC_SHARED_D_READER_IMPL(Histogram, WorksheetElement::BackgroundType, fillingType, fillingType)
BASIC_SHARED_D_READER_IMPL(Histogram, WorksheetElement::BackgroundColorStyle, fillingColorStyle, fillingColorStyle)
BASIC_SHARED_D_READER_IMPL(Histogram, WorksheetElement::BackgroundImageStyle, fillingImageStyle, fillingImageStyle)
BASIC_SHARED_D_READER_IMPL(Histogram, Qt::BrushStyle, fillingBrushStyle, fillingBrushStyle)
BASIC_SHARED_D_READER_IMPL(Histogram, QColor, fillingFirstColor, fillingFirstColor)
BASIC_SHARED_D_READER_IMPL(Histogram, QColor, fillingSecondColor, fillingSecondColor)
BASIC_SHARED_D_READER_IMPL(Histogram, QString, fillingFileName, fillingFileName)
BASIC_SHARED_D_READER_IMPL(Histogram, qreal, fillingOpacity, fillingOpacity)

//error bars
BASIC_SHARED_D_READER_IMPL(Histogram, Histogram::ErrorType, errorType, errorType)
BASIC_SHARED_D_READER_IMPL(Histogram, XYCurve::ErrorBarsType, errorBarsType, errorBarsType)
BASIC_SHARED_D_READER_IMPL(Histogram, qreal, errorBarsCapSize, errorBarsCapSize)
BASIC_SHARED_D_READER_IMPL(Histogram, QPen, errorBarsPen, errorBarsPen)
BASIC_SHARED_D_READER_IMPL(Histogram, qreal, errorBarsOpacity, errorBarsOpacity)

double Histogram::xMinimum() const {
	Q_D(const Histogram);
	return d->xMinimum();
}

double Histogram::xMaximum() const {
	Q_D(const Histogram);
	return d->xMaximum();
}

double Histogram::yMinimum() const {
	Q_D(const Histogram);
	return d->yMinimum();
}

double Histogram::yMaximum() const {
	Q_D(const Histogram);
	return d->yMaximum();
}


const AbstractColumn* Histogram::bins() const {
	D(Histogram);
	return d->bins();
}

const AbstractColumn* Histogram::binValues() const {
	D(Histogram);
	return d->binValues();
}

//##############################################################################
//#################  setter methods and undo commands ##########################
//##############################################################################

//General
STD_SETTER_CMD_IMPL_F_S(Histogram, SetDataColumn, const AbstractColumn*, dataColumn, recalcHistogram)
void Histogram::setDataColumn(const AbstractColumn* column) {
	Q_D(Histogram);
	if (column != d->dataColumn) {
		exec(new HistogramSetDataColumnCmd(d, column, ki18n("%1: set data column")));

		if (column) {
			connect(column, &AbstractColumn::dataChanged, this, &Histogram::dataChanged);

			//update the curve itself on changes
			connect(column, &AbstractColumn::dataChanged, this, &Histogram::recalcHistogram);
			connect(column->parentAspect(), &AbstractAspect::aspectAboutToBeRemoved,
					this, &Histogram::dataColumnAboutToBeRemoved);
			//TODO: add disconnect in the undo-function
		}
	}
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetHistogramType, Histogram::HistogramType, type, updateType)
void Histogram::setType(Histogram::HistogramType type) {
	Q_D(Histogram);
	if (type != d->type)
		exec(new HistogramSetHistogramTypeCmd(d, type, ki18n("%1: set histogram type")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetHistogramOrientation, Histogram::HistogramOrientation, orientation, updateOrientation)
void Histogram::setOrientation(Histogram::HistogramOrientation orientation) {
	Q_D(Histogram);
	if (orientation != d->orientation)
		exec(new HistogramSetHistogramOrientationCmd(d, orientation, ki18n("%1: set histogram orientation")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetHistogramNormalization, Histogram::HistogramNormalization, normalization, updateOrientation)
void Histogram::setNormalization(Histogram::HistogramNormalization normalization) {
	Q_D(Histogram);
	if (normalization != d->normalization)
		exec(new HistogramSetHistogramNormalizationCmd(d, normalization, ki18n("%1: set histogram normalization")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetBinningMethod, Histogram::BinningMethod, binningMethod, recalcHistogram)
void Histogram::setBinningMethod(Histogram::BinningMethod method) {
	Q_D(Histogram);
	if (method != d->binningMethod)
		exec(new HistogramSetBinningMethodCmd(d, method, ki18n("%1: set binning method")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetBinCount, int, binCount, recalcHistogram)
void Histogram::setBinCount(int count) {
	Q_D(Histogram);
	if (count != d->binCount)
		exec(new HistogramSetBinCountCmd(d, count, ki18n("%1: set bin count")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetBinWidth, float, binWidth, recalcHistogram)
void Histogram::setBinWidth(float width) {
	Q_D(Histogram);
	if (width != d->binWidth)
		exec(new HistogramSetBinWidthCmd(d, width, ki18n("%1: set bin width")));
}

class HistogramSetAutoBinRangesCmd : public QUndoCommand {
public:
	HistogramSetAutoBinRangesCmd(HistogramPrivate* private_obj, bool autoBinRanges) :
		m_private(private_obj), m_autoBinRanges(autoBinRanges) {
		setText(i18n("%1: change auto bin ranges", m_private->name()));
	};

	void redo() override {
		m_autoBinRangesOld = m_private->autoBinRanges;
		m_private->autoBinRanges = m_autoBinRanges;
		if (m_autoBinRanges) {
			m_binRangesMinOld = m_private->binRangesMin;
			m_binRangesMaxOld = m_private->binRangesMax;
			m_private->q->recalcHistogram();
		}
		Q_EMIT m_private->q->autoBinRangesChanged(m_autoBinRanges);
	};

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
			m_private->recalcHistogram();
		}
		Q_EMIT m_private->q->autoBinRangesChanged(m_autoBinRangesOld);
	}

private:
	HistogramPrivate* m_private;
	bool m_autoBinRanges;
	bool m_autoBinRangesOld{false};
	double m_binRangesMinOld{0.0};
	double m_binRangesMaxOld{0.0};
};

void Histogram::setAutoBinRanges(bool autoBinRanges) {
	Q_D(Histogram);
	if (autoBinRanges != d->autoBinRanges)
		exec(new HistogramSetAutoBinRangesCmd(d, autoBinRanges));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetBinRangesMin, double, binRangesMin, recalcHistogram)
void Histogram::setBinRangesMin(double binRangesMin) {
	Q_D(Histogram);
	if (binRangesMin != d->binRangesMin)
		exec(new HistogramSetBinRangesMinCmd(d, binRangesMin, ki18n("%1: set bin ranges start")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetBinRangesMax, double, binRangesMax, recalcHistogram)
void Histogram::setBinRangesMax(double binRangesMax) {
	Q_D(Histogram);
	if (binRangesMax != d->binRangesMax)
		exec(new HistogramSetBinRangesMaxCmd(d, binRangesMax, ki18n("%1: set bin ranges end")));
}

//Line
STD_SETTER_CMD_IMPL_F_S(Histogram, SetLineType, Histogram::LineType, lineType, updateLines)
void Histogram::setLineType(LineType type) {
	Q_D(Histogram);
	if (type != d->lineType)
		exec(new HistogramSetLineTypeCmd(d, type, ki18n("%1: line type changed")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetLinePen, QPen, linePen, recalcShapeAndBoundingRect)
void Histogram::setLinePen(const QPen &pen) {
	Q_D(Histogram);
	if (pen != d->linePen)
		exec(new HistogramSetLinePenCmd(d, pen, ki18n("%1: set line style")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetLineOpacity, qreal, lineOpacity, updatePixmap)
void Histogram::setLineOpacity(qreal opacity) {
	Q_D(Histogram);
	if (opacity != d->lineOpacity)
		exec(new HistogramSetLineOpacityCmd(d, opacity, ki18n("%1: set line opacity")));
}

//Values
STD_SETTER_CMD_IMPL_F_S(Histogram, SetValuesType, Histogram::ValuesType, valuesType, updateValues)
void Histogram::setValuesType(Histogram::ValuesType type) {
	Q_D(Histogram);
	if (type != d->valuesType)
		exec(new HistogramSetValuesTypeCmd(d, type, ki18n("%1: set values type")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetValuesColumn, const AbstractColumn*, valuesColumn, updateValues)
void Histogram::setValuesColumn(const AbstractColumn* column) {
	Q_D(Histogram);
	if (column != d->valuesColumn) {
		exec(new HistogramSetValuesColumnCmd(d, column, ki18n("%1: set values column")));
		if (column) {
			connect(column, &AbstractColumn::dataChanged, this, &Histogram::updateValues);
			connect(column->parentAspect(), &AbstractAspect::aspectAboutToBeRemoved,
					this, &Histogram::valuesColumnAboutToBeRemoved);
		}
	}
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetValuesPosition, Histogram::ValuesPosition, valuesPosition, updateValues)
void Histogram::setValuesPosition(ValuesPosition position) {
	Q_D(Histogram);
	if (position != d->valuesPosition)
		exec(new HistogramSetValuesPositionCmd(d, position, ki18n("%1: set values position")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetValuesDistance, qreal, valuesDistance, updateValues)
void Histogram::setValuesDistance(qreal distance) {
	Q_D(Histogram);
	if (distance != d->valuesDistance)
		exec(new HistogramSetValuesDistanceCmd(d, distance, ki18n("%1: set values distance")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetValuesRotationAngle, qreal, valuesRotationAngle, updateValues)
void Histogram::setValuesRotationAngle(qreal angle) {
	Q_D(Histogram);
	if (!qFuzzyCompare(1 + angle, 1 + d->valuesRotationAngle))
		exec(new HistogramSetValuesRotationAngleCmd(d, angle, ki18n("%1: rotate values")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetValuesOpacity, qreal, valuesOpacity, updatePixmap)
void Histogram::setValuesOpacity(qreal opacity) {
	Q_D(Histogram);
	if (opacity != d->valuesOpacity)
		exec(new HistogramSetValuesOpacityCmd(d, opacity, ki18n("%1: set values opacity")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetValuesNumericFormat, char, valuesNumericFormat, updateValues)
void Histogram::setValuesNumericFormat(char format) {
	Q_D(Histogram);
	if (format != d->valuesNumericFormat)
		exec(new HistogramSetValuesNumericFormatCmd(d, format, ki18n("%1: set values numeric format")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetValuesPrecision, int, valuesPrecision, updateValues)
void Histogram::setValuesPrecision(int precision) {
	Q_D(Histogram);
	if (precision != d->valuesPrecision)
		exec(new HistogramSetValuesPrecisionCmd(d, precision, ki18n("%1: set values precision")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetValuesDateTimeFormat, QString, valuesDateTimeFormat, updateValues)
void Histogram::setValuesDateTimeFormat(const QString& format) {
	Q_D(Histogram);
	if (format != d->valuesDateTimeFormat)
		exec(new HistogramSetValuesDateTimeFormatCmd(d, format, ki18n("%1: set values datetime format")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetValuesPrefix, QString, valuesPrefix, updateValues)
void Histogram::setValuesPrefix(const QString& prefix) {
	Q_D(Histogram);
	if (prefix!= d->valuesPrefix)
		exec(new HistogramSetValuesPrefixCmd(d, prefix, ki18n("%1: set values prefix")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetValuesSuffix, QString, valuesSuffix, updateValues)
void Histogram::setValuesSuffix(const QString& suffix) {
	Q_D(Histogram);
	if (suffix!= d->valuesSuffix)
		exec(new HistogramSetValuesSuffixCmd(d, suffix, ki18n("%1: set values suffix")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetValuesFont, QFont, valuesFont, updateValues)
void Histogram::setValuesFont(const QFont& font) {
	Q_D(Histogram);
	if (font!= d->valuesFont)
		exec(new HistogramSetValuesFontCmd(d, font, ki18n("%1: set values font")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetValuesColor, QColor, valuesColor, updatePixmap)
void Histogram::setValuesColor(const QColor& color) {
	Q_D(Histogram);
	if (color != d->valuesColor)
		exec(new HistogramSetValuesColorCmd(d, color, ki18n("%1: set values color")));
}

//Filling
STD_SETTER_CMD_IMPL_F_S(Histogram, SetFillingEnabled, bool, fillingEnabled, updateFilling)
void Histogram::setFillingEnabled(bool enabled) {
	Q_D(Histogram);
	if (enabled != d->fillingEnabled)
		exec(new HistogramSetFillingEnabledCmd(d, enabled, ki18n("%1: filling changed")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetFillingType, WorksheetElement::BackgroundType, fillingType, updatePixmap)
void Histogram::setFillingType(WorksheetElement::BackgroundType type) {
	Q_D(Histogram);
	if (type != d->fillingType)
		exec(new HistogramSetFillingTypeCmd(d, type, ki18n("%1: filling type changed")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetFillingColorStyle, WorksheetElement::BackgroundColorStyle, fillingColorStyle, updatePixmap)
void Histogram::setFillingColorStyle(WorksheetElement::BackgroundColorStyle style) {
	Q_D(Histogram);
	if (style != d->fillingColorStyle)
		exec(new HistogramSetFillingColorStyleCmd(d, style, ki18n("%1: filling color style changed")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetFillingImageStyle, WorksheetElement::BackgroundImageStyle, fillingImageStyle, updatePixmap)
void Histogram::setFillingImageStyle(WorksheetElement::BackgroundImageStyle style) {
	Q_D(Histogram);
	if (style != d->fillingImageStyle)
		exec(new HistogramSetFillingImageStyleCmd(d, style, ki18n("%1: filling image style changed")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetFillingBrushStyle, Qt::BrushStyle, fillingBrushStyle, updatePixmap)
void Histogram::setFillingBrushStyle(Qt::BrushStyle style) {
	Q_D(Histogram);
	if (style != d->fillingBrushStyle)
		exec(new HistogramSetFillingBrushStyleCmd(d, style, ki18n("%1: filling brush style changed")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetFillingFirstColor, QColor, fillingFirstColor, updatePixmap)
void Histogram::setFillingFirstColor(const QColor& color) {
	Q_D(Histogram);
	if (color!= d->fillingFirstColor)
		exec(new HistogramSetFillingFirstColorCmd(d, color, ki18n("%1: set filling first color")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetFillingSecondColor, QColor, fillingSecondColor, updatePixmap)
void Histogram::setFillingSecondColor(const QColor& color) {
	Q_D(Histogram);
	if (color!= d->fillingSecondColor)
		exec(new HistogramSetFillingSecondColorCmd(d, color, ki18n("%1: set filling second color")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetFillingFileName, QString, fillingFileName, updatePixmap)
void Histogram::setFillingFileName(const QString& fileName) {
	Q_D(Histogram);
	if (fileName!= d->fillingFileName)
		exec(new HistogramSetFillingFileNameCmd(d, fileName, ki18n("%1: set filling image")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetFillingOpacity, qreal, fillingOpacity, updatePixmap)
void Histogram::setFillingOpacity(qreal opacity) {
	Q_D(Histogram);
	if (opacity != d->fillingOpacity)
		exec(new HistogramSetFillingOpacityCmd(d, opacity, ki18n("%1: set filling opacity")));
}

//Error bars
STD_SETTER_CMD_IMPL_F_S(Histogram, SetErrorType, Histogram::ErrorType, errorType, updateErrorBars)
void Histogram::setErrorType(ErrorType type) {
	Q_D(Histogram);
	if (type != d->errorType)
		exec(new HistogramSetErrorTypeCmd(d, type, ki18n("%1: x-error type changed")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetErrorBarsCapSize, qreal, errorBarsCapSize, updateErrorBars)
void Histogram::setErrorBarsCapSize(qreal size) {
	Q_D(Histogram);
	if (size != d->errorBarsCapSize)
		exec(new HistogramSetErrorBarsCapSizeCmd(d, size, ki18n("%1: set error bar cap size")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetErrorBarsType, XYCurve::ErrorBarsType, errorBarsType, updateErrorBars)
void Histogram::setErrorBarsType(XYCurve::ErrorBarsType type) {
	Q_D(Histogram);
	if (type != d->errorBarsType)
		exec(new HistogramSetErrorBarsTypeCmd(d, type, ki18n("%1: error bar type changed")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetErrorBarsPen, QPen, errorBarsPen, recalcShapeAndBoundingRect)
void Histogram::setErrorBarsPen(const QPen& pen) {
	Q_D(Histogram);
	if (pen != d->errorBarsPen)
		exec(new HistogramSetErrorBarsPenCmd(d, pen, ki18n("%1: set error bar style")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetErrorBarsOpacity, qreal, errorBarsOpacity, updatePixmap)
void Histogram::setErrorBarsOpacity(qreal opacity) {
	Q_D(Histogram);
	if (opacity != d->errorBarsOpacity)
		exec(new HistogramSetErrorBarsOpacityCmd(d, opacity, ki18n("%1: set error bar opacity")));
}

//##############################################################################
//#################################  SLOTS  ####################################
//##############################################################################
void Histogram::retransform() {
	d_ptr->retransform();
}

void Histogram::recalcHistogram() {
	D(Histogram);
	d->recalcHistogram();
}

//TODO
void Histogram::handleResize(double horizontalRatio, double /*verticalRatio*/, bool /*pageResize*/) {
	Q_D(const Histogram);

	//setValuesDistance(d->distance*);
	QFont font = d->valuesFont;
	font.setPointSizeF(font.pointSizeF()*horizontalRatio);
	setValuesFont(font);

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
	}
}

void Histogram::valuesColumnAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(Histogram);
	if (aspect == d->valuesColumn) {
		d->valuesColumn = nullptr;
		d->updateValues();
	}
}

//##############################################################################
//######################### Private implementation #############################
//##############################################################################
HistogramPrivate::HistogramPrivate(Histogram *owner) : WorksheetElementPrivate(owner), q(owner) {
	setFlag(QGraphicsItem::ItemIsSelectable, true);
	setAcceptHoverEvents(false);
}

HistogramPrivate::~HistogramPrivate() {
	if (m_histogram)
		gsl_histogram_free(m_histogram);
}

QRectF HistogramPrivate::boundingRect() const {
	return boundingRectangle;
}

double HistogramPrivate::getMaximumOccuranceofHistogram() const {
	if (m_histogram) {
		double yMaxRange = -qInf();
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
				point+= gsl_histogram_get(m_histogram,i);
				if (point > yMaxRange) {
					yMaxRange = point;
				}
			}
			//yMaxRange = dataColumn->rowCount();
			break;
		}
		case Histogram::AvgShift: {
			//TODO
		}
		}

		switch (normalization) {
		case Histogram::Count:
			break;
		case Histogram::Probability:
			yMaxRange = yMaxRange/totalCount;
			break;
		case Histogram::CountDensity: {
			const double width = (binRangesMax - binRangesMin)/m_bins;
			yMaxRange = yMaxRange/width;
			break;
		}
		case Histogram::ProbabilityDensity: {
			const double width = (binRangesMax - binRangesMin)/m_bins;
			yMaxRange = yMaxRange/totalCount/width;
			break;
		}
		}

		return yMaxRange;
	}

	return -qInf();
}

double HistogramPrivate::xMinimum() const {
	switch (orientation) {
	case Histogram::Vertical:
		return autoBinRanges ? dataColumn->minimum() : binRangesMin;
	case Histogram::Horizontal:
		return 0;
	}
	return qInf();
}

double HistogramPrivate::xMaximum() const {
	switch (orientation) {
	case Histogram::Vertical:
		return autoBinRanges ? dataColumn->maximum() : binRangesMax;
	case Histogram::Horizontal:
		return getMaximumOccuranceofHistogram();
	}
	return -qInf();
}

double HistogramPrivate::yMinimum() const {
	switch (orientation) {
	case Histogram::Vertical:
		return 0;
	case Histogram::Horizontal:
		return autoBinRanges ? dataColumn->minimum() : binRangesMin;
	}
	return qInf();
}

double HistogramPrivate::yMaximum() const {
	switch (orientation) {
	case Histogram::Vertical:
		return getMaximumOccuranceofHistogram();
	case Histogram::Horizontal:
		return autoBinRanges ? dataColumn->maximum() : binRangesMax;
	}
	return qInf();
}

const AbstractColumn* HistogramPrivate::bins() {
	if (!m_binsColumn) {
		m_binsColumn = new Column("bins");

		const double width = (binRangesMax - binRangesMin)/m_bins;
		m_binsColumn->resizeTo(m_bins);
		for (size_t i = 0; i < m_bins; ++i) {
			const double x = binRangesMin + i*width;
			m_binsColumn->setValueAt(i, x);
		}
	}

	return m_binsColumn;
}

const AbstractColumn* HistogramPrivate::binValues() {
	if (!m_binValuesColumn) {
		m_binValuesColumn = new Column("values");

		m_binValuesColumn->resizeTo(m_bins);
		const double width = (binRangesMax - binRangesMin)/m_bins;
		for (size_t i = 0; i < m_bins; ++i)
			m_binValuesColumn->setValueAt(i, gsl_histogram_get(m_histogram, i)/totalCount/width); //probability density normalization
	}

	return m_binValuesColumn;
}


/*!
  Returns the shape of the Histogram as a QPainterPath in local coordinates
  */
QPainterPath HistogramPrivate::shape() const {
	return curveShape;
}

void HistogramPrivate::contextMenuEvent(QGraphicsSceneContextMenuEvent* event) {
	q->createContextMenu()->exec(event->screenPos());
}

/*!
  called when the size of the plot or its data ranges (manual changes, zooming, etc.) were changed.
  recalculates the position of the scene points to be drawn.
  triggers the update of lines, drop lines, symbols etc.
*/
void HistogramPrivate::retransform() {
	if (m_suppressRetransform || q->isLoading())
		return;

	if (!isVisible())
		return;

	PERFTRACE(name() + Q_FUNC_INFO);

	if (!dataColumn) {
		linePath = QPainterPath();
		symbolsPath = QPainterPath();
		valuesPath = QPainterPath();
		curveShape = QPainterPath();
		lines.clear();
		pointsLogical.clear();
		pointsScene.clear();
		visiblePoints.clear();
		valuesPoints.clear();
		valuesStrings.clear();
		fillPolygon.clear();
		recalcShapeAndBoundingRect();
		return;
	}

	m_suppressRecalc = true;
	updateLines();
	updateSymbols();
	m_suppressRecalc = false;
	updateValues();
}

/*!
 * called when the data was changed. recalculates the histogram.
 */
void HistogramPrivate::recalcHistogram() {
	PERFTRACE(name() + Q_FUNC_INFO);

	if (m_histogram) {
		gsl_histogram_free(m_histogram);
		m_histogram = nullptr;
	}

	if (!dataColumn)
		return;

	//calculate the number of valid data points
	int count = 0;
	for (int row = 0; row < dataColumn->rowCount(); ++row) {
		if (dataColumn->isValid(row) && !dataColumn->isMasked(row))
			++count;
	}

	//calculate the number of bins
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
			Q_EMIT q->info(i18n("Calculation of the histogram not possible. The max value must be bigger then the min value."));
			return;
		}

		switch (binningMethod) {
		case Histogram::ByNumber:
			m_bins = (size_t)binCount;
			break;
		case Histogram::ByWidth:
			m_bins = (size_t) (binRangesMax-binRangesMin)/binWidth;
			break;
		case Histogram::SquareRoot:
			m_bins = (size_t)sqrt(count);
			break;
		case Histogram::Rice:
			m_bins = (size_t)2*cbrt(count);
			break;
		case Histogram::Sturges:
			m_bins = (size_t) 1 + log2(count);
			break;
		case Histogram::Doane: {
			const double skewness = static_cast<const Column*>(dataColumn)->statistics().skewness;
			m_bins = (size_t)( 1 + log2(count) + log2(1 + abs(skewness)/sqrt((double)6*(count-2)/(count+1)/(count+3))) );
			break;
		}
		case Histogram::Scott: {
			const double sigma = static_cast<const Column*>(dataColumn)->statistics().standardDeviation;
			const double width = 3.5*sigma/cbrt(count);
			m_bins = (size_t)(binRangesMax - binRangesMin)/width;
			break;
		}
		}

		DEBUG("min " << binRangesMin)
		DEBUG("max " << binRangesMax)
		DEBUG("number of bins " << m_bins)

		//calculate the histogram
		if (m_bins > 0) {
			m_histogram = gsl_histogram_alloc (m_bins);
			gsl_histogram_set_ranges_uniform (m_histogram, binRangesMin, binRangesMax);

			switch (dataColumn->columnMode()) {
			case AbstractColumn::ColumnMode::Double:
			case AbstractColumn::ColumnMode::Integer:
			case AbstractColumn::ColumnMode::BigInt:
				for (int row = 0; row < dataColumn->rowCount(); ++row) {
					if ( dataColumn->isValid(row) && !dataColumn->isMasked(row) )
						gsl_histogram_increment(m_histogram, dataColumn->valueAt(row));
				}
				break;
			case AbstractColumn::ColumnMode::DateTime:
				for (int row = 0; row < dataColumn->rowCount(); ++row) {
					if ( dataColumn->isValid(row) && !dataColumn->isMasked(row) )
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

			//fill the columns for the positions and values of the bins
			if (m_binsColumn) {
				m_binsColumn->resizeTo(m_bins);
				const double width = (binRangesMax - binRangesMin)/m_bins;
				for (size_t i = 0; i < m_bins; ++i)
					m_binsColumn->setValueAt(i, binRangesMin + i*width);
			}

			if (m_binValuesColumn) {
				m_binValuesColumn->resizeTo(m_bins);
				const double width = (binRangesMax - binRangesMin)/m_bins;
				for (size_t i = 0; i < m_bins; ++i)
					m_binValuesColumn->setValueAt(i, gsl_histogram_get(m_histogram, i)/totalCount/width); //probability density normalization
			}
		} else
			DEBUG("Number of bins must be positiv integer")
	}

	//histogram changed because of the actual data changes or because of new bin settings,
	//Q_EMIT dataChanged() in order to recalculate everything with the new size/shape of the histogram
	Q_EMIT q->dataChanged();
}

void HistogramPrivate::updateType() {
	//type (ordinary or cumulative) changed,
	//Q_EMIT dataChanged() in order to recalculate everything with the new size/shape of the histogram
	Q_EMIT q->dataChanged();
}

void HistogramPrivate::updateOrientation() {
	//orientation (horizontal or vertical) changed
	//Q_EMIT dataChanged() in order to recalculate everything with the new size/shape of the histogram
	Q_EMIT q->dataChanged();
}

/*!
  recalculates the painter path for the lines connecting the data points.
  Called each time when the type of this connection is changed.
  */
void HistogramPrivate::updateLines() {
	PERFTRACE(name() + Q_FUNC_INFO);

	linePath = QPainterPath();
	lines.clear();
	pointsLogical.clear();
	pointsScene.clear();

	if (orientation == Histogram::Vertical)
		verticalHistogram();
	else
		horizontalHistogram();

	//map the lines and the symbol positions to the scene coordinates
	const auto* plot = static_cast<const CartesianPlot*>(q->parentAspect());
	const auto* cSystem{ plot->defaultCoordinateSystem() };
	lines = cSystem->mapLogicalToScene(lines);
	visiblePoints = std::vector<bool>(pointsLogical.count(), false);
	cSystem->mapLogicalToScene(pointsLogical, pointsScene, visiblePoints);

	//new line path
	for (const auto& line : lines) {
		linePath.moveTo(line.p1());
		linePath.lineTo(line.p2());
	}

	updateFilling();
	recalcShapeAndBoundingRect();
}

void HistogramPrivate::histogramValue(double& value, int bin) {
	switch (normalization) {
	case Histogram::Count: {
		if (type == Histogram::Ordinary)
			value = gsl_histogram_get(m_histogram, bin);
		else
			value += gsl_histogram_get(m_histogram, bin);
		break;
	}
	case Histogram::Probability: {
		if (type == Histogram::Ordinary)
			value = gsl_histogram_get(m_histogram, bin)/totalCount;
		else
			value += gsl_histogram_get(m_histogram, bin)/totalCount;
		break;
	}
	case Histogram::CountDensity: {
		const double width = (binRangesMax - binRangesMin)/m_bins;
		if (type == Histogram::Ordinary)
			value = gsl_histogram_get(m_histogram, bin)/width;
		else
			value += gsl_histogram_get(m_histogram, bin)/width;
		break;
	}
	case Histogram::ProbabilityDensity: {
		const double width = (binRangesMax - binRangesMin)/m_bins;
		if (type == Histogram::Ordinary)
			value = gsl_histogram_get(m_histogram, bin)/totalCount/width;
		else
			value += gsl_histogram_get(m_histogram, bin)/totalCount/width;
		break;
	}
	}
}

void HistogramPrivate::verticalHistogram() {
	if (!m_histogram)
		return;

	const double width = (binRangesMax - binRangesMin)/m_bins;
	double value = 0.;
	switch (lineType) {
	case Histogram::Bars: {
		for (size_t i = 0; i < m_bins; ++i) {
			histogramValue(value, i);
			const double x = binRangesMin + i*width;
			lines.append(QLineF(x, 0., x, value));
			lines.append(QLineF(x, value, x + width, value));
			lines.append(QLineF(x + width, value, x + width, 0.));
			pointsLogical.append(QPointF(x+width/2, value));
		}
		break;
	}
	case Histogram::NoLine:
	case Histogram::Envelope: {
		double prevValue = 0.;
		for (size_t i = 0; i < m_bins; ++i) {
			histogramValue(value, i);
			const double x = binRangesMin + i*width;
			lines.append(QLineF(x, prevValue, x, value));
			lines.append(QLineF(x, value, x + width, value));
			pointsLogical.append(QPointF(x+width/2, value));

			if (i == m_bins - 1)
				lines.append(QLineF(x + width, value, x + width, 0.));

			prevValue = value;
		}
		break;
	}
	case Histogram::DropLines: {
		for (size_t i = 0; i < m_bins; ++i) {
			histogramValue(value, i);
			const double x = binRangesMin + i*width + width/2;
			lines.append(QLineF(x, 0., x, value));
			pointsLogical.append(QPointF(x, value));
		}
		break;
	}
	case Histogram::HalfBars :{
		for (size_t i = 0; i < m_bins; ++i) {
			histogramValue(value, i);
			const double x = binRangesMin + i*width + width/2;
			lines.append(QLineF(x, 0., x, value));
			lines.append(QLineF(x, value, x - width/4, value));
			pointsLogical.append(QPointF(x, value));
		}
		break;
	}
	}

	if (lineType != Histogram::DropLines && lineType != Histogram::HalfBars)
		lines.append(QLineF(binRangesMax, 0., binRangesMin, 0.));
}

void HistogramPrivate::horizontalHistogram() {
	if (!m_histogram)
		return;

	const double width = (binRangesMax - binRangesMin)/m_bins;
	double value = 0.;
	switch (lineType) {
	case Histogram::Bars: {
		for (size_t i = 0; i < m_bins; ++i) {
			histogramValue(value, i);
			const double y = binRangesMin + i*width;
			lines.append(QLineF(0., y, value, y));
			lines.append(QLineF(value, y, value, y + width));
			lines.append(QLineF(value, y + width, 0., y + width));
			pointsLogical.append(QPointF(value, y+width/2));
		}
		break;
	}
	case Histogram::NoLine:
	case Histogram::Envelope: {
		double prevValue = 0.;
		for (size_t i = 0; i < m_bins; ++i) {
			histogramValue(value, i);
			const double y = binRangesMin + i*width;
			lines.append(QLineF(prevValue, y, value, y));
			lines.append(QLineF(value, y, value, y + width));
			pointsLogical.append(QPointF(value, y+width/2));

			if (i == m_bins - 1)
				lines.append(QLineF(value, y + width, 0., y + width));

			prevValue = value;
		}
		break;
	}
	case Histogram::DropLines: {
		for (size_t i = 0; i < m_bins; ++i) {
			histogramValue(value, i);
			const double y = binRangesMin + i*width + width/2;
			lines.append(QLineF(0., y, value, y));
			pointsLogical.append(QPointF(value, y));
		}
		break;
	}
	case Histogram::HalfBars :{
		for (size_t i = 0; i < m_bins; ++i) {
			histogramValue(value, i);
			const double y = binRangesMin + i*width + width/2;
			lines.append(QLineF(0., y, value, y));
			lines.append(QLineF(value, y, value, y + width/4));
			pointsLogical.append(QPointF(value, y));
		}
		break;
	}
	}

	if (lineType != Histogram::DropLines && lineType != Histogram::HalfBars)
		lines.append(QLineF(0., binRangesMin, 0., binRangesMax));
}

void HistogramPrivate::updateSymbols() {
	symbolsPath = QPainterPath();
	if (symbol->style() != Symbol::Style::NoSymbols) {
		QPainterPath path = Symbol::stylePath(symbol->style());

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

	if (valuesType == Histogram::NoValues || !m_histogram) {
		recalcShapeAndBoundingRect();
		return;
	}

	//determine the value string for all points that are currently visible in the plot
	if (valuesType == Histogram::ValuesBinEntries) {
		switch (type) {
		case Histogram::Ordinary:
			for (size_t i = 0; i < m_bins; ++i) {
				if (!visiblePoints[i]) continue;
				valuesStrings << valuesPrefix + QString::number(gsl_histogram_get(m_histogram, i)) + valuesSuffix;
			}
			break;
		case Histogram::Cumulative: {
			value = 0;
			for (size_t i = 0; i < m_bins; ++i) {
				if (!visiblePoints[i]) continue;
				value += gsl_histogram_get(m_histogram, i);
				valuesStrings << valuesPrefix + QString::number(value) + valuesSuffix;
			}
			break;
		}
		case Histogram::AvgShift:
			break;
		}
	} else if (valuesType == Histogram::ValuesCustomColumn) {
		if (!valuesColumn) {
			recalcShapeAndBoundingRect();
			return;
		}

		const int endRow = qMin(pointsLogical.size(), valuesColumn->rowCount());
		const auto xColMode = valuesColumn->columnMode();
		for (int i = 0; i < endRow; ++i) {
			if (!visiblePoints[i]) continue;

			if ( !valuesColumn->isValid(i) || valuesColumn->isMasked(i) )
				continue;

			switch (xColMode) {
			case AbstractColumn::ColumnMode::Double:
				valuesStrings << valuesPrefix + QString::number(valuesColumn->valueAt(i), valuesNumericFormat, valuesPrecision) + valuesSuffix;
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
				valuesStrings << valuesPrefix + valuesColumn->dateTimeAt(i).toString(valuesDateTimeFormat) + valuesSuffix;
				break;
			}
		}
	}

	//Calculate the coordinates where to paint the value strings.
	//The coordinates depend on the actual size of the string.
	QPointF tempPoint;
	QFontMetrics fm(valuesFont);
	qreal w;
	const qreal h = fm.ascent();
	switch (valuesPosition) {
	case Histogram::ValuesAbove:
		for (int i = 0; i < valuesStrings.size(); i++) {
			w = fm.boundingRect(valuesStrings.at(i)).width();
			tempPoint.setX( pointsScene.at(i).x() -w/2);
			tempPoint.setY( pointsScene.at(i).y() - valuesDistance );
			valuesPoints.append(tempPoint);
		}
		break;
	case Histogram::ValuesUnder:
		for (int i = 0; i < valuesStrings.size(); i++) {
			w = fm.boundingRect(valuesStrings.at(i)).width();
			tempPoint.setX( pointsScene.at(i).x() -w/2);
			tempPoint.setY( pointsScene.at(i).y() + valuesDistance + h/2);
			valuesPoints.append(tempPoint);
		}
		break;
	case Histogram::ValuesLeft:
		for (int i = 0; i < valuesStrings.size(); i++) {
			w = fm.boundingRect(valuesStrings.at(i)).width();
			tempPoint.setX( pointsScene.at(i).x() - valuesDistance - w - 1);
			tempPoint.setY( pointsScene.at(i).y());
			valuesPoints.append(tempPoint);
		}
		break;
	case Histogram::ValuesRight:
		for (int i = 0; i < valuesStrings.size(); i++) {
			tempPoint.setX( pointsScene.at(i).x() + valuesDistance - 1);
			tempPoint.setY( pointsScene.at(i).y() );
			valuesPoints.append(tempPoint);
		}
		break;
	}

	QTransform trafo;
	QPainterPath path;
	for (int i = 0; i < valuesPoints.size(); i++) {
		path = QPainterPath();
		path.addText( QPoint(0,0), valuesFont, valuesStrings.at(i) );

		trafo.reset();
		trafo.translate( valuesPoints.at(i).x(), valuesPoints.at(i).y() );
		if (valuesRotationAngle != 0.)
			trafo.rotate( -valuesRotationAngle );

		valuesPath.addPath(trafo.map(path));
	}

	recalcShapeAndBoundingRect();
}

void HistogramPrivate::updateFilling() {
	fillPolygon.clear();

	if (!fillingEnabled || lineType == Histogram::DropLines || lineType == Histogram::HalfBars) {
		recalcShapeAndBoundingRect();
		return;
	}

	QVector<QLineF> fillLines;
	const auto* plot = static_cast<const CartesianPlot*>(q->parentAspect());
	const AbstractCoordinateSystem* cSystem{ plot->defaultCoordinateSystem() };

	//if there're no interpolation lines available (Histogram::NoLine selected), create line-interpolation,
	//use already available lines otherwise.
	if (lines.size())
		fillLines = lines;
	else {
		for (int i = 0; i < pointsLogical.count()-1; i++)
			fillLines.append(QLineF(pointsLogical.at(i), pointsLogical.at(i+1)));
		fillLines = cSystem->mapLogicalToScene(fillLines);
	}

	//no lines available (no points), nothing to do
	if (!fillLines.size())
		return;

	//create the filling polygon for the visible lines

	//in case the histogram is zoomed, handle the clipping on the l.h.s.
	const QPointF& firstPoint = fillLines.constFirst().p1();
	QPointF start;
	bool visible;
	if (plot->xRange().start() > binRangesMin) {
		start = cSystem->mapLogicalToScene(QPointF(plot->xRange().start(), plot->yRange().start() > 0 ? plot->yRange().start() : 0), visible);

		if (visible && !qFuzzyCompare(start.x(), firstPoint.x()) )
			fillPolygon << QPointF(start.x(), firstPoint.y());
	}

	//add the first point of the first visible line
	fillPolygon << firstPoint;

	//iterate over all visible lines and add unique points.
	//skip the last closing line, the filling polygon will be closed below.
	QPointF p1, p2;
	for (int i = 0; i < fillLines.size() - 1; ++i) {
		const QLineF& line = fillLines.at(i);
		p1 = line.p1();
		p2 = line.p2();
		if (i != 0 && p1 != fillLines.at(i-1).p2())
			fillPolygon << p1;

		fillPolygon << p2;
	}

	//in case the histogram is zoomed, handle the clipping on the r.h.s.
	const QPointF& lastPoint = fillLines.at(fillLines.size()-2).p2();
	QPointF end;
	if (plot->xRange().end() < binRangesMax) {
		end = cSystem->mapLogicalToScene(QPointF(plot->xRange().end(), plot->yRange().start() > 0 ? plot->yRange().start() : 0), visible);

		if (visible && end.y() != lastPoint.y())
			fillPolygon << QPointF(end.x(), lastPoint.y());
	} else
		end = cSystem->mapLogicalToScene(QPointF(binRangesMax, plot->yRange().start() > 0 ? plot->yRange().start() : 0), visible);

	//close the polygon
	fillPolygon << end;
	if (plot->xRange().start() > binRangesMin)
		fillPolygon << start;

	recalcShapeAndBoundingRect();
}

void HistogramPrivate::updateErrorBars() {

}

/*!
  recalculates the outer bounds and the shape of the curve.
  */
void HistogramPrivate::recalcShapeAndBoundingRect() {
	if (m_suppressRecalc)
		return;

	prepareGeometryChange();
	curveShape = QPainterPath();
	if (lineType != Histogram::NoLine)
		curveShape.addPath(WorksheetElement::shapeFromPath(linePath, linePen));

	if (symbol->style() != Symbol::Style::NoSymbols)
		curveShape.addPath(symbolsPath);

	if (valuesType != Histogram::NoValues)
		curveShape.addPath(valuesPath);

	boundingRectangle = curveShape.boundingRect();

	boundingRectangle = boundingRectangle.united(fillPolygon.boundingRect());

	//TODO: when the selection is painted, line intersections are visible.
	//simplified() removes those artifacts but is horrible slow for curves with large number of points.
	//search for an alternative.
	//curveShape = curveShape.simplified();

	updatePixmap();
}

void HistogramPrivate::draw(QPainter* painter) {
	PERFTRACE(name() + Q_FUNC_INFO);

	//drawing line
	if (lineType != Histogram::NoLine) {
		painter->setOpacity(lineOpacity);
		painter->setPen(linePen);
		painter->setBrush(Qt::NoBrush);
		painter->drawPath(linePath);
	}

	//draw filling
	if (fillingEnabled) {
		painter->setOpacity(fillingOpacity);
		painter->setPen(Qt::SolidLine);
		drawFilling(painter);
	}

	//draw symbols
	if (symbol->style() != Symbol::Style::NoSymbols) {
		painter->setOpacity(symbol->opacity());
		painter->setPen(symbol->pen());
		painter->setBrush(symbol->brush());
		drawSymbols(painter);
	}

	//draw values
	if (valuesType != Histogram::NoValues) {
		painter->setOpacity(valuesOpacity);
		painter->setPen(QPen(valuesColor));
		painter->setFont(valuesFont);
		drawValues(painter);
	}
}

void HistogramPrivate::updatePixmap() {
	QPixmap pixmap(boundingRectangle.width(), boundingRectangle.height());
	if (boundingRectangle.width() == 0 || boundingRectangle.height() == 0) {
		m_pixmap = pixmap;
		m_hoverEffectImageIsDirty = true;
		m_selectionEffectImageIsDirty = true;
		return;
	}
	pixmap.fill(Qt::transparent);
	QPainter painter(&pixmap);
	painter.setRenderHint(QPainter::Antialiasing, true);
	painter.translate(-boundingRectangle.topLeft());

	draw(&painter);
	painter.end();

	m_pixmap = pixmap;
	m_hoverEffectImageIsDirty = true;
	m_selectionEffectImageIsDirty = true;
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

	if ( KSharedConfig::openConfig()->group("Settings_Worksheet").readEntry<bool>("DoubleBuffering", true) )
		painter->drawPixmap(boundingRectangle.topLeft(), m_pixmap); //draw the cached pixmap (fast)
	else
		draw(painter); //draw directly again (slow)

	if (m_hovered && !isSelected() && !q->isPrinting()) {
		if (m_hoverEffectImageIsDirty) {
			QPixmap pix = m_pixmap;
			QPainter p(&pix);
			p.setCompositionMode(QPainter::CompositionMode_SourceIn);	// source (shadow) pixels merged with the alpha channel of the destination (m_pixmap)
			p.fillRect(pix.rect(), QApplication::palette().color(QPalette::Shadow));
			p.end();

			m_hoverEffectImage = ImageTools::blurred(pix.toImage(), m_pixmap.rect(), 5);
			m_hoverEffectImageIsDirty = false;
		}

		painter->drawImage(boundingRectangle.topLeft(), m_hoverEffectImage, m_pixmap.rect());
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

		painter->drawImage(boundingRectangle.topLeft(), m_selectionEffectImage, m_pixmap.rect());
		return;
	}
}

void HistogramPrivate::drawSymbols(QPainter* painter) {
	QPainterPath path = Symbol::stylePath(symbol->style());

	QTransform trafo;
	trafo.scale(symbol->size(), symbol->size());
	if (symbol->rotationAngle() != 0.)
		trafo.rotate(-symbol->rotationAngle());

	path = trafo.map(path);

	for (const auto& point : pointsScene) {
		trafo.reset();
		trafo.translate(point.x(), point.y());
		painter->drawPath(trafo.map(path));
	}
}

void HistogramPrivate::drawValues(QPainter* painter) {
	int i = 0;
	for (const auto& point : qAsConst(valuesPoints)) {
		painter->translate(point);
		if (valuesRotationAngle != 0.)
			painter->rotate(-valuesRotationAngle);

		painter->drawText(QPoint(0, 0), valuesStrings.at(i++));

		if (valuesRotationAngle != 0.)
			painter->rotate(valuesRotationAngle);
		painter->translate(-point);
	}
}

void HistogramPrivate::drawFilling(QPainter* painter) {
	const QRectF& rect = fillPolygon.boundingRect();
	if (fillingType == WorksheetElement::BackgroundType::Color) {
		switch (fillingColorStyle) {
		case WorksheetElement::BackgroundColorStyle::SingleColor: {
			painter->setBrush(QBrush(fillingFirstColor));
			break;
		}
		case WorksheetElement::BackgroundColorStyle::HorizontalLinearGradient: {
			QLinearGradient linearGrad(rect.topLeft(), rect.topRight());
			linearGrad.setColorAt(0, fillingFirstColor);
			linearGrad.setColorAt(1, fillingSecondColor);
			painter->setBrush(QBrush(linearGrad));
			break;
		}
		case WorksheetElement::BackgroundColorStyle::VerticalLinearGradient: {
			QLinearGradient linearGrad(rect.topLeft(), rect.bottomLeft());
			linearGrad.setColorAt(0, fillingFirstColor);
			linearGrad.setColorAt(1, fillingSecondColor);
			painter->setBrush(QBrush(linearGrad));
			break;
		}
		case WorksheetElement::BackgroundColorStyle::TopLeftDiagonalLinearGradient: {
			QLinearGradient linearGrad(rect.topLeft(), rect.bottomRight());
			linearGrad.setColorAt(0, fillingFirstColor);
			linearGrad.setColorAt(1, fillingSecondColor);
			painter->setBrush(QBrush(linearGrad));
			break;
		}
		case WorksheetElement::BackgroundColorStyle::BottomLeftDiagonalLinearGradient: {
			QLinearGradient linearGrad(rect.bottomLeft(), rect.topRight());
			linearGrad.setColorAt(0, fillingFirstColor);
			linearGrad.setColorAt(1, fillingSecondColor);
			painter->setBrush(QBrush(linearGrad));
			break;
		}
		case WorksheetElement::BackgroundColorStyle::RadialGradient: {
			QRadialGradient radialGrad(rect.center(), rect.width()/2);
			radialGrad.setColorAt(0, fillingFirstColor);
			radialGrad.setColorAt(1, fillingSecondColor);
			painter->setBrush(QBrush(radialGrad));
			break;
		}
		}
	} else if (fillingType == WorksheetElement::BackgroundType::Image) {
		if ( !fillingFileName.trimmed().isEmpty() ) {
			QPixmap pix(fillingFileName);
			switch (fillingImageStyle) {
			case WorksheetElement::BackgroundImageStyle::ScaledCropped:
				pix = pix.scaled(rect.size().toSize(),Qt::KeepAspectRatioByExpanding,Qt::SmoothTransformation);
				painter->setBrush(QBrush(pix));
				painter->setBrushOrigin(pix.size().width()/2,pix.size().height()/2);
				break;
			case WorksheetElement::BackgroundImageStyle::Scaled:
				pix = pix.scaled(rect.size().toSize(),Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
				painter->setBrush(QBrush(pix));
				painter->setBrushOrigin(pix.size().width()/2,pix.size().height()/2);
				break;
			case WorksheetElement::BackgroundImageStyle::ScaledAspectRatio:
				pix = pix.scaled(rect.size().toSize(),Qt::KeepAspectRatio,Qt::SmoothTransformation);
				painter->setBrush(QBrush(pix));
				painter->setBrushOrigin(pix.size().width()/2,pix.size().height()/2);
				break;
			case WorksheetElement::BackgroundImageStyle::Centered: {
				QPixmap backpix(rect.size().toSize());
				backpix.fill();
				QPainter p(&backpix);
				p.drawPixmap(QPointF(0,0),pix);
				p.end();
				painter->setBrush(QBrush(backpix));
				painter->setBrushOrigin(-pix.size().width()/2,-pix.size().height()/2);
				break;
			}
			case WorksheetElement::BackgroundImageStyle::Tiled:
				painter->setBrush(QBrush(pix));
				break;
			case WorksheetElement::BackgroundImageStyle::CenterTiled:
				painter->setBrush(QBrush(pix));
				painter->setBrushOrigin(pix.size().width()/2,pix.size().height()/2);
			}
		}
	} else if (fillingType == WorksheetElement::BackgroundType::Pattern)
		painter->setBrush(QBrush(fillingFirstColor,fillingBrushStyle));

	painter->drawPolygon(fillPolygon);
}

void HistogramPrivate::hoverEnterEvent(QGraphicsSceneHoverEvent*) {
	const auto* plot = static_cast<const CartesianPlot*>(q->parentAspect());
	if (plot->mouseMode() == CartesianPlot::MouseMode::Selection && !isSelected()) {
		m_hovered = true;
		Q_EMIT q->hovered();
		update();
	}
}

void HistogramPrivate::hoverLeaveEvent(QGraphicsSceneHoverEvent*) {
	const auto* plot = static_cast<const CartesianPlot*>(q->parentAspect());
	if (plot->mouseMode() == CartesianPlot::MouseMode::Selection && m_hovered) {
		m_hovered = false;
		Q_EMIT q->unhovered();
		update();
	}
}

bool HistogramPrivate::activateCurve(QPointF mouseScenePos, double /*maxDist*/) {
	if (!isVisible())
		return false;

	return curveShape.contains(mouseScenePos);
}

/*!
 * checks if the mousePress event was done near the histogram shape
 * and selects the graphics item if it is the case.
 * \p event
 */
void HistogramPrivate::mousePressEvent(QGraphicsSceneMouseEvent* event) {
	if (static_cast<const CartesianPlot*>(q->parentAspect())->mouseMode()
		!= CartesianPlot::MouseMode::Selection) {
		event->ignore();
		return QGraphicsItem::mousePressEvent(event);
	}

	if(q->activateCurve(event->pos())){
		setSelected(true);
		return;
	}

	event->ignore();
	setSelected(false);
	QGraphicsItem::mousePressEvent(event);
}

/*!
 * Is called in CartesianPlot::hoverMoveEvent where it is determined which curve to hover.
 * \p on
 */
void HistogramPrivate::setHover(bool on) {
	if(on == m_hovered)
		return; // don't update if state not changed

	m_hovered = on;
	on ? Q_EMIT q->hovered() : emit q->unhovered();
	update();
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
//! Save as XML
void Histogram::save(QXmlStreamWriter* writer) const {
	Q_D(const Histogram);

	writer->writeStartElement("Histogram");
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	//general
	writer->writeStartElement("general");
	WRITE_COLUMN(d->dataColumn, dataColumn);
	writer->writeAttribute( "type", QString::number(d->type) );
	writer->writeAttribute( "orientation", QString::number(d->orientation) );
	writer->writeAttribute( "normalization", QString::number(d->normalization) );
	writer->writeAttribute( "binningMethod", QString::number(d->binningMethod) );
	writer->writeAttribute( "binCount", QString::number(d->binCount));
	writer->writeAttribute( "binWidth", QString::number(d->binWidth));
	writer->writeAttribute( "autoBinRanges", QString::number(d->autoBinRanges) );
	writer->writeAttribute( "binRangesMin", QString::number(d->binRangesMin) );
	writer->writeAttribute( "binRangesMax", QString::number(d->binRangesMax) );
	writer->writeAttribute( "plotRangeIndex", QString::number(m_cSystemIndex) );
	writer->writeAttribute( "visible", QString::number(d->isVisible()) );
	writer->writeEndElement();

	//Line
	writer->writeStartElement("line");
	writer->writeAttribute( "type", QString::number(d->lineType) );
	WRITE_QPEN(d->linePen);
	writer->writeAttribute( "opacity", QString::number(d->lineOpacity) );
	writer->writeEndElement();

	//Symbols
	d->symbol->save(writer);

	//Values
	writer->writeStartElement("values");
	writer->writeAttribute( "type", QString::number(d->valuesType) );
	WRITE_COLUMN(d->valuesColumn, valuesColumn);
	writer->writeAttribute( "position", QString::number(d->valuesPosition) );
	writer->writeAttribute( "distance", QString::number(d->valuesDistance) );
	writer->writeAttribute( "rotation", QString::number(d->valuesRotationAngle) );
	writer->writeAttribute( "opacity", QString::number(d->valuesOpacity) );
	writer->writeAttribute("numericFormat", QString(d->valuesNumericFormat));
	writer->writeAttribute("dateTimeFormat", d->valuesDateTimeFormat);
	writer->writeAttribute("precision", QString::number(d->valuesPrecision));
	writer->writeAttribute( "prefix", d->valuesPrefix );
	writer->writeAttribute( "suffix", d->valuesSuffix );
	WRITE_QCOLOR(d->valuesColor);
	WRITE_QFONT(d->valuesFont);
	writer->writeEndElement();

	//Filling
	writer->writeStartElement("filling");
	writer->writeAttribute( "enabled", QString::number(d->fillingEnabled) );
	writer->writeAttribute( "type", QString::number(static_cast<int>(d->fillingType)) );
	writer->writeAttribute( "colorStyle", QString::number(static_cast<int>(d->fillingColorStyle)) );
	writer->writeAttribute( "imageStyle", QString::number(static_cast<int>(d->fillingImageStyle)) );
	writer->writeAttribute( "brushStyle", QString::number(d->fillingBrushStyle) );
	writer->writeAttribute( "firstColor_r", QString::number(d->fillingFirstColor.red()) );
	writer->writeAttribute( "firstColor_g", QString::number(d->fillingFirstColor.green()) );
	writer->writeAttribute( "firstColor_b", QString::number(d->fillingFirstColor.blue()) );
	writer->writeAttribute( "secondColor_r", QString::number(d->fillingSecondColor.red()) );
	writer->writeAttribute( "secondColor_g", QString::number(d->fillingSecondColor.green()) );
	writer->writeAttribute( "secondColor_b", QString::number(d->fillingSecondColor.blue()) );
	writer->writeAttribute( "fileName", d->fillingFileName );
	writer->writeAttribute( "opacity", QString::number(d->fillingOpacity) );
	writer->writeEndElement();

	writer->writeEndElement(); //close "Histogram" section
}

//! Load from XML
bool Histogram::load(XmlStreamReader* reader, bool preview) {
	Q_D(Histogram);

	if (!readBasicAttributes(reader))
		return false;

	KLocalizedString attributeWarning = ki18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == "Histogram")
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == "comment") {
			if (!readCommentElement(reader)) return false;
		} else if (!preview && reader->name() == "general") {
			attribs = reader->attributes();

			READ_COLUMN(dataColumn);
			READ_INT_VALUE("type", type, Histogram::HistogramType);
			READ_INT_VALUE("orientation", orientation, Histogram::HistogramOrientation);
			READ_INT_VALUE("normalization", normalization, Histogram::HistogramNormalization);
			READ_INT_VALUE("binningMethod", binningMethod, Histogram::BinningMethod);
			READ_INT_VALUE("binCount", binCount, int);
			READ_DOUBLE_VALUE("binWidth", binWidth);
			READ_INT_VALUE("autoBinRanges", autoBinRanges, bool);
			READ_DOUBLE_VALUE("binRangesMin", binRangesMin);
			READ_DOUBLE_VALUE("binRangesMax", binRangesMax);

			READ_INT_VALUE_DIRECT("plotRangeIndex", m_cSystemIndex, int);

			str = attribs.value("visible").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("visible").toString());
			else
				d->setVisible(str.toInt());
		} else if (!preview && reader->name() == "line") {
			attribs = reader->attributes();

			READ_INT_VALUE("type", lineType, Histogram::LineType);
			READ_QPEN(d->linePen);
			READ_DOUBLE_VALUE("opacity", lineOpacity);
		} else if (!preview && reader->name() == "symbols") {
			d->symbol->load(reader, preview);
		} else if (!preview && reader->name() == "values") {
			attribs = reader->attributes();

			READ_INT_VALUE("type", valuesType, Histogram::ValuesType);
			READ_COLUMN(valuesColumn);
			READ_INT_VALUE("position", valuesPosition, Histogram::ValuesPosition);
			READ_DOUBLE_VALUE("distance", valuesRotationAngle);
			READ_DOUBLE_VALUE("rotation", valuesRotationAngle);
			READ_DOUBLE_VALUE("opacity", valuesOpacity);

			str = attribs.value("numericFormat").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("numericFormat").toString());
			else
				d->valuesNumericFormat = *(str.toLatin1().data());

			READ_STRING_VALUE("dateTimeFormat", valuesDateTimeFormat);
			READ_INT_VALUE("precision", valuesPrecision, int);

			//don't produce any warning if no prefix or suffix is set (empty string is allowed here in xml)
			d->valuesPrefix = attribs.value("prefix").toString();
			d->valuesSuffix = attribs.value("suffix").toString();

			READ_QCOLOR(d->valuesColor);
			READ_QFONT(d->valuesFont);
		} else if (!preview && reader->name() == "filling") {
			attribs = reader->attributes();

			READ_INT_VALUE("enabled", fillingEnabled, bool);
			READ_INT_VALUE("type", fillingType, WorksheetElement::BackgroundType);
			READ_INT_VALUE("colorStyle", fillingColorStyle, WorksheetElement::BackgroundColorStyle);
			READ_INT_VALUE("imageStyle", fillingImageStyle, WorksheetElement::BackgroundImageStyle);
			READ_INT_VALUE("brushStyle", fillingBrushStyle, Qt::BrushStyle);

			str = attribs.value("firstColor_r").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("firstColor_r").toString());
			else
				d->fillingFirstColor.setRed(str.toInt());

			str = attribs.value("firstColor_g").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("firstColor_g").toString());
			else
				d->fillingFirstColor.setGreen(str.toInt());

			str = attribs.value("firstColor_b").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("firstColor_b").toString());
			else
				d->fillingFirstColor.setBlue(str.toInt());

			str = attribs.value("secondColor_r").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("secondColor_r").toString());
			else
				d->fillingSecondColor.setRed(str.toInt());

			str = attribs.value("secondColor_g").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("secondColor_g").toString());
			else
				d->fillingSecondColor.setGreen(str.toInt());

			str = attribs.value("secondColor_b").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("secondColor_b").toString());
			else
				d->fillingSecondColor.setBlue(str.toInt());

			d->fillingFileName = attribs.value("fileName").toString();
			READ_DOUBLE_VALUE("opacity", fillingOpacity);
		}
	}
	return true;
}

//##############################################################################
//#########################  Theme management ##################################
//##############################################################################
void Histogram::loadThemeConfig(const KConfig& config) {
	KConfigGroup group;
	if (config.hasGroup(QLatin1String("Theme")))
		group = config.group("XYCurve"); //when loading from the theme config, use the same properties as for XYCurve
	else
		group = config.group("Histogram");

	const auto* plot = static_cast<const CartesianPlot*>(parentAspect());
	int index = plot->curveChildIndex(this);
	const QColor themeColor = plot->themeColorPalette(index);

	QPen p;

	Q_D(Histogram);
	d->m_suppressRecalc = true;

	//Line
	p.setStyle((Qt::PenStyle) group.readEntry("LineStyle", (int)Qt::SolidLine));
	p.setWidthF(group.readEntry("LineWidth", Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Point)));
	p.setWidthF(group.readEntry("LineWidth", this->linePen().widthF()));
	p.setColor(themeColor);
	this->setLinePen(p);
	this->setLineOpacity(group.readEntry("LineOpacity", 1.0));

	//Symbol
	d->symbol->loadThemeConfig(group, themeColor);

	//Values
	this->setValuesOpacity(group.readEntry("ValuesOpacity", 1.0));
	this->setValuesColor(group.readEntry("ValuesColor", themeColor));

	//Filling
	this->setFillingBrushStyle((Qt::BrushStyle)group.readEntry("FillingBrushStyle", (int)Qt::SolidPattern));
	this->setFillingColorStyle((WorksheetElement::BackgroundColorStyle)group.readEntry("FillingColorStyle", static_cast<int>(WorksheetElement::BackgroundColorStyle::SingleColor)));
	this->setFillingOpacity(group.readEntry("FillingOpacity", 0.5));
	this->setFillingFirstColor(themeColor);
	this->setFillingSecondColor(group.readEntry("FillingSecondColor", QColor(Qt::black)));
	this->setFillingType((WorksheetElement::BackgroundType)group.readEntry("FillingType", static_cast<int>(WorksheetElement::BackgroundType::Color)));

	//Error Bars
	//TODO:
// 	p.setStyle((Qt::PenStyle)group.readEntry("ErrorBarsStyle",(int) this->errorBarsPen().style()));
// 	p.setWidthF(group.readEntry("ErrorBarsWidth", this->errorBarsPen().widthF()));
// 	p.setColor(themeColor);
// 	this->setErrorBarsPen(p);
// 	this->setErrorBarsOpacity(group.readEntry("ErrorBarsOpacity",this->errorBarsOpacity()));

	if (plot->theme() == QLatin1String("Tufte"))
		setLineType(Histogram::LineType::HalfBars);

	d->m_suppressRecalc = false;
	d->recalcShapeAndBoundingRect();
}

void Histogram::saveThemeConfig(const KConfig& config) {
	KConfigGroup group = config.group("Histogram");

	//Line
	group.writeEntry("LineOpacity", this->lineOpacity());
	group.writeEntry("LineStyle",(int) this->linePen().style());
	group.writeEntry("LineWidth", this->linePen().widthF());

	//Error Bars
// 	group.writeEntry("ErrorBarsCapSize",this->errorBarsCapSize());
// 	group.writeEntry("ErrorBarsOpacity",this->errorBarsOpacity());
// 	group.writeEntry("ErrorBarsColor",(QColor) this->errorBarsPen().color());
// 	group.writeEntry("ErrorBarsStyle",(int) this->errorBarsPen().style());
// 	group.writeEntry("ErrorBarsWidth", this->errorBarsPen().widthF());

	//Filling
	group.writeEntry("FillingBrushStyle",(int) this->fillingBrushStyle());
	group.writeEntry("FillingColorStyle",(int) this->fillingColorStyle());
	group.writeEntry("FillingOpacity", this->fillingOpacity());
	group.writeEntry("FillingSecondColor",(QColor) this->fillingSecondColor());
	group.writeEntry("FillingType",(int) this->fillingType());

	//Symbol
	Q_D(const Histogram);
	d->symbol->saveThemeConfig(group);

	//Values
	group.writeEntry("ValuesOpacity", this->valuesOpacity());
	group.writeEntry("ValuesColor", (QColor) this->valuesColor());
	group.writeEntry("ValuesFont", this->valuesFont());

	int index = parentAspect()->indexOfChild<Histogram>(this);
	if (index < 5) {
		KConfigGroup themeGroup = config.group("Theme");
		for (int i = index; i < 5; i++) {
			QString s = "ThemePaletteColor" + QString::number(i+1);
			themeGroup.writeEntry(s,(QColor) this->linePen().color());
		}
	}
}
