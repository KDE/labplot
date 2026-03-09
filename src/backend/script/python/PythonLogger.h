/*
	File                 : PythonLogger.h
	Project              : LabPlot
	Description          : Python Logger
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2025 Israel Galadima <izzygaladima@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PYTHON_LOGGER_H
#define PYTHON_LOGGER_H

#include <QString>

class PythonScriptRuntime;

class PythonLogger {
public:
	explicit PythonLogger(PythonScriptRuntime*, bool);
	void write(const QString&);

private:
	PythonScriptRuntime* m_pythonScript{nullptr};
	bool m_isStdErr{false};
};

#endif
