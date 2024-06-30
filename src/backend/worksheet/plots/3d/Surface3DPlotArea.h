#ifndef SURFACE3DPLOTAREA_H
#define SURFACE3DPLOTAREA_H

#include <QtGraphs/Q3DSurface>

#include <backend/worksheet/WorksheetElement.h>
class Surface3DPlotAreaPrivate;
class Surface3DPlotArea : public WorksheetElement {
    Q_OBJECT
public:
    Surface3DPlotArea(const QString& name);
	~Surface3DPlotArea() override;
	void handleResize(double horizontalRatio, double verticalRatio, bool pageResize = false) override;
    void retransform() override;

    // Q3DSurface* graph() const;
    Q3DSurface* m_surface;

private:
    Q_DECLARE_PRIVATE(Surface3DPlotArea)
};
#endif //SURFACE3DPLOTAREA_H
