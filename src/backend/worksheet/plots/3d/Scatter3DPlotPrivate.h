#ifndef Scatter3DPlotPRIVATE_H
#define Scatter3DPlotPRIVATE_H
#include "Base3DAreaPrivate.h"
#include "backend/worksheet/plots/3d/Scatter3DPlot.h"
#include <backend/core/AbstractColumn.h>
#include <backend/worksheet/WorksheetElementContainerPrivate.h>
#include <backend/worksheet/plots/cartesian/Symbol.h>
class WorksheetElementContainerPrivate;
class Scatter3DPlot;
class Scatter3DPlotPrivate : public Base3DAreaPrivate {
public:
	explicit Scatter3DPlotPrivate(Scatter3DPlot*);

	const AbstractColumn* xColumn{nullptr};
	const AbstractColumn* yColumn{nullptr};
	const AbstractColumn* zColumn{nullptr};
	Scatter3DPlot::PointStyle pointStyle{Scatter3DPlot::PointStyle::Sphere};
	QColor color;
	QString xColumnPath;
	QString yColumnPath;
	QString zColumnPath;
	void retransform() override;
	void recalcShapeAndBoundingRect() override;
	void recalc();

	Scatter3DPlot* const q{nullptr};

	void updatePointStyle();
	void updateColor();
};

#endif // Scatter3DPlotPRIVATE_H
