/***************************************************************************
	File                 : Scatter3DPlotDock.h
	Project              : LabPlot
	Description          : widget for Scatter3DScene properties
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024-2026 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2024 Kuntal Bar <barkuntal6@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
***************************************************************************/

#ifndef SCATTER3DPLOTDOCK_H
#define SCATTER3DPLOTDOCK_H

#include "BaseDock.h"
#include "backend/worksheet/plots/3d/Scatter3DScene.h"

#include "ui_scatter3dplotdock.h"

class AbstractColumn;

class Scatter3DPlotDock : public BaseDock {
	Q_OBJECT

public:
	explicit Scatter3DPlotDock(QWidget*);
	void setScatters(const QList<Scatter3DScene*>&);

private:
	void updateUiVisibility();
	void load();
	void loadConfig(KConfig&);

private Q_SLOTS:
	void retranslateUi();

	// SLOTs for changes triggered in Scatter3DPlotDock
	void xColumnChanged(const QModelIndex&);
	void yColumnChanged(const QModelIndex&);
	void zColumnChanged(const QModelIndex&);
	void xRotationChanged(int);
	void yRotationChanged(int);
	void zoomLevelChanged(int);
	void shadowQualityChanged(int);
	void pointStyleChanged(int);
	void themeChanged(int);
	void colorChanged(QColor);

	// SLOTs for changes triggered in Scatter3DPlot
	void scatterXColumnChanged(const AbstractColumn*);
	void scatterYColumnChanged(const AbstractColumn*);
	void scatterZColumnChanged(const AbstractColumn*);
	void scatterXRotationChanged(int);
	void scatterYRotationChanged(int);
	void scatterZoomLevelChanged(int);
	void scatterShadowQualityChanged(Base3DPlot::ShadowQuality);
	void scatterPointStyleChanged(Scatter3DScene::PointStyle);
	void scatterThemeChanged(Base3DPlot::Theme);
	void scatterColorChanged(QColor);

private:
	Ui::Scatter3DPlotDock ui;
	QList<Scatter3DScene*> m_scatters;
	Scatter3DScene* m_scatter{nullptr};

Q_SIGNALS:
	void info(const QString&);
	void elementVisibilityChanged();
};

#endif // SCATTER3DPLOTDOCK_H
