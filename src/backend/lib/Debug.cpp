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
bool _debugOutputEnabled = false;
bool _traceOutputEnabled = false;
}

bool debugOutputEnabled() {
	return _debugOutputEnabled;
}
void enableDebugOutput(bool enabled) {
	_debugOutputEnabled = enabled;
}

bool traceOutputEnabled() {
	return _traceOutputEnabled;
}

void enableTraceOutput(bool enabled) {
	_traceOutputEnabled = enabled;
}
