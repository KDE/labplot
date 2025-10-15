/*
	File                 : SeasonalDecompositionDock.h
	Project              : LabPlot
	Description          : widget for properties of a time series seasonal decomposition
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2025 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SEASONALDECOMPOSITIONDOCK_H
#define SEASONALDECOMPOSITIONDOCK_H

#include "backend/timeseriesanalysis/SeasonalDecomposition.h"
#include "frontend/dockwidgets/BaseDock.h"
#include "ui_seasonaldecompositiondock.h"

class TreeViewComboBox;

class SeasonalDecompositionDock : public BaseDock {
	Q_OBJECT

public:
	explicit SeasonalDecompositionDock(QWidget*);
	~SeasonalDecompositionDock() override;

	void retranslateUi() override;
	void setDecompositions(QList<SeasonalDecomposition*>);

private:
	TreeViewComboBox* cbXColumn{nullptr};
	TreeViewComboBox* cbYColumn{nullptr};

	void updateValuesWidgets();
	void load();
	void loadConfig(KConfig&);

protected:
	Ui::SeasonalDecompositionDock ui;
	SeasonalDecomposition* m_decomposition{nullptr};
	QList<SeasonalDecomposition*> m_decompositions;

	virtual void setModel();

private Q_SLOTS:
	// SLOTs for changes triggered in SeasonalDecompositionDock
	// General-Tab
	void xColumnChanged(const QModelIndex&);
	void yColumnChanged(const QModelIndex&);
	void methodChanged(int);

	// SLOTs for changes triggered in Histogram
	// General-Tab
	void decompositionXColumnChanged(const AbstractColumn*);
	void decompositionYColumnChanged(const AbstractColumn*);
	void decompositionMethodChanged(SeasonalDecomposition::Method);

	// load and save
	void loadConfigFromTemplate(KConfig&);
	void saveConfigAsTemplate(KConfig&);

Q_SIGNALS:
	void info(const QString&);
};

#endif
