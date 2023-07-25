/*
	File                 : QQPlot.h
	Project              : LabPlot
	Description          : QQ-Plot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022-2023 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef QQPLOT_H
#define QQPLOT_H

#include "Plot.h"
#include "backend/lib/Range.h"
#include "backend/lib/macros.h"
#include "backend/worksheet/WorksheetElement.h"
#include "backend/worksheet/plots/cartesian/CartesianCoordinateSystem.h"

#include <QFont>
#include <QPen>

class AbstractColumn;
class Background;
class Line;
class Symbol;
class QQPlotPrivate;

#ifdef SDK
#include "labplot_export.h"
class LABPLOT_EXPORT QQPlot : public Plot {
#else
class QQPlot : public Plot {
#endif
	Q_OBJECT

public:
	friend class QQPlotSetDataColumnCmd;

	explicit QQPlot(const QString& name);
	~QQPlot() override;

	void finalizeAdd() override;

	QIcon icon() const override;
	QMenu* createContextMenu() override;
	QGraphicsItem* graphicsItem() const override;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

	void loadThemeConfig(const KConfig&) override;
	void saveThemeConfig(const KConfig&) override;

	bool activatePlot(QPointF mouseScenePos, double maxDist = -1) override;
	void setHover(bool on) override;

	POINTER_D_ACCESSOR_DECL(const AbstractColumn, dataColumn, DataColumn)
	CLASS_D_ACCESSOR_DECL(QString, dataColumnPath, DataColumnPath)

	Line* line() const;
	Symbol* symbol() const;

	void retransform() override;
	void recalc();
	void handleResize(double horizontalRatio, double verticalRatio, bool pageResize) override;

	double minimum(CartesianCoordinateSystem::Dimension) const override;
	double maximum(CartesianCoordinateSystem::Dimension) const override;
	bool hasData() const override;

	typedef QQPlotPrivate Private;

private Q_SLOTS:
	void dataColumnAboutToBeRemoved(const AbstractAspect*);
	void dataColumnNameChanged();

protected:
	QQPlot(const QString& name, QQPlotPrivate* dd);

private:
	Q_DECLARE_PRIVATE(QQPlot)
	void init();
	void initActions();
	void connectDataColumn(const AbstractColumn*);

	QAction* visibilityAction{nullptr};
	QAction* navigateToAction{nullptr};
	bool m_menusInitialized{false};

Q_SIGNALS:
	void linesUpdated(const QQPlot*, const QVector<QLineF>&);

	// General-Tab
	void dataChanged(); // emitted when the actual curve data to be plotted was changed to re-adjust the plot
	void dataDataChanged();
	void dataColumnChanged(const AbstractColumn*);
};

#endif
