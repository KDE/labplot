#ifndef BAR3DPLOTAREADOCK_H
#define BAR3DPLOTAREADOCK_H

#include "BaseDock.h"
#include "backend/worksheet/plots/3d/Bar3DPlotArea.h"
#include "ui_bar3dplotareadock.h"

class AbstractColumn;
class TreeViewComboBox;
class Bar3DPlotArea;
class Bar3DPlotAreaDock : public BaseDock {
	Q_OBJECT
public:
	explicit Bar3DPlotAreaDock(QWidget* parent);
	void setBars(const QList<Bar3DPlotArea*>& bars);

private:
	// for data columns
	QGridLayout* m_gridLayout;
	QPushButton* m_buttonNew;
	QVector<TreeViewComboBox*> m_dataComboBoxes;
	QVector<QPushButton*> m_removeButtons;

private:
	void updateUiVisibility();
	void load();
	void loadConfig(KConfig&);
	void setDataColumns();

private Q_SLOTS:
	void retranslateUi();
	void addDataColumn();
	void removeDataColumn();
	// SLOTs for changes triggered in Bar3DPlotAreaDock
	void columnChanged(const QModelIndex&);
	void xRotationChanged(int);
	void yRotationChanged(int);
	void zoomLevelChanged(int);
	void shadowQualityChanged(int);
	void opacityChanged(double);
	void themeChanged(int);

	// SLOTs for changes triggered in Bar3DPlotArea
	void barColumnsChanged(const QVector<const AbstractColumn*>&);
	void barXRotationChanged(int);
	void barYRotationChanged(int);
	void barZoomLevelChanged(int);
	void barShadowQualityChanged(Bar3DPlotArea::ShadowQuality);
	void barOpacityChanged(double);
	void barThemeChanged(Bar3DPlotArea::Theme);

private:
	Ui::Bar3DPlotAreaDock ui;
	QList<Bar3DPlotArea*> m_bars;
	Bar3DPlotArea* m_bar{nullptr};

Q_SIGNALS:
	void info(const QString&);
	void elementVisibilityChanged();
};
#endif // BAR3DPLOTAREA_H
