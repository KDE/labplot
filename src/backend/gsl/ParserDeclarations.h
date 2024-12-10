/*
	File                 : parser.h
	Project              : LabPlot
	Description          : parser header for the generated parser. Do not include it directly. Use Parser.h!
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2014-2020 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-FileCopyrightText: 2014 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PARSER_H
#define PARSER_H

#include "backend/lib/Debug.h"
#include "constants.h"
#include "functions.h"
#include "parserFunctionTypes.h"
#include <gsl/gsl_version.h>
#include <memory>
#include <variant>

namespace Parsing {

class Parser;

// variables to pass to parser
#define MAX_VARNAME_LENGTH 10
typedef struct parser_var {
	char name[MAX_VARNAME_LENGTH];
	double value;
} parser_var;

enum class UsedSymbols {
	No, // ignore used symbols functionality
	Initialize,
	Only // Use only used symbols found previously with Initialize
};

/* params passed to yylex (and yyerror) */
typedef struct param {
	size_t pos{0}; /* current position in string */
	std::string_view string; /* the string to parse */
	const char* locale{nullptr}; /* name of locale to convert numbers */
	Parser* parser{nullptr};
	double result{std::nan("0")};
	size_t variablesCounter{0};
	size_t errorCount{0};
} param;

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
	std::string_view name; // name of symbol
	int type; // type of symbol: either VAR or FNCT
	std::variant<double, funs*, special_function_def> value;
};

// structure for list of symbols
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
	char* nameAlloc; // name of symbol
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
} // namespace Parsing

#endif /*PARSER_H*/
