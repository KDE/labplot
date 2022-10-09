/*
	File                 : Histogram.h
	Project              : LabPlot
	Description          : Histogram
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2016 Anu Mittal <anu22mittal@gmail.com>
	SPDX-FileCopyrightText: 2018-2022 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include "backend/worksheet/plots/cartesian/XYCurve.h"

class AbstractColumn;
class HistogramPrivate;
class Background;
class Line;
class Symbol;
class Value;

#ifdef SDK
#include "labplot_export.h"
class LABPLOT_EXPORT Histogram : public WorksheetElement, public Curve {
#else
class Histogram : public WorksheetElement, public Curve {
#endif
	Q_OBJECT

public:
	enum HistogramType { Ordinary, Cumulative, AvgShift };
	enum HistogramOrientation { Vertical, Horizontal };
	enum HistogramNormalization { Count, Probability, CountDensity, ProbabilityDensity };
	enum BinningMethod { ByNumber, ByWidth, SquareRoot, Rice, Sturges, Doane, Scott };
	enum LineType { NoLine, Bars, Envelope, DropLines, HalfBars };
	enum ValuesType { NoValues, ValuesBinEntries, ValuesCustomColumn };
	enum ValuesPosition { ValuesAbove, ValuesUnder, ValuesLeft, ValuesRight };
	enum ErrorType { NoError, Poisson, CustomSymmetric, CustomAsymmetric };

	explicit Histogram(const QString& name);

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

	Line* line() const;
	Background* background() const;
	Symbol* symbol() const;
	Value* value() const;

	// error bars
	BASIC_D_ACCESSOR_DECL(ErrorType, errorType, ErrorType)
	POINTER_D_ACCESSOR_DECL(const AbstractColumn, errorPlusColumn, ErrorPlusColumn)
	CLASS_D_ACCESSOR_DECL(QString, errorPlusColumnPath, ErrorPlusColumnPath)
	POINTER_D_ACCESSOR_DECL(const AbstractColumn, errorMinusColumn, ErrorMinusColumn)
	CLASS_D_ACCESSOR_DECL(QString, errorMinusColumnPath, ErrorMinusColumnPath)
	BASIC_D_ACCESSOR_DECL(XYCurve::ErrorBarsType, errorBarsType, ErrorBarsType)
	BASIC_D_ACCESSOR_DECL(qreal, errorBarsCapSize, ErrorBarsCapSize)
	CLASS_D_ACCESSOR_DECL(QPen, errorBarsPen, ErrorBarsPen)
	BASIC_D_ACCESSOR_DECL(qreal, errorBarsOpacity, ErrorBarsOpacity)

	// margin plots
	BASIC_D_ACCESSOR_DECL(bool, rugEnabled, RugEnabled)
	BASIC_D_ACCESSOR_DECL(double, rugOffset, RugOffset)
	BASIC_D_ACCESSOR_DECL(double, rugLength, RugLength)
	BASIC_D_ACCESSOR_DECL(double, rugWidth, RugWidth)

	double minimum(CartesianCoordinateSystem::Dimension dim) const;
	double maximum(CartesianCoordinateSystem::Dimension dim) const;

	const AbstractColumn* bins() const;
	const AbstractColumn* binValues() const;
	const AbstractColumn* binPDValues() const;

	typedef WorksheetElement BaseClass;
	typedef HistogramPrivate Private;

public Q_SLOTS:
	void retransform() override;
	void recalcHistogram();
	void handleResize(double horizontalRatio, double verticalRatio, bool pageResize) override;
	void createDataSpreadsheet();

private Q_SLOTS:
	void updateValues();
	void updateErrorBars();
	void dataColumnAboutToBeRemoved(const AbstractAspect*);
	void errorPlusColumnAboutToBeRemoved(const AbstractAspect*);
	void errorMinusColumnAboutToBeRemoved(const AbstractAspect*);

protected:
	Histogram(const QString& name, HistogramPrivate* dd);

private:
	Q_DECLARE_PRIVATE(Histogram)
	void init();
	void initActions();
	QAction* visibilityAction{nullptr};

Q_SIGNALS:
	// General-Tab
	void dataChanged();
	void dataColumnChanged(const AbstractColumn*);

	void typeChanged(Histogram::HistogramType);
	void orientationChanged(Histogram::HistogramOrientation);
	void normalizationChanged(Histogram::HistogramNormalization);
	void binningMethodChanged(Histogram::BinningMethod);
	void binCountChanged(int);
	void binWidthChanged(float);
	void autoBinRangesChanged(bool);
	void binRangesMinChanged(double);
	void binRangesMaxChanged(double);

	// Error bars
	void errorTypeChanged(Histogram::ErrorType);
	void errorPlusColumnChanged(const AbstractColumn*);
	void errorMinusColumnChanged(const AbstractColumn*);
	void errorBarsTypeChanged(XYCurve::ErrorBarsType);
	void errorBarsPenChanged(QPen);
	void errorBarsCapSizeChanged(qreal);
	void errorBarsOpacityChanged(qreal);

	// Margin Plots
	void rugEnabledChanged(bool);
	void rugLengthChanged(double);
	void rugWidthChanged(double);
	void rugOffsetChanged(double);
};

#endif
