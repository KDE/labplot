/*
	File                 : PythonScriptRuntime.h
	Project              : LabPlot
	Description          : Python Script Runtime
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2025 Israel Galadima <izzygaladima@gmail.com>
	SPDX-FileCopyrightText: 2026 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PYTHONSCRIPTRUNTIME_H
#define PYTHONSCRIPTRUNTIME_H

#include "backend/script/ScriptRuntime.h"
#include <Python.h>

class PythonLogger;

class PythonScriptRuntime : public ScriptRuntime {
	Q_OBJECT

public:
	explicit PythonScriptRuntime(Script*);
	virtual ~PythonScriptRuntime();

	virtual bool init() override;
	virtual bool cancel() override;
	virtual bool exec(const QString&) override;
	bool supportsDebugging() const override;
	bool debug(const QString& code, const QList<int>& breakpoints) override;
	void setBreakpoints(const QList<int>& breakpoints) override;
	QList<VariableInfo> childVariables(int handle) override;

	// Static helpers for code completion
	static QStringList getPylabplotSymbols();
	static QString pyUnicodeToQString(PyObject*);

	// called by the python debugger helper when the execution is paused
	static PyObject* debugPauseCallback(PyObject* self, PyObject* args);

private:
	PythonLogger* m_loggerStdOut{nullptr}; // PythonLogger instance to replace sys.stdout in the python interpreter
	PythonLogger* m_loggerStdErr{nullptr}; // PythonLogger instance to replace sys.stderr in the python interpreter
	PyObject* m_localDict{nullptr};
	bool m_outputRedirected{false};
	PyObject* m_pausedDebugger{nullptr}; // borrowed, the bdb instance of the paused debug session
	QList<PyObject*> m_variableHandles; // owned references to the expandable variables shown in the variable viewer

	// instance methods
	bool reset();
	bool redirectStream(const char* streamName, PythonLogger* loggerInstance);
	bool redirectOutput();
	bool restoreStream(PyObject* sysModule, const char* streamName, PyObject* original);
	bool unRedirectOutput();
	bool populateVariableInfo(PyObject* variables = nullptr);
	VariableInfo variableInfo(const QString& name, PyObject* obj);
	void clearVariableHandles();
	DebugCommand debugPause(int line, PyObject* locals, PyObject* debugger);
	void trimTracebackToScript();
	PyObject* createDebuggerRunFunction();
	PyObject* createLocalDict();
	void printPyError();

	// singleton methods (called once for all PythonScripts)
	static bool initPython();

	// utilities
	static PyObject* getModuleDict(const QString&);
	static PyTypeObject* getPythonLoggerType();
	static PyObject* shibokenConvertToPyObject(PythonLogger*);
	static int getPyErrorLine();

	// singletons (shared between PythonScripts)
	static bool ready;
	static PyObject* sysStdOut;
	static PyObject* sysStdErr;
	static bool needsRestart;
};

#endif
