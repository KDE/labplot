#ifndef SURFACE3DPLOTAREAPRIVATE_H
#define SURFACE3DPLOTAREAPRIVATE_H

#include <backend/worksheet/WorksheetElementPrivate.h>


class Surface3DPlotArea;
class WorksheetElementPrivate;
class Surface3DPlotAreaPrivate : public WorksheetElementPrivate {
public:
	explicit Surface3DPlotAreaPrivate(Surface3DPlotArea* owner);
	Surface3DPlotArea* const q{nullptr};
	void retransform() override;
	void recalcShapeAndBoundingRect() override;
};
#endif // SURFACE3DPLOTAREAPRIVATE_H
