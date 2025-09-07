/*
	File                 : HypothesisTestDock.h
	Project              : LabPlot
	Description          : Dock for Hypothesis Tests
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023-2025 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2025 Kuntal Bar <barkuntal6@gmail.com>
	SPDX-FileCopyrightText: 2025 Israel Galadima <izzygaladima@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef HYPOTHESISTESTDOCK_H
#define HYPOTHESISTESTDOCK_H

#include "backend/statistics/HypothesisTest.h"
#include "frontend/dockwidgets/BaseDock.h"
#include "ui_hypothesistestdock.h"

class TreeViewComboBox;
class KMessageWidget;
class QPushButton;

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
	QVector<TreeViewComboBox*> m_dataComboBoxes;
	QVector<QPushButton*> m_removeButtons;
	QPushButton* m_buttonNew{nullptr};
	KMessageWidget* m_messageWidget{nullptr};

	void load();
	void hideControls();
	void addVariable();
	void removeVariable();
	void ensureHypothesis(HypothesisTest::Test);
	void ensureVariableCount(HypothesisTest::Test);
	void manageAddRemoveVariable();
	void manageRecalculate();
	void updateColumns();

private Q_SLOTS:
	// SLOTs for changes triggered in HypothesisTestDock
	// General-Tab
	void testChanged();
	void recalculate();

	// SLOTs for changes triggered in HypothesisTest
	// TODO

	void showStatusInfo(const QString&);
};

#endif // HYPOTHESISTESTDOCK_H
