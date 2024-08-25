#ifndef PLOT3DAREAPRIVATE_H
#define PLOT3DAREAPRIVATE_H

#include "Axis3D.h"
#include "Bar3DPlot.h"
#include "Plot3DArea.h"
#include "Scatter3DPlot.h"
#include "Surface3DPlot.h"

#include <backend/worksheet/WorksheetElementContainerPrivate.h>

class Plot3DArea;

class Plot3DAreaPrivate : public WorksheetElementContainerPrivate {
	Q_OBJECT
public:
	explicit Plot3DAreaPrivate(Plot3DArea* owner);
	~Plot3DAreaPrivate() override;
	void init();

	Axis3D* axes;
	QSet<Surface3DPlot*> surfaces;
	QSet<Scatter3DPlot*> scatters;
	QSet<Bar3DPlot*> bars;

	// General parameters

	Plot3DArea* const q;
	bool isInitialized;
	void retransform() override;



};
#endif // PLOT3DAREAPRIVATE_H
