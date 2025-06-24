#ifndef SCRIPTRUNTIME_H
#define SCRIPTRUNTIME_H

#include <QAbstractTableModel>
#include <QObject>

class Script;
class VariablesInfoModel;

class ScriptRuntime : public QObject {
	Q_OBJECT

public:
	explicit ScriptRuntime(const QString&, Script*);
	virtual ~ScriptRuntime() = default;

	struct VariableInfo {
		QString value;
		QString type;
	};

	const QString lang;

	int errorLine() const;
	void clearErrorLine();

	QAbstractItemModel* variableModel() const;

	virtual bool init() = 0;

	virtual bool cancel() = 0;

	virtual bool exec(const QString& code) = 0;

protected:
	const QString m_name;
	int m_errorLine{-1};
	bool m_isCancellable{false};
	VariablesInfoModel* m_variableModel{nullptr};

Q_SIGNALS:
	/*!
	 * \brief Writes the output from the runtime to the ScriptEditor output.
	 * \param isErr - Indicates if the output is from stderr or from stdout
	 * \param output - The text to display in the ScriptEditor output
	 */
	void writeOutput(bool isErr, const QString& output);
};

class VariablesInfoModel : public QAbstractTableModel {
public:
	explicit VariablesInfoModel(ScriptRuntime*);

	void setVariablesInfo(const QMap<QString, ScriptRuntime::VariableInfo>&);

protected:
	int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant headerData(int, Qt::Orientation, int role = Qt::DisplayRole) const override;
	QVariant data(const QModelIndex&, int role = Qt::DisplayRole) const override;

private:
	QStringList m_variableNames;
	QMap<QString, ScriptRuntime::VariableInfo> m_variablesInfo;
};

#endif