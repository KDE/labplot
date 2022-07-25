/*
	File                 : BarPlot.h
	Project              : LabPlot
	Description          : Bar Plot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef BARPLOT_H
#define BARPLOT_H

#include "backend/lib/macros.h"
#include "backend/worksheet/WorksheetElement.h"
#include "backend/worksheet/plots/cartesian/CartesianCoordinateSystem.h"
#include "backend/worksheet/plots/cartesian/Curve.h"

class BarPlotPrivate;
class AbstractColumn;
class Background;
class Symbol;

#ifdef SDK
#include "labplot_export.h"
class LABPLOT_EXPORT BarPlot : public WorksheetElement, Curve {
#else
class BarPlot : public WorksheetElement, Curve {
#endif
	Q_OBJECT

public:
	enum class Type { Grouped, Stacked };

	explicit BarPlot(const QString&);
	~BarPlot() override;

	QIcon icon() const override;
	QMenu* createContextMenu() override;
	QGraphicsItem* graphicsItem() const override;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;
	void loadThemeConfig(const KConfig&) override;

	// reimplemented from Curve
	bool activateCurve(QPointF mouseScenePos, double maxDist = -1) override;
	void setHover(bool on) override;

	// general
	POINTER_D_ACCESSOR_DECL(const AbstractColumn, xColumn, XColumn)
	QString& xColumnPath() const;
	BASIC_D_ACCESSOR_DECL(QVector<const AbstractColumn*>, dataColumns, DataColumns)
	QVector<QString>& dataColumnPaths() const;
	BASIC_D_ACCESSOR_DECL(BarPlot::Type, type, Type)
	BASIC_D_ACCESSOR_DECL(BarPlot::Orientation, orientation, Orientation)
	BASIC_D_ACCESSOR_DECL(double, widthFactor, WidthFactor)

	// box filling
	Background* backgroundAt(int) const;

	// box border
	CLASS_D_ACCESSOR_DECL(QPen, borderPen, BorderPen)
	BASIC_D_ACCESSOR_DECL(qreal, borderOpacity, BorderOpacity)

	void retransform() override;
	void handleResize(double horizontalRatio, double verticalRatio, bool pageResize) override;

	double minimum(CartesianCoordinateSystem::Dimension dim) const;
	double maximum(CartesianCoordinateSystem::Dimension dim) const;

	typedef BarPlotPrivate Private;

protected:
	BarPlot(const QString& name, BarPlotPrivate* dd);

private:
	Q_DECLARE_PRIVATE(BarPlot)
	void init();
	void initActions();
	void initMenus();

	QAction* orientationHorizontalAction{nullptr};
	QAction* orientationVerticalAction{nullptr};
	QAction* visibilityAction{nullptr};
	QMenu* orientationMenu{nullptr};

public Q_SLOTS:
	void recalc();

private Q_SLOTS:
	// SLOTs for changes triggered via QActions in the context menu
	void orientationChangedSlot(QAction*);
	void visibilityChangedSlot();

	void dataColumnAboutToBeRemoved(const AbstractAspect*);

Q_SIGNALS:
	// General-Tab
	void dataChanged();
	void xColumnChanged(const AbstractColumn*);
	void dataColumnsChanged(const QVector<const AbstractColumn*>&);
	void typeChanged(BarPlot::Type);
	void orientationChanged(BarPlot::Orientation);
	void widthFactorChanged(double);

	// box border
	void borderPenChanged(QPen&);
	void borderOpacityChanged(float);
};

#endif
