#ifndef AXIS3DPRIVATE_H
#define AXIS3DPRIVATE_H

#include <backend/core/AspectPrivate.h>
#include <backend/worksheet/WorksheetElementPrivate.h>
#include <backend/worksheet/plots/3d/Axis3D.h>

class WorksheetElementPrivate;
class AbstractAspectPrivate;
class Axis3DPrivate : public AbstractAspectPrivate {
public:
	explicit Axis3DPrivate(Axis3D* owner, const QString& name);
	Axis3D* const q{nullptr};
	Axis3D::Format axisFormat;
    Axis3D::Type type;

	float minRange;
	float maxRange;
	QString title;
	int segmentCount;
	int subSegmentCount;
};

#endif // AXIS3DPRIVATE_H
