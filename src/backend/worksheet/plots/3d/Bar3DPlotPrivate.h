#ifndef BAR3DPRIVATEPLOTAREA_H
#define BAR3DPRIVATEPLOTAREA_H

#include "Bar3DPlot.h"
#include "Base3DAreaPrivate.h"

#include <backend/worksheet/WorksheetElementContainerPrivate.h>

class WorksheetElementContainerPrivate;
class Bar3DPlotPrivate : public Base3DAreaPrivate {
public:
	explicit Bar3DPlotPrivate(Bar3DPlot* owner);
	QVector<AbstractColumn*> dataColumns{nullptr};
	QVector<QString> columnPaths;
	Bar3DPlot* const q{nullptr};
	QColor color;
	void retransform() override;
	void recalcShapeAndBoundingRect() override;
	void recalc();
	void updateColor();
};

#endif // BAR3DPRIVATEPLOTAREA_H
