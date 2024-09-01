#ifndef PLOT3DAREAPRIVATE_H
#define PLOT3DAREAPRIVATE_H

#include "Axis3D.h"
#include "Bar3DPlot.h"
#include "Plot3DArea.h"
#include "Scatter3DPlot.h"
#include "Surface3DPlot.h"

#include <backend/worksheet/WorksheetElementContainerPrivate.h>

#include <backend/worksheet/plots/AbstractPlotPrivate.h>

class Plot3DAreaPrivate : public AbstractPlotPrivate {
public:
	explicit Plot3DAreaPrivate(Plot3DArea*);
	void init();

	Axis3D* axes;
	QSet<Surface3DPlot*> surfaces;
	QSet<Scatter3DPlot*> scatters;
	QSet<Bar3DPlot*> bars;

	// General parameters

	Plot3DArea* const q;
	bool isInitialized;
	void retransform() override;
	void recalcShapeAndBoundingRect() override;
};
#endif // PLOT3DAREAPRIVATE_H
