/*
	File                 : CorrelationDock.h
	Project              : LabPlot
	Description          : Dock for Correlation Coefficients/Tests
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2019  Devanshu Agarwal(agarwaldevanshu8@gmail.com)
	SPDX-FileCopyrightText: 2023 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CORRELATIONDOCK_H
#define CORRELATIONDOCK_H

#include "kdefrontend/dockwidgets/BaseDock.h"
#include "ui_correlationdock.h"

class Column;
class Spreadsheet;
class AspectTreeModel;
class Correlation;
class TreeViewComboBox;

class CorrelationDock : public BaseDock {
	Q_OBJECT

public:
	explicit CorrelationDock(QWidget*);
	void setCorrelation(Correlation*);

private:
	Ui::CorrelationDock ui;
	TreeViewComboBox* cbSpreadsheet{nullptr};
	Correlation* m_correlation{nullptr};
	AspectTreeModel* m_aspectTreeModel{nullptr};
	QList<Column*> m_onlyValuesCols;
	QList<Column*> m_twoCategoricalCols;
	QList<Column*> m_multiCategoricalCols;

	void countPartitions(Column*, int &np, int &total_rows);

	void setModelIndexFromAspect(TreeViewComboBox*, const AbstractAspect*);
	void setColumnsComboBoxModel(Spreadsheet*);
	void setColumnsComboBoxView();
	bool nonEmptySelectedColumns();

private Q_SLOTS:
	//SLOTs for changes triggered in CorrelationDock
	void methodChanged();
	void spreadsheetChanged(const QModelIndex&);
	void col1IndexChanged(int);
	void changeCbCol2Label();
	void chbColumnStatsStateChanged();
	void leNRowsChanged();
	void leNColumnsChanged();

	void exportStatsTableToSpreadsheet();
	void recalculate();

	//SLOTs for changes triggered in CorrelationCoefficient
};
#endif // CORRELATIONCOEFFICIENTDOCK_H
