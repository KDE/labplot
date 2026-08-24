/***************************************************************************
	File                 : Surface3DPlotDock.h
	Project              : LabPlot
	Description          : widget for Surface3DPlot properties
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024-2026 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2024 Kuntal Bar <barkuntal6@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
***************************************************************************/

#ifndef SURFACE3DPLOTDOCK_H
#define SURFACE3DPLOTDOCK_H

#include "BaseDock.h"
#include "backend/worksheet/plots/3d/Surface3DPlot.h"

#include "ui_surface3dplotdock.h"

class Surface3DPlot;
class Matrix;
class AbstractColumn;
class AspectTreeModel;
class TreeViewComboBox;

class Surface3DPlotDock : public BaseDock {
	Q_OBJECT

public:
	explicit Surface3DPlotDock(QWidget*);
	void setPlots(const QList<Surface3DPlot*>&);

private:
	void updateUiVisibility();

private Q_SLOTS:
	void retranslateUi();

	// SLOTs for changes triggered in Surface3DPlotDock
	void dataSourceTypeChanged(int);
	void xColumnChanged(const QModelIndex&);
	void yColumnChanged(const QModelIndex&);
	void zColumnChanged(const QModelIndex&);
	void matrixChanged(const QModelIndex&);
	void drawModeChanged(int);
	void flatShadingChanged(bool);
	void smoothChanged(bool);
	void colorChanged(QColor);

	// SLOTs for changes triggered in Surface3DPlot
	void plotDrawModeChanged(Surface3DPlot::DrawMode);
	void plotFlatShadingChanged(bool);
	void plotSourceTypeChanged(Surface3DPlot::DataSource);
	void plotXColumnChanged(const AbstractColumn*);
	void plotYColumnChanged(const AbstractColumn*);
	void plotZColumnChanged(const AbstractColumn*);
	void plotMatrixChanged(const Matrix*);
	void plotSmoothChanged(bool);
	void plotColorChanged(QColor);

private:
	Ui::Surface3DPlotDock ui;
	QList<Surface3DPlot*> m_plots;
	Surface3DPlot* m_plot{nullptr};

	void load();
	void loadConfig(KConfig&);

Q_SIGNALS:
	void info(const QString&);
};

#endif // SURFACE3DPLOTDOCK_H
