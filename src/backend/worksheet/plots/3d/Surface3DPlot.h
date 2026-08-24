#ifndef SURFACE3DPLOT_H
#define SURFACE3DPLOT_H

#include "backend/worksheet/WorksheetElement.h"

class Surface3DPlotPrivate;
class AbstractColumn;
class Matrix;
class QSurface3DSeries;

class Surface3DPlot : public WorksheetElement {
	Q_OBJECT
public:
	explicit Surface3DPlot(const QString& name);
	~Surface3DPlot() override;

	enum DrawMode {
		DrawWireframe = 1,
		DrawSurface = 2,
		DrawWireframeSurface = 3,
	};

	enum DataSource {
		DataSource_Spreadsheet = 0,
		DataSource_Matrix = 1,
		DataSource_Empty = 2,
	};

	BASIC_D_ACCESSOR_DECL(Surface3DPlot::DrawMode, drawMode, DrawMode)
	BASIC_D_ACCESSOR_DECL(Surface3DPlot::DataSource, dataSource, DataSource)
	BASIC_D_ACCESSOR_DECL(bool, flatShading, FlatShading)
	BASIC_D_ACCESSOR_DECL(bool, smooth, Smooth)
	BASIC_D_ACCESSOR_DECL(QColor, color, Color)

	POINTER_D_ACCESSOR_DECL(const AbstractColumn, xColumn, XColumn)
	POINTER_D_ACCESSOR_DECL(const AbstractColumn, yColumn, YColumn)
	POINTER_D_ACCESSOR_DECL(const AbstractColumn, zColumn, ZColumn)
	POINTER_D_ACCESSOR_DECL(const Matrix, matrix, Matrix)

	QSurface3DSeries* series() const;

	typedef Surface3DPlotPrivate Private;

Q_SIGNALS:
	void drawModeChanged(Surface3DPlot::DrawMode);
	void sourceTypeChanged(Surface3DPlot::DataSource);
	void flatShadingChanged(bool);
	void smoothChanged(bool);
	void colorChanged(QColor);
	void xColumnChanged(const AbstractColumn*);
	void yColumnChanged(const AbstractColumn*);
	void zColumnChanged(const AbstractColumn*);
	void matrixChanged(const Matrix*);

private:
	Q_DECLARE_PRIVATE(Surface3DPlot)
};

#endif // SURFACE3DPLOT_H
