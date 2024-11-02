/*
	File                 : QQPlot.h
	Project              : LabPlot
	Description          : QQ-Plot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef QQPLOT_H
#define QQPLOT_H

#include "Plot.h"
#include "backend/nsl/nsl_sf_stats.h"

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

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

	void loadThemeConfig(const KConfig&) override;
	void saveThemeConfig(const KConfig&) override;

	POINTER_D_ACCESSOR_DECL(const AbstractColumn, dataColumn, DataColumn)
	CLASS_D_ACCESSOR_DECL(QString, dataColumnPath, DataColumnPath)
	BASIC_D_ACCESSOR_DECL(nsl_sf_stats_distribution, distribution, Distribution)

	Line* line() const;
	Symbol* symbol() const;

	void retransform() override;
	void recalc() override;
	void handleResize(double horizontalRatio, double verticalRatio, bool pageResize) override;
	void setVisible(bool) override;

	bool minMax(const CartesianCoordinateSystem::Dimension dim, const Range<int>& indexRange, Range<double>& r, bool includeErrorBars = true) const override;
	double minimum(CartesianCoordinateSystem::Dimension) const override;
	double maximum(CartesianCoordinateSystem::Dimension) const override;
	bool hasData() const override;
	bool usingColumn(const AbstractColumn*) const override;
	void handleAspectUpdated(const QString& aspectPath, const AbstractAspect* element) override;
	QColor color() const override;

	typedef QQPlotPrivate Private;

private Q_SLOTS:
	void dataColumnAboutToBeRemoved(const AbstractAspect*);

protected:
	QQPlot(const QString& name, QQPlotPrivate* dd);

private:
	Q_DECLARE_PRIVATE(QQPlot)
	void init();
	void connectDataColumn(const AbstractColumn*);

	QAction* navigateToAction{nullptr};
	bool m_menusInitialized{false};

Q_SIGNALS:
	void linesUpdated(const QQPlot*, const QVector<QLineF>&);

	// General-Tab
	void dataDataChanged();
	void dataColumnChanged(const AbstractColumn*);
	void distributionChanged(nsl_sf_stats_distribution);
};

#endif
