#ifndef BASE3DPLOT_H
#define BASE3DPLOT_H

#include <backend/worksheet/WorksheetElementContainer.h>

class Base3DPlotPrivate;
class Q3DSurfaceWidgetItem;
class Q3DScatterWidgetItem;
class Q3DBarsWidgetItem;

class Base3DPlot : public WorksheetElementContainer {
	Q_OBJECT
public:
	enum Type {
		Scatter,
		Surface,
		Bar,
	};
	explicit Base3DPlot(const QString& name, Base3DPlotPrivate*, Base3DPlot::Type, AspectType);
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

	BASIC_D_ACCESSOR_DECL(Base3DPlot::ShadowQuality, shadowQuality, ShadowQuality)
	BASIC_D_ACCESSOR_DECL(Base3DPlot::Theme, theme, Theme)
	BASIC_D_ACCESSOR_DECL(int, zoomLevel, ZoomLevel)
	BASIC_D_ACCESSOR_DECL(int, xRotation, XRotation)
	BASIC_D_ACCESSOR_DECL(int, yRotation, YRotation)
	BASIC_D_ACCESSOR_DECL(Base3DPlot::Type, type, Type)

	void setRect(const QRectF&) override;
	void setPrevRect(const QRectF&) override;

	Q3DSurfaceWidgetItem* m_surface;
	Q3DScatterWidgetItem* m_scatter;
	Q3DBarsWidgetItem* m_bar;
	typedef Base3DPlotPrivate Private;

private:
	Q_DECLARE_PRIVATE(Base3DPlot)

protected:
	void handleResize(double horizontalRatio, double verticalRatio, bool pageResize = false) override;
	void retransform() override;
Q_SIGNALS:
	void themeChanged(Base3DPlot::Theme);
	void shadowQualityChanged(Base3DPlot::ShadowQuality);
	void xRotationChanged(int);
	void yRotationChanged(int);
	void zoomLevelChanged(int);
	void rectChanged(QRectF&);
};

#endif // BASE3DPLOT_H
