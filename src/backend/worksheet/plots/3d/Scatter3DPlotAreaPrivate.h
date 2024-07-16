#ifndef SCATTER3DPLOTAREAPRIVATE_H
#define SCATTER3DPLOTAREAPRIVATE_H
#include "qfont.h"
#include <backend/core/AbstractColumn.h>
#include <backend/worksheet/WorksheetElementContainerPrivate.h>
#include <backend/worksheet/plots/cartesian/Symbol.h>
class WorksheetElementContainerPrivate;
class Scatter3DPlotArea;
class Scatter3DPlotAreaPrivate : public WorksheetElementContainerPrivate {
public:
	explicit Scatter3DPlotAreaPrivate(Scatter3DPlotArea*);

	const AbstractColumn* xColumn;
	const AbstractColumn* yColumn;
	const AbstractColumn* zColumn;
	QString xColumnPath;
	QString yColumnPath;
	QString zColumnPath;
	void retransform() override;
	void recalcShapeAndBoundingRect() override;
	void generateData();

	Scatter3DPlotArea* const q;
};

#endif // SCATTER3DPLOTAREAPRIVATE_H
