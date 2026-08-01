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
		bool isFunction = false;
		bool isClass = false;
		bool isEnum = false;
		bool isVariable = false;
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
	ScriptEditor* m_editor{nullptr};
	QList<CompletionItem> m_matches;
	QList<CompletionItem> m_pylabplotSymbols;
	QStringList m_userVariables;
	QStringList m_pythonBuiltins;

	QTimer* m_debounceTimer{nullptr};
	KTextEditor::View* m_pendingView{nullptr};
	KTextEditor::Range m_pendingRange;

	void updateUserVariables(const QString& scriptText);
	void initPythonBuiltins();
};

#endif // SCRIPTCOMPLETIONMODEL_H
