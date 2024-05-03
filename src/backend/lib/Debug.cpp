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
}

bool debugOutputEnabled() {
	return _debugOutputEnabled;
}
void setDebugOutputEnable(bool enabled) {
	_debugOutputEnabled = enabled;
}
