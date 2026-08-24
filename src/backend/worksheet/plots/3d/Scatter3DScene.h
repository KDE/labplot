#ifndef SCATTER3DSCENE_H
#define SCATTER3DSCENE_H

#include "Base3DPlot.h"
#include "backend/core/AbstractColumn.h"
#include <QtGraphsWidgets/Q3DScatterWidgetItem>
#include <backend/worksheet/WorksheetElementContainer.h>

class Scatter3DScenePrivate;
class WorksheetElementContainer;
class Scatter3DScene : public Base3DPlot {
	Q_OBJECT
public:
	Scatter3DScene(const QString& name);
	~Scatter3DScene() override;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;
	void finalizeLoad();
	void init(bool transform = true);
	void finalizeAdd() override;
	void retransform() override;
	enum Theme {
		Qt = 0,
		PrimaryColors = 1,
		StoneMoss = 2,
		ArmyBlue = 3,
		Retro = 4,
		Ebony = 5,
		Isabelle = 6,
	};

	enum ShadowQuality {
		None = 0,
		Low = 1,
		Medium = 2,
		High = 3,
		SoftLow = 4,
		SoftMedium = 5,
		SoftHigh = 6,
	};

	enum PointStyle {
		Sphere = 0,
		Cube = 1,
		Cone = 2,
		Pyramid = 3,
	};
	POINTER_D_ACCESSOR_DECL(const AbstractColumn, xColumn, XColumn)
	POINTER_D_ACCESSOR_DECL(const AbstractColumn, yColumn, YColumn)
	POINTER_D_ACCESSOR_DECL(const AbstractColumn, zColumn, ZColumn)
	BASIC_D_ACCESSOR_DECL(PointStyle, pointStyle, PointStyle)
	BASIC_D_ACCESSOR_DECL(QColor, color, Color)
	CLASS_D_ACCESSOR_DECL(QString, xColumnPath, XColumnPath)
	CLASS_D_ACCESSOR_DECL(QString, yColumnPath, YColumnPath)
	CLASS_D_ACCESSOR_DECL(QString, zColumnPath, ZColumnPath)
	typedef Scatter3DScenePrivate Private;
	void recalc();

	QMenu* createContextMenu() override;

public Q_SLOTS:
	void addPlot();

private:
	void initMenus();
	void handleChildAdded(const AbstractAspect*);
	QMenu* m_addNewMenu{nullptr};
	bool m_menusInitialized{false};

private Q_SLOTS:
	void xColumnAboutToBeRemoved(const AbstractAspect*);
	void yColumnAboutToBeRemoved(const AbstractAspect*);
	void zColumnAboutToBeRemoved(const AbstractAspect*);

protected:
	void handleResize(double horizontalRatio, double verticalRatio, bool pageResize = false) override;

private:
	Q_DECLARE_PRIVATE(Scatter3DScene)

Q_SIGNALS:
	// General-Tab
	void xColumnChanged(const AbstractColumn*);
	void yColumnChanged(const AbstractColumn*);
	void zColumnChanged(const AbstractColumn*);
	void pointStyleChanged(Scatter3DScene::PointStyle);
	void colorChanged(QColor);
};

#endif // SCATTER3DSCENE_H
