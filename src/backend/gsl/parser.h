/*
	File                 : parser.h
	Project              : LabPlot
	Description          : Parser for mathematical expressions
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2014-2020 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-FileCopyrightText: 2014 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PARSER_H
#define PARSER_H

#include "constants.h"
#include "functions.h"
#include <gsl/gsl_version.h>

/* uncomment to enable parser specific debugging */
/* #define PDEBUG 1 */

/* variables to pass to parser */
#define MAX_VARNAME_LENGTH 10
typedef struct parser_var {
	char name[MAX_VARNAME_LENGTH];
	double value;
} parser_var;

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

struct special_function_def {
	funs* funsptr;
	void* payload;
};

/* structure for list of symbols */
typedef struct symbol {
	char* name; /* name of symbol */
	int type; /* type of symbol: either VAR or FNCT */
	union {
		double var; /* value of a VAR */
		funs* funsptr; /* value of a FNCT */
		special_function_def special_function; /* value of SPECFNCT */
	} value;
	struct symbol* next; /* next symbol */
} symbol;

int variablesCounter();

void init_table(void); /* initialize symbol table */
void delete_table(void); /* delete symbol table */
int parse_errors(void);
symbol* assign_symbol(const char* symbol_name, double value);
int remove_symbol(const char* symbol_name);
double parse(const char* string, const char* locale);
double parse_with_vars(const char[], const parser_var[], int nvars, const char* locale);

#endif /*PARSER_H*/
