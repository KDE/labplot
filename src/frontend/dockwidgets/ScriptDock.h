/*
	File                 : ScriptDock.h
	Project              : LabPlot
	Description          : Dock for configuring script
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2025 Israel Galadima <izzygaladima@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SCRIPTDOCK_H
#define SCRIPTDOCK_H

#include "frontend/dockwidgets/BaseDock.h"
#include "ui_scriptdock.h"

class Script;
class KConfig;
class QAction;

class ScriptDock : public BaseDock {
	Q_OBJECT

public:
	explicit ScriptDock(QWidget* parent);
	void setScriptsList(QList<Script*>);
	void retranslateUi() override;

private:
	Ui::ScriptDock ui;
	Script* m_script{nullptr};
	QList<Script*> m_scriptsList;

	QAction* m_caseSensitiveAction{nullptr};
	QAction* m_matchCompleteWordAction{nullptr};

	QAction* m_copyNameAction{nullptr};
	QAction* m_copyValueAction{nullptr};
	QAction* m_copyNameValueAction{nullptr};

	void toggleFilterOptionsMenu(bool);
	void filterTextChanged(const QString&);
	void copy(const QAction*) const;
	void contextMenuEvent(QContextMenuEvent*) override;

Q_SIGNALS:
    void info(const QString&);
};

#endif // SCRIPTDOCK_H
