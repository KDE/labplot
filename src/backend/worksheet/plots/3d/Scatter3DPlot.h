#ifndef Scatter3DPlot_H
#define Scatter3DPlot_H

#include "Plot3DArea.h"
#include "backend/core/AbstractColumn.h"
#include <Q3DScatter>
#include <backend/worksheet/WorksheetElementContainer.h>

class Scatter3DPlotPrivate;
class WorksheetElementContainer;
class Scatter3DPlot : public Plot3DArea {
	Q_OBJECT
public:
	Scatter3DPlot(const QString& name);
	~Scatter3DPlot() override;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;
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

	void setRect(const QRectF&) override;
	void setPrevRect(const QRectF&) override;
	CLASS_D_ACCESSOR_DECL(QString, xColumnPath, XColumnPath)
	CLASS_D_ACCESSOR_DECL(QString, yColumnPath, YColumnPath)
	CLASS_D_ACCESSOR_DECL(QString, zColumnPath, ZColumnPath)
	typedef Scatter3DPlotPrivate Private;
	void recalc();
private Q_SLOTS:
	void xColumnAboutToBeRemoved(const AbstractAspect*);
	void yColumnAboutToBeRemoved(const AbstractAspect*);
	void zColumnAboutToBeRemoved(const AbstractAspect*);

protected:
	void handleResize(double horizontalRatio, double verticalRatio, bool pageResize = false) override;
	void retransform() override;

private:
	Q_DECLARE_PRIVATE(Scatter3DPlot)

Q_SIGNALS:
	// General-Tab
	void rectChanged(QRectF&);
	void xColumnChanged(const AbstractColumn*);
	void yColumnChanged(const AbstractColumn*);
	void zColumnChanged(const AbstractColumn*);
	void pointStyleChanged(Scatter3DPlot::PointStyle);
	void colorChanged(QColor);
};

#endif // Scatter3DPlot_H
