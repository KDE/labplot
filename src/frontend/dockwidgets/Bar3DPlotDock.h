/***************************************************************************
	File                 : Bar3DPlotDock.h
	Project              : LabPlot
	Description          : widget for Bar3DPlot properties
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024-2026 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2024 Kuntal Bar <barkuntal6@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
***************************************************************************/

#ifndef BAR3DPLOTDOCK_H
#define BAR3DPLOTDOCK_H

#include "BaseDock.h"
#include "backend/worksheet/plots/3d/Bar3DPlot.h"
#include "ui_bar3dplotdock.h"

class AbstractColumn;
class TreeViewComboBox;

class Bar3DPlotDock : public BaseDock {
	Q_OBJECT
public:
	explicit Bar3DPlotDock(QWidget* parent);
	void setPlots(const QList<Bar3DPlot*>& plots);

private:
	// for data columns
	QGridLayout* m_gridLayout;
	QPushButton* m_buttonNew;
	QVector<TreeViewComboBox*> m_dataComboBoxes;
	QVector<QPushButton*> m_removeButtons;

	void setDataColumns();
	void loadDataColumns();
	void load();
	void loadConfig(KConfig&);

private Q_SLOTS:
	void retranslateUi();
	void addDataColumn();
	void removeDataColumn();

	// SLOTs for changes triggered in Bar3DPlotDock
	void columnChanged(const QModelIndex&);
	void colorChanged(QColor);

	// SLOTs for changes triggered in Bar3DPlot
	void plotColumnsChanged(const QVector<const AbstractColumn*>&);
	void plotColorChanged(QColor);

private:
	Ui::Bar3DPlotDock ui;
	QList<Bar3DPlot*> m_plots;
	Bar3DPlot* m_plot{nullptr};

Q_SIGNALS:
	void info(const QString&);
};
#endif // BAR3DPLOTDOCK_H
