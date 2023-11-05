/*
	File                 : QQPlotPrivate.h
	Project              : LabPlot
	Description          : Private members of QQPlot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef QQPLOTPRIVATE_H
#define QQPLOTPRIVATE_H

#include "backend/worksheet/plots/cartesian/PlotPrivate.h"

class Column;
class Line;
class QQPlot;

class QQPlotPrivate : public PlotPrivate {
public:
	explicit QQPlotPrivate(QQPlot* owner);
	~QQPlotPrivate() override;

	void retransform() override;
	void recalc();
	void updateDistribution();
	void recalcShapeAndBoundingRect() override;

	XYCurve* referenceCurve{nullptr};
	Column* xReferenceColumn{nullptr};
	QString xReferenceColumnPath;
	Column* yReferenceColumn{nullptr};
	QString yReferenceColumnPath;

	XYCurve* percentilesCurve{nullptr};
	Column* xPercentilesColumn{nullptr};
	QString xPercentilesColumnPath;
	Column* yPercentilesColumn{nullptr};
	QString yPercentilesColumnPath;

	// General
	const AbstractColumn* dataColumn{nullptr};
	QString dataColumnPath;
	nsl_sf_stats_distribution distribution{nsl_sf_stats_gaussian};

	QQPlot* const q;

private:
	void copyValidData(QVector<double>&) const;
};

#endif
