#ifndef PLOT3DAREAPRIVATE_H
#define PLOT3DAREAPRIVATE_H
#include "Plot3DArea.h"

#include <backend/worksheet/WorksheetElementContainerPrivate.h>
class WorksheetElementContainerPrivate;
class Plot3DAreaPrivate : public WorksheetElementContainerPrivate {
public:
	explicit Plot3DAreaPrivate(Plot3DArea* owner, Plot3DArea::Type);

	Plot3DArea::Type type;
	int xRotation;
	int yRotation;
	Plot3DArea::Theme theme;
	int zoomLevel;
	Plot3DArea::ShadowQuality shadowQuality;
	void retransform() override;
	void recalcShapeAndBoundingRect() override;

	Plot3DArea* q{nullptr};

	// trigger update
	void updateXRotation();
	void updateYRotation();
	void updateTheme();
	void updateZoomLevel();
	void updateShadowQuality();
};

#endif // PLOT3DAREAPRIVATE_H
