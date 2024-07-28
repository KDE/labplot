#ifndef BAR3DPRIVATEPLOTAREA_H
#define BAR3DPRIVATEPLOTAREA_H

#include "Bar3DPlotArea.h"

#include <backend/worksheet/WorksheetElementContainerPrivate.h>
class WorksheetElementContainerPrivate;
class Bar3DPlotAreaPrivate : public WorksheetElementContainerPrivate {
public:
	explicit Bar3DPlotAreaPrivate(Bar3DPlotArea* owner);
	QVector<AbstractColumn*> columns{nullptr};
	QVector<QString> columnPaths;
	Bar3DPlotArea* const q{nullptr};
	Bar3DPlotArea::ShadowQuality shadowQuality;
	int xRotation;
	int yRotation;
	int zoomLevel;
	Bar3DPlotArea::Theme theme;
	QColor color;
	void retransform() override;
	void recalcShapeAndBoundingRect() override;
	void recalc();

	void updateTheme();
	void updateShadowQuality();
	void updateXRotation();
	void updateYRotation();
	void updateZoomLevel();
	void updateColor();
};

#endif // BAR3DPRIVATEPLOTAREA_H
