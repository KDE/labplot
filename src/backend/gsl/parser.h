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

namespace Parser {

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

struct BaseSymbol {
	std::string_view name; /* name of symbol */
	int type; /* type of symbol: either VAR or FNCT */
	std::variant<double, funs*, special_function_def> value;
};

/* structure for list of symbols */
struct Symbol : public BaseSymbol {
	Symbol(const char* symbol_name, int len, int _type) {
		type = _type;
		init(symbol_name, len);
	}
	Symbol(const char* symbol_name, int len, int _type, special_function_def fn) {
		type = _type;
		init(symbol_name, len);
		value = fn;
	}
	~Symbol() {
		free(nameAlloc);
	}

private:
	inline void init(const char* symbol_name, int len) {
		nameAlloc = (char*)malloc(len + 1);
		strcpy(nameAlloc, symbol_name);
		name = std::string_view(nameAlloc);
	}
	char* nameAlloc; /* name of symbol */
};

struct StaticSymbol : public BaseSymbol {
	// I did not find a way to use the bison token and setting the type automatically
	StaticSymbol(std::string_view _name, int _type, double _value) {
		name = _name;
		type = _type;
		value = _value;
	}
	StaticSymbol(std::string_view _name, int _type, funs* fn) {
		name = _name;
		type = _type;
		value = fn;
	}
};

enum class UsedSymbols {
	No, // ignore used symbols functionality
	Initialize,
	Only // Use only used symbols found previously with Initialize
};

int variablesCounter();

void init_table(void); /* initialize symbol table */
void delete_table(void); /* delete symbol table */
int parse_errors(void);
BaseSymbol* assign_symbol(const char* symbol_name, double value, UsedSymbols usedSymbols = UsedSymbols::No);
int remove_symbol(const char* symbol_name);
double parse(const char* string, const char* locale, UsedSymbols usedSymbols = UsedSymbols::No);
double parse_with_vars(const char[], const parser_var[], int nvars, const char* locale);
bool set_specialfunction0(const char* function_name, func_tPayload function, std::shared_ptr<Payload> payload);
bool set_specialfunction1(const char* function_name, func_t1Payload function, std::shared_ptr<Payload> payload);
bool set_specialfunction2(const char* function_name, func_t2Payload function, std::shared_ptr<Payload> payload);
bool set_specialfunction3(const char* function_name, func_t3Payload function, std::shared_ptr<Payload> payload);
bool set_specialfunction4(const char* function_name, func_t4Payload function, std::shared_ptr<Payload> payload);
std::string lastErrorMessage();
std::vector<std::string> get_used_symbols();

extern bool skipSpecialFunctionEvaluation;

} // namespace Parser

#endif /*PARSER_H*/
