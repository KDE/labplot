#ifndef SCATTER3DPLOTPRIVATE_H
#define SCATTER3DPLOTPRIVATE_H

#include "backend/worksheet/WorksheetElementPrivate.h"

class QScatter3DSeries;

class Scatter3DPlotPrivate : public WorksheetElementPrivate {
public:
	explicit Scatter3DPlotPrivate(Scatter3DPlot*);

	Scatter3DPlot::PointStyle pointStyle{Scatter3DPlot::Sphere};
	QColor color{Qt::blue};

	const AbstractColumn* xColumn{nullptr};
	const AbstractColumn* yColumn{nullptr};
	const AbstractColumn* zColumn{nullptr};

	QScatter3DSeries* series{nullptr};

	Scatter3DPlot* const q;

	void retransform() override {
	}
	void recalcShapeAndBoundingRect() override {
	}
};

#endif // SCATTER3DPLOTPRIVATE_H
