#ifndef SURFACE3DPLOTPRIVATE_H
#define SURFACE3DPLOTPRIVATE_H

#include "backend/worksheet/WorksheetElementPrivate.h"

class QSurface3DSeries;

class Surface3DPlotPrivate : public WorksheetElementPrivate {
public:
	explicit Surface3DPlotPrivate(Surface3DPlot*);

	Surface3DPlot::DrawMode drawMode{Surface3DPlot::DrawSurface};
	Surface3DPlot::DataSource dataSource{Surface3DPlot::DataSource_Empty};
	bool flatShading{false};
	bool smooth{false};
	QColor color{Qt::blue};

	const AbstractColumn* xColumn{nullptr};
	const AbstractColumn* yColumn{nullptr};
	const AbstractColumn* zColumn{nullptr};
	const Matrix* matrix{nullptr};

	QSurface3DSeries* series{nullptr};

	Surface3DPlot* const q;

	void retransform() override {}
	void recalcShapeAndBoundingRect() override {}
};

#endif // SURFACE3DPLOTPRIVATE_H
