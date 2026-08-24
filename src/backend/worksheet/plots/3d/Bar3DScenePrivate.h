#ifndef BAR3DPRIVATEPLOTAREA_H
#define BAR3DPRIVATEPLOTAREA_H

#include "Bar3DScene.h"
#include "Base3DPlotPrivate.h"

#include <backend/worksheet/WorksheetElementContainerPrivate.h>

class WorksheetElementContainerPrivate;
class Bar3DScenePrivate : public Base3DPlotPrivate {
public:
	explicit Bar3DScenePrivate(Bar3DScene* owner);
	QVector<AbstractColumn*> dataColumns{nullptr};
	QVector<QString> columnPaths;
	Bar3DScene* const q{nullptr};
	QColor color;
	void retransform() override;
	void recalcShapeAndBoundingRect() override;
	void recalc();
	void updateColor();
};

#endif // BAR3DPRIVATEPLOTAREA_H
