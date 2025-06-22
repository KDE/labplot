#ifndef PYTHONSCRIPTRUNTIME_H
#define PYTHONSCRIPTRUNTIME_H

#include <sbkpython.h>

#include "backend/script/ScriptRuntime.h"

class PythonLogger;

class PythonScriptRuntime : public ScriptRuntime {
	Q_OBJECT

public:
	explicit PythonScriptRuntime(Script*);
	virtual ~PythonScriptRuntime();

	virtual bool init() override;
	virtual bool cancel() override;
	virtual bool exec(const QString&) override;

private:
	PythonLogger* m_loggerStdOut{nullptr};
	PythonLogger* m_loggerStdErr{nullptr};
	PyObject* m_localDict{nullptr};

	// instance methods
	bool reset();
	bool redirectOutput();
	bool unRedirectOutput();
	bool populateVariableInfo();
	PyObject* createLocalDict();

	// singleton methods (called once for all PythonScripts)
	static bool initPython();

	// utilities
	static PyObject* getModuleDict(const QString&);
	static PyObject* shibokenConvertToPyObject(int, void*);
	static int getPyErrorLine();
	static QString pyUnicodeToQString(PyObject*);

	// singletons (shared between PythonScripts)
	static bool ready;
	static PyObject* sysStdOutWrite;
	static PyObject* sysStdErrWrite;
};

#endif