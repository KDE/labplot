/*
	File                 : ParetoChart.h
	Project              : LabPlot
	Description          : Pareto Chart
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PARETOCHART_H
#define PARETOCHART_H

#include "Plot.h"

class AbstractColumn;
class Background;
class BarPlot;
class Line;
class ParetoChartPrivate;
class Symbol;
class XYCurve;

#ifdef SDK
#include "labplot_export.h"
class LABPLOT_EXPORT ParetoChart : public Plot {
#else
class ParetoChart : public Plot {
#endif
	Q_OBJECT

public:
	friend class ParetoChartSetDataColumnCmd;

	enum class CenterMetric { Average, Median };

	explicit ParetoChart(const QString& name);
	~ParetoChart() override;

	void finalizeAdd() override;

	QIcon icon() const override;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

	void loadThemeConfig(const KConfig&) override;
	void saveThemeConfig(const KConfig&) override;

	POINTER_D_ACCESSOR_DECL(const AbstractColumn, dataColumn, DataColumn)
	CLASS_D_ACCESSOR_DECL(QString, dataColumnPath, DataColumnPath)

	Line* barLine() const;
	Background* barBackground() const;
	Line* line() const;
	Symbol* symbol() const;

	void retransform() override;
	void recalc() override;
	void handleResize(double horizontalRatio, double verticalRatio, bool pageResize) override;
	void setVisible(bool) override;

	int xIndexCount() const;
	bool indicesMinMax(const Dimension dim, double v1, double v2, int& start, int& end) const override;
	bool minMax(const CartesianCoordinateSystem::Dimension dim, const Range<int>& indexRange, Range<double>& r, bool includeErrorBars = true) const override;
	double minimum(CartesianCoordinateSystem::Dimension) const override;
	double maximum(CartesianCoordinateSystem::Dimension) const override;
	bool hasData() const override;
	int dataCount(Dimension) const override;
	bool usingColumn(const AbstractColumn*, bool indirect) const override;
	void handleAspectUpdated(const QString& aspectPath, const AbstractAspect* element) override;
	QColor color() const override;

	typedef ParetoChartPrivate Private;

private Q_SLOTS:
	void dataColumnAboutToBeRemoved(const AbstractAspect*);
	void renameInternalCurves();

protected:
	ParetoChart(const QString& name, ParetoChartPrivate* dd);

private:
	Q_DECLARE_PRIVATE(ParetoChart)
	void init();
	void connectDataColumn(const AbstractColumn*);

	// private methods used in tests
	friend class StatisticalPlotsTest;
	BarPlot* barPlot() const;
	XYCurve* linePlot() const;

Q_SIGNALS:
	void linesUpdated(const ParetoChart*, const QVector<QLineF>&);

	// General-Tab
	void dataDataChanged();
	void dataColumnChanged(const AbstractColumn*);
};

#endif
