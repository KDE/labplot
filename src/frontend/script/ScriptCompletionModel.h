/*
	File                 : ScriptCompletionModel.h
	Project              : LabPlot
	Description          : Code completion model for Python script editor
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2026 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SCRIPTCOMPLETIONMODEL_H
#define SCRIPTCOMPLETIONMODEL_H

#include <KTextEditor/CodeCompletionModel>
#include <KTextEditor/CodeCompletionModelControllerInterface>
#include <KTextEditor/Range>

class ScriptEditor;
class QTimer;

class ScriptCompletionModel : public KTextEditor::CodeCompletionModel,
							   public KTextEditor::CodeCompletionModelControllerInterface {
	Q_OBJECT
	Q_INTERFACES(KTextEditor::CodeCompletionModelControllerInterface)

public:
	struct CompletionItem {
		QString name;
		QString signature;  // For methods: "method(arg1, arg2)"
		QString docstring;  // Brief documentation
		bool isFunction = false;
		bool isClass = false;
		bool isEnum = false;
		bool isVariable = false;
		bool isMember = false;  // Is a class member
	};

	explicit ScriptCompletionModel(ScriptEditor* parent);
	~ScriptCompletionModel() override;

	void completionInvoked(KTextEditor::View*, const KTextEditor::Range&, InvocationType) override;
	void executeCompletionItem(KTextEditor::View*, const KTextEditor::Range&, const QModelIndex&) const override;
	QVariant data(const QModelIndex&, int) const override;

	KTextEditor::Range completionRange(KTextEditor::View*, const KTextEditor::Cursor&) override;
	bool shouldStartCompletion(KTextEditor::View*, const QString&, bool, const KTextEditor::Cursor&) override;
	void abortCompletion();

	bool initPylabplotSymbols();
	const QList<CompletionItem>& matches() const { return m_matches; }

Q_SIGNALS:
	void modelIsReady(const QList<CompletionItem>&);

private Q_SLOTS:
	void startCompletionRequest();

private:
	enum class CompletionContext {
		Global,      // Normal completion (variables, functions, classes)
		Member,      // After '.' - show members
	};

	ScriptEditor* m_editor{nullptr};
	QList<CompletionItem> m_matches;
	QList<CompletionItem> m_pylabplotSymbols;
	QStringList m_userVariables;
	QStringList m_pythonBuiltins;
	QMap<QString, QList<CompletionItem>> m_memberCache;  // Class -> members map

	QTimer* m_debounceTimer{nullptr};
	KTextEditor::View* m_pendingView{nullptr};
	KTextEditor::Range m_pendingRange;

	void updateUserVariables(const QString& scriptText);
	void initPythonBuiltins();
	CompletionContext detectContext(KTextEditor::View* view, const KTextEditor::Cursor& cursor, QString& objectName);
	QList<CompletionItem> getMembersForType(const QString& typeName);
	QString inferType(const QString& varName, const QString& scriptText);
};

#endif // SCRIPTCOMPLETIONMODEL_H
