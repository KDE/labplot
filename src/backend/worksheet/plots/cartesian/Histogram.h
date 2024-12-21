/*
	File                 : Histogram.h
	Project              : LabPlot
	Description          : Histogram
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2016 Anu Mittal <anu22mittal@gmail.com>
	SPDX-FileCopyrightText: 2018-2025 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include "backend/worksheet/plots/cartesian/Plot.h"

class HistogramPrivate;
class Background;
class ErrorBar;
class ErrorBarStyle;
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
	enum Type { Ordinary, Cumulative, AvgShift };
	enum Normalization { Count, Probability, CountDensity, ProbabilityDensity };
	enum BinningMethod { ByNumber, ByWidth, SquareRoot, Rice, Sturges, Doane, Scott };
	enum LineType { NoLine, Bars, Envelope, DropLines, HalfBars };
	enum ValuesType { NoValues, ValuesBinEntries, ValuesCustomColumn };
	enum ValuesPosition { ValuesAbove, ValuesUnder, ValuesLeft, ValuesRight };

	explicit Histogram(const QString& name, bool loading = false);
	~Histogram() override;

	QIcon icon() const override;
	QMenu* createContextMenu() override;
	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;
	void loadThemeConfig(const KConfig&) override;
	void saveThemeConfig(const KConfig&) override;

	void retransform() override;
	void recalc() override;
	void handleResize(double horizontalRatio, double verticalRatio, bool pageResize) override;
	void updateLocale() override;

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
	ErrorBar* errorBar() const;

	// margin plots
	BASIC_D_ACCESSOR_DECL(bool, rugEnabled, RugEnabled)
	BASIC_D_ACCESSOR_DECL(double, rugOffset, RugOffset)
	BASIC_D_ACCESSOR_DECL(double, rugLength, RugLength)
	BASIC_D_ACCESSOR_DECL(double, rugWidth, RugWidth)

	bool indicesMinMax(const Dimension dim, double, double, int& start, int& end) const override;
	bool minMax(const Dimension dim, const Range<int>& indexRange, Range<double>& r, bool includeErrorBars = true) const override;
	double minimum(CartesianCoordinateSystem::Dimension) const override;
	double maximum(CartesianCoordinateSystem::Dimension) const override;
	bool hasData() const override;
	bool usingColumn(const AbstractColumn*, bool indirect) const override;
	QColor color() const override;

	const AbstractColumn* bins() const;
	const AbstractColumn* binValues() const;
	const AbstractColumn* binPDValues() const;

	typedef WorksheetElement BaseClass;
	typedef HistogramPrivate Private;

public Q_SLOTS:
	void createDataSpreadsheet();

private Q_SLOTS:
	void updateValues();
	void updateErrorBars();

	void dataColumnAboutToBeRemoved(const AbstractAspect*);

protected:
	Histogram(const QString& name, HistogramPrivate* dd);
	virtual void handleAspectUpdated(const QString& aspectPath, const AbstractAspect*) override;

private:
	Q_DECLARE_PRIVATE(Histogram)
	void init(bool loading);
	void connectDataColumn(const AbstractColumn*);

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

	// Margin Plots
	void rugEnabledChanged(bool);
	void rugLengthChanged(double);
	void rugWidthChanged(double);
	void rugOffsetChanged(double);
};

#endif
