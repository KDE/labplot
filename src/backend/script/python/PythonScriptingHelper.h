/*
	File                 : PythonScriptingHelper.h
	Project              : LabPlot
	Description          : Helper functions for Python scripting (frontend-safe)
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2026 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PYTHONSCRIPTINGHELPER_H
#define PYTHONSCRIPTINGHELPER_H

#include <QStringList>

struct PylabplotMemberInfo {
	QString name;
	bool isMethod;
	bool isProperty;
	QString signature; // For methods: "method(arg1, arg2)"
	QString docstring; // Brief documentation
	QString returnType; // Return type annotation if available
};

// Helper function to get pylabplot symbols without requiring Python.h
// This allows frontend code to query symbols without Python header dependencies
QStringList pylabplotSymbolsHelper();

// Get members (methods and properties) of a specific pylabplot class
QList<PylabplotMemberInfo> pylabplotClassMembersHelper(const QString& className);

#endif // PYTHONSCRIPTINGHELPER_H
