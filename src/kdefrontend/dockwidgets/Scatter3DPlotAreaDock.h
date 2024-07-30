#ifndef SCATTER3DPLOTAREADOCK_H
#define SCATTER3DPLOTAREADOCK_H
#include "BaseDock.h"
#include "backend/worksheet/plots/3d/Scatter3DPlotArea.h"

#include "ui_scatter3dplotareadock.h"

class Scatter3DPlotArea;
class AbstractColumn;
class TreeViewComboBox;

class Scatter3DPlotAreaDock : public BaseDock {
	Q_OBJECT

public:
	explicit Scatter3DPlotAreaDock(QWidget* parent);
	void setScatters(const QList<Scatter3DPlotArea*>& scatters);

private:
	void updateUiVisibility();
	void load();
	void loadConfig(KConfig&);

private Q_SLOTS:
	void retranslateUi();

	// SLOTs for changes triggered in Scatter3DPlotAreaDock
	void xColumnChanged(const QModelIndex&);
	void yColumnChanged(const QModelIndex&);
	void zColumnChanged(const QModelIndex&);
	void xRotationChanged(int);
	void yRotationChanged(int);
	void zoomLevelChanged(int);
	void shadowQualityChanged(int);
	void pointStyleChanged(int);
	void opacityChanged(double);
	void themeChanged(int);
	void colorChanged(QColor);

	// SLOTs for changes triggered in Scatter3DPlotArea
	void scatterXColumnChanged(const AbstractColumn*);
	void scatterYColumnChanged(const AbstractColumn*);
	void scatterZColumnChanged(const AbstractColumn*);
	void scatterXRotationChanged(int);
	void scatterYRotationChanged(int);
	void scatterZoomLevelChanged(int);
	void scatterShadowQualityChanged(Scatter3DPlotArea::ShadowQuality);
	void scatterPointStyleChanged(Scatter3DPlotArea::PointStyle);
	void scatterOpacityChanged(double);
	void scatterThemeChanged(Scatter3DPlotArea::Theme);
	void scatterColorChanged(QColor);

private:
	Ui::Scatter3DPlotAreaDock ui;
	QList<Scatter3DPlotArea*> m_scatters;
	Scatter3DPlotArea* m_scatter{nullptr};

Q_SIGNALS:
	void info(const QString&);
	void elementVisibilityChanged();
};
#endif // SCATTER3DPLOTAREADOCK_H
