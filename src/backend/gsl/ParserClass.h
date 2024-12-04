/*
 *	File                 : Parser.h
 *	Project              : LabPlot
 *	Description          : Parser for mathematical expressions
 *	--------------------------------------------------------------------
 *	SPDX-FileCopyrightText: 2024 Martin Marmsoler <martin.marmsoler@gmail.com>
 *	SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef PARSERCLASS_H
#define PARSERCLASS_H

#include <string>
#include <memory>
#include <cmath>

#include "parserFunctionTypes.h"
#include "parser.h"

class ExpressionParser;

namespace Parsing {

struct BaseSymbol;
struct Symbol;
struct Payload;
struct StaticSymbol;
struct parser_var;
struct param;

class Parser {
public:
	Parser(bool highPerformance = true);

	double parse(const char* string, const char* locale);
	double parse_with_vars(const char *str, const parser_var *vars, int nvars, const char* locale);

	int parseErrors() const;
	std::string lastErrorMessage() const;
	size_t variablesCounter() const;

	void setSkipSpecialFunctionEvaluation(bool skip);
	bool skipSpecialFunctionEvaluation() const;

	/* add new symbol with value or just set value if symbol is a variable */
	BaseSymbol* assign_symbol(const char* symbol_name, double value);
	static bool set_specialfunctionPayload(const char* function_name, func_tPayload function, std::weak_ptr<Payload> payload);
	static bool set_specialfunctionValuePayload(const char* function_name, func_tValuePayload function, std::shared_ptr<Payload> payload);
	static bool set_specialfunction2ValuePayload(const char* function_name, func_t2ValuePayload function, std::shared_ptr<Payload> payload);
	static bool set_specialfunctionVariablePayload(const char* function_name, func_tVariablePayload function, std::shared_ptr<Payload> payload);
	static bool set_specialfunctionValueVariablePayload(const char* function_name, func_tValueVariablePayload function, std::shared_ptr<Payload> payload);
	static bool set_specialfunction2ValueVariablePayload(const char* function_name, func_t2ValueVariablePayload function, std::shared_ptr<Payload> payload);
	static bool set_specialfunction3ValueVariablePayload(const char* function_name, func_t3ValueVariablePayload function, std::shared_ptr<Payload> payload);

	UsedSymbols usedSymbolsState() const;
	void addUsedSymbol(BaseSymbol* s);
	BaseSymbol* get_used_symbol(const char* symbol_name);
	int remove_symbol(const char *symbol_name);
	std::vector<std::string> get_used_symbols();

	void setLastErrorMessage(const std::string& str);

private:
	double mResult {std::nan("0")};
	int mParseErrors {0};
	std::string mLastErrorMessage;
	size_t mVariablesCounter{0};
	UsedSymbols mUsedSymbolsStateMachine; /* if Only, only the symbols in the "used_symbols" are used (performance reason) */
	bool mSkipSpecialFunctionEvaluation {false};
	std::vector<BaseSymbol*> mUsedSymbols;

	friend class ExpressionParser;
};

} // namespace Parsing

#endif // PARSERCLASS_H
