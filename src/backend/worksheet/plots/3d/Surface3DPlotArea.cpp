#include "Surface3DPlotArea.h"
#include "Surface3DPlotAreaPrivate.h"
#include <QtGraphs/Q3DSurface>

Surface3DPlotArea::Surface3DPlotArea(const QString& name)
	: WorksheetElement(name, new Surface3DPlotAreaPrivate(this), AspectType::SurfacePlot)
	, m_surface{new Q3DSurface()} {
}

Surface3DPlotArea::~Surface3DPlotArea() {
    delete m_surface;
}

void Surface3DPlotArea::handleResize(double horizontalRatio, double verticalRatio, bool pageResize) {
}
void Surface3DPlotArea::retransform(){};

// #####################################################################
// ################### Private implementation ##########################
// #####################################################################
Surface3DPlotAreaPrivate::Surface3DPlotAreaPrivate(Surface3DPlotArea* owner)
	: WorksheetElementPrivate(owner)
	, q(owner) {
}

void Surface3DPlotAreaPrivate::retransform() {
}
void Surface3DPlotAreaPrivate::recalcShapeAndBoundingRect() {
}
