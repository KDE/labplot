/*
	File                 : BarPlot.h
	Project              : LabPlot
	Description          : Bar Plot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022-2025 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef BARPLOT_H
#define BARPLOT_H

#include "backend/worksheet/plots/cartesian/Plot.h"

class BarPlotPrivate;
class Background;
class ErrorBar;
class Line;
class Value;

#ifdef SDK
#include "labplot_export.h"
class LABPLOT_EXPORT BarPlot : public Plot {
#else
class BarPlot : public Plot {
#endif
	Q_OBJECT

public:
	enum class Type { Grouped, Stacked, Stacked_100_Percent };

	explicit BarPlot(const QString&);
	~BarPlot() override;

	QIcon icon() const override;
	QMenu* createContextMenu() override;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;
	void loadThemeConfig(const KConfig&) override;

	// general
	POINTER_D_ACCESSOR_DECL(const AbstractColumn, xColumn, XColumn)
	CLASS_D_ACCESSOR_DECL(QString, xColumnPath, XColumnPath)
	BASIC_D_ACCESSOR_DECL(QVector<const AbstractColumn*>, dataColumns, DataColumns)
	CLASS_D_ACCESSOR_DECL(QVector<QString>, dataColumnPaths, DataColumnPaths)
	BASIC_D_ACCESSOR_DECL(BarPlot::Type, type, Type)
	BASIC_D_ACCESSOR_DECL(BarPlot::Orientation, orientation, Orientation)
	BASIC_D_ACCESSOR_DECL(double, widthFactor, WidthFactor)

	Background* backgroundAt(int) const; // box filling
	Line* lineAt(int) const; // box border line
	Value* value() const;
	ErrorBar* errorBarAt(int) const;

	void retransform() override;
	void recalc() override;
	void handleResize(double horizontalRatio, double verticalRatio, bool pageResize) override;
	void updateLocale() override;

	bool indicesMinMax(const Dimension dim, double, double, int& start, int& end) const override;
	bool minMax(const Dimension dim, const Range<int>& indexRange, Range<double>& r, bool includeErrorBars = true) const override;
	double minimum(CartesianCoordinateSystem::Dimension) const override;
	double maximum(CartesianCoordinateSystem::Dimension) const override;
	bool hasData() const override;
	int dataCount(Dimension) const override;
	bool usingColumn(const AbstractColumn*, bool indirect) const override;
	void handleAspectUpdated(const QString& aspectPath, const AbstractAspect* element) override;
	QColor color() const override;
	QColor colorAt(int) const;

	typedef BarPlotPrivate Private;

protected:
	BarPlot(const QString& name, BarPlotPrivate* dd);

private:
	Q_DECLARE_PRIVATE(BarPlot)
	void init();
	void initActions();
	void initMenus();
	void connectXColumn(const AbstractColumn*);
	void connectDataColumn(const AbstractColumn*);

	QAction* orientationHorizontalAction{nullptr};
	QAction* orientationVerticalAction{nullptr};
	QMenu* orientationMenu{nullptr};

private Q_SLOTS:
	// SLOTs for changes triggered via QActions in the context menu
	void orientationChangedSlot(QAction*);
	void xColumnAboutToBeRemoved(const AbstractAspect*);
	void dataColumnAboutToBeRemoved(const AbstractAspect*);

Q_SIGNALS:
	// General-Tab
	void xColumnChanged(const AbstractColumn*);
	void xDataChanged();
	void dataColumnsChanged(const QVector<const AbstractColumn*>&);
	void dataDataChanged();
	void typeChanged(BarPlot::Type);
	void orientationChanged(BarPlot::Orientation);
	void widthFactorChanged(double);

	// box border
	void borderPenChanged(QPen&);
	void borderOpacityChanged(float);

	friend class BarPlotSetXColumnCmd;
	friend class BarPlotSetDataColumnsCmd;
};

#endif
