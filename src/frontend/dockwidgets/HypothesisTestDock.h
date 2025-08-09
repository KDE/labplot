/*
	File                 : HypothesisTestDock.h
	Project              : LabPlot
	Description          : Dock for Hypothesis Tests
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2019  Devanshu Agarwal <agarwaldevanshu8@gmail.com>
	SPDX-FileCopyrightText: 2025 Kuntal Bar <barkuntal6@gmail.com>
	SPDX-FileCopyrightText: 2023-205 Alexander Semke <alexander.semke@web.de>

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
	QVector<QMetaObject::Connection> m_aspectConnections;

	void hideControls();
	void addVariable();
	void removeVariable();
	void ensureHypothesis();
	void ensureVariableCount();
	void manageAddRemoveVariable();
	void manageRecalculate();
	void testChanged();
	void updateColumns();
	void recalculate();
};

#endif // HYPOTHESISTESTDOCK_H
