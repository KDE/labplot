#ifndef SCRIPTRUNTIME_H
#define SCRIPTRUNTIME_H

#include <QObject>

class Script;

class ScriptRuntime : public QObject {
	Q_OBJECT

public:
	explicit ScriptRuntime(const QString&, Script*);
	virtual ~ScriptRuntime() = default;

	const QString lang;

	int errorLine() const;

	virtual bool init() = 0;

	virtual bool cancel() = 0;

	virtual bool exec(const QString& code) = 0;

protected:
	const QString m_name;
	int m_errorLine{-1};
	bool isCancellable{false};

Q_SIGNALS:
	/*!
	 * \brief Writes the output from the runtime to the ScriptEditor output.
	 * \param isErr - Indicates if the output is from stderr or from stdout
	 * \param output - The text to display in the ScriptEditor output
	 */
	void writeOutput(bool isErr, const QString& output);
};

#endif