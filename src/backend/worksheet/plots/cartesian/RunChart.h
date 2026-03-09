/*
	File                 : RunChart.h
	Project              : LabPlot
	Description          : Run Chart
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RUNCHART_H
#define RUNCHART_H

#include "Plot.h"

class Line;
class RunChartPrivate;
class Symbol;
class XYCurve;

#ifdef SDK
#include "labplot_export.h"
class LABPLOT_EXPORT RunChart : public Plot {
#else
class RunChart : public Plot {
#endif
	Q_OBJECT

public:
	friend class RunChartSetDataColumnCmd;

	enum class CenterMetric { Average, Median };

	explicit RunChart(const QString& name);
	~RunChart() override;

	void finalizeAdd() override;

	QIcon icon() const override;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

	void loadThemeConfig(const KConfig&) override;
	void saveThemeConfig(const KConfig&) override;

	BASIC_D_ACCESSOR_DECL(RunChart::CenterMetric, centerMetric, CenterMetric)
	POINTER_D_ACCESSOR_DECL(const AbstractColumn, dataColumn, DataColumn)
	CLASS_D_ACCESSOR_DECL(QString, dataColumnPath, DataColumnPath)

	Symbol* dataSymbol() const;
	Line* dataLine() const;
	Line* centerLine() const;

	void retransform() override;
	void recalc() override;
	void handleResize(double horizontalRatio, double verticalRatio, bool pageResize) override;
	void setVisible(bool) override;
	void setZValue(qreal) override;

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

	typedef RunChartPrivate Private;

private Q_SLOTS:
	void dataColumnAboutToBeRemoved(const AbstractAspect*);
	void renameInternalCurves();

protected:
	RunChart(const QString& name, RunChartPrivate* dd);

private:
	Q_DECLARE_PRIVATE(RunChart)
	void init();
	void connectDataColumn(const AbstractColumn*);

	// private methods used in tests
	friend class StatisticalPlotsTest;
	double center() const;
	XYCurve* dataCurve() const;

Q_SIGNALS:
	void linesUpdated(const RunChart*, const QVector<QLineF>&);

	// General-Tab
	void centerMetricChanged(RunChart::CenterMetric);
	void dataDataChanged();
	void dataColumnChanged(const AbstractColumn*);
};

#endif
