/*
	File                 : ProcessBehaviorChart.h
	Project              : LabPlot
	Description          : Process Behavior Chart
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PROCESSBEHAVIORCHART_H
#define PROCESSBEHAVIORCHART_H

#include "Plot.h"

class AbstractColumn;
class Line;
class Symbol;
class ProcessBehaviorChartPrivate;

#ifdef SDK
#include "labplot_export.h"
class LABPLOT_EXPORT ProcessBehaviorChart : public Plot {
#else
class ProcessBehaviorChart : public Plot {
#endif
	Q_OBJECT

public:
	friend class ProcessBehaviorChartSetDataColumnCmd;

	enum class Type { XmR, mR, XbarR, R, XbarS, S };
	enum class LimitsMetric { Average, Median };

	explicit ProcessBehaviorChart(const QString& name);
	~ProcessBehaviorChart() override;

	void finalizeAdd() override;

	QIcon icon() const override;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

	void loadThemeConfig(const KConfig&) override;
	void saveThemeConfig(const KConfig&) override;

	BASIC_D_ACCESSOR_DECL(ProcessBehaviorChart::Type, type, Type)
	BASIC_D_ACCESSOR_DECL(ProcessBehaviorChart::LimitsMetric, limitsMetric, LimitsMetric)
	POINTER_D_ACCESSOR_DECL(const AbstractColumn, dataColumn, DataColumn)
	CLASS_D_ACCESSOR_DECL(QString, dataColumnPath, DataColumnPath)
	BASIC_D_ACCESSOR_DECL(int, subgroupSize, SubgroupSize)

	Symbol* dataSymbol() const;
	Line* dataLine() const;
	Line* centerLine() const;
	Line* upperLimitLine() const;
	Line* lowerLimitLine() const;

	void retransform() override;
	void recalc() override;
	void handleResize(double horizontalRatio, double verticalRatio, bool pageResize) override;
	void setVisible(bool) override;

	bool minMax(const CartesianCoordinateSystem::Dimension dim, const Range<int>& indexRange, Range<double>& r, bool includeErrorBars = true) const override;
	double minimum(CartesianCoordinateSystem::Dimension) const override;
	double maximum(CartesianCoordinateSystem::Dimension) const override;
	bool hasData() const override;
	bool usingColumn(const Column*) const override;
	void handleAspectUpdated(const QString& aspectPath, const AbstractAspect* element) override;
	QColor color() const override;

	typedef ProcessBehaviorChartPrivate Private;

private Q_SLOTS:
	void dataColumnAboutToBeRemoved(const AbstractAspect*);
	void renameInternalCurves();

protected:
	ProcessBehaviorChart(const QString& name, ProcessBehaviorChartPrivate* dd);

private:
	Q_DECLARE_PRIVATE(ProcessBehaviorChart)
	void init();
	void connectDataColumn(const AbstractColumn*);

	QAction* navigateToAction{nullptr};
	bool m_menusInitialized{false};

Q_SIGNALS:
	void linesUpdated(const ProcessBehaviorChart*, const QVector<QLineF>&);

	// General-Tab
	void typeChanged(ProcessBehaviorChart::Type);
	void limitsMetricChanged(ProcessBehaviorChart::LimitsMetric);
	void dataDataChanged();
	void dataColumnChanged(const AbstractColumn*);
	void subgroupSizeChanged(int);
};

#endif
