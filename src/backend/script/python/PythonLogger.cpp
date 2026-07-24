/*
	File                 : PythonLogger.cpp
	Project              : LabPlot
	Description          : Python Logger
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2025 Israel Galadima <izzygaladima@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <backend/lib/macros.h>

#include "PythonLogger.h"
#include "PythonScriptRuntime.h"

PythonLogger::PythonLogger(PythonScriptRuntime* pythonScript, bool isStdErr)
	: m_pythonScript(pythonScript)
	, m_isStdErr(isStdErr) {
}

void PythonLogger::write(const QString& text) {
	Q_EMIT m_pythonScript->writeOutput(m_isStdErr, text);
}
