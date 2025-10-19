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

class KMessageWidget;
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
	KMessageWidget* m_messageWidget{nullptr};

	virtual void setModel();

private Q_SLOTS:
	// SLOTs for changes triggered in SeasonalDecompositionDock
	// General-Tab
	void xColumnChanged(const QModelIndex&);
	void yColumnChanged(const QModelIndex&);
	void methodChanged(int);

	// STL parameters
	void stlPeriodChanged(int);
	void stlRobustChanged(bool);
	void stlSeasonalLengthChanged(int);
	void stlTrendLengthChanged(int);
	void stlTrendLengthAutoChanged(bool);
	void stlLowPassLengthChanged(int);
	void stlLowPassLengthAutoChanged(bool);
	void stlSeasonalDegreeChanged(int);
	void stlTrendDegreeChanged(int);
	void stlLowPassDegreeChanged(int);
	void stlSeasonalJumpChanged(int);
	void stlTrendJumpChanged(int);
	void stlLowPassJumpChanged(int);

	// SLOTs for changes triggered in SeasonalDecomposition
	// General-Tab
	void decompositionXColumnChanged(const AbstractColumn*);
	void decompositionYColumnChanged(const AbstractColumn*);
	void decompositionMethodChanged(SeasonalDecomposition::Method);

	// STL parameters
	void decompositionSTLPeriodChanged(int);
	void decompositionSTLRobustChanged(bool);
	void decompositionSTLSeasonalLengthChanged(int);
	void decompositionSTLTrendLengthChanged(int);
	void decompositionSTLTrendLengthAutoChanged(bool);
	void decompositionSTLLowPassLengthChanged(int);
	void decompositionSTLLowPassLengthAutoChanged(bool);
	void decompositionSTLSeasonalDegreeChanged(int);
	void decompositionSTLTrendDegreeChanged(int);
	void decompositionSTLLowPassDegreeChanged(int);
	void decompositionSTLSeasonalJumpChanged(int);
	void decompositionSTLTrendJumpChanged(int);
	void decompositionSTLLowPassJumpChanged(int);

	void showStatusInfo(const QString&);

	// load and save
	void loadConfigFromTemplate(KConfig&);
	void saveConfigAsTemplate(KConfig&);

Q_SIGNALS:
	void info(const QString&);
};

#endif
