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
		bool persist{false};
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
	explicit VariablesInfoModel(ScriptRuntime* parent);
	const QMap<QString, ScriptRuntime::VariableInfo>& variablesInfo() const;
	void clearVariablesInfo();
	void setVariablesInfo(const QMap<QString, ScriptRuntime::VariableInfo>& variablesInfo);

protected:
	int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	Qt::ItemFlags flags(const QModelIndex& index) const override;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

private:
	QStringList m_variableNames;
	QMap<QString, ScriptRuntime::VariableInfo> m_variablesInfo;
};

#endif