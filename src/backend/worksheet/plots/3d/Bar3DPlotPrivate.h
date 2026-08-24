#ifndef BAR3DPLOTPRIVATE_H
#define BAR3DPLOTPRIVATE_H

#include "backend/worksheet/WorksheetElementPrivate.h"

class QBar3DSeries;

class Bar3DPlotPrivate : public WorksheetElementPrivate {
public:
	explicit Bar3DPlotPrivate(Bar3DPlot*);

	QColor color{Qt::blue};
	QVector<const AbstractColumn*> dataColumns;

	QBar3DSeries* series{nullptr};

	Bar3DPlot* const q;

	void retransform() override {}
	void recalcShapeAndBoundingRect() override {}
};

#endif // BAR3DPLOTPRIVATE_H
