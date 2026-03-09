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

VariablesInfoModel::VariablesInfoModel(ScriptRuntime* parent)
	: QAbstractTableModel(parent) {
}

int VariablesInfoModel::columnCount(const QModelIndex& parent) const {
	if (parent.isValid())
		return 0;
	else
		return 3;
}

int VariablesInfoModel::rowCount(const QModelIndex& parent) const {
	if (parent.isValid())
		return 0;
	else
		return m_variablesInfo.size();
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
	return QVariant();
}

QVariant VariablesInfoModel::data(const QModelIndex& index, int role) const {
	if ((role != Qt::DisplayRole) || !index.isValid())
		return QVariant();

	const auto& variable = m_variableNames.at(index.row());
	const auto& variableInfo = m_variablesInfo.value(variable);

	switch (index.column()) {
	case 0:
		return QVariant(variable);
	case 1:
		return QVariant(variableInfo.value);
	case 2:
		return QVariant(variableInfo.type);
	}

	return {};
}

void VariablesInfoModel::setVariablesInfo(const QMap<QString, ScriptRuntime::VariableInfo>& variablesInfo) {
	beginRemoveRows(QModelIndex(), 0, m_variablesInfo.size() - 1);
	m_variablesInfo.clear();
	m_variableNames.clear();
	endRemoveRows();

	beginInsertRows(QModelIndex(), 0, variablesInfo.size() - 1);
	m_variablesInfo = variablesInfo;
	m_variableNames = variablesInfo.keys();
	endInsertRows();
}
