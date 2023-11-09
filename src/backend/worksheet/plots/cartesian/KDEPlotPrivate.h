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

	void retransform() override;
	void recalc();
	void updateDistribution();
	void recalcShapeAndBoundingRect() override;

	// General
	const AbstractColumn* dataColumn{nullptr};
	QString dataColumnPath;
	nsl_kernel_type kernelType{nsl_kernel_gauss};
	nsl_kde_bandwidth_type bandwidthType{nsl_kde_bandwidth_silverman};
	double bandwidth{0.1};
	int gridPointsCount{200}; // number of equaly spaced points at which the density is to be evaluated

	// KDE curve
	XYCurve* estimationCurve{nullptr};
	Column* xEstimationColumn{nullptr};
	QString xEstimationColumnPath;
	Column* yEstimationColumn{nullptr};
	QString yEstimationColumnPath;

	XYCurve* rugCurve{nullptr};

	KDEPlot* const q;

private:
	void copyValidData(QVector<double>&) const;
};

#endif
