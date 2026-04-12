/*
	File                 : PythonScriptingInfo.cpp
	Project              : LabPlot
	Description          : Python runtime information queries
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2026 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "PythonScriptingInfo.h"
#include <Python.h>

namespace PythonScriptingInfo {

bool isInitialized() {
	return Py_IsInitialized();
}

QString version() {
	if (!Py_IsInitialized())
		return {};
	// Py_GetVersion() returns e.g. "3.11.11 (main, ...)" — extract just the version number
	const char* full = Py_GetVersion();
	return QString::fromUtf8(full).section(QLatin1Char(' '), 0, 0);
}

QString prefix() {
	if (!Py_IsInitialized())
		return {};
	return QString::fromWCharArray(Py_GetPrefix());
}

}
