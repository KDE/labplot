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

/* Function types */
#ifdef _MSC_VER /* MSVC needs void argument */
typedef double (*func_t)(void);
#else
typedef double (*func_t)();
typedef double (*func_tPayload)(void*);
#endif
typedef double (*func_t1)(double);
typedef double (*func_t2)(double, double);
typedef double (*func_t3)(double, double, double);
typedef double (*func_t4)(double, double, double, double);
typedef double (*func_t1Payload)(double, void*);
typedef double (*func_t2Payload)(double, double, void*);
typedef double (*func_t3Payload)(double, double, double, void*);
typedef double (*func_t4Payload)(double, double, double, double, void*);

#endif /*PARSERFUNCTIONTYPES_H*/
