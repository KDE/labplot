/*
	File                 : KDEPlot.h
	Project              : LabPlot
	Description          : KDE-Plot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEPLOT_H
#define KDEPLOT_H

#include "Plot.h"
#include "backend/nsl/nsl_kde.h"
#include "backend/nsl/nsl_sf_kernel.h"

class KDEPlotPrivate;
class XYCurve;

#ifdef SDK
#include "labplot_export.h"
class LABPLOT_EXPORT KDEPlot : public Plot {
#else
class KDEPlot : public Plot {
#endif
	Q_OBJECT

public:
	friend class KDEPlotSetDataColumnCmd;

	explicit KDEPlot(const QString& name);
	~KDEPlot() override;

	void finalizeAdd() override;

	QIcon icon() const override;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

	void loadThemeConfig(const KConfig&) override;
	void saveThemeConfig(const KConfig&) override;

	POINTER_D_ACCESSOR_DECL(const AbstractColumn, dataColumn, DataColumn)
	CLASS_D_ACCESSOR_DECL(QString, dataColumnPath, DataColumnPath)
	BASIC_D_ACCESSOR_DECL(nsl_kernel_type, kernelType, KernelType)
	BASIC_D_ACCESSOR_DECL(nsl_kde_bandwidth_type, bandwidthType, BandwidthType)
	BASIC_D_ACCESSOR_DECL(double, bandwidth, Bandwidth)

	XYCurve* estimationCurve() const;
	XYCurve* rugCurve() const;

	void retransform() override;
	void recalc() override;
	void handleResize(double horizontalRatio, double verticalRatio, bool pageResize) override;
	void setVisible(bool) override;

	bool indicesMinMax(const Dimension dim, double v1, double v2, int& start, int& end) const override;
	bool minMax(const CartesianCoordinateSystem::Dimension dim, const Range<int>& indexRange, Range<double>& r, bool includeErrorBars = true) const override;
	double minimum(CartesianCoordinateSystem::Dimension) const override;
	double maximum(CartesianCoordinateSystem::Dimension) const override;
	int gridPointsCount() const;
	bool hasData() const override;
	int dataCount(Dimension) const override;
	bool usingColumn(const AbstractColumn*, bool indirect) const override;
	void handleAspectUpdated(const QString& aspectPath, const AbstractAspect* element) override;
	QColor color() const override;

	typedef KDEPlotPrivate Private;

private Q_SLOTS:
	void dataColumnAboutToBeRemoved(const AbstractAspect*);

protected:
	KDEPlot(const QString& name, KDEPlotPrivate* dd);

private:
	Q_DECLARE_PRIVATE(KDEPlot)
	void init();
	void connectDataColumn(const AbstractColumn*);
	QAction* navigateToAction{nullptr};
	bool m_menusInitialized{false};

Q_SIGNALS:
	void linesUpdated(const KDEPlot*, const QVector<QLineF>&);

	// General-Tab
	void dataDataChanged();
	void dataColumnChanged(const AbstractColumn*);
	void kernelTypeChanged(nsl_kernel_type);
	void bandwidthTypeChanged(nsl_kde_bandwidth_type);
	void bandwidthChanged(double);
};

#endif
