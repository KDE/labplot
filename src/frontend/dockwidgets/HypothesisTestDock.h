/***************************************************************************
	File                 : HypothesisTestDock.h
	Project              : LabPlot
	Description          : Dock for One-Sample T-Test
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2019  Devanshu Agarwal (agarwaldevanshu8@gmail.com)
	SPDX-FileCopyrightText: 2023 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2025 Kuntal Bar <barkuntal6@gmail.com>

 SPDX-License-Identifier: GPL-2.0-or-later
***************************************************************************/

#ifndef HYPOTHESISTESTDOCK_H
#define HYPOTHESISTESTDOCK_H

#include "backend/statistics/HypothesisTest.h"
#include "frontend/dockwidgets/BaseDock.h"
#include "ui_hypothesistestdock.h"

class AbstractAspect;
class AspectTreeModel;
class TreeViewComboBox;
class Spreadsheet;
class Column;
class AbstractColumn;

class HypothesisTestDock : public BaseDock {
	Q_OBJECT

public:
	explicit HypothesisTestDock(QWidget*);
	void initializeTest(HypothesisTest* test);

private:
	Ui::HypothesisTestDock ui;
	TreeViewComboBox* spreadsheetComboBox{nullptr};
	HypothesisTest* m_test{nullptr};
	AspectTreeModel* m_aspectTreeModel{nullptr};
	HypothesisTest::HypothesisTailType m_tail;
	void updateAspectComboBoxIndex(TreeViewComboBox*, const AbstractAspect*);
	void refreshColumnComboBox(Spreadsheet*);
	bool hasSelectedColumns() const;

private Q_SLOTS:
	void runHypothesisTest();
	void onSpreadsheetSelectionChanged(const QModelIndex&);
	void onNullTwoTailClicked();
	void onNullOneTail1Clicked();
	void onNullOneTail2Clicked();

Q_SIGNALS:
};

#endif // HYPOTHESISTESTDOCK_H
