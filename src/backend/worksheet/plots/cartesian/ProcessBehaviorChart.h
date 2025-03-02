/*
	File                 : ProcessBehaviorChart.h
	Project              : LabPlot
	Description          : Process Behavior Chart
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024-2025 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PROCESSBEHAVIORCHART_H
#define PROCESSBEHAVIORCHART_H

#include "Plot.h"
#include "backend/worksheet/TextLabel.h"

class Line;
class ProcessBehaviorChartPrivate;
class Symbol;
class XYCurve;

#ifdef SDK
#include "labplot_export.h"
class LABPLOT_EXPORT ProcessBehaviorChart : public Plot {
#else
class ProcessBehaviorChart : public Plot {
#endif
	Q_OBJECT

public:
	friend class ProcessBehaviorChartSetDataColumnCmd;
	friend class ProcessBehaviorChartSetData2ColumnCmd;

	enum class Type { XmR, mR, XbarR, R, XbarS, S, P, NP, C, U };
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
	POINTER_D_ACCESSOR_DECL(const AbstractColumn, data2Column, Data2Column)
	CLASS_D_ACCESSOR_DECL(QString, dataColumnPath, DataColumnPath)
	CLASS_D_ACCESSOR_DECL(QString, data2ColumnPath, Data2ColumnPath)
	BASIC_D_ACCESSOR_DECL(int, sampleSize, SampleSize)
	BASIC_D_ACCESSOR_DECL(bool, negativeLowerLimitEnabled, NegativeLowerLimitEnabled)
	BASIC_D_ACCESSOR_DECL(bool, exactLimitsEnabled, ExactLimitsEnabled)

	BASIC_D_ACCESSOR_DECL(bool, valuesEnabled, ValuesEnabled)
	BASIC_D_ACCESSOR_DECL(QColor, valuesFontColor, ValuesFontColor)
	BASIC_D_ACCESSOR_DECL(QColor, valuesBackgroundColor, ValuesBackgroundColor)
	CLASS_D_ACCESSOR_DECL(QFont, valuesFont, ValuesFont)
	BASIC_D_ACCESSOR_DECL(TextLabel::BorderShape, valuesBorderShape, ValuesBorderShape)
	Line* valuesBorderLine() const;

	Symbol* dataSymbol() const;
	Line* dataLine() const;
	Line* centerLine() const;
	Line* upperLimitLine() const;
	Line* lowerLimitLine() const;
	bool lowerLimitAvailable() const;

	void retransform() override;
	void recalc() override;
	void handleResize(double horizontalRatio, double verticalRatio, bool pageResize) override;
	void setVisible(bool) override;
	void setZValue(qreal) override;

	int xIndexCount() const;
	bool minMax(const CartesianCoordinateSystem::Dimension, const Range<int>& indexRange, Range<double>&, bool includeErrorBars = true) const override;
	double minimum(CartesianCoordinateSystem::Dimension) const override;
	double maximum(CartesianCoordinateSystem::Dimension) const override;
	bool hasData() const override;
	bool usingColumn(const AbstractColumn*, bool indirect) const override;
	void handleAspectUpdated(const QString& path, const AbstractAspect*) override;
	QColor color() const override;

	typedef ProcessBehaviorChartPrivate Private;

private Q_SLOTS:
	void dataColumnAboutToBeRemoved(const AbstractAspect*);
	void data2ColumnAboutToBeRemoved(const AbstractAspect*);
	void renameInternalCurves();
	void valuesBorderStyleChanged(Qt::PenStyle);
	void valuesBorderWidthChanged(double);
	void valuesBorderColorChanged(const QColor&);
	void valuesBorderOpacityChanged(float);

protected:
	ProcessBehaviorChart(const QString& name, ProcessBehaviorChartPrivate* dd);

private:
	Q_DECLARE_PRIVATE(ProcessBehaviorChart)
	void init();
	void connectDataColumn(const AbstractColumn*);
	void connectData2Column(const AbstractColumn*);

	// private methods used in tests
	friend class StatisticalPlotsTest;
	double center() const;
	double upperLimit() const;
	double lowerLimit() const;
	XYCurve* dataCurve() const;

Q_SIGNALS:
	void linesUpdated(const ProcessBehaviorChart*, const QVector<QLineF>&);

	// General
	void typeChanged(ProcessBehaviorChart::Type);
	void limitsMetricChanged(ProcessBehaviorChart::LimitsMetric);
	void dataDataChanged();
	void data2DataChanged();
	void dataColumnChanged(const AbstractColumn*);
	void data2ColumnChanged(const AbstractColumn*);
	void sampleSizeChanged(int);
	void negativeLowerLimitEnabledChanged(bool);
	void exactLimitsEnabledChanged(bool);

	// Values
	void valuesEnabledChanged(bool);
	void valuesFontChanged(const QFont);
	void valuesFontColorChanged(const QColor);
	void valuesBackgroundColorChanged(const QColor);
	void valuesBorderShapeChanged(TextLabel::BorderShape);
};

#endif
