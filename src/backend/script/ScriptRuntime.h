/*
	File                 : Script.h
	Project              : LabPlot
	Description          : Script Runtime
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2025 Israel Galadima <izzygaladima@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SCRIPTRUNTIME_H
#define SCRIPTRUNTIME_H

#include <QAbstractItemModel>
#include <QObject>

class Script;
class VariablesInfoModel;
class QEventLoop;

class ScriptRuntime : public QObject {
	Q_OBJECT

public:
	explicit ScriptRuntime(const QString&, Script*);
	virtual ~ScriptRuntime();

	struct VariableInfo {
		QString name;
		QString value;
		QString type;
		bool hasChildren{false}; // the variable can be expanded (container, object with attributes)
		int handle{-1}; // runtime specific handle used to fetch the children, see childVariables()
	};

	enum class DebugCommand { Continue, StepOver, StepInto, StepOut, Stop };

	const QString lang;

	int errorLine() const;
	void clearErrorLine();

	QAbstractItemModel* variableModel() const;
	virtual QList<VariableInfo> childVariables(int handle);

	virtual bool init() = 0;

	virtual bool cancel() = 0;

	virtual bool exec(const QString& code) = 0;

	virtual bool supportsDebugging() const;
	virtual bool debug(const QString& code, const QList<int>& breakpoints);
	virtual void setBreakpoints(const QList<int>& breakpoints);
	bool isDebugPaused() const;
	void sendDebugCommand(DebugCommand);

protected:
	const QString m_name;
	int m_errorLine{-1};
	bool m_isCancellable{false};
	VariablesInfoModel* m_variableModel{nullptr};

	DebugCommand waitForDebugCommand(int line);

private:
	QEventLoop* m_debugLoop{nullptr};
	DebugCommand* m_pendingDebugCommand{nullptr};

Q_SIGNALS:
	/*!
	 * \brief Writes the output from the runtime to the ScriptEditor output.
	 * \param isErr - Indicates if the output is from stderr or from stdout
	 * \param output - The text to display in the ScriptEditor output
	 */
	void writeOutput(bool isErr, const QString& output);

	/*!
	 * \brief Emitted when the execution was paused by the debugger (breakpoint or step finished)
	 * \param line - 0-based line in the script where the execution is paused
	 */
	void debugPaused(int line);

	/*!
	 * \brief Emitted when the execution continues after a pause
	 */
	void debugResumed();
};

/*!
 * tree model for the variables of the script, the children of expandable variables
 * (containers, objects) are fetched lazily from the runtime when the item is expanded.
 */
class VariablesInfoModel : public QAbstractItemModel {
public:
	explicit VariablesInfoModel(ScriptRuntime*);
	~VariablesInfoModel() override;

	void setVariablesInfo(const QList<ScriptRuntime::VariableInfo>&);

	QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
	QModelIndex parent(const QModelIndex&) const override;
	int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	bool hasChildren(const QModelIndex& parent = QModelIndex()) const override;
	bool canFetchMore(const QModelIndex& parent) const override;
	void fetchMore(const QModelIndex& parent) override;
	QVariant headerData(int, Qt::Orientation, int role = Qt::DisplayRole) const override;
	QVariant data(const QModelIndex&, int role = Qt::DisplayRole) const override;

private:
	struct Node {
		ScriptRuntime::VariableInfo info;
		Node* parent{nullptr};
		QList<Node*> children;
		bool childrenFetched{false};
		~Node() {
			qDeleteAll(children);
		}
	};

	ScriptRuntime* m_runtime{nullptr};
	Node m_root;

	Node* node(const QModelIndex&) const;
	void setChildren(Node*, const QList<ScriptRuntime::VariableInfo>&);
};

#endif
