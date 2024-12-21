/*
	File                 : BoxPlot.h
	Project              : LabPlot
	Description          : Box Plot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2021-2023 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef BOXPLOT_H
#define BOXPLOT_H

#include "backend/worksheet/plots/cartesian/Plot.h"

class Background;
class BoxPlotPrivate;
class Line;
class Symbol;

#ifdef SDK
#include "labplot_export.h"
class LABPLOT_EXPORT BoxPlot : public Plot {
#else
class BoxPlot : public Plot {
#endif
	Q_OBJECT

public:
	enum class Ordering { None, MedianAscending, MedianDescending, MeanAscending, MeanDescending };
	enum class WhiskersType { MinMax, IQR, SD, MAD, PERCENTILES_10_90, PERCENTILES_5_95, PERCENTILES_1_99 };

	explicit BoxPlot(const QString&, bool loading = false);
	~BoxPlot() override;

	QIcon icon() const override;
	static QIcon staticIcon();
	virtual QMenu* createContextMenu() override;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;
	void loadThemeConfig(const KConfig&) override;

	// general
	BASIC_D_ACCESSOR_DECL(QVector<const AbstractColumn*>, dataColumns, DataColumns)
	QVector<QString>& dataColumnPaths() const;
	BASIC_D_ACCESSOR_DECL(BoxPlot::Ordering, ordering, Ordering)
	BASIC_D_ACCESSOR_DECL(BoxPlot::Orientation, orientation, Orientation)
	BASIC_D_ACCESSOR_DECL(bool, variableWidth, VariableWidth)
	BASIC_D_ACCESSOR_DECL(double, widthFactor, WidthFactor)
	BASIC_D_ACCESSOR_DECL(bool, notchesEnabled, NotchesEnabled)

	// box
	Background* backgroundAt(int) const;
	Line* borderLineAt(int) const;
	Line* medianLineAt(int) const;

	// symbols
	Symbol* symbolMean() const;
	Symbol* symbolMedian() const;
	Symbol* symbolOutlier() const;
	Symbol* symbolFarOut() const;
	Symbol* symbolData() const;
	Symbol* symbolWhiskerEnd() const;
	BASIC_D_ACCESSOR_DECL(bool, jitteringEnabled, JitteringEnabled)

	// whiskers
	BASIC_D_ACCESSOR_DECL(BoxPlot::WhiskersType, whiskersType, WhiskersType)
	BASIC_D_ACCESSOR_DECL(double, whiskersRangeParameter, WhiskersRangeParameter)
	Line* whiskersLine() const;
	BASIC_D_ACCESSOR_DECL(double, whiskersCapSize, WhiskersCapSize)
	Line* whiskersCapLine() const;

	// margin plots
	BASIC_D_ACCESSOR_DECL(bool, rugEnabled, RugEnabled)
	BASIC_D_ACCESSOR_DECL(double, rugOffset, RugOffset)
	BASIC_D_ACCESSOR_DECL(double, rugLength, RugLength)
	BASIC_D_ACCESSOR_DECL(double, rugWidth, RugWidth)

	void retransform() override;
	void recalc() override;
	void handleResize(double horizontalRatio, double verticalRatio, bool pageResize) override;

	bool indicesMinMax(const Dimension dim, double, double, int& start, int& end) const override;
	bool minMax(const Dimension dim, const Range<int>& indexRange, Range<double>& r, bool includeErrorBars = true) const;
	double minimum(CartesianCoordinateSystem::Dimension) const override;
	double maximum(CartesianCoordinateSystem::Dimension) const override;
	bool hasData() const override;
	bool usingColumn(const AbstractColumn*, bool indirect) const override;
	QColor color() const override;
	QColor colorAt(int) const;

	typedef BoxPlotPrivate Private;

protected:
	BoxPlot(const QString& name, BoxPlotPrivate* dd);
	virtual void handleAspectUpdated(const QString& aspectPath, const AbstractAspect*) override;

private:
	Q_DECLARE_PRIVATE(BoxPlot)
	void init(bool loading);
	void initActions();
	void initMenus();
	void connectDataColumn(const AbstractColumn*);

	QAction* orientationHorizontalAction{nullptr};
	QAction* orientationVerticalAction{nullptr};
	QMenu* orientationMenu{nullptr};

public Q_SLOTS:
	void createDataSpreadsheet();

private Q_SLOTS:
	// SLOTs for changes triggered via QActions in the context menu
	void orientationChangedSlot(QAction*);

	void dataColumnAboutToBeRemoved(const AbstractAspect*);

Q_SIGNALS:
	// General-Tab
	void dataColumnsChanged(const QVector<const AbstractColumn*>&);
	void dataDataChanged();
	void orderingChanged(BoxPlot::Ordering);
	void orientationChanged(BoxPlot::Orientation);
	void variableWidthChanged(bool);
	void widthFactorChanged(double);
	void notchesEnabledChanged(bool);

	// symbols
	void jitteringEnabledChanged(bool);

	// whiskers
	void whiskersTypeChanged(BoxPlot::WhiskersType);
	void whiskersRangeParameterChanged(double);
	void whiskersCapSizeChanged(double);

	// Margin Plots
	void rugEnabledChanged(bool);
	void rugLengthChanged(double);
	void rugWidthChanged(double);
	void rugOffsetChanged(double);

	friend class BoxPlotSetDataColumnsCmd;
};

#endif
