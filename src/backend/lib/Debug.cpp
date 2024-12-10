/*
	File                 : Debug.cpp
	Project              : LabPlot
	Description          : Debug utilities
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-FileCopyrightText: 2024 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "Debug.h"

namespace {
bool _infoTraceEnabled = false;
bool _debugTraceEnabled = false;
bool _perfTraceEnabled = false;

#define PDEBUG 0
#if PDEBUG == 1
bool _debugParser = true;
#else
bool _debugParser = false;
#endif
}

bool infoTraceEnabled() {
	return _infoTraceEnabled;
}
void enableInfoTrace(bool enabled) {
	_infoTraceEnabled = enabled;
}

bool debugTraceEnabled() {
	return _debugTraceEnabled;
}
void enableDebugTrace(bool enabled) {
	_debugTraceEnabled = enabled;
}
bool debugParserEnabled() {
	return _debugParser;
}

bool perfTraceEnabled() {
	return _perfTraceEnabled;
}
void enablePerfTrace(bool enabled) {
	_perfTraceEnabled = enabled;
}
