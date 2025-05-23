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

private Q_SLOTS:
	void loadConfigFromTemplate(KConfig&);
	void saveConfigAsTemplate(KConfig&);
    void setEditorFont(const QFont&);
    void setOutputFont(const QFont&);
    void setScriptEditorFont(const QFont&);
    void setScriptOutputFont(const QFont&);
    void setEditorTheme(const QString&);
    void setScriptEditorTheme(const QString&);

Q_SIGNALS:
    void info(const QString&);
};

#endif // SCRIPTDOCK_H
