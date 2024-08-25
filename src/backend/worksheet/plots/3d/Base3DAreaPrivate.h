#ifndef Base3DAREAPRIVATE_H
#define Base3DAREAPRIVATE_H
#include "Base3DArea.h"

#include <backend/worksheet/WorksheetElementContainerPrivate.h>
class WorksheetElementContainerPrivate;
class Base3DAreaPrivate : public WorksheetElementContainerPrivate {
public:
	explicit Base3DAreaPrivate(Base3DArea* owner);
	Base3DArea::Type type;
	int xRotation;
	int yRotation;
	Base3DArea::Theme theme;
	int zoomLevel;
	Base3DArea::ShadowQuality shadowQuality;
	void retransform() override;
	void recalcShapeAndBoundingRect() override;

	Base3DArea* q{nullptr};

	// trigger update
	void updateXRotation();
	void updateYRotation();
	void updateTheme();
	void updateZoomLevel();
	void updateShadowQuality();
};

#endif // Base3DAREAPRIVATE_H
