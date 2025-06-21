/*
	File                 : HypothesisTestDock.h
	Project              : LabPlot
	Description          : Dock for Hypothesis Tests
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2019  Devanshu Agarwal <agarwaldevanshu8@gmail.com>
	SPDX-FileCopyrightText: 2023-205 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2025 Kuntal Bar <barkuntal6@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef HYPOTHESISTESTDOCK_H
#define HYPOTHESISTESTDOCK_H

#include "backend/statistics/HypothesisTest.h"
#include "frontend/dockwidgets/BaseDock.h"
#include "ui_hypothesistestdock.h"

class AspectTreeModel;
class TreeViewComboBox;

class HypothesisTestDock : public BaseDock {
	Q_OBJECT

public:
	explicit HypothesisTestDock(QWidget*);
	void setTest(HypothesisTest*);
	void retranslateUi() override;

private:
	Ui::HypothesisTestDock ui;
	HypothesisTest* m_test{nullptr};
	AspectTreeModel* m_aspectTreeModel{nullptr};
	TreeViewComboBox* cbColumn1{nullptr};
	TreeViewComboBox* cbColumn2{nullptr};
	TreeViewComboBox* cbColumn3{nullptr};

private Q_SLOTS:
	void testChanged(int);
	void runHypothesisTest();
	void onNullTwoTailClicked();
	void onNullOneTail1Clicked();
	void onNullOneTail2Clicked();
	void enableRecalculate();

Q_SIGNALS:

};

#endif // HYPOTHESISTESTDOCK_H
