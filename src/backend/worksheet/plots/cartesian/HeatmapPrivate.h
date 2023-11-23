/*
	File                 : HeatmapPrivate.h
	Project              : LabPlot
	Description          : Heatmap - private implementation
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Martin Marmsoler <martin.marmsoler@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef HEATMAPPRIVATE_H
#define HEATMAPPRIVATE_H

#include "Heatmap.h"
#include "PlotPrivate.h"

class AbstractColumn;
class Matrix;

class HeatmapPrivate : public PlotPrivate {
public:
	explicit HeatmapPrivate(Heatmap*);

	// Members
	Heatmap::DataSource dataSource{Heatmap::DataSource::Matrix};
	bool drawEmpty{false};
	bool numBinsEqual{true};
	unsigned int xNumBins{10};
	unsigned int yNumBins{10};
	const AbstractColumn* xColumn{nullptr};
	const AbstractColumn* yColumn{nullptr};
	QString xColumnPath;
	QString yColumnPath;
	const Matrix* matrix{nullptr};
	QString matrixPath;
	bool automaticLimits{true};
	Heatmap::Format format;

	void retransform() override;
	void recalcShapeAndBoundingRect() override;
	void recalcShapeAndBoundingRect(const QRectF&);

	struct Data {
		QRectF rect;
		QColor color;
	};
	std::vector<Data> data;

	Heatmap* const q;

private:
	void draw(QPainter*);
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* widget = nullptr) override;
	void update();
	void updatePixmap();
};

#endif // HEATMAPPRIVATE_H
