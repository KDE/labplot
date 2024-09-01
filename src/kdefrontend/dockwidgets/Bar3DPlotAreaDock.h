#ifndef BAR3DPLOTAREADOCK_H
#define BAR3DPLOTAREADOCK_H

#include "BaseDock.h"
#include "backend/worksheet/plots/3d/Bar3DPlot.h"
#include "ui_bar3dplotareadock.h"

class AbstractColumn;
class TreeViewComboBox;
class Bar3DPlot;
class Bar3DPlotAreaDock : public BaseDock {
	Q_OBJECT
public:
	explicit Bar3DPlotAreaDock(QWidget* parent);
	void setBars(const QList<Bar3DPlot*>& bars);

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
	void loadDataColumns();
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
	void themeChanged(int);
	void colorChanged(QColor);

	// SLOTs for changes triggered in Bar3DPlotArea
	void barColumnsChanged(const QVector<const AbstractColumn*>&);
	void barXRotationChanged(int);
	void barYRotationChanged(int);
	void barZoomLevelChanged(int);
	void barShadowQualityChanged(Base3DArea::ShadowQuality);
	void barThemeChanged(Base3DArea::Theme);
	void barColorChanged(QColor);

private:
	Ui::Bar3DPlotAreaDock ui;
	QList<Bar3DPlot*> m_bars;
	Bar3DPlot* m_bar{nullptr};

Q_SIGNALS:
	void info(const QString&);
	void elementVisibilityChanged();
};
#endif // BAR3DPLOTAREA_H
