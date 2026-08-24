/***************************************************************************
	File                 : Scatter3DPlotDock.h
	Project              : LabPlot
	Description          : widget for Scatter3DPlot properties
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024-2026 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2024 Kuntal Bar <barkuntal6@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
***************************************************************************/

#ifndef SCATTER3DPLOTDOCK_H
#define SCATTER3DPLOTDOCK_H

#include "BaseDock.h"
#include "backend/worksheet/plots/3d/Scatter3DPlot.h"

#include "ui_scatter3dplotdock.h"

class AbstractColumn;

class Scatter3DPlotDock : public BaseDock {
	Q_OBJECT

public:
	explicit Scatter3DPlotDock(QWidget*);
	void setPlots(const QList<Scatter3DPlot*>&);

private:
	void load();
	void loadConfig(KConfig&);

private Q_SLOTS:
	void retranslateUi();

	// SLOTs for changes triggered in Scatter3DPlotDock
	void xColumnChanged(const QModelIndex&);
	void yColumnChanged(const QModelIndex&);
	void zColumnChanged(const QModelIndex&);
	void pointStyleChanged(int);
	void colorChanged(QColor);

	// SLOTs for changes triggered in Scatter3DPlot
	void plotXColumnChanged(const AbstractColumn*);
	void plotYColumnChanged(const AbstractColumn*);
	void plotZColumnChanged(const AbstractColumn*);
	void plotPointStyleChanged(Scatter3DPlot::PointStyle);
	void plotColorChanged(QColor);

private:
	Ui::Scatter3DPlotDock ui;
	QList<Scatter3DPlot*> m_plots;
	Scatter3DPlot* m_plot{nullptr};

Q_SIGNALS:
	void info(const QString&);
};

#endif // SCATTER3DPLOTDOCK_H
