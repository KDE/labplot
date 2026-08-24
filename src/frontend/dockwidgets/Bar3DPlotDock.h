/***************************************************************************
	File                 : Bar3DPlotDock.h
	Project              : LabPlot
	Description          : widget for Bar3DScene properties
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024-2026 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2024 Kuntal Bar <barkuntal6@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
***************************************************************************/

#ifndef BAR3DPLOTDOCK_H
#define BAR3DPLOTDOCK_H

#include "BaseDock.h"
#include "backend/worksheet/plots/3d/Bar3DScene.h"
#include "ui_bar3dplotdock.h"

class AbstractColumn;
class TreeViewComboBox;

class Bar3DPlotDock : public BaseDock {
	Q_OBJECT
public:
	explicit Bar3DPlotDock(QWidget* parent);
	void setBars(const QList<Bar3DScene*>& bars);

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
	// SLOTs for changes triggered in Bar3DPlotDock
	void columnChanged(const QModelIndex&);
	void xRotationChanged(int);
	void yRotationChanged(int);
	void zoomLevelChanged(int);
	void shadowQualityChanged(int);
	void themeChanged(int);
	void colorChanged(QColor);

	// SLOTs for changes triggered in Bar3DPlot
	void barColumnsChanged(const QVector<const AbstractColumn*>&);
	void barXRotationChanged(int);
	void barYRotationChanged(int);
	void barZoomLevelChanged(int);
	void barShadowQualityChanged(Base3DPlot::ShadowQuality);
	void barThemeChanged(Base3DPlot::Theme);
	void barColorChanged(QColor);

private:
	Ui::Bar3DPlotDock ui;
	QList<Bar3DScene*> m_bars;
	Bar3DScene* m_bar{nullptr};

Q_SIGNALS:
	void info(const QString&);
	void elementVisibilityChanged();
};
#endif // BAR3DPLOTDOCK_H
