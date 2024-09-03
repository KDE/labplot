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
	friend class ProcessBehaviorChartSetXDataColumnCmd;
	friend class ProcessBehaviorChartSetYDataColumnCmd;

	enum class Type {XmR, mR, XbarR, R, XbarS, S, P, NP, C, U};

	explicit ProcessBehaviorChart(const QString& name);
	~ProcessBehaviorChart() override;

	void finalizeAdd() override;

	QIcon icon() const override;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

	void loadThemeConfig(const KConfig&) override;
	void saveThemeConfig(const KConfig&) override;

	BASIC_D_ACCESSOR_DECL(ProcessBehaviorChart::Type, type, Type)
	POINTER_D_ACCESSOR_DECL(const AbstractColumn, xDataColumn, XDataColumn)
	CLASS_D_ACCESSOR_DECL(QString, xDataColumnPath, XDataColumnPath)
	POINTER_D_ACCESSOR_DECL(const AbstractColumn, yDataColumn, YDataColumn)
	CLASS_D_ACCESSOR_DECL(QString, yDataColumnPath, YDataColumnPath)
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

protected:
	ProcessBehaviorChart(const QString& name, ProcessBehaviorChartPrivate* dd);

private:
	Q_DECLARE_PRIVATE(ProcessBehaviorChart)
	void init();
	void connectXDataColumn(const AbstractColumn*);
	void connectYDataColumn(const AbstractColumn*);

	QAction* navigateToAction{nullptr};
	bool m_menusInitialized{false};

Q_SIGNALS:
	void linesUpdated(const ProcessBehaviorChart*, const QVector<QLineF>&);

	// General-Tab
	void typeChanged(ProcessBehaviorChart::Type);
	void dataChanged(); // emitted when the actual curve data to be plotted was changed to re-adjust the plot
	void xDataDataChanged();
	void yDataDataChanged();
	void xDataColumnChanged(const AbstractColumn*);
	void yDataColumnChanged(const AbstractColumn*);
	void subgroupSizeChanged(int);
};

#endif
