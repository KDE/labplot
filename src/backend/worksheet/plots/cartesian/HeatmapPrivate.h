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

#include <vector>

class AbstractColumn;
class Matrix;

class HeatmapPrivate : public PlotPrivate {
public:
	explicit HeatmapPrivate(Heatmap*);

	// Members
	Heatmap::DataSource dataSource{Heatmap::DataSource::Matrix};
	bool drawEmpty{false};
	bool equalNumberBins{true};
	bool sourceNumberBins{true}; // Use number of cells of the matrix as bins
	unsigned int xNumberBins{10};
	unsigned int yNumberBins{10};
	const AbstractColumn* xColumn{nullptr};
	const AbstractColumn* yColumn{nullptr};
	QString xColumnPath;
	QString yColumnPath;
	const Matrix* matrix{nullptr};
	QString matrixPath;
	bool automaticLimits{true};
	Heatmap::Format format;

	void retransform() override;
	void recalcAndRetransform();
	void recalcShapeAndBoundingRect() override;
	void recalcShapeAndBoundingRect(const QRectF&);
	void recalc();

	struct Data {
		QRectF rect;
		QColor color;
	};
	std::vector<Data> data;

	Heatmap* const q;

private:
	void draw(QPainter*);
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* = nullptr) override;
	QRectF calculateScenePoints();
	void updatePixmap();

	// Full logical grid, independent of the visible plot ranges.
	std::vector<std::vector<double>> map;
	int xBinCount{0};
	int yBinCount{0};
	double xMin{0.};
	double yMin{0.};
	double xBinSize{0.};
	double yBinSize{0.};
};

#endif // HEATMAPPRIVATE_H
