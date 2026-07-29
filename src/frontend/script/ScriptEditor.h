/*
	File                 : ScriptEditor.h
	Project              : LabPlot
	Description          : Script editor
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2025 Israel Galadima <izzygaladima@gmail.com>
	SPDX-FileCopyrightText: 2026 Alexander Semke <alexander.semke@web.de>
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
class ScriptCompletionModel;
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

private Q_SLOTS:
	void handleAnchorClicked(const QUrl&);
	void showOutputContextMenu(const QPoint&);

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
	QAction* m_copySelectedAction{nullptr};
	QAction* m_copyAllOutputAction{nullptr};
	ScriptCompletionModel* m_completionModel{nullptr};

	void initActions();
	void initMenus();
	void setSplitterState(const QByteArray&);
	QByteArray splitterState();
	void setOutputFont(const QFont&);
	QFont outputFont();

	QString processOutputText(bool isErr, const QString& text);
	void applyOutputFormatting(const QString& html, bool isErr);
};
#endif
