/*
	File                 : Debug.h
	Project              : LabPlot
	Description          : Debug utilities
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-FileCopyrightText: 2024 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DEBUG_H
#define DEBUG_H

#include <iomanip>
#include <iostream>

#ifdef SDK
#include "labplot_export.h"
#endif

bool infoTraceEnabled();
void enableInfoTrace(bool);
#ifdef SDK
bool LABPLOT_EXPORT debugTraceEnabled(); // required by Range template class in SDK
#else
bool debugTraceEnabled();
#endif
void enableDebugTrace(bool);
bool perfTraceEnabled();
void enablePerfTrace(bool);
bool debugParserEnabled();

// show warnings with info
#define WARN(x) INFO(x)

// INFO: only when info trace enabled
#define INFO(x)                                                                                                                                                \
	if (infoTraceEnabled()) {                                                                                                                                  \
		std::cout << std::dec << std::setprecision(std::numeric_limits<double>::digits10 + 1) << std::boolalpha << x                                           \
				  << std::resetiosflags(std::ios_base::boolalpha) << std::setprecision(-1) << std::endl;                                                       \
	}

// (Q)DEBUG: only in Debug mode and when debug trace enabled
#ifndef NDEBUG
#include <QDebug>
#define QDEBUG(x)                                                                                                                                              \
	if (debugTraceEnabled()) {                                                                                                                                 \
		qDebug() << x;                                                                                                                                         \
	}

#define DEBUG(x)                                                                                                                                               \
	if (debugTraceEnabled()) {                                                                                                                                 \
		std::cout << std::dec << std::setprecision(std::numeric_limits<double>::digits10 + 1) << std::boolalpha << x                                           \
				  << std::resetiosflags(std::ios_base::boolalpha) << std::setprecision(-1) << std::endl;                                                       \
	}

#define DEBUG_PARSER(x)                                                                                                                                        \
	if (debugParserEnabled()) {                                                                                                                                \
		std::cout << x << std::endl;                                                                                                                           \
	}

#else
#define QDEBUG(x)                                                                                                                                              \
	{ }
#define DEBUG(x)                                                                                                                                               \
	{ }
#define DEBUG_PARSER(x)                                                                                                                                        \
	{ }
#endif

#define DEBUG_TEXTLABEL_BOUNDING_RECT 0
#define DEBUG_TEXTLABEL_GLUEPOINTS 0
#define DEBUG_AXIS_BOUNDING_RECT 0

#endif // DEBUG_H
