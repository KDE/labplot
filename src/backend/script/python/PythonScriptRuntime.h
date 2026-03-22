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

private:
	PythonLogger* m_loggerStdOut{nullptr}; // PythonLogger instance to replace sys.stdout in the python interpreter
	PythonLogger* m_loggerStdErr{nullptr}; // PythonLogger instance to replace sys.stderr in the python interpreter
	PyObject* m_localDict{nullptr};
	bool m_outputRedirected{false};

	// instance methods
	bool reset();
	bool redirectStream(const char* streamName, PythonLogger* loggerInstance);
	bool redirectOutput();
	bool restoreStream(PyObject* sysModule, const char* streamName, PyObject* original);
	bool unRedirectOutput();
	bool populateVariableInfo();
	PyObject* createLocalDict();

	// singleton methods (called once for all PythonScripts)
	static bool initPython();

	// utilities
	static PyObject* getModuleDict(const QString&);
	static PyTypeObject* getPythonLoggerType();
	static PyObject* shibokenConvertToPyObject(PythonLogger*);
	static int getPyErrorLine();
	static QString pyUnicodeToQString(PyObject*);

	// singletons (shared between PythonScripts)
	static bool ready;
	static PyObject* sysStdOut;
	static PyObject* sysStdErr;
};

#endif
