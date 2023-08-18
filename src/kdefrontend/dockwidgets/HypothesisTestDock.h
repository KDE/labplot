/*
	File                 : HypothesisTestDock.h
	Project              : LabPlot
	Description          : Dock for Hypothesis Test
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2019  Devanshu Agarwal(agarwaldevanshu8@gmail.com)
	SPDX-FileCopyrightText: 2023 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef HYPOTHESISTESTDOCK_H
#define HYPOTHESISTESTDOCK_H

#include "kdefrontend/dockwidgets/BaseDock.h"
#include "backend/generalTest/HypothesisTest.h"
#include "ui_hypothesistestdock.h"

class AbstractAspect;
class AspectTreeModel;
class TreeViewComboBox;

class HypothesisTestDock : public BaseDock {
	Q_OBJECT

public:
	explicit HypothesisTestDock(QWidget*);
	void setHypothesisTest(HypothesisTest*);

private:
	Ui::HypothesisTestDock ui;
	TreeViewComboBox* cbSpreadsheet{nullptr};
	HypothesisTest* m_test{nullptr};
	AspectTreeModel* m_aspectTreeModel{nullptr};
	void setModelIndexFromAspect(TreeViewComboBox*, const AbstractAspect*);
	int m_type;
	HypothesisTest::HypothesisTailType m_tail;

	void countPartitions(Column*, int &np, int &total_rows);
	void setColumnsComboBoxModel(Spreadsheet*);
	void setColumnsComboBoxView();
	bool nonEmptySelectedColumns();

	int testType(int test);
	int testSubtype(int test);

	QList<Column*> m_onlyValuesCols;
	QList<Column*> m_twoCategoricalCols;
	QList<Column*> m_multiCategoricalCols;

private Q_SLOTS:
	//SLOTs for changes triggered in HypothesisTestDock
	void showTestType();
	void showHypothesisTest();
	void doHypothesisTest();
	void performLeveneTest();
	void spreadsheetChanged(const QModelIndex&);
	void changeCbCol2Label();
	void chbPopulationSigmaStateChanged();
	void col1IndexChanged(int index);
	void chbCalculateStatsStateChanged();

	void onRbH1OneTail1Toggled(bool checked);
	void onRbH1OneTail2Toggled(bool checked);
	void onRbH1TwoTailToggled(bool checked);

Q_SIGNALS:
	//        void info(const QString&);
};

#endif // HYPOTHESISTESTDOCK_H
