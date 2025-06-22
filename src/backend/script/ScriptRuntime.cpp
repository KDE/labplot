#include <KLocalizedString>

#include "ScriptRuntime.h"

#include "Script.h"

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
ScriptRuntime::ScriptRuntime(const QString& lang, Script* script)
	: QObject()
	, lang(lang)
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

const QMap<QString, ScriptRuntime::VariableInfo>& VariablesInfoModel::variablesInfo() const {
	return m_variablesInfo;
}

int VariablesInfoModel::columnCount(const QModelIndex& parent) const {
	if (parent.isValid())
		return 0;
	else
		return 4;
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
		case 3:
			return i18nc("@title:column", "Persist");
		}
	}
	return QVariant();
}

Qt::ItemFlags VariablesInfoModel::flags(const QModelIndex& index) const {
	if (index.column() == 3)
		return QAbstractItemModel::flags(index) | Qt::ItemIsUserCheckable;
	else
		return QAbstractItemModel::flags(index);
}

QVariant VariablesInfoModel::data(const QModelIndex& index, int role) const {
	if (((role != Qt::DisplayRole) && (role != Qt::CheckStateRole)) || !index.isValid())
		return QVariant();

	const auto& variable = m_variableNames.at(index.row());

	const ScriptRuntime::VariableInfo& variableInfo = m_variablesInfo.value(variable);

	if (role == Qt::DisplayRole) {
		switch (index.column()) {
		case 0:
			return QVariant(variable);
		case 1:
			return QVariant(variableInfo.value);
		case 2:
			return QVariant(variableInfo.type);
		}
	} else if (role == Qt::CheckStateRole) {
		switch (index.column()) {
		case 3:
			return static_cast<int>(variableInfo.persist ? Qt::Checked : Qt::Unchecked);
		}
	}

	return {};
}

bool VariablesInfoModel::setData(const QModelIndex& index, const QVariant& value, int role) {
	if ((role != Qt::CheckStateRole) || !value.isValid() || !index.isValid())
		return false;

	if (index.column() != 3)
		return false;

	QString name = data(index.siblingAtColumn(0)).toString();

	ScriptRuntime::VariableInfo variableInfo = m_variablesInfo.value(name);
	variableInfo.persist = value.toBool();

	m_variablesInfo.insert(name, variableInfo);

	Q_EMIT dataChanged(index, index);

	return true;
}

void VariablesInfoModel::clearVariablesInfo() {
	beginRemoveRows(QModelIndex(), 0, m_variablesInfo.size() - 1);
	m_variablesInfo.clear();
	m_variableNames.clear();
	endRemoveRows();
}

void VariablesInfoModel::setVariablesInfo(const QMap<QString, ScriptRuntime::VariableInfo>& variablesInfo) {
	beginInsertRows(QModelIndex(), 0, variablesInfo.size() - 1);
	m_variablesInfo = variablesInfo;
	m_variableNames = variablesInfo.keys();
	endInsertRows();
}
