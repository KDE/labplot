#ifndef BAR3DPLOTAREA_H
#define BAR3DPLOTAREA_H

#include "backend/core/AbstractColumn.h"
#include <Q3DBars>
#include <backend/worksheet/WorksheetElementContainer.h>

class Bar3DPlotAreaPrivate;
class WorsheetElementContainer;
class Bar3DPlotArea : public WorksheetElementContainer {
	Q_OBJECT
public:
	explicit Bar3DPlotArea(const QString& name);
	~Bar3DPlotArea() override;
	enum ShadowQuality { None = 0, Low = 1, Medium = 2, High = 3, SoftLow = 4, SoftMedium = 5, SoftHigh = 6 };

	enum Theme {
		Qt = 0,
		PrimaryColors = 1,
		StoneMoss = 2,
		ArmyBlue = 3,
		Retro = 4,
		Ebony = 5,
		Isabelle = 6,
	};
	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;
	void recalc();
	BASIC_D_ACCESSOR_DECL(QVector<AbstractColumn*>, dataColumns, DataColumns)
	CLASS_D_ACCESSOR_DECL(QVector<QString>, columnPaths, ColumnPaths)
	BASIC_D_ACCESSOR_DECL(Bar3DPlotArea::ShadowQuality, shadowQuality, ShadowQuality);
	BASIC_D_ACCESSOR_DECL(Bar3DPlotArea::Theme, theme, Theme)
	BASIC_D_ACCESSOR_DECL(QColor, color, Color)
	BASIC_D_ACCESSOR_DECL(int, zoomLevel, ZoomLevel)
	BASIC_D_ACCESSOR_DECL(int, xRotation, XRotation)
	BASIC_D_ACCESSOR_DECL(int, yRotation, YRotation)
	void setRect(const QRectF&) override;
	void setPrevRect(const QRectF&) override;
	typedef Bar3DPlotAreaPrivate Private;
	Q3DBars* m_bar;

private:
	Q_DECLARE_PRIVATE(Bar3DPlotArea)
protected:
	void handleResize(double horizontalRatio, double verticalRatio, bool pageResize = false) override;
	void retransform() override;
private Q_SLOTS:
	void columnAboutToBeRemoved(const AbstractAspect*);
Q_SIGNALS:
	void dataChanged();
	void dataColumnsChanged(QVector<AbstractColumn*>);
	void themeChanged(Bar3DPlotArea::Theme);
	void shadowQualityChanged(Bar3DPlotArea::ShadowQuality);
	void colorChanged(QColor);
	void xRotationChanged(int);
	void yRotationChanged(int);
	void zoomLevelChanged(int);
	void rectChanged(QRectF&);
};
#endif // BAR3DPLOTAREA_H
