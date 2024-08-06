#ifndef PLOT3DAREAPRIVATE_H
#define PLOT3DAREAPRIVATE_H
#include "Plot3DArea.h"

#include <backend/worksheet/WorksheetElementContainerPrivate.h>
class WorksheetElementContainerPrivate;
class Plot3DAreaPrivate : public WorksheetElementContainerPrivate {
public:
	explicit Plot3DAreaPrivate(Plot3DArea* owner);
	int xRotation;
	int yRotation;
	Plot3DArea::Theme theme;
	int zoomLevel;
	Plot3DArea::ShadowQuality shadowQuality;
	void retransform() override;
	void recalcShapeAndBoundingRect() override;

	Plot3DArea* q{nullptr};

	// trigger update
	void updateXRotation(Plot3DArea::Type);
	void updateYRotation(Plot3DArea::Type);
	void updateTheme(Plot3DArea::Type);
	void updateZoomLevel(Plot3DArea::Type);
	void updateOpacity(Plot3DArea::Type);
	void updateShadowQuality(Plot3DArea::Type);
};

#endif // PLOT3DAREAPRIVATE_H
