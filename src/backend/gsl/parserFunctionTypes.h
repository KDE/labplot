/*
	File                 : parserFunctionTypes.h
	Project              : LabPlot
	Description          : Parser for mathematical expressions
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PARSERFUNCTIONTYPES_H
#define PARSERFUNCTIONTYPES_H

#include <memory>

struct Payload;

/* Function types */
#ifdef _MSC_VER /* MSVC needs void argument */
typedef double (*func_t)(void);
#else
typedef double (*func_t)();
#endif
typedef double (*func_t1)(double);
typedef double (*func_t2)(double, double);
typedef double (*func_t3)(double, double, double);
typedef double (*func_t4)(double, double, double, double);
typedef double (*func_tPayload)(const std::weak_ptr<Payload>);
typedef double (*func_t1Payload)(const char* variable, const std::weak_ptr<Payload>);
typedef double (*func_t2Payload)(double value1, const char* variable, const std::weak_ptr<Payload>);
typedef double (*func_t3Payload)(double value1, double value2, const char* variable, const std::weak_ptr<Payload>);
typedef double (*func_t4Payload)(double value1, double value2, double value3, const char* variable, const std::weak_ptr<Payload>);

#endif /*PARSERFUNCTIONTYPES_H*/
