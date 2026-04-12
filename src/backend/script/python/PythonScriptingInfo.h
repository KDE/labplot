/*
	File                 : PythonScriptingInfo.h
	Project              : LabPlot
	Description          : Python runtime information queries (no Python.h dependency)
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2026 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PYTHONSCRIPTINGINFO_H
#define PYTHONSCRIPTINGINFO_H

#include <QString>

namespace PythonScriptingInfo {

// Returns true if the Python interpreter has been initialized
bool isInitialized();

// Returns the Python version string (e.g. "3.11.11"), or empty if not initialized
QString version();

// Returns the Python prefix path (e.g. "/usr"), or empty if not initialized
QString prefix();

}

#endif
