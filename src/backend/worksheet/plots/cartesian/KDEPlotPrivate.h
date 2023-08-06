/*
	File                 : KDEPlotPrivate.h
	Project              : LabPlot
	Description          : Private members of KDEPlot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEPLOTPRIVATE_H
#define KDEPLOTPRIVATE_H

#include "backend/worksheet/plots/cartesian/PlotPrivate.h"

class Column;
class Line;
class KDEPlot;

class KDEPlotPrivate : public PlotPrivate {
public:
	explicit KDEPlotPrivate(KDEPlot* owner);
	~KDEPlotPrivate() override;

	QRectF boundingRect() const override;
	QPainterPath shape() const override;

	void retransform() override;
	void recalc();
	void updateDistribution();
	void recalcShapeAndBoundingRect() override;

	void setHover(bool on);
	bool activateCurve(QPointF mouseScenePos, double maxDist);

	QPainterPath curveShape;
	bool m_suppressRecalc{false};

	// General
	const AbstractColumn* dataColumn{nullptr};
	QString dataColumnPath;
	nsl_kernel_type kernelType{nsl_kernel_gauss};
	nsl_kde_bandwidth_type bandwidthType{nsl_kde_bandwidth_gaussian};
	double bandwidth{0.1};

	// KDE curve
	XYCurve* estimationCurve{nullptr};
	Column* xEstimationColumn{nullptr};
	QString xEstimationColumnPath;
	Column* yEstimationColumn{nullptr};
	QString yEstimationColumnPath;

	Histogram* histogram{nullptr};

	KDEPlot* const q;

private:
	bool m_hovered{false};
	QPixmap m_pixmap;
	QImage m_hoverEffectImage;
	QImage m_selectionEffectImage;
	bool m_hoverEffectImageIsDirty{false};
	bool m_selectionEffectImageIsDirty{false};

	void copyValidData(QVector<double>&) const;
};

#endif
