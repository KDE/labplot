#ifndef SCATTER3DPLOTAREAPRIVATE_H
#define SCATTER3DPLOTAREAPRIVATE_H
#include "backend/worksheet/plots/3d/Scatter3DPlotArea.h"
#include <backend/core/AbstractColumn.h>
#include <backend/worksheet/WorksheetElementContainerPrivate.h>
#include <backend/worksheet/plots/cartesian/Symbol.h>
class WorksheetElementContainerPrivate;
class Scatter3DPlotArea;
class Scatter3DPlotAreaPrivate : public WorksheetElementContainerPrivate {
public:
	explicit Scatter3DPlotAreaPrivate(Scatter3DPlotArea*);

	const AbstractColumn* xColumn{nullptr};
	const AbstractColumn* yColumn{nullptr};
	const AbstractColumn* zColumn{nullptr};
	Scatter3DPlotArea::Theme theme{Scatter3DPlotArea::Theme::Qt};
	Scatter3DPlotArea::PointStyle pointStyle{Scatter3DPlotArea::PointStyle::Sphere};
	Scatter3DPlotArea::ShadowQuality shadowQuality{Scatter3DPlotArea::ShadowQuality::High};
	double opacity;
	QColor color;
	QString xColumnPath;
	QString yColumnPath;
	QString zColumnPath;
	int xRotation;
	int yRotation;
	int zoomLevel;
	void retransform() override;
	void recalcShapeAndBoundingRect() override;
	void recalc();

	Scatter3DPlotArea* const q{nullptr};

	void updateTheme();
	void updatePointStyle();
	void updateShadowQuality();
	void updateColor();
	void updateOpacity();
	void updateXRotation();
	void updateYRotation();
	void updateZoomLevel();
};

#endif // SCATTER3DPLOTAREAPRIVATE_H
