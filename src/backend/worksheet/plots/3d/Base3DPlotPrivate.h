#ifndef BASE3DPLOTPRIVATE_H
#define BASE3DPLOTPRIVATE_H
#include "Base3DPlot.h"

#include <backend/worksheet/WorksheetElementContainerPrivate.h>
class WorksheetElementContainerPrivate;
class Base3DPlotPrivate : public WorksheetElementContainerPrivate {
public:
	explicit Base3DPlotPrivate(Base3DPlot* owner);
	Base3DPlot::Type type;
	int xRotation;
	int yRotation;
	Base3DPlot::Theme theme;
	int zoomLevel;
	Base3DPlot::ShadowQuality shadowQuality;
	void retransform() override;
	void recalcShapeAndBoundingRect() override;

	Base3DPlot* q{nullptr};

	// trigger update
	void updateXRotation();
	void updateYRotation();
	void updateTheme();
	void updateZoomLevel();
	void updateShadowQuality();
};

#endif // BASE3DPLOTPRIVATE_H
