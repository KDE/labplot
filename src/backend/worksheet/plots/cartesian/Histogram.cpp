/***************************************************************************
    File                 : Histogram.cpp
    Project              : LabPlot
    Description          : Histogram
    --------------------------------------------------------------------
    Copyright            : (C) 2016 Anu Mittal (anu22mittal@gmail.com)
    Copyright            : (C) 2016-2018 by Alexander Semke (alexander.semke@web.de)
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

/*!
  \class Histogram
  \brief A 2D-curve, provides an interface for editing many properties of the curve.

  \ingroup worksheet
  */
#include "Histogram.h"
#include "HistogramPrivate.h"
#include "backend/core/column/Column.h"
#include "backend/worksheet/plots/cartesian/CartesianCoordinateSystem.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
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
	: WorksheetElement(name), d_ptr(new HistogramPrivate(this)) {
		init();
}

Histogram::Histogram(const QString &name, HistogramPrivate *dd)
	: WorksheetElement(name), d_ptr(dd) {
		init();
}

void Histogram::init() {
	Q_D(Histogram);

	KConfig config;
	KConfigGroup group = config.group("Histogram");

	d->dataColumn = nullptr;

	d->type = (Histogram::HistogramType) group.readEntry("Type", (int)Histogram::Ordinary);
	d->orientation = (Histogram::HistogramOrientation) group.readEntry("Orientation", (int)Histogram::Vertical);
	d->binningMethod = (Histogram::BinningMethod) group.readEntry("BinningMethod", (int)Histogram::SquareRoot);
	d->binCount = group.readEntry("BinCount", 10);
	d->binWidth = group.readEntry("BinWidth", 1.0f);
	d->autoBinRanges = group.readEntry("AutoBinRanges", true);
	d->binRangesMin = 0.0;
	d->binRangesMax = 1.0;

	d->lineType = (Histogram::LineType) group.readEntry("LineType", (int)Histogram::Bars);
	d->linePen.setStyle( (Qt::PenStyle) group.readEntry("LineStyle", (int)Qt::SolidLine) );
	d->linePen.setColor( group.readEntry("LineColor", QColor(Qt::black)) );
	d->linePen.setWidthF( group.readEntry("LineWidth", Worksheet::convertToSceneUnits(1.0, Worksheet::Point)) );
	d->lineOpacity = group.readEntry("LineOpacity", 1.0);

	d->symbolsStyle = (Symbol::Style)group.readEntry("SymbolStyle", (int)Symbol::NoSymbols);
	d->symbolsSize = group.readEntry("SymbolSize", Worksheet::convertToSceneUnits(5, Worksheet::Point));
	d->symbolsRotationAngle = group.readEntry("SymbolRotation", 0.0);
	d->symbolsOpacity = group.readEntry("SymbolOpacity", 1.0);
	d->symbolsBrush.setStyle( (Qt::BrushStyle)group.readEntry("SymbolFillingStyle", (int)Qt::SolidPattern) );
	d->symbolsBrush.setColor( group.readEntry("SymbolFillingColor", QColor(Qt::black)) );
	d->symbolsPen.setStyle( (Qt::PenStyle)group.readEntry("SymbolBorderStyle", (int)Qt::SolidLine) );
	d->symbolsPen.setColor( group.readEntry("SymbolBorderColor", QColor(Qt::black)) );
	d->symbolsPen.setWidthF( group.readEntry("SymbolBorderWidth", Worksheet::convertToSceneUnits(0.0, Worksheet::Point)) );

	d->valuesType = (Histogram::ValuesType) group.readEntry("ValuesType", (int)Histogram::NoValues);
	d->valuesColumn = nullptr;
	d->valuesPosition = (Histogram::ValuesPosition) group.readEntry("ValuesPosition", (int)Histogram::ValuesAbove);
	d->valuesDistance = group.readEntry("ValuesDistance", Worksheet::convertToSceneUnits(5, Worksheet::Point));
	d->valuesRotationAngle = group.readEntry("ValuesRotation", 0.0);
	d->valuesOpacity = group.readEntry("ValuesOpacity", 1.0);
	d->valuesPrefix = group.readEntry("ValuesPrefix", "");
	d->valuesSuffix = group.readEntry("ValuesSuffix", "");
	d->valuesFont = group.readEntry("ValuesFont", QFont());
	d->valuesFont.setPixelSize( Worksheet::convertToSceneUnits( 8, Worksheet::Point ) );
	d->valuesColor = group.readEntry("ValuesColor", QColor(Qt::black));

	d->fillingEnabled = group.readEntry("FillingEnabled", true);
	d->fillingType = (PlotArea::BackgroundType) group.readEntry("FillingType", (int)PlotArea::Color);
	d->fillingColorStyle = (PlotArea::BackgroundColorStyle) group.readEntry("FillingColorStyle", (int) PlotArea::SingleColor);
	d->fillingImageStyle = (PlotArea::BackgroundImageStyle) group.readEntry("FillingImageStyle", (int) PlotArea::Scaled);
	d->fillingBrushStyle = (Qt::BrushStyle) group.readEntry("FillingBrushStyle", (int) Qt::SolidPattern);
	d->fillingFileName = group.readEntry("FillingFileName", QString());
	d->fillingFirstColor = group.readEntry("FillingFirstColor", QColor(Qt::white));
	d->fillingSecondColor = group.readEntry("FillingSecondColor", QColor(Qt::black));
	d->fillingOpacity = group.readEntry("FillingOpacity", 1.0);

	d->errorType = (Histogram::ErrorType) group.readEntry("ErrorType", (int)Histogram::NoError);
	d->errorBarsType = (XYCurve::ErrorBarsType) group.readEntry("ErrorBarsType", (int)XYCurve::ErrorBarsSimple);
	d->errorBarsCapSize = group.readEntry( "ErrorBarsCapSize", Worksheet::convertToSceneUnits(10, Worksheet::Point) );
	d->errorBarsPen.setStyle( (Qt::PenStyle)group.readEntry("ErrorBarsStyle", (int)Qt::SolidLine) );
	d->errorBarsPen.setColor( group.readEntry("ErrorBarsColor", QColor(Qt::black)) );
	d->errorBarsPen.setWidthF( group.readEntry("ErrorBarsWidth", Worksheet::convertToSceneUnits(1.0, Worksheet::Point)) );
	d->errorBarsOpacity = group.readEntry("ErrorBarsOpacity", 1.0);

	this->initActions();
}

void Histogram::initActions() {
	visibilityAction = new QAction(i18n("Visible"), this);
	visibilityAction->setCheckable(true);
	connect(visibilityAction, &QAction::triggered, this, &Histogram::visibilityChangedSlot);
}

QMenu* Histogram::createContextMenu() {
	QMenu *menu = WorksheetElement::createContextMenu();
	QAction* firstAction = menu->actions().at(1); //skip the first action because of the "title-action"
	visibilityAction->setChecked(isVisible());
	menu->insertAction(firstAction, visibilityAction);
	return menu;
}

/*!
  Returns an icon to be used in the project explorer.
  */
QIcon Histogram::icon() const {
	return QIcon::fromTheme("labplot-xy-curve");
}

QGraphicsItem* Histogram::graphicsItem() const {
	return d_ptr;
}

STD_SWAP_METHOD_SETTER_CMD_IMPL(Histogram, SetVisible, bool, swapVisible)
void Histogram::setVisible(bool on) {
	Q_D(Histogram);
	exec(new HistogramSetVisibleCmd(d, on, on ? ki18n("%1: set visible") : ki18n("%1: set invisible")));
}

bool Histogram::isVisible() const {
	Q_D(const Histogram);
	return d->isVisible();
}

void Histogram::setPrinting(bool on) {
	Q_D(Histogram);
	d->m_printing = on;
}

//##############################################################################
//##########################  getter methods  ##################################
//##############################################################################
//general
BASIC_SHARED_D_READER_IMPL(Histogram, Histogram::HistogramType, type, type)
BASIC_SHARED_D_READER_IMPL(Histogram, Histogram::HistogramOrientation, orientation, orientation)
BASIC_SHARED_D_READER_IMPL(Histogram, Histogram::BinningMethod, binningMethod, binningMethod)
BASIC_SHARED_D_READER_IMPL(Histogram, int, binCount, binCount)
BASIC_SHARED_D_READER_IMPL(Histogram, float, binWidth, binWidth)
BASIC_SHARED_D_READER_IMPL(Histogram, bool, autoBinRanges, autoBinRanges)
BASIC_SHARED_D_READER_IMPL(Histogram, double, binRangesMin, binRangesMin)
BASIC_SHARED_D_READER_IMPL(Histogram, double, binRangesMax, binRangesMax)
BASIC_SHARED_D_READER_IMPL(Histogram, const AbstractColumn*, dataColumn, dataColumn)

QString& Histogram::dataColumnPath() const {
	return d_ptr->dataColumnPath;
}

//line
BASIC_SHARED_D_READER_IMPL(Histogram, Histogram::LineType, lineType, lineType)
CLASS_SHARED_D_READER_IMPL(Histogram, QPen, linePen, linePen)
BASIC_SHARED_D_READER_IMPL(Histogram, qreal, lineOpacity, lineOpacity)

//symbols
BASIC_SHARED_D_READER_IMPL(Histogram, Symbol::Style, symbolsStyle, symbolsStyle)
BASIC_SHARED_D_READER_IMPL(Histogram, qreal, symbolsOpacity, symbolsOpacity)
BASIC_SHARED_D_READER_IMPL(Histogram, qreal, symbolsRotationAngle, symbolsRotationAngle)
BASIC_SHARED_D_READER_IMPL(Histogram, qreal, symbolsSize, symbolsSize)
CLASS_SHARED_D_READER_IMPL(Histogram, QBrush, symbolsBrush, symbolsBrush)
CLASS_SHARED_D_READER_IMPL(Histogram, QPen, symbolsPen, symbolsPen)

//values
BASIC_SHARED_D_READER_IMPL(Histogram, Histogram::ValuesType, valuesType, valuesType)
BASIC_SHARED_D_READER_IMPL(Histogram, const AbstractColumn *, valuesColumn, valuesColumn)
QString& Histogram::valuesColumnPath() const {
	return d_ptr->valuesColumnPath;
}
BASIC_SHARED_D_READER_IMPL(Histogram, Histogram::ValuesPosition, valuesPosition, valuesPosition)
BASIC_SHARED_D_READER_IMPL(Histogram, qreal, valuesDistance, valuesDistance)
BASIC_SHARED_D_READER_IMPL(Histogram, qreal, valuesRotationAngle, valuesRotationAngle)
BASIC_SHARED_D_READER_IMPL(Histogram, qreal, valuesOpacity, valuesOpacity)
CLASS_SHARED_D_READER_IMPL(Histogram, QString, valuesPrefix, valuesPrefix)
CLASS_SHARED_D_READER_IMPL(Histogram, QString, valuesSuffix, valuesSuffix)
CLASS_SHARED_D_READER_IMPL(Histogram, QColor, valuesColor, valuesColor)
CLASS_SHARED_D_READER_IMPL(Histogram, QFont, valuesFont, valuesFont)

//filling
BASIC_SHARED_D_READER_IMPL(Histogram, bool, fillingEnabled, fillingEnabled)
BASIC_SHARED_D_READER_IMPL(Histogram, PlotArea::BackgroundType, fillingType, fillingType)
BASIC_SHARED_D_READER_IMPL(Histogram, PlotArea::BackgroundColorStyle, fillingColorStyle, fillingColorStyle)
BASIC_SHARED_D_READER_IMPL(Histogram, PlotArea::BackgroundImageStyle, fillingImageStyle, fillingImageStyle)
CLASS_SHARED_D_READER_IMPL(Histogram, Qt::BrushStyle, fillingBrushStyle, fillingBrushStyle)
CLASS_SHARED_D_READER_IMPL(Histogram, QColor, fillingFirstColor, fillingFirstColor)
CLASS_SHARED_D_READER_IMPL(Histogram, QColor, fillingSecondColor, fillingSecondColor)
CLASS_SHARED_D_READER_IMPL(Histogram, QString, fillingFileName, fillingFileName)
BASIC_SHARED_D_READER_IMPL(Histogram, qreal, fillingOpacity, fillingOpacity)

//error bars
BASIC_SHARED_D_READER_IMPL(Histogram, Histogram::ErrorType, errorType, errorType)
BASIC_SHARED_D_READER_IMPL(Histogram, XYCurve::ErrorBarsType, errorBarsType, errorBarsType)
BASIC_SHARED_D_READER_IMPL(Histogram, qreal, errorBarsCapSize, errorBarsCapSize)
CLASS_SHARED_D_READER_IMPL(Histogram, QPen, errorBarsPen, errorBarsPen)
BASIC_SHARED_D_READER_IMPL(Histogram, qreal, errorBarsOpacity, errorBarsOpacity)

double Histogram::getYMaximum() const {
	return d_ptr->getYMaximum();
}

double Histogram::getYMinimum() const {
	return d_ptr->getYMinimum();
}

double Histogram::getXMaximum() const {
	return d_ptr->getXMaximum();
}

double Histogram::getXMinimum() const {
	return d_ptr->getXMinimum();
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
		if (m_autoBinRanges) {
			m_binRangesMinOld = m_private->binRangesMin;
			m_binRangesMaxOld = m_private->binRangesMax;
			m_private->q->recalcHistogram();
		}
		m_private->autoBinRanges = m_autoBinRanges;
		emit m_private->q->autoBinRangesChanged(m_autoBinRanges);
	};

	void undo() override {
		if (!m_autoBinRangesOld) {
			m_private->binRangesMin = m_binRangesMinOld;
			m_private->binRangesMax = m_binRangesMaxOld;
			m_private->recalcHistogram();
		}
		m_private->autoBinRanges = m_autoBinRangesOld;
		emit m_private->q->autoBinRangesChanged(m_autoBinRangesOld);
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

STD_SETTER_CMD_IMPL_F_S(Histogram, SetLineOpacity, qreal, lineOpacity, updatePixmap);
void Histogram::setLineOpacity(qreal opacity) {
	Q_D(Histogram);
	if (opacity != d->lineOpacity)
		exec(new HistogramSetLineOpacityCmd(d, opacity, ki18n("%1: set line opacity")));
}

// Symbols
STD_SETTER_CMD_IMPL_F_S(Histogram, SetSymbolsStyle, Symbol::Style, symbolsStyle, updateSymbols)
void Histogram::setSymbolsStyle(Symbol::Style style) {
	Q_D(Histogram);
	if (style != d->symbolsStyle)
		exec(new HistogramSetSymbolsStyleCmd(d, style, ki18n("%1: set symbol style")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetSymbolsSize, qreal, symbolsSize, updateSymbols)
void Histogram::setSymbolsSize(qreal size) {
	Q_D(Histogram);
	if (!qFuzzyCompare(1 + size, 1 + d->symbolsSize))
		exec(new HistogramSetSymbolsSizeCmd(d, size, ki18n("%1: set symbol size")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetSymbolsRotationAngle, qreal, symbolsRotationAngle, updateSymbols)
void Histogram::setSymbolsRotationAngle(qreal angle) {
	Q_D(Histogram);
	if (!qFuzzyCompare(1 + angle, 1 + d->symbolsRotationAngle))
		exec(new HistogramSetSymbolsRotationAngleCmd(d, angle, ki18n("%1: rotate symbols")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetSymbolsBrush, QBrush, symbolsBrush, updatePixmap)
void Histogram::setSymbolsBrush(const QBrush &brush) {
	Q_D(Histogram);
	if (brush != d->symbolsBrush)
		exec(new HistogramSetSymbolsBrushCmd(d, brush, ki18n("%1: set symbol filling")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetSymbolsPen, QPen, symbolsPen, updateSymbols)
void Histogram::setSymbolsPen(const QPen &pen) {
	Q_D(Histogram);
	if (pen != d->symbolsPen)
		exec(new HistogramSetSymbolsPenCmd(d, pen, ki18n("%1: set symbol outline style")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetSymbolsOpacity, qreal, symbolsOpacity, updatePixmap)
void Histogram::setSymbolsOpacity(qreal opacity) {
	Q_D(Histogram);
	if (opacity != d->symbolsOpacity)
		exec(new HistogramSetSymbolsOpacityCmd(d, opacity, ki18n("%1: set symbols opacity")));
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

//TODO: Format, Precision

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

STD_SETTER_CMD_IMPL_F_S(Histogram, SetFillingType, PlotArea::BackgroundType, fillingType, updatePixmap)
void Histogram::setFillingType(PlotArea::BackgroundType type) {
	Q_D(Histogram);
	if (type != d->fillingType)
		exec(new HistogramSetFillingTypeCmd(d, type, ki18n("%1: filling type changed")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetFillingColorStyle, PlotArea::BackgroundColorStyle, fillingColorStyle, updatePixmap)
void Histogram::setFillingColorStyle(PlotArea::BackgroundColorStyle style) {
	Q_D(Histogram);
	if (style != d->fillingColorStyle)
		exec(new HistogramSetFillingColorStyleCmd(d, style, ki18n("%1: filling color style changed")));
}

STD_SETTER_CMD_IMPL_F_S(Histogram, SetFillingImageStyle, PlotArea::BackgroundImageStyle, fillingImageStyle, updatePixmap)
void Histogram::setFillingImageStyle(PlotArea::BackgroundImageStyle style) {
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
	d_ptr->recalcHistogram();
}

//TODO
void Histogram::handleResize(double horizontalRatio, double verticalRatio, bool pageResize) {
	Q_UNUSED(pageResize);
	Q_UNUSED(verticalRatio);
	Q_D(const Histogram);

	//setValuesDistance(d->distance*);
	QFont font=d->valuesFont;
	font.setPointSizeF(font.pointSizeF()*horizontalRatio);
	setValuesFont(font);

	retransform();
}

void Histogram::updateValues() {
	d_ptr->updateValues();
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
//######  SLOTs for changes triggered via QActions in the context menu  ########
//##############################################################################
void Histogram::visibilityChangedSlot() {
	Q_D(const Histogram);
	this->setVisible(!d->isVisible());
}

//##############################################################################
//######################### Private implementation #############################
//##############################################################################
HistogramPrivate::HistogramPrivate(Histogram *owner) : q(owner) {
	setFlag(QGraphicsItem::ItemIsSelectable, true);
	setAcceptHoverEvents(true);
}

QString HistogramPrivate::name() const {
	return q->name();
}

QRectF HistogramPrivate::boundingRect() const {
	return boundingRectangle;
}

double HistogramPrivate::getMaximumOccuranceofHistogram() {
	if (m_histogram) {
		double yMaxRange = -INFINITY;
		switch(type) {
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
		return yMaxRange;
	}

	return -INFINITY;
}

double HistogramPrivate::getXMinimum() {
	switch(orientation) {
	case Histogram::Vertical:
		return autoBinRanges ? dataColumn->minimum() : binRangesMin;
	case Histogram::Horizontal:
		return 0;
	}
	return INFINITY;
}

double HistogramPrivate::getXMaximum() {
	switch(orientation) {
	case Histogram::Vertical:
		return autoBinRanges ? dataColumn->maximum() : binRangesMax;
	case Histogram::Horizontal:
		return getMaximumOccuranceofHistogram();
	}
	return -INFINITY;
}

double HistogramPrivate::getYMinimum() {
	switch(orientation) {
	case Histogram::Vertical:
		return 0;
	case Histogram::Horizontal:
		return autoBinRanges ? dataColumn->minimum() : binRangesMin;
	}
	return INFINITY;
}

double HistogramPrivate::getYMaximum() {
	switch(orientation) {
	case Histogram::Vertical:
		return getMaximumOccuranceofHistogram();
	case Histogram::Horizontal:
		return autoBinRanges ? dataColumn->maximum() : binRangesMax;
	}
	return INFINITY;
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

bool HistogramPrivate::swapVisible(bool on) {
	bool oldValue = isVisible();
	setVisible(on);
	emit q->visibilityChanged(on);
	return oldValue;
}

/*!
  Triggers the update of lines, drop lines, symbols etc.
  */
void HistogramPrivate::retransform() {
	if (m_suppressRetransform)
		return;

	PERFTRACE(name().toLatin1() + ", HistogramPrivate::retransform()");

	symbolPointsScene.clear();
	symbolPointsLogical.clear();

	if (!dataColumn) {
		linePath = QPainterPath();
		symbolsPath = QPainterPath();
		valuesPath = QPainterPath();
		curveShape = QPainterPath();
		lines.clear();
		symbolPointsLogical.clear();
		symbolPointsScene.clear();
		visiblePoints.clear();
		valuesPoints.clear();
		valuesStrings.clear();
		fillPolygons.clear();
		recalcShapeAndBoundingRect();
		return;
	}

	m_suppressRecalc = true;
	updateLines();
	updateSymbols();
	updateValues();
	m_suppressRecalc = false;
}

void HistogramPrivate::recalcHistogram() {
	PERFTRACE(name().toLatin1() + ", HistogramPrivate::recalcHistogram()");

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
				emit q->binRangesMinChanged(binRangesMin);
			}

			if (binRangesMax != dataColumn->maximum()) {
				binRangesMax = dataColumn->maximum();
				emit q->binRangesMaxChanged(binRangesMax);
			}
		} else {
			if (binRangesMin >= binRangesMax) {
				emit q->dataChanged();
				return;
			}
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
			const double skewness = dynamic_cast<const Column*>(dataColumn)->statistics().skewness;
			m_bins = (size_t)( 1 + log2(count) + log2(1 + abs(skewness)/sqrt((double)6*(count-2)/(count+1)/(count+3))) );
			break;
		}
		case Histogram::Scott: {
			const double sigma = dynamic_cast<const Column*>(dataColumn)->statistics().standardDeviation;
			const double width = 3.5*sigma/cbrt(count);
			DEBUG("blablub " << sigma << "  " << width << "  " <<(binRangesMax - binRangesMin)/width);
			m_bins = (size_t)(binRangesMax - binRangesMin)/width;
			break;
		}
		}

		DEBUG("min " << binRangesMin);
		DEBUG("max " << binRangesMax);
		DEBUG("number of bins " << m_bins);

		//calculate the histogram
		if (m_bins > 0) {
			m_histogram = gsl_histogram_alloc (m_bins);
			gsl_histogram_set_ranges_uniform (m_histogram, binRangesMin, binRangesMax+1);

			for (int row = 0; row < dataColumn->rowCount(); ++row) {
				if ( dataColumn->isValid(row) && !dataColumn->isMasked(row) )
					gsl_histogram_increment(m_histogram, dataColumn->valueAt(row));
			}
		} else
			DEBUG("Number of bins must be positiv integer");
	}

	//histogram changed because of the actual data changes or because of new bin settings,
	//emit dataChanged() in order to recalculate everything with the new size/shape of the histogram
	emit q->dataChanged();
}

void HistogramPrivate::updateType() {
	//type (ordinary or cumulative) changed,
	//emit dataChanged() in order to recalculate everything with the new size/shape of the histogram
	emit q->dataChanged();
}

void HistogramPrivate::updateOrientation() {
	//orientation (horizontal or vertical) changed
	//emit dataChanged() in order to recalculate everything with the new size/shape of the histogram
	emit q->dataChanged();
}

/*!
  recalculates the painter path for the lines connecting the data points.
  Called each time when the type of this connection is changed.
  */
void HistogramPrivate::updateLines() {
	PERFTRACE(name().toLatin1() + ", HistogramPrivate::updateLines()");

	linePath = QPainterPath();
	lines.clear();
	symbolPointsLogical.clear();

	if (orientation == Histogram::Vertical)
		verticalHistogram();
	else
		horizontalHistogram();

	//map the lines and the symbol positions to the scene coordinates
	const auto* plot = dynamic_cast<const CartesianPlot*>(q->parentAspect());
	const auto* cSystem = dynamic_cast<const CartesianCoordinateSystem*>(plot->coordinateSystem());
	Q_ASSERT(cSystem);
	lines = cSystem->mapLogicalToScene(lines);
	visiblePoints = std::vector<bool>(symbolPointsLogical.count(), false);
	cSystem->mapLogicalToScene(symbolPointsLogical, symbolPointsScene, visiblePoints);

	//new line path
	for (const auto& line : lines) {
		linePath.moveTo(line.p1());
		linePath.lineTo(line.p2());
	}

	updateFilling();
	recalcShapeAndBoundingRect();
}

void HistogramPrivate::verticalHistogram() {
	if (!m_histogram)
		return;

	const double min = gsl_histogram_min(m_histogram);
	const double max = gsl_histogram_max(m_histogram);
	const double width = (max - min)/m_bins;
	double value = 0.;
	if (lineType == Histogram::Bars) {
		for (size_t i = 0; i < m_bins; ++i) {
			if (type == Histogram::Ordinary)
				value = gsl_histogram_get(m_histogram, i);
			else
				value += gsl_histogram_get(m_histogram, i);

			const double x = min + i*width;
			lines.append(QLineF(x, 0., x, value));
			lines.append(QLineF(x, value, x + width, value));
			lines.append(QLineF(x + width, value, x + width, 0.));
			symbolPointsLogical.append(QPointF(x+width/2, value));
		}
	} else if (lineType == Histogram::NoLine || lineType == Histogram::Envelope) {
		double prevValue = 0.;
		for (size_t i = 0; i < m_bins; ++i) {
			if (type == Histogram::Ordinary)
				value = gsl_histogram_get(m_histogram, i);
			else
				value += gsl_histogram_get(m_histogram, i);

			const double x = min + i*width;
			lines.append(QLineF(x, prevValue, x, value));
			lines.append(QLineF(x, value, x + width, value));
			symbolPointsLogical.append(QPointF(x+width/2, value));

			if (i== m_bins - 1)
				lines.append(QLineF(x + width, value, x + width, 0.));

			prevValue = value;
		}
	} else { //drop lines
		for (size_t i = 0; i < m_bins; ++i) {
			if (type == Histogram::Ordinary)
				value = gsl_histogram_get(m_histogram, i);
			else
				value += gsl_histogram_get(m_histogram, i);

			const double x = min + i*width - width/2;
			lines.append(QLineF(x, 0., x, value));
			symbolPointsLogical.append(QPointF(x, value));
		}
	}

	lines.append(QLineF(min, 0., max, 0.));
}

void HistogramPrivate::horizontalHistogram() {
	if (!m_histogram)
		return;

	const double min = gsl_histogram_min(m_histogram);
	const double max = gsl_histogram_max(m_histogram);
	const double width = (max - min)/m_bins;
	double value = 0.;
	if (lineType == Histogram::Bars) {
		for (size_t i = 0; i < m_bins; ++i) {
			if (type == Histogram::Ordinary)
				value = gsl_histogram_get(m_histogram,i);
			else
				value += gsl_histogram_get(m_histogram,i);

			const double y = min + i*width;
			lines.append(QLineF(0., y, value, y));
			lines.append(QLineF(value, y, value, y + width));
			lines.append(QLineF(value, y + width, 0., y + width));
			symbolPointsLogical.append(QPointF(value, y+width/2));
		}
	} else if (lineType == Histogram::NoLine || lineType == Histogram::Envelope) {
		double prevValue = 0.;
		for (size_t i = 0; i < m_bins; ++i) {
			if (type == Histogram::Ordinary)
				value = gsl_histogram_get(m_histogram, i);
			else
				value += gsl_histogram_get(m_histogram, i);

			const double y = min + i*width;
			lines.append(QLineF(prevValue, y, value, y));
			lines.append(QLineF(value, y, value, y + width));
			symbolPointsLogical.append(QPointF(value, y+width/2));

			if (i== m_bins - 1)
				lines.append(QLineF(value, y + width, 0., y + width));

			prevValue = value;
		}
	} else { //drop lines
		for (size_t i = 0; i < m_bins; ++i) {
			if (type == Histogram::Ordinary)
				value = gsl_histogram_get(m_histogram, i);
			else
				value += gsl_histogram_get(m_histogram, i);

			const double y = min + i*width - width/2;
			lines.append(QLineF(0., y, value, y));
			symbolPointsLogical.append(QPointF(value, y));
		}
	}

	lines.append(QLineF(0., min, 0., max));
}

void HistogramPrivate::updateSymbols() {
	symbolsPath = QPainterPath();
	if (symbolsStyle != Symbol::NoSymbols) {
		QPainterPath path = Symbol::pathFromStyle(symbolsStyle);

		QTransform trafo;
		trafo.scale(symbolsSize, symbolsSize);
		path = trafo.map(path);
		trafo.reset();

		if (symbolsRotationAngle != 0) {
			trafo.rotate(symbolsRotationAngle);
			path = trafo.map(path);
		}

		for (const auto& point : symbolPointsScene) {
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
		switch(type) {
		case Histogram::Ordinary:
			for (size_t i=0; i<m_bins; ++i) {
				if (!visiblePoints[i]) continue;
				valuesStrings << valuesPrefix + QString::number(gsl_histogram_get(m_histogram, i)) + valuesSuffix;
			}
			break;
		case Histogram::Cumulative: {
			value = 0;
			for (size_t i=0; i<m_bins; ++i) {
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

		const int endRow = qMin(symbolPointsLogical.size(), valuesColumn->rowCount());
		const AbstractColumn::ColumnMode xColMode = valuesColumn->columnMode();
		for (int i = 0; i < endRow; ++i) {
			if (!visiblePoints[i]) continue;

			if ( !valuesColumn->isValid(i) || valuesColumn->isMasked(i) )
				continue;

			switch (xColMode) {
				case AbstractColumn::Numeric:
					valuesStrings << valuesPrefix + QString::number(valuesColumn->valueAt(i)) + valuesSuffix;
					break;
				case AbstractColumn::Text:
					valuesStrings << valuesPrefix + valuesColumn->textAt(i) + valuesSuffix;
				case AbstractColumn::Integer:
				case AbstractColumn::DateTime:
				case AbstractColumn::Month:
				case AbstractColumn::Day:
					//TODO
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
	switch(valuesPosition) {
		case Histogram::ValuesAbove:
			for (int i = 0; i < valuesStrings.size(); i++) {
				w = fm.width(valuesStrings.at(i));
				tempPoint.setX( symbolPointsScene.at(i).x() -w/2);
				tempPoint.setY( symbolPointsScene.at(i).y() - valuesDistance );
				valuesPoints.append(tempPoint);
			}
			break;
		case Histogram::ValuesUnder:
			for (int i = 0; i < valuesStrings.size(); i++) {
				w = fm.width(valuesStrings.at(i));
				tempPoint.setX( symbolPointsScene.at(i).x() -w/2);
				tempPoint.setY( symbolPointsScene.at(i).y() + valuesDistance + h/2);
				valuesPoints.append(tempPoint);
			}
			break;
		case Histogram::ValuesLeft:
			for (int i = 0; i < valuesStrings.size(); i++) {
				w = fm.width(valuesStrings.at(i));
				tempPoint.setX( symbolPointsScene.at(i).x() - valuesDistance - w - 1);
				tempPoint.setY( symbolPointsScene.at(i).y());
				valuesPoints.append(tempPoint);
			}
			break;
		case Histogram::ValuesRight:
			for (int i = 0; i < valuesStrings.size(); i++) {
				tempPoint.setX( symbolPointsScene.at(i).x() + valuesDistance - 1);
				tempPoint.setY( symbolPointsScene.at(i).y() );
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
		if (valuesRotationAngle!=0)
			trafo.rotate( -valuesRotationAngle );

		valuesPath.addPath(trafo.map(path));
	}

	recalcShapeAndBoundingRect();
}

void HistogramPrivate::updateFilling() {
	fillPolygons.clear();

	if (!fillingEnabled || lineType == Histogram::DropLines) {
		recalcShapeAndBoundingRect();
		return;
	}

	QVector<QLineF> fillLines;
	const auto* plot = dynamic_cast<const CartesianPlot*>(q->parentAspect());
	const AbstractCoordinateSystem* cSystem = plot->coordinateSystem();

	//if there're no interpolation lines available (Histogram::NoLine selected), create line-interpolation,
	//use already available lines otherwise.
	if (lines.size())
		fillLines = lines;
	else {
		for (int i = 0; i < symbolPointsLogical.count()-1; i++)
			fillLines.append(QLineF(symbolPointsLogical.at(i), symbolPointsLogical.at(i+1)));
		fillLines = cSystem->mapLogicalToScene(fillLines);
	}

	//no lines available (no points), nothing to do
	if (!fillLines.size())
		return;


	//create polygon(s):
	//1. Depending on the current zoom-level, only a subset of the curve may be visible in the plot
	//and more of the filling area should be shown than the area defined by the start and end points of the currently visible points.
	//We check first whether the curve crosses the boundaries of the plot and determine new start and end points and put them to the boundaries.
	//2. Furthermore, depending on the current filling type we determine the end point (x- or y-coordinate) where all polygons are closed at the end.
	QPolygonF pol;
	QPointF start = fillLines.at(0).p1(); //starting point of the current polygon, initialize with the first visible point
	QPointF end = fillLines.at(fillLines.size()-1).p2(); //starting point of the current polygon, initialize with the last visible point
	const QPointF& first = symbolPointsLogical.at(0); //first point of the curve, may not be visible currently
	const QPointF& last = symbolPointsLogical.at(symbolPointsLogical.size()-1);//first point of the curve, may not be visible currently
	QPointF edge;
	float yEnd=0;

	//fillingPosition == Histogram::FillingZeroBaseline)
	edge = cSystem->mapLogicalToScene(QPointF(plot->xMin(), plot->yMax()));

	//start point
	if (AbstractCoordinateSystem::essentiallyEqual(start.y(), edge.y())) {
		if (plot->yMax()>0) {
			if (first.x() < plot->xMin())
				start = edge;
			else if (first.x() > plot->xMax())
				start = cSystem->mapLogicalToScene(QPointF(plot->xMax(), plot->yMax()));
			else
				start = cSystem->mapLogicalToScene(QPointF(first.x(), plot->yMax()));
		} else {
			if (first.x() < plot->xMin())
				start = edge;
			else if (first.x() > plot->xMax())
				start = cSystem->mapLogicalToScene(QPointF(plot->xMax(), plot->yMin()));
			else
				start = cSystem->mapLogicalToScene(QPointF(first.x(), plot->yMin()));
		}
	}

	//end point
	if (AbstractCoordinateSystem::essentiallyEqual(end.y(), edge.y())) {
		if (plot->yMax()>0) {
			if (last.x() < plot->xMin())
				end = edge;
			else if (last.x() > plot->xMax())
				end = cSystem->mapLogicalToScene(QPointF(plot->xMax(), plot->yMax()));
			else
				end = cSystem->mapLogicalToScene(QPointF(last.x(), plot->yMax()));
		} else {
			if (last.x() < plot->xMin())
				end = edge;
			else if (last.x() > plot->xMax())
				end = cSystem->mapLogicalToScene(QPointF(plot->xMax(), plot->yMin()));
			else
				end = cSystem->mapLogicalToScene(QPointF(last.x(), plot->yMin()));
		}
	}

	yEnd = cSystem->mapLogicalToScene(QPointF(plot->xMin(), plot->yMin()>0 ? plot->yMin() : 0)).y();

	if (start != fillLines.at(0).p1())
		pol << start;

	QPointF p1, p2;
	for (int i=0; i<fillLines.size(); ++i) {
		const QLineF& line = fillLines.at(i);
		p1 = line.p1();
		p2 = line.p2();
		if (i!=0 && p1!=fillLines.at(i-1).p2())
			pol << fillLines.at(i-1).p2() << p1;

		pol << p1 << p2;
	}

	if (p2!=end)
		pol << end;

	//close the last polygon
	pol << QPointF(end.x(), yEnd);
	pol << QPointF(start.x(), yEnd);

	fillPolygons << pol;
	recalcShapeAndBoundingRect();
}

void HistogramPrivate::updateErrorBars() {

}

/*!
  recalculates the outer bounds and the shape of the curve.
  */
void HistogramPrivate::recalcShapeAndBoundingRect() {
	//if (m_suppressRecalc)
	//	return;

	prepareGeometryChange();
	curveShape = QPainterPath();
	if (lineType != Histogram::NoLine)
		curveShape.addPath(WorksheetElement::shapeFromPath(linePath, linePen));

	if (symbolsStyle != Symbol::NoSymbols)
		curveShape.addPath(symbolsPath);

	if (valuesType != Histogram::NoValues)
		curveShape.addPath(valuesPath);

	boundingRectangle = curveShape.boundingRect();

	for (const auto& pol : fillPolygons)
		boundingRectangle = boundingRectangle.united(pol.boundingRect());

	//TODO: when the selection is painted, line intersections are visible.
	//simplified() removes those artifacts but is horrible slow for curves with large number of points.
	//search for an alternative.
	//curveShape = curveShape.simplified();

	updatePixmap();
}

void HistogramPrivate::draw(QPainter* painter) {
	PERFTRACE(name().toLatin1() + ", HistogramPrivate::draw()");

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
	if (symbolsStyle != Symbol::NoSymbols) {
		painter->setOpacity(symbolsOpacity);
		painter->setPen(symbolsPen);
		painter->setBrush(symbolsBrush);
		drawSymbols(painter);
	}

	//draw values
	if (valuesType != Histogram::NoValues) {
		painter->setOpacity(valuesOpacity);
		painter->setPen(valuesColor);
		painter->setBrush(Qt::SolidPattern);
		drawValues(painter);
	}
}

void HistogramPrivate::updatePixmap() {
	QPixmap pixmap(boundingRectangle.width(), boundingRectangle.height());
	if (boundingRectangle.width()==0 || boundingRectangle.width()==0) {
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
}

/*!
  Reimplementation of QGraphicsItem::paint(). This function does the actual painting of the curve.
  \sa QGraphicsItem::paint().
  */
void HistogramPrivate::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
	Q_UNUSED(option);
	Q_UNUSED(widget);
	if (!isVisible())
		return;

	painter->setPen(Qt::NoPen);
	painter->setBrush(Qt::NoBrush);
	painter->setRenderHint(QPainter::SmoothPixmapTransform, true);

	if ( KSharedConfig::openConfig()->group("Settings_Worksheet").readEntry<bool>("DoubleBuffering", true) )
		painter->drawPixmap(boundingRectangle.topLeft(), m_pixmap); //draw the cached pixmap (fast)
	else
		draw(painter); //draw directly again (slow)

	if (m_hovered && !isSelected() && !m_printing) {
		if (m_hoverEffectImageIsDirty) {
			QPixmap pix = m_pixmap;
			pix.fill(QApplication::palette().color(QPalette::Shadow));
			pix.setAlphaChannel(m_pixmap.alphaChannel());
			m_hoverEffectImage = ImageTools::blurred(pix.toImage(), m_pixmap.rect(), 5);
			m_hoverEffectImageIsDirty = false;
		}

		painter->drawImage(boundingRectangle.topLeft(), m_hoverEffectImage, m_pixmap.rect());
		return;
	}

	if (isSelected() && !m_printing) {
		if (m_selectionEffectImageIsDirty) {
			QPixmap pix = m_pixmap;
			pix.fill(QApplication::palette().color(QPalette::Highlight));
			pix.setAlphaChannel(m_pixmap.alphaChannel());
			m_selectionEffectImage = ImageTools::blurred(pix.toImage(), m_pixmap.rect(), 5);
			m_selectionEffectImageIsDirty = false;
		}

		painter->drawImage(boundingRectangle.topLeft(), m_selectionEffectImage, m_pixmap.rect());
		return;
	}
}

void HistogramPrivate::drawSymbols(QPainter* painter) {
	QPainterPath path = Symbol::pathFromStyle(symbolsStyle);

	QTransform trafo;
	trafo.scale(symbolsSize, symbolsSize);
	path = trafo.map(path);
	trafo.reset();
	if (symbolsRotationAngle != 0) {
		trafo.rotate(-symbolsRotationAngle);
		path = trafo.map(path);
	}
	for (const auto& point : symbolPointsScene) {
		trafo.reset();
		trafo.translate(point.x(), point.y());
		painter->drawPath(trafo.map(path));
	}
}

void HistogramPrivate::drawValues(QPainter* painter) {
	QTransform trafo;
	QPainterPath path;
	for (int i=0; i<valuesPoints.size(); i++) {
		path = QPainterPath();
		path.addText( QPoint(0,0), valuesFont, valuesStrings.at(i) );

		trafo.reset();
		trafo.translate( valuesPoints.at(i).x(), valuesPoints.at(i).y() );
		if (valuesRotationAngle!=0)
			trafo.rotate(-valuesRotationAngle );

		painter->drawPath(trafo.map(path));
	}
}

void HistogramPrivate::drawFilling(QPainter* painter) {
	for (const auto& pol : fillPolygons) {
		QRectF rect = pol.boundingRect();
		if (fillingType == PlotArea::Color) {
			switch (fillingColorStyle) {
			case PlotArea::SingleColor: {
				painter->setBrush(QBrush(fillingFirstColor));
				break;
			}
			case PlotArea::HorizontalLinearGradient: {
				QLinearGradient linearGrad(rect.topLeft(), rect.topRight());
				linearGrad.setColorAt(0, fillingFirstColor);
				linearGrad.setColorAt(1, fillingSecondColor);
				painter->setBrush(QBrush(linearGrad));
				break;
			}
			case PlotArea::VerticalLinearGradient: {
				QLinearGradient linearGrad(rect.topLeft(), rect.bottomLeft());
				linearGrad.setColorAt(0, fillingFirstColor);
				linearGrad.setColorAt(1, fillingSecondColor);
				painter->setBrush(QBrush(linearGrad));
				break;
			}
			case PlotArea::TopLeftDiagonalLinearGradient: {
				QLinearGradient linearGrad(rect.topLeft(), rect.bottomRight());
				linearGrad.setColorAt(0, fillingFirstColor);
				linearGrad.setColorAt(1, fillingSecondColor);
				painter->setBrush(QBrush(linearGrad));
				break;
			}
			case PlotArea::BottomLeftDiagonalLinearGradient: {
				QLinearGradient linearGrad(rect.bottomLeft(), rect.topRight());
				linearGrad.setColorAt(0, fillingFirstColor);
				linearGrad.setColorAt(1, fillingSecondColor);
				painter->setBrush(QBrush(linearGrad));
				break;
			}
			case PlotArea::RadialGradient: {
				QRadialGradient radialGrad(rect.center(), rect.width()/2);
				radialGrad.setColorAt(0, fillingFirstColor);
				radialGrad.setColorAt(1, fillingSecondColor);
				painter->setBrush(QBrush(radialGrad));
				break;
			}
			}
		} else if (fillingType == PlotArea::Image) {
			if ( !fillingFileName.trimmed().isEmpty() ) {
				QPixmap pix(fillingFileName);
				switch (fillingImageStyle) {
				case PlotArea::ScaledCropped:
					pix = pix.scaled(rect.size().toSize(),Qt::KeepAspectRatioByExpanding,Qt::SmoothTransformation);
					painter->setBrush(QBrush(pix));
					painter->setBrushOrigin(pix.size().width()/2,pix.size().height()/2);
					break;
				case PlotArea::Scaled:
					pix = pix.scaled(rect.size().toSize(),Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
					painter->setBrush(QBrush(pix));
					painter->setBrushOrigin(pix.size().width()/2,pix.size().height()/2);
					break;
				case PlotArea::ScaledAspectRatio:
					pix = pix.scaled(rect.size().toSize(),Qt::KeepAspectRatio,Qt::SmoothTransformation);
					painter->setBrush(QBrush(pix));
					painter->setBrushOrigin(pix.size().width()/2,pix.size().height()/2);
					break;
				case PlotArea::Centered: {
					QPixmap backpix(rect.size().toSize());
					backpix.fill();
					QPainter p(&backpix);
					p.drawPixmap(QPointF(0,0),pix);
					p.end();
					painter->setBrush(QBrush(backpix));
					painter->setBrushOrigin(-pix.size().width()/2,-pix.size().height()/2);
					break;
				}
				case PlotArea::Tiled:
					painter->setBrush(QBrush(pix));
					break;
				case PlotArea::CenterTiled:
					painter->setBrush(QBrush(pix));
					painter->setBrushOrigin(pix.size().width()/2,pix.size().height()/2);
				}
			}
		} else if (fillingType == PlotArea::Pattern)
			painter->setBrush(QBrush(fillingFirstColor,fillingBrushStyle));

		painter->drawPolygon(pol);
	}
}

void HistogramPrivate::hoverEnterEvent(QGraphicsSceneHoverEvent*) {
	const auto* plot = dynamic_cast<const CartesianPlot*>(q->parentAspect());
	if (plot->mouseMode() == CartesianPlot::SelectionMode && !isSelected()) {
		m_hovered = true;
		emit q->hovered();
		update();
	}
}

void HistogramPrivate::hoverLeaveEvent(QGraphicsSceneHoverEvent*) {
	const auto* plot = dynamic_cast<const CartesianPlot*>(q->parentAspect());
	if (plot->mouseMode() == CartesianPlot::SelectionMode && m_hovered) {
		m_hovered = false;
		emit q->unhovered();
		update();
	}
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
	writer->writeAttribute( "binningMethod", QString::number(d->binningMethod) );
	writer->writeAttribute( "binCount", QString::number(d->binCount));
	writer->writeAttribute( "binWidth", QString::number(d->binWidth));
	writer->writeAttribute( "autoBinRanges", QString::number(d->autoBinRanges) );
	writer->writeAttribute( "binRangesMin", QString::number(d->binRangesMin) );
	writer->writeAttribute( "binRangesMax", QString::number(d->binRangesMax) );
	writer->writeAttribute( "visible", QString::number(d->isVisible()) );
	writer->writeEndElement();

	//Line
	writer->writeStartElement("line");
	writer->writeAttribute( "type", QString::number(d->lineType) );
	WRITE_QPEN(d->linePen);
	writer->writeAttribute( "opacity", QString::number(d->lineOpacity) );
	writer->writeEndElement();

	//Symbols
	writer->writeStartElement( "symbols" );
	writer->writeAttribute( "symbolsStyle", QString::number(d->symbolsStyle) );
	writer->writeAttribute( "opacity", QString::number(d->symbolsOpacity) );
	writer->writeAttribute( "rotation", QString::number(d->symbolsRotationAngle) );
	writer->writeAttribute( "size", QString::number(d->symbolsSize) );
	WRITE_QBRUSH(d->symbolsBrush);
	WRITE_QPEN(d->symbolsPen);
	writer->writeEndElement();

	//Values
	writer->writeStartElement("values");
	writer->writeAttribute( "type", QString::number(d->valuesType) );
	WRITE_COLUMN(d->valuesColumn, valuesColumn);
	writer->writeAttribute( "position", QString::number(d->valuesPosition) );
	writer->writeAttribute( "distance", QString::number(d->valuesDistance) );
	writer->writeAttribute( "rotation", QString::number(d->valuesRotationAngle) );
	writer->writeAttribute( "opacity", QString::number(d->valuesOpacity) );
	//TODO values format and precision
	writer->writeAttribute( "prefix", d->valuesPrefix );
	writer->writeAttribute( "suffix", d->valuesSuffix );
	WRITE_QCOLOR(d->valuesColor);
	WRITE_QFONT(d->valuesFont);
	writer->writeEndElement();

	//Filling
	writer->writeStartElement("filling");
	writer->writeAttribute( "enalbed", QString::number(d->fillingEnabled) );
	writer->writeAttribute( "type", QString::number(d->fillingType) );
	writer->writeAttribute( "colorStyle", QString::number(d->fillingColorStyle) );
	writer->writeAttribute( "imageStyle", QString::number(d->fillingImageStyle) );
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
			READ_INT_VALUE("binningMethod", binningMethod, Histogram::BinningMethod);
			READ_INT_VALUE("binCount", binCount, int);
			READ_DOUBLE_VALUE("binWidth", binWidth);
			READ_INT_VALUE("autoBinRanges", autoBinRanges, bool);
			READ_DOUBLE_VALUE("binRangesMin", binRangesMin);
			READ_DOUBLE_VALUE("binRangesMax", binRangesMax);

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
			attribs = reader->attributes();

			READ_INT_VALUE("symbolsStyle", symbolsStyle, Symbol::Style);
			READ_DOUBLE_VALUE("opacity", symbolsOpacity);
			READ_DOUBLE_VALUE("rotation", symbolsRotationAngle);
			READ_DOUBLE_VALUE("size", symbolsSize);

			READ_QBRUSH(d->symbolsBrush);
			READ_QPEN(d->symbolsPen);
		} else if (!preview && reader->name() == "values") {
			attribs = reader->attributes();

			READ_INT_VALUE("type", valuesType, Histogram::ValuesType);
			READ_COLUMN(valuesColumn);
			READ_INT_VALUE("position", valuesPosition, Histogram::ValuesPosition);
			READ_DOUBLE_VALUE("distance", valuesRotationAngle);
			READ_DOUBLE_VALUE("rotation", valuesRotationAngle);
			READ_DOUBLE_VALUE("opacity", valuesOpacity);

			//don't produce any warning if no prefix or suffix is set (empty string is allowed here in xml)
			d->valuesPrefix = attribs.value("prefix").toString();
			d->valuesSuffix = attribs.value("suffix").toString();

			READ_QCOLOR(d->valuesColor);
			READ_QFONT(d->valuesFont);
		} else if (!preview && reader->name() == "filling") {
			attribs = reader->attributes();

			READ_INT_VALUE("enabled", fillingEnabled, bool);
			READ_INT_VALUE("type", fillingType, PlotArea::BackgroundType);
			READ_INT_VALUE("colorStyle", fillingColorStyle, PlotArea::BackgroundColorStyle);
			READ_INT_VALUE("imageStyle", fillingImageStyle, PlotArea::BackgroundImageStyle);
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
	KConfigGroup group = config.group("XYCurve");

	int index = parentAspect()->indexOfChild<Histogram>(this);
	const auto* plot = dynamic_cast<const CartesianPlot*>(parentAspect());
	QColor themeColor;
	if (index<plot->themeColorPalette().size())
		themeColor = plot->themeColorPalette().at(index);
	else {
		if (plot->themeColorPalette().size())
			themeColor = plot->themeColorPalette().last();
	}

	QPen p;

	Q_D(Histogram);
	d->m_suppressRecalc = true;

	//Line
	p.setStyle((Qt::PenStyle)group.readEntry("LineStyle", (int)this->linePen().style()));
	p.setWidthF(group.readEntry("LineWidth", this->linePen().widthF()));
	p.setColor(themeColor);
	this->setLinePen(p);
	this->setLineOpacity(group.readEntry("LineOpacity", this->lineOpacity()));

	//Symbol
	this->setSymbolsOpacity(group.readEntry("SymbolOpacity", this->symbolsOpacity()));
	QBrush brush = symbolsBrush();
	brush.setColor(themeColor);
	this->setSymbolsBrush(brush);
	p = symbolsPen();
	p.setColor(themeColor);
	this->setSymbolsPen(p);

	//Values
	this->setValuesOpacity(group.readEntry("ValuesOpacity", this->valuesOpacity()));
	this->setValuesColor(group.readEntry("ValuesColor", this->valuesColor()));

	//Filling
	this->setFillingBrushStyle((Qt::BrushStyle)group.readEntry("FillingBrushStyle",(int) this->fillingBrushStyle()));
	this->setFillingColorStyle((PlotArea::BackgroundColorStyle)group.readEntry("FillingColorStyle",(int) this->fillingColorStyle()));
	this->setFillingOpacity(group.readEntry("FillingOpacity", this->fillingOpacity()));
	this->setFillingSecondColor(group.readEntry("FillingSecondColor",(QColor) this->fillingSecondColor()));
	this->setFillingFirstColor(themeColor);
	this->setFillingType((PlotArea::BackgroundType)group.readEntry("FillingType",(int) this->fillingType()));

	//Error Bars
	//TODO:
// 	p.setStyle((Qt::PenStyle)group.readEntry("ErrorBarsStyle",(int) this->errorBarsPen().style()));
// 	p.setWidthF(group.readEntry("ErrorBarsWidth", this->errorBarsPen().widthF()));
// 	p.setColor(themeColor);
// 	this->setErrorBarsPen(p);
// 	this->setErrorBarsOpacity(group.readEntry("ErrorBarsOpacity",this->errorBarsOpacity()));

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
	group.writeEntry("SymbolOpacity", this->symbolsOpacity());

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
