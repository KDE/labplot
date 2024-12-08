/*
	File                 : HeatmapDock.h
	Project              : LabPlot
	Description          : Dock widget for the heatmap plot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef HEATMAPDOCK_H
#define HEATMAPDOCK_H

#include "backend/worksheet/plots/cartesian/Heatmap.h"
#include "frontend/dockwidgets/BaseDock.h"
#include "ui_heatmapdock.h"

class AbstractAspect;
class AspectTreeModel;
class LineWidget;
class SymbolWidget;
class ValueWidget;
class HeatmapDock;
class TreeViewComboBox;
class KConfig;

class HeatmapDock : public BaseDock {
	Q_OBJECT

public:
	explicit HeatmapDock(QWidget*);
	void setPlots(QList<Heatmap*>);
	//	void updateLocale() override;

private:
	Ui::HeatmapDock ui;
	Heatmap* m_plot{nullptr};
	QList<Heatmap*> m_plots;
	AspectTreeModel* m_aspectTreeModelColumn{nullptr};
	AspectTreeModel* m_aspectTreeModelMatrix{nullptr};
	TreeViewComboBox* cbXColumn{nullptr};
	TreeViewComboBox* cbYColumn{nullptr};
	TreeViewComboBox* cbMatrix{nullptr};

	void setModel();
	//	void load();
	//	void loadConfig(KConfig&);
	//	void setDataColumns() const;
	//	void loadDataColumns();

private Q_SLOTS:
	// SLOTs for changes triggered in the dock
	//"General"-tab
	void matrixChanged(const QModelIndex&);
	void xColumnChanged(const QModelIndex&);
	void yColumnChanged(const QModelIndex&);
	void automaticLimitsChanged(bool);
	void limitsMinChanged(double);
	void limitsMaxChanged(double);
	void visibilityChanged(bool);

private Q_SLOTS:
	// SLOTs for changes triggered in the Heatmap
	// general
	void plotXColumnChanged(const AbstractColumn*);
	void plotYColumnChanged(const AbstractColumn*);
	void plotMatrixChanged(const Matrix*);
	void plotVisibilityChanged(bool);

	void plotAutomaticLimitsChanged(bool);
	void plotLimitsMinChanged(double);
	void plotLimitsMaxChanged(double);

	// load and save
	void loadConfig(KConfig& config);
	void loadConfigFromTemplate(KConfig&);
	void saveConfigAsTemplate(KConfig&);

Q_SIGNALS:
	void info(const QString&);
};

#endif // HEATMAPDOCK_H
