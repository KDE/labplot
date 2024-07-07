#ifndef SURFACE3DPLOTAREA_H
#define SURFACE3DPLOTAREA_H

#include <QtGraphs/Q3DSurface>

#include <backend/worksheet/WorksheetElement.h>

#include <backend/core/AbstractColumn.h>

#include <backend/matrix/Matrix.h>

class AbstractColumn;
class Surface3DPlotAreaPrivate;
class WorksheetElemet;
class Surface3DPlotArea : public WorksheetElement {
	Q_OBJECT
public:
    Surface3DPlotArea(const QString& name);
    ~Surface3DPlotArea() override;
    void handleResize(double horizontalRatio, double verticalRatio, bool pageResize = false) override;
    void retransform() override;
    enum DataSource { DataSource_Spreadsheet = 0, DataSource_Matrix = 1, DataSource_Empty = 2 };
    enum DrawMode { DrawSurface = 0, DrawWireframe = 1 };
    enum MeshType {
        UserDefined = 0,
        Bar = 1,
        Cube = 2,
        Pyramid = 3,
        Cone = 4,
        Cylinder = 5,
        BevelBar = 6,
        BevelCube = 7,
        Sphere = 8,
        Minimal = 9,
        Arrow = 10,
        Point = 11
    };
    enum ShadowQuality { None = 0, Low = 1, Medium = 2, High = 3, SoftLow = 4, SoftMedium = 5, SoftHigh = 6 };

	BASIC_D_ACCESSOR_DECL(DataSource, dataSource, DataSource)
	BASIC_D_ACCESSOR_DECL(DrawMode, drawMode, DrawMode)
	BASIC_D_ACCESSOR_DECL(MeshType, meshType, MeshType)
	BASIC_D_ACCESSOR_DECL(QColor, color, Color)
	BASIC_D_ACCESSOR_DECL(double, opacity, Opacity)
	BASIC_D_ACCESSOR_DECL(bool, flatShading, FlatShading)
	BASIC_D_ACCESSOR_DECL(bool, gridVisibility, GridVisibility)
    BASIC_D_ACCESSOR_DECL(ShadowQuality, shadowQuality, ShadowQuality)
    BASIC_D_ACCESSOR_DECL(bool, smooth, Smooth)
    BASIC_D_ACCESSOR_DECL(int, zoomLevel, ZoomLevel)
    BASIC_D_ACCESSOR_DECL(int, xRotation, XRotation)
    BASIC_D_ACCESSOR_DECL(int, yRotation, YRotation)

    // Matrix parameters
    POINTER_D_ACCESSOR_DECL(const Matrix, matrix, Matrix)
    const QString& matrixPath() const;

    // Spreadsheet parameters
	POINTER_D_ACCESSOR_DECL(const AbstractColumn, xColumn, XColumn)
	POINTER_D_ACCESSOR_DECL(const AbstractColumn, yColumn, YColumn)
	POINTER_D_ACCESSOR_DECL(const AbstractColumn, zColumn, ZColumn)

    const QString& xColumnPath() const;
    const QString& yColumnPath() const;
    const QString& zColumnPath() const;

    void show(bool visible);

	Q3DSurface* m_surface;
	QSurfaceDataProxy* m_proxy;
	QSurface3DSeries* m_series;
	typedef Surface3DPlotAreaPrivate Private;

private:
	Q_DECLARE_PRIVATE(Surface3DPlotArea)
private Q_SLOTS:
	// Spreadsheet slots
	void xColumnAboutToBeRemoved(const AbstractAspect*);
	void yColumnAboutToBeRemoved(const AbstractAspect*);
    void zColumnAboutToBeRemoved(const AbstractAspect*);

    // Matrix slots
    void matrixAboutToBeRemoved(const AbstractAspect*);

Q_SIGNALS:
	void sourceTypeChanged(Surface3DPlotArea::DataSource);
    void drawModeChanged(Surface3DPlotArea::DrawMode);
    void meshTypeChanged(Surface3DPlotArea::MeshType);
    void colorChanged(QColor);
    void opacityChanged(double);
    void flatShadingChanged(bool);
    void gridVisibilityChanged(bool);
    void lightningChanged(bool);
    void shadowQualityChanged(Surface3DPlotArea::ShadowQuality);
    void smoothChanged(bool);
    void matrixChanged(const Matrix*);
    void xColumnChanged(const AbstractColumn*);
    void yColumnChanged(const AbstractColumn*);
    void zColumnChanged(const AbstractColumn*);
    void zoomChanged(int);
    void xRotationChanged(int);
    void yRotationChanged(int);
};
#endif // SURFACE3DPLOTAREA_H
