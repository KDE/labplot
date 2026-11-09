/*
	File                 : Script.h
	Project              : LabPlot
	Description          : Script
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2025 Israel Galadima <izzygaladima@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SCRIPT_H
#define SCRIPT_H

#include "backend/core/AbstractPart.h"
#include "backend/core/Folder.h"

class ScriptEditor;
namespace KTextEditor {
class Document;
}
class ScriptRuntime;
class QAbstractItemModel;

class Script : public AbstractPart {
	Q_OBJECT

public:
	explicit Script(const QString&, const QString& = {});
	~Script();

	ScriptEditor* view() const;
	QIcon icon() const override;

	bool exportView() const override;
	bool printView() override;
	bool printPreview() const override;

	QMenu* createContextMenu() override;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

	ScriptRuntime* scriptRuntime() const;
	int scriptErrorLine() const;
	KTextEditor::Document* kTextEditorDocument() const;
	QString language() const;
	bool setRuntime(const QString&);

	bool isInitialized();

	QAbstractItemModel* variableModel();

	// debugging
	bool supportsDebugging() const;
	bool isDebugging() const;
	bool isDebugPaused() const;
	QList<int> breakpoints() const;
	int executionLine() const;
	void toggleBreakpoint(int line);
	static bool isAnyScriptDebugging();

private:
	QString m_language;
	ScriptRuntime* m_scriptRuntime{nullptr};
	KTextEditor::Document* m_kTextEditorDocument{nullptr};
	bool m_initialized{false};
	QString m_outputHtml;

	void prepareDocument() const;
	void initMarks();
	void clearRunMarks();
	void setExecutionLine(int line);
	void prepareRun();
	void finishRun();
	QWidget* createView() const override;

public Q_SLOTS:
	void runScript();
	void debugScript();
	void debugContinue();
	void debugStepOver();
	void debugStepInto();
	void debugStepOut();
	void debugStop();
	void cancelScript();

Q_SIGNALS:
	void requestProjectContextMenu(QMenu*);
	void viewPrint();
	void viewPrintPreview() const;
	void editorFontChanged(const QFont&);
	void outputFontChanged(const QFont&);
	void editorThemeChanged(const QString&);
	void debugStateChanged(); // debugging started/stopped or the execution was paused/resumed

public:
	static QStringList languages;

	static QString readRuntime(XmlStreamReader*);
	static QIcon icon(const QString&);

private:
	static bool m_isRunning;
	static Script* m_lastRunScript;
	static Script* m_debuggingScript;
	int m_executionLine{-1};

	static ScriptRuntime* newScriptRuntime(const QString&, Script*);
	static void runScript(Script*, const QString&);
	static bool cancelScript(Script*);
};

#endif
