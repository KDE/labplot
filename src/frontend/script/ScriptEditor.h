/*
	File                 : ScriptEditor.h
	Project              : LabPlot
	Description          : Script editor
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2025 Israel Galadima <izzygaladima@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SCRIPTEDITOR_H
#define SCRIPTEDITOR_H

#include <QWidget>
#include "ui_scripteditorwidget.h"

class QAction;
class QMenu;
class Script;
class QToolBar;
class QToolButton;
namespace KTextEditor{
class View;
}

class ScriptEditor : public QWidget {
	Q_OBJECT

public:
	explicit ScriptEditor(Script*, QWidget* parent = nullptr);
	~ScriptEditor();

	bool isInitialized() const;

	void writeOutput(bool, const QString&);
	QString outputText();

	void registerShortcuts();
	void unregisterShortcuts();

public Q_SLOTS:
	void createContextMenu(QMenu*);
	void run();
	void clearOutput();

private:
	Ui::ScriptEditorWidget ui;
	Script* m_script{nullptr};
	KTextEditor::View* m_kTextEditorView{nullptr};
	QAction* m_runScriptAction{nullptr};
	QAction* m_clearOutputAction{nullptr};

	void initActions();
	void initMenus();
	void setSplitterState(const QByteArray&);
	QByteArray splitterState();
	void setOutputFont(const QFont&);
	QFont outputFont();
};
#endif
