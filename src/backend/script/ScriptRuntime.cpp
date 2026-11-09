/*
	File                 : ScriptRuntime.cpp
	Project              : LabPlot
	Description          : Script Runtime
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2025 Israel Galadima <izzygaladima@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ScriptRuntime.h"
#include "Script.h"

#include <KLocalizedString>

#include <QEventLoop>
#include <QPointer>
/*!
 * \class ScriptRuntime
 * \brief Abstract class providing an interface for scripting runtimes
 *
 * Each runtime hides its implementation details and we only call methods
 * implemented from the ScriptRuntime class
 *
 * \ingroup backend
 */

/*!
 * \fn ScriptRuntime::ScriptRuntime
 * \brief Constructs a new ScriptRuntime
 * \param lang - The programming language which the script runtime executes
 * \param script - The Script instance that owns this script runtime
 * \return returns a ScriptRuntime instance
 */
ScriptRuntime::ScriptRuntime(const QString& language, Script* script)
	: QObject()
	, lang(language)
	, m_name(script->name())
	, m_variableModel(new VariablesInfoModel(this)) {
}

/*!
 * \brief Gets the line in the most recently executed script where an error occurred
 * \return returns the line where the error occurred or -1 if no error occurred or the line is unknown
 */
int ScriptRuntime::errorLine() const {
	return m_errorLine;
}

void ScriptRuntime::clearErrorLine() {
	m_errorLine = -1;
}

QAbstractItemModel* ScriptRuntime::variableModel() const {
	return m_variableModel;
}

ScriptRuntime::~ScriptRuntime() {
	// don't leave a paused debug session waiting for a command that will never come
	if (m_debugLoop) {
		*m_pendingDebugCommand = DebugCommand::Stop;
		m_debugLoop->quit();
	}
}

/*!
 * \brief Returns whether the runtime supports debugging via \c debug()
 */
bool ScriptRuntime::supportsDebugging() const {
	return false;
}

/*!
 * \brief Executes the code under the control of the debugger, pausing at the given breakpoints
 * \param code - The statements to execute
 * \param breakpoints - 1-based lines where the execution should pause
 * \return Whether the execution was successful or not
 */
bool ScriptRuntime::debug(const QString& /* code */, const QList<int>& /* breakpoints */) {
	return false;
}

/*!
 * \brief Updates the breakpoints of the paused debug session
 * \param breakpoints - 1-based lines where the execution should pause
 */
void ScriptRuntime::setBreakpoints(const QList<int>& /* breakpoints */) {
}

/*!
 * \brief Returns true if the execution is currently paused by the debugger and is waiting for a command
 */
bool ScriptRuntime::isDebugPaused() const {
	return m_debugLoop != nullptr;
}

/*!
 * \brief Resumes a paused debug session with the given command
 */
void ScriptRuntime::sendDebugCommand(DebugCommand command) {
	if (!m_debugLoop)
		return;

	*m_pendingDebugCommand = command;
	m_debugLoop->quit();
}

/*!
 * \brief Pauses the execution and waits until the user sends a debug command via \c sendDebugCommand().
 *
 * The scripts are executed in the GUI thread, a nested event loop is used to keep the GUI responsive while paused.
 * \param line - 0-based line in the script where the execution is paused
 */
ScriptRuntime::DebugCommand ScriptRuntime::waitForDebugCommand(int line) {
	if (m_debugLoop) // already paused, shouldn't happen
		return DebugCommand::Continue;

	QEventLoop loop;
	DebugCommand command = DebugCommand::Stop;
	m_debugLoop = &loop;
	m_pendingDebugCommand = &command;

	QPointer<ScriptRuntime> guard(this);
	Q_EMIT debugPaused(line);
	loop.exec();

	if (guard) {
		m_debugLoop = nullptr;
		m_pendingDebugCommand = nullptr;
		Q_EMIT debugResumed();
	}

	return command;
}

/*!
 * \fn virtual bool ScriptRuntime::init() = 0;
 * \brief Initializes the script runtime
 *
 * This is the first method called for every instance of
 * the script runtime
 *
 * \return Whether the runtime initialization was successful or not
 */

/*!
 * \fn virtual bool ScriptRuntime::cancel() = 0;
 * \brief Stops the current execution
 * \return Whether stopping the execution was successful or not
 */

/*!
 * \fn virtual bool ScriptRuntime::exec(const QString& code) = 0;
 * \brief Executes the statements using the runtime
 * \param code - The statements to execute
 * \return Whether the execution was successful or not
 */

/*!
 * \brief Returns the children (elements, attributes) of the variable with the given handle
 * \param handle - the handle of the variable as provided in VariableInfo::handle
 */
QList<ScriptRuntime::VariableInfo> ScriptRuntime::childVariables(int /* handle */) {
	return {};
}

VariablesInfoModel::VariablesInfoModel(ScriptRuntime* parent)
	: QAbstractItemModel(parent)
	, m_runtime(parent) {
	m_root.childrenFetched = true;
}

VariablesInfoModel::~VariablesInfoModel() = default;

VariablesInfoModel::Node* VariablesInfoModel::node(const QModelIndex& index) const {
	if (!index.isValid())
		return const_cast<Node*>(&m_root);
	return static_cast<Node*>(index.internalPointer());
}

void VariablesInfoModel::setChildren(Node* parent, const QList<ScriptRuntime::VariableInfo>& variables) {
	for (const auto& info : variables) {
		auto* child = new Node;
		child->info = info;
		child->parent = parent;
		child->childrenFetched = !info.hasChildren;
		parent->children << child;
	}
}

void VariablesInfoModel::setVariablesInfo(const QList<ScriptRuntime::VariableInfo>& variables) {
	beginResetModel();
	qDeleteAll(m_root.children);
	m_root.children.clear();
	setChildren(&m_root, variables);
	endResetModel();
}

QModelIndex VariablesInfoModel::index(int row, int column, const QModelIndex& parent) const {
	const auto* parentNode = node(parent);
	if (row < 0 || row >= parentNode->children.size() || column < 0 || column >= columnCount())
		return {};
	return createIndex(row, column, parentNode->children.at(row));
}

QModelIndex VariablesInfoModel::parent(const QModelIndex& index) const {
	if (!index.isValid())
		return {};

	auto* parentNode = node(index)->parent;
	if (!parentNode || parentNode == &m_root)
		return {};

	const int row = parentNode->parent->children.indexOf(parentNode);
	return createIndex(row, 0, parentNode);
}

int VariablesInfoModel::columnCount(const QModelIndex& /* parent */) const {
	return 3;
}

int VariablesInfoModel::rowCount(const QModelIndex& parent) const {
	if (parent.column() > 0)
		return 0;
	return node(parent)->children.size();
}

bool VariablesInfoModel::hasChildren(const QModelIndex& parent) const {
	if (parent.column() > 0)
		return false;
	const auto* n = node(parent);
	return n->childrenFetched ? !n->children.isEmpty() : n->info.hasChildren;
}

bool VariablesInfoModel::canFetchMore(const QModelIndex& parent) const {
	if (!parent.isValid() || parent.column() > 0)
		return false;
	return !node(parent)->childrenFetched;
}

void VariablesInfoModel::fetchMore(const QModelIndex& parent) {
	auto* n = node(parent);
	if (n->childrenFetched)
		return;
	n->childrenFetched = true;

	const auto& children = m_runtime ? m_runtime->childVariables(n->info.handle) : QList<ScriptRuntime::VariableInfo>();
	if (children.isEmpty()) {
		// update the expand indicator
		Q_EMIT dataChanged(parent, parent);
		return;
	}

	beginInsertRows(parent, 0, children.size() - 1);
	setChildren(n, children);
	endInsertRows();
}

QVariant VariablesInfoModel::headerData(int section, Qt::Orientation orientation, int role) const {
	if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
		switch (section) {
		case 0:
			return i18nc("@title:column", "Name");
		case 1:
			return i18nc("@title:column", "Value");
		case 2:
			return i18nc("@title:column", "Type");
		}
	}
	return {};
}

QVariant VariablesInfoModel::data(const QModelIndex& index, int role) const {
	if (!index.isValid() || (role != Qt::DisplayRole && role != Qt::ToolTipRole))
		return {};

	const auto& info = node(index)->info;
	switch (index.column()) {
	case 0:
		return info.name;
	case 1:
		return info.value;
	case 2:
		return info.type;
	}

	return {};
}
