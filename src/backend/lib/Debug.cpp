/*
	File                 : Debug.cpp
	Project              : LabPlot
	Description          : Debug utilities
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "Debug.h"

namespace {
bool _debugTraceEnabled = false;
bool _perfTraceEnabled = false;
}

bool debugTraceEnabled() {
	return _debugTraceEnabled;
}
void enableDebugTrace(bool enabled) {
	_debugTraceEnabled = enabled;
}

bool perfTraceEnabled() {
	return _perfTraceEnabled;
}

void enablePerfTrace(bool enabled) {
	_perfTraceEnabled = enabled;
}
