/*
    File                 : Histogram.h
    Project              : LabPlot
    Description          : Histogram
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016 Anu Mittal <anu22mittal@gmail.com>
    SPDX-FileCopyrightText: 2018-2021 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include "backend/worksheet/plots/cartesian/XYCurve.h"

class AbstractColumn;
class HistogramPrivate;
class Symbol;

#ifdef SDK
#include "labplot_export.h"
class LABPLOT_EXPORT Histogram : public WorksheetElement, public Curve {
#else
class Histogram : public WorksheetElement, public Curve {
#endif
	Q_OBJECT

public:
	enum HistogramType {Ordinary,Cumulative, AvgShift};
	enum HistogramOrientation {Vertical, Horizontal};
	enum HistogramNormalization {Count, Probability, CountDensity, ProbabilityDensity};
	enum BinningMethod {ByNumber, ByWidth, SquareRoot, Rice, Sturges, Doane, Scott};
	enum LineType {NoLine, Bars, Envelope, DropLines, HalfBars};
	enum ValuesType {NoValues, ValuesBinEntries, ValuesCustomColumn};
	enum ValuesPosition {ValuesAbove, ValuesUnder, ValuesLeft, ValuesRight};
	enum ErrorType {NoError};

	explicit Histogram(const QString &name);

	QIcon icon() const override;
	QMenu* createContextMenu() override;
	QGraphicsItem* graphicsItem() const override;
	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;
	void loadThemeConfig(const KConfig&) override;
	void saveThemeConfig(const KConfig&) override;

	bool activateCurve(QPointF mouseScenePos, double maxDist = -1) override;
	void setHover(bool on) override;

	POINTER_D_ACCESSOR_DECL(const AbstractColumn, dataColumn, DataColumn)
	QString& dataColumnPath() const;

	BASIC_D_ACCESSOR_DECL(Histogram::HistogramType, type, Type)
	BASIC_D_ACCESSOR_DECL(Histogram::HistogramOrientation, orientation, Orientation)
	BASIC_D_ACCESSOR_DECL(Histogram::HistogramNormalization, normalization, Normalization)
	BASIC_D_ACCESSOR_DECL(Histogram::BinningMethod, binningMethod, BinningMethod)
	BASIC_D_ACCESSOR_DECL(int, binCount, BinCount)
	BASIC_D_ACCESSOR_DECL(float, binWidth, BinWidth)
	BASIC_D_ACCESSOR_DECL(bool, autoBinRanges, AutoBinRanges)
	BASIC_D_ACCESSOR_DECL(double, binRangesMin, BinRangesMin)
	BASIC_D_ACCESSOR_DECL(double, binRangesMax, BinRangesMax)

	BASIC_D_ACCESSOR_DECL(float, xMin, XMin)
	BASIC_D_ACCESSOR_DECL(float, xMax, XMax)
	BASIC_D_ACCESSOR_DECL(float, yMin, YMin)
	BASIC_D_ACCESSOR_DECL(float, yMax, YMax)

	BASIC_D_ACCESSOR_DECL(LineType, lineType, LineType)
	CLASS_D_ACCESSOR_DECL(QPen, linePen, LinePen)
	BASIC_D_ACCESSOR_DECL(qreal, lineOpacity, LineOpacity)

	Symbol* symbol() const;

	BASIC_D_ACCESSOR_DECL(ValuesType, valuesType, ValuesType)
	POINTER_D_ACCESSOR_DECL(const AbstractColumn, valuesColumn, ValuesColumn)
	QString& valuesColumnPath() const;
	BASIC_D_ACCESSOR_DECL(ValuesPosition, valuesPosition, ValuesPosition)
	BASIC_D_ACCESSOR_DECL(qreal, valuesDistance, ValuesDistance)
	BASIC_D_ACCESSOR_DECL(qreal, valuesRotationAngle, ValuesRotationAngle)
	BASIC_D_ACCESSOR_DECL(qreal, valuesOpacity, ValuesOpacity)
	BASIC_D_ACCESSOR_DECL(char, valuesNumericFormat, ValuesNumericFormat)
	BASIC_D_ACCESSOR_DECL(int, valuesPrecision, ValuesPrecision)
	CLASS_D_ACCESSOR_DECL(QString, valuesDateTimeFormat, ValuesDateTimeFormat)
	CLASS_D_ACCESSOR_DECL(QString, valuesPrefix, ValuesPrefix)
	CLASS_D_ACCESSOR_DECL(QString, valuesSuffix, ValuesSuffix)
	CLASS_D_ACCESSOR_DECL(QColor, valuesColor, ValuesColor)
	CLASS_D_ACCESSOR_DECL(QFont, valuesFont, ValuesFont)

	BASIC_D_ACCESSOR_DECL(bool, fillingEnabled, FillingEnabled)
	BASIC_D_ACCESSOR_DECL(WorksheetElement::BackgroundType, fillingType, FillingType)
	BASIC_D_ACCESSOR_DECL(WorksheetElement::BackgroundColorStyle, fillingColorStyle, FillingColorStyle)
	BASIC_D_ACCESSOR_DECL(WorksheetElement::BackgroundImageStyle, fillingImageStyle, FillingImageStyle)
	BASIC_D_ACCESSOR_DECL(Qt::BrushStyle, fillingBrushStyle, FillingBrushStyle)
	CLASS_D_ACCESSOR_DECL(QColor, fillingFirstColor, FillingFirstColor)
	CLASS_D_ACCESSOR_DECL(QColor, fillingSecondColor, FillingSecondColor)
	CLASS_D_ACCESSOR_DECL(QString, fillingFileName, FillingFileName)
	BASIC_D_ACCESSOR_DECL(qreal, fillingOpacity, FillingOpacity)

	BASIC_D_ACCESSOR_DECL(ErrorType, errorType, ErrorType)
	BASIC_D_ACCESSOR_DECL(XYCurve::ErrorBarsType, errorBarsType, ErrorBarsType)
	BASIC_D_ACCESSOR_DECL(qreal, errorBarsCapSize, ErrorBarsCapSize)
	CLASS_D_ACCESSOR_DECL(QPen, errorBarsPen, ErrorBarsPen)
	BASIC_D_ACCESSOR_DECL(qreal, errorBarsOpacity, ErrorBarsOpacity)

	void suppressRetransform(bool);

	double xMinimum() const;
	double xMaximum() const;
	double yMinimum() const;
	double yMaximum() const;

	const AbstractColumn* bins() const;
	const AbstractColumn* binValues() const;

	typedef WorksheetElement BaseClass;
	typedef HistogramPrivate Private;

public slots:
	void retransform() override;
	void recalcHistogram();
	void handleResize(double horizontalRatio, double verticalRatio, bool pageResize) override;

private slots:
	void updateValues();
	void dataColumnAboutToBeRemoved(const AbstractAspect*);
	void valuesColumnAboutToBeRemoved(const AbstractAspect*);

	//SLOTs for changes triggered via QActions in the context menu
	void visibilityChangedSlot();

protected:
	Histogram(const QString& name, HistogramPrivate* dd);

private:
	Q_DECLARE_PRIVATE(Histogram)
	void init();
	void initActions();
	QAction* visibilityAction{nullptr};

signals:
	//General-Tab
	void dataChanged();
	void dataColumnChanged(const AbstractColumn*);
	void visibilityChanged(bool);

	void typeChanged(Histogram::HistogramType);
	void orientationChanged(Histogram::HistogramOrientation);
	void normalizationChanged(Histogram::HistogramNormalization);
	void binningMethodChanged(Histogram::BinningMethod);
	void binCountChanged(int);
	void binWidthChanged(float);
	void autoBinRangesChanged(bool);
	void binRangesMinChanged(double);
	void binRangesMaxChanged(double);

	//Line-Tab
	void lineTypeChanged(Histogram::LineType);
	void linePenChanged(const QPen&);
	void lineOpacityChanged(qreal);

	//Values-Tab
	void valuesTypeChanged(Histogram::ValuesType);
	void valuesColumnChanged(const AbstractColumn*);
	void valuesPositionChanged(Histogram::ValuesPosition);
	void valuesDistanceChanged(qreal);
	void valuesRotationAngleChanged(qreal);
	void valuesOpacityChanged(qreal);
	void valuesNumericFormatChanged(char);
	void valuesPrecisionChanged(int);
	void valuesDateTimeFormatChanged(QString);
	void valuesPrefixChanged(QString);
	void valuesSuffixChanged(QString);
	void valuesFontChanged(QFont);
	void valuesColorChanged(QColor);

	//Filling
	void fillingEnabledChanged(bool);
	void fillingTypeChanged(WorksheetElement::BackgroundType);
	void fillingColorStyleChanged(WorksheetElement::BackgroundColorStyle);
	void fillingImageStyleChanged(WorksheetElement::BackgroundImageStyle);
	void fillingBrushStyleChanged(Qt::BrushStyle);
	void fillingFirstColorChanged(QColor&);
	void fillingSecondColorChanged(QColor&);
	void fillingFileNameChanged(QString&);
	void fillingOpacityChanged(float);

	//Error bars
	void errorTypeChanged(Histogram::ErrorType);
	void errorBarsTypeChanged(XYCurve::ErrorBarsType);
	void errorBarsPenChanged(QPen);
	void errorBarsCapSizeChanged(qreal);
	void errorBarsOpacityChanged(qreal);
};

#endif
