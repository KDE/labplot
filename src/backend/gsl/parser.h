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
#include "parserFunctionTypes.h"
#include <gsl/gsl_version.h>
#include <memory>
#include <variant>

/* uncomment to enable parser specific debugging */
/* #define PDEBUG 1 */

/* variables to pass to parser */
#define MAX_VARNAME_LENGTH 10
typedef struct parser_var {
	char name[MAX_VARNAME_LENGTH];
	double value;
} parser_var;

struct Payload {
	explicit Payload(bool constant = false)
		: constant(constant) {
	}
	virtual ~Payload() {
	}
	bool constant{false};
};

struct special_function_def {
	special_function_def()
		: funsptr(nullptr)
		, payload(std::weak_ptr<Payload>()) {
	}
	funs* funsptr;
	std::weak_ptr<Payload> payload;
};

/* structure for list of symbols */
typedef struct symbol {
	char* name; /* name of symbol */
	int type; /* type of symbol: either VAR or FNCT */
	std::variant<double, funs*, special_function_def> value;
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
bool set_specialfunction0(const char* function_name, func_tPayload function, std::shared_ptr<Payload> payload);
bool set_specialfunction1(const char* function_name, func_t1Payload function, std::shared_ptr<Payload> payload);
bool set_specialfunction2(const char* function_name, func_t2Payload function, std::shared_ptr<Payload> payload);
bool set_specialfunction3(const char* function_name, func_t3Payload function, std::shared_ptr<Payload> payload);
bool set_specialfunction4(const char* function_name, func_t4Payload function, std::shared_ptr<Payload> payload);
const char* lastErrorMessage();

extern bool skipSpecialFunctionEvaluation;
#endif /*PARSER_H*/
