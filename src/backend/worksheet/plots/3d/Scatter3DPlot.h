#ifndef SCATTER3DPLOT_H
#define SCATTER3DPLOT_H

#include "backend/worksheet/WorksheetElement.h"

class Scatter3DPlotPrivate;
class AbstractColumn;
class QScatter3DSeries;
class KConfig;

class Scatter3DPlot : public WorksheetElement {
	Q_OBJECT
public:
	explicit Scatter3DPlot(const QString& name);
	~Scatter3DPlot() override;

	enum PointStyle {
		Sphere = 0,
		Cube = 1,
		Cone = 2,
		Pyramid = 3,
	};

	BASIC_D_ACCESSOR_DECL(Scatter3DPlot::PointStyle, pointStyle, PointStyle)
	BASIC_D_ACCESSOR_DECL(QColor, color, Color)

	POINTER_D_ACCESSOR_DECL(const AbstractColumn, xColumn, XColumn)
	POINTER_D_ACCESSOR_DECL(const AbstractColumn, yColumn, YColumn)
	POINTER_D_ACCESSOR_DECL(const AbstractColumn, zColumn, ZColumn)

	QScatter3DSeries* series() const;

	void loadThemeConfig(const KConfig&) override;
	void saveThemeConfig(const KConfig&) override;

	void handleResize(double horizontalRatio, double verticalRatio, bool pageResize = false) override;
	void retransform() override;

	typedef Scatter3DPlotPrivate Private;

Q_SIGNALS:
	void pointStyleChanged(Scatter3DPlot::PointStyle);
	void colorChanged(QColor);
	void xColumnChanged(const AbstractColumn*);
	void yColumnChanged(const AbstractColumn*);
	void zColumnChanged(const AbstractColumn*);

private:
	Q_DECLARE_PRIVATE(Scatter3DPlot)
};

#endif // SCATTER3DPLOT_H
