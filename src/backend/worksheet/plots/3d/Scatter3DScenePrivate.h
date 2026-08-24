#ifndef Scatter3DScenePRIVATE_H
#define Scatter3DScenePRIVATE_H
#include "Base3DPlotPrivate.h"
#include "backend/worksheet/plots/3d/Scatter3DScene.h"
#include <backend/core/AbstractColumn.h>
#include <backend/worksheet/WorksheetElementContainerPrivate.h>
#include <backend/worksheet/plots/cartesian/Symbol.h>
class WorksheetElementContainerPrivate;
class Scatter3DScene;
class Scatter3DScenePrivate : public Base3DPlotPrivate {
public:
	explicit Scatter3DScenePrivate(Scatter3DScene*);

	const AbstractColumn* xColumn{nullptr};
	const AbstractColumn* yColumn{nullptr};
	const AbstractColumn* zColumn{nullptr};
	Scatter3DScene::PointStyle pointStyle{Scatter3DScene::PointStyle::Sphere};
	QColor color;
	QString xColumnPath;
	QString yColumnPath;
	QString zColumnPath;
	void retransform() override;
	void recalcShapeAndBoundingRect() override;
	void recalc();

	Scatter3DScene* const q{nullptr};

	void updatePointStyle();
	void updateColor();
};

#endif // Scatter3DScenePRIVATE_H
