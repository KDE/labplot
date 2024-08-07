#ifndef PLOT3DAREA_H
#define PLOT3DAREA_H
#include <backend/worksheet/WorksheetElementContainer.h>

#include <Q3DBars>
#include <Q3DScatter>
#include <Q3DSurface>
class WorksheetElementContainer;
class Plot3DAreaPrivate;
class Plot3DArea : public WorksheetElementContainer {
	Q_OBJECT
public:
	explicit Plot3DArea(const QString& name, Plot3DAreaPrivate*, AspectType);
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
	enum Type {
		Scatter,
		Surface,
		Bar,
	};

	BASIC_D_ACCESSOR_DECL(Plot3DArea::ShadowQuality, shadowQuality, ShadowQuality)
	BASIC_D_ACCESSOR_DECL(Plot3DArea::Theme, theme, Theme)
	BASIC_D_ACCESSOR_DECL(int, zoomLevel, ZoomLevel)
	BASIC_D_ACCESSOR_DECL(int, xRotation, XRotation)
	BASIC_D_ACCESSOR_DECL(int, yRotation, YRotation)
	BASIC_D_ACCESSOR_DECL(Plot3DArea::Type, type, Type)

	void setRect(const QRectF&) override;
	void setPrevRect(const QRectF&) override;

	Q3DSurface* m_surface;
	Q3DScatter* m_scatter;
	Q3DBars* m_bar;
	typedef Plot3DAreaPrivate Private;

private:
	Q_DECLARE_PRIVATE(Plot3DArea)
protected:
	Plot3DAreaPrivate* const d_ptr;

protected:
	void handleResize(double horizontalRatio, double verticalRatio, bool pageResize = false) override;
	void retransform() override;
Q_SIGNALS:
	void themeChanged(Plot3DArea::Theme);
	void shadowQualityChanged(Plot3DArea::ShadowQuality);
	void xRotationChanged(int);
	void yRotationChanged(int);
	void zoomLevelChanged(int);
	void rectChanged(QRectF&);
};

#endif // PLOT3DAREA_H
