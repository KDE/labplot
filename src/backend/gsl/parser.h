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

#include "parserFunctionTypes.h"
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

struct Payload {
	virtual ~Payload() {}
};

struct special_function_def {
	funs* funsptr;
	Payload* payload;
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
bool set_specialfunction0(const char* function_name, func_tPayload function, Payload* payload);
bool set_specialfunction1(const char* function_name, func_t1Payload function, Payload* payload);
bool set_specialfunction2(const char* function_name, func_t2Payload function, Payload* payload);
bool set_specialfunction3(const char* function_name, func_t3Payload function, Payload* payload);
bool set_specialfunction4(const char* function_name, func_t4Payload function, Payload* payload);

#endif /*PARSER_H*/
