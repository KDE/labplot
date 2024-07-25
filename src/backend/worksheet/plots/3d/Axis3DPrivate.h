#ifndef AXIS3DPRIVATE_H
#define AXIS3DPRIVATE_H

#include <backend/core/AspectPrivate.h>
#include <backend/worksheet/WorksheetElementPrivate.h>
#include <backend/worksheet/plots/3d/Axis3D.h>

class WorksheetElementPrivate;
class AbstractAspectPrivate;
class Axis3DPrivate {
public:
	explicit Axis3DPrivate(Axis3D* owner);
	Axis3D* const q{nullptr};
	Axis3D::Format axisFormat;
	Axis3D::Type type;
	QString name() const;
	float minRange;
	float maxRange;
	QString title;
	int segmentCount;
	int subSegmentCount;
	QString formatToString(Axis3D::Format format);
	void updateTitle();
	void updateMaxRange();
	void updateMinRange();
	void updateSegmentCount();
	void updateSubSegmentCount();
	void updateFormat();
};

#endif // AXIS3DPRIVATE_H
