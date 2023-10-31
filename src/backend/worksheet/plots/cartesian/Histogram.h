/*
	File                 : Histogram.h
	Project              : LabPlot
	Description          : Histogram
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2016 Anu Mittal <anu22mittal@gmail.com>
	SPDX-FileCopyrightText: 2018-2023 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include "backend/worksheet/plots/cartesian/Plot.h"

class AbstractColumn;
class HistogramPrivate;
class Background;
class Line;
class Symbol;
class Value;

#ifdef SDK
#include "labplot_export.h"
class LABPLOT_EXPORT Histogram : public Plot {
#else
class Histogram : public Plot {
#endif
	Q_OBJECT

public:
	friend class HistogramSetDataColumnCmd;
	friend class HistogramSetErrorPlusColumnCmd;
	friend class HistogramSetErrorMinusColumnCmd;

	enum Type { Ordinary, Cumulative, AvgShift };
	enum Orientation { Vertical, Horizontal };
	enum Normalization { Count, Probability, CountDensity, ProbabilityDensity };
	enum BinningMethod { ByNumber, ByWidth, SquareRoot, Rice, Sturges, Doane, Scott };
	enum LineType { NoLine, Bars, Envelope, DropLines, HalfBars };
	enum ValuesType { NoValues, ValuesBinEntries, ValuesCustomColumn };
	enum ValuesPosition { ValuesAbove, ValuesUnder, ValuesLeft, ValuesRight };
	enum ErrorType { NoError, Poisson, CustomSymmetric, CustomAsymmetric };

	explicit Histogram(const QString& name);
	~Histogram() override;

	QIcon icon() const override;
	QMenu* createContextMenu() override;
	QGraphicsItem* graphicsItem() const override;
	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;
	void loadThemeConfig(const KConfig&) override;
	void saveThemeConfig(const KConfig&) override;

	bool activatePlot(QPointF mouseScenePos, double maxDist = -1) override;

	POINTER_D_ACCESSOR_DECL(const AbstractColumn, dataColumn, DataColumn)
	CLASS_D_ACCESSOR_DECL(QString, dataColumnPath, DataColumnPath)

	BASIC_D_ACCESSOR_DECL(Histogram::Type, type, Type)
	BASIC_D_ACCESSOR_DECL(Histogram::Orientation, orientation, Orientation)
	BASIC_D_ACCESSOR_DECL(Histogram::Normalization, normalization, Normalization)
	BASIC_D_ACCESSOR_DECL(Histogram::BinningMethod, binningMethod, BinningMethod)
	BASIC_D_ACCESSOR_DECL(int, binCount, BinCount)
	BASIC_D_ACCESSOR_DECL(double, binWidth, BinWidth)
	BASIC_D_ACCESSOR_DECL(bool, autoBinRanges, AutoBinRanges)
	BASIC_D_ACCESSOR_DECL(double, binRangesMin, BinRangesMin)
	BASIC_D_ACCESSOR_DECL(double, binRangesMax, BinRangesMax)

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
	Line* errorBarsLine() const;

	// margin plots
	BASIC_D_ACCESSOR_DECL(bool, rugEnabled, RugEnabled)
	BASIC_D_ACCESSOR_DECL(double, rugOffset, RugOffset)
	BASIC_D_ACCESSOR_DECL(double, rugLength, RugLength)
	BASIC_D_ACCESSOR_DECL(double, rugWidth, RugWidth)

	double minimum(CartesianCoordinateSystem::Dimension) const override;
	double maximum(CartesianCoordinateSystem::Dimension) const override;
	bool hasData() const override;

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
	void dataColumnNameChanged();

	void errorPlusColumnAboutToBeRemoved(const AbstractAspect*);
	void errorPlusColumnNameChanged();

	void errorMinusColumnAboutToBeRemoved(const AbstractAspect*);
	void errorMinusColumnNameChanged();

protected:
	Histogram(const QString& name, HistogramPrivate* dd);

private:
	Q_DECLARE_PRIVATE(Histogram)
	void init();
	void initActions();
	void connectDataColumn(const AbstractColumn*);
	void connectErrorPlusColumn(const AbstractColumn*);
	void connectErrorMinusColumn(const AbstractColumn*);

	QAction* visibilityAction{nullptr};

Q_SIGNALS:
	// General-Tab
	void dataDataChanged();
	void dataColumnChanged(const AbstractColumn*);

	void typeChanged(Histogram::Type);
	void orientationChanged(Histogram::Orientation);
	void normalizationChanged(Histogram::Normalization);
	void binningMethodChanged(Histogram::BinningMethod);
	void binCountChanged(int);
	void binWidthChanged(double);
	void autoBinRangesChanged(bool);
	void binRangesMinChanged(double);
	void binRangesMaxChanged(double);

	// Error bars
	void errorTypeChanged(Histogram::ErrorType);
	void errorPlusDataChanged();
	void errorPlusColumnChanged(const AbstractColumn*);
	void errorMinusDataChanged();
	void errorMinusColumnChanged(const AbstractColumn*);

	// Margin Plots
	void rugEnabledChanged(bool);
	void rugLengthChanged(double);
	void rugWidthChanged(double);
	void rugOffsetChanged(double);
};

#endif
