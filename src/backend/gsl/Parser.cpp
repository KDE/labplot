#include "Parser.h"
#include "ParserDeclarations.h"
#include "parser_private.h"
#include "backend/lib/Debug.h"

namespace Parsing {

Parser::Parser(bool highPerformance): mUsedSymbolsStateMachine(highPerformance ? UsedSymbols::Initialize : UsedSymbols::No) {

}

double Parser::parse(const char* string, const char* locale) {
	DEBUG_PARSER("PARSER: parse('" << string << "') len = " << (int)strlen(string));
	DEBUG_PARSER("********************************");

	/* be sure that the symbol table has been initialized */
	if (variable_symbols.empty() || static_symbols.empty()) {
		init_table();
	}
	mLastErrorMessage.clear();

	param p;
	p.locale = locale;
	p.parser = this;
	p.string = string;
	/* pdebug("PARSER: Call yyparse() for \"%s\" (len = %d)\n", p.string, (int)strlen(p.string)); */

	/* parameter for yylex */
	mResult = NAN;	/* default value */
	mParseErrors = 0;	/* reset error count */
	yyparse(&p);

	mResult = p.result;
	mVariablesCounter = p.variablesCounter;
	mParseErrors = p.errorCount;
	DEBUG_PARSER("PARSER: parse() DONE (result = " << mResult << ", errors = " << parseErrors() << ")");
	DEBUG_PARSER("*******************************");

	if (mUsedSymbolsStateMachine == UsedSymbols::Initialize)
		mUsedSymbolsStateMachine = UsedSymbols::Only;

	return mResult;
}

double Parser::parse_with_vars(const char *str, const parser_var *vars, int nvars, const char* locale) {
	DEBUG_PARSER("PARSER: parse_with_var('" << str << "') len = " << (int)strlen(str));

	int i;
	for(i = 0; i < nvars; i++) {	/*assign vars */
		DEBUG_PARSER("PARSER: Assign '" << vars[i].name << "' the value " << vars[i].value);
		assign_symbol(vars[i].name, vars[i].value);
	}

	return parse(str, locale);
}

int Parser::parseErrors() const {
	return mParseErrors;
}

std::string Parser::lastErrorMessage() const {
	return mLastErrorMessage;
}

size_t Parser::variablesCounter() const {
	return mVariablesCounter;
}

void Parser::setSkipSpecialFunctionEvaluation(bool skip) {
	mSkipSpecialFunctionEvaluation = skip;
}

bool Parser::skipSpecialFunctionEvaluation() const {
	return mSkipSpecialFunctionEvaluation;
}

int yyerror(param *p, const char *s) {
	/* remove trailing newline */
	DEBUG_PARSER("PARSER ERROR: " << s << " @ position " << (int)p->pos << " of string '" << p->string << "'");

	return 0;
}

BaseSymbol* Parser::get_used_symbol(const char* symbol_name) {
	for (auto* s: mUsedSymbols) {
		if (s->name == symbol_name) {
			DEBUG_PARSER("PARSER:		USED SYMBOL FOUND\n");
			return s;
		}
	}
	DEBUG_PARSER("PARSER:		USED_SYMBOL NOT FOUND\n");
	return nullptr;
}

int Parser::remove_symbol(const char *symbol_name) {
	return remove_symbol_(symbol_name);
}

bool Parser::set_specialfunctionPayload(const char* function_name, func_tPayload function, std::weak_ptr<Payload> payload) {
	DEBUG_PARSER("PARSER: set_SpecialFunction0()");


	auto* ptr = get_variable_symbol(function_name);
	if (!ptr) // function name not found
		return false;

	assert(std::get<special_function_def>(ptr->value).funsptr->argc == 0);
	std::get<special_function_def>(ptr->value).funsptr->fnct = function;
	std::get<special_function_def>(ptr->value).funsptr->name = function_name;
	std::get<special_function_def>(ptr->value).payload = payload;
	return true;
}

bool Parser::set_specialfunctionValuePayload(const char* function_name, func_tValuePayload function, std::shared_ptr<Payload> payload) {
	DEBUG_PARSER("PARSER: set_specialfunctionValuePayload()");


	auto* ptr = get_variable_symbol(function_name);
	if (!ptr) // function name not found
		return false;
	assert(std::get<special_function_def>(ptr->value).funsptr->argc == 1);
	std::get<special_function_def>(ptr->value).funsptr->fnct = function;
	std::get<special_function_def>(ptr->value).funsptr->name = function_name;
	std::get<special_function_def>(ptr->value).payload = payload;
	return true;
}

bool Parser::set_specialfunction2ValuePayload(const char* function_name, func_t2ValuePayload function, std::shared_ptr<Payload> payload) {
	DEBUG_PARSER("PARSER: set_specialfunction2ValuePayload()");


	auto* ptr = get_variable_symbol(function_name);
	if (!ptr) // function name not found
		return false;
	assert(std::get<special_function_def>(ptr->value).funsptr->argc == 2);
	std::get<special_function_def>(ptr->value).funsptr->fnct = function;
	std::get<special_function_def>(ptr->value).funsptr->name = function_name;
	std::get<special_function_def>(ptr->value).payload = payload;
	return true;
}

bool Parser::set_specialfunctionVariablePayload(const char* function_name, func_tVariablePayload function, std::shared_ptr<Payload> payload) {
	DEBUG_PARSER("PARSER: set_specialfunctionVariablePayload()");


	auto* ptr = get_variable_symbol(function_name);
	if (!ptr) // function name not found
		return false;
	assert(std::get<special_function_def>(ptr->value).funsptr->argc == 1);
	std::get<special_function_def>(ptr->value).funsptr->fnct = function;
	std::get<special_function_def>(ptr->value).funsptr->name = function_name;
	std::get<special_function_def>(ptr->value).payload = payload;
	return true;
}

bool Parser::set_specialfunctionValueVariablePayload(const char* function_name, func_tValueVariablePayload function, std::shared_ptr<Payload> payload) {
	DEBUG_PARSER("PARSER: set_specialfunctionValueVariablePayload()");


	auto* ptr = get_variable_symbol(function_name);
	if (!ptr) // function name not found
		return false;

	assert(std::get<special_function_def>(ptr->value).funsptr->argc == 2);
	std::get<special_function_def>(ptr->value).funsptr->fnct = function;
	std::get<special_function_def>(ptr->value).funsptr->name = function_name;
	std::get<special_function_def>(ptr->value).payload = payload;
	return true;
}

bool Parser::set_specialfunction2ValueVariablePayload(const char* function_name, func_t2ValueVariablePayload function, std::shared_ptr<Payload> payload) {
	DEBUG_PARSER("PARSER: set_specialfunction2ValueVariablePayload()");


	auto* ptr = get_variable_symbol(function_name);
	if (!ptr) // function name not found
		return false;

	assert(std::get<special_function_def>(ptr->value).funsptr->argc == 3);
	std::get<special_function_def>(ptr->value).funsptr->fnct = function;
	std::get<special_function_def>(ptr->value).funsptr->name = function_name;
	std::get<special_function_def>(ptr->value).payload = payload;
	return true;
}

bool Parser::set_specialfunction3ValueVariablePayload(const char* function_name, func_t3ValueVariablePayload function, std::shared_ptr<Payload> payload) {
	DEBUG_PARSER("PARSER: set_specialfunction3ValueVariablePayload()");


	auto* ptr = get_variable_symbol(function_name);
	if (!ptr) // function name not found
		return false;

	assert(std::get<special_function_def>(ptr->value).funsptr->argc == 4);
	std::get<special_function_def>(ptr->value).funsptr->fnct = function;
	std::get<special_function_def>(ptr->value).funsptr->name = function_name;
	std::get<special_function_def>(ptr->value).payload = payload;
	return true;
}

UsedSymbols Parser::usedSymbolsState() const {
	return mUsedSymbolsStateMachine;
}

std::vector<std::string> Parser::get_used_symbols() {
	std::vector<std::string> names;
	for (const auto* s: mUsedSymbols) {
		names.push_back(std::string(s->name));
	}
	return names;
}

void Parser::addUsedSymbol(BaseSymbol* s) {
	mUsedSymbols.push_back(s);
}

void Parser::setLastErrorMessage(const std::string& str) {
	mLastErrorMessage = str;
}

// ######################################################################

Symbol* get_variable_symbol(const char* symbol_name) {
	for (auto* s: variable_symbols) {
		if (s->name == symbol_name) {
			DEBUG_PARSER("PARSER:		SYMBOL FOUND\n");
			return s;
		}
	}
	DEBUG_PARSER("PARSER:		SYMBOL NOT FOUND\n");
	return nullptr;
}

StaticSymbol* get_static_symbol(const char* symbol_name) {
	for (auto* s: static_symbols) {
		if (s->name == symbol_name) {
			DEBUG_PARSER("PARSER:		SYMBOL FOUND\n");
			return s;
		}
	}
	DEBUG_PARSER("PARSER:		SYMBOL NOT FOUND\n");
	return nullptr;
}

void delete_table(void) {
	DEBUG_PARSER("PARSER: delete_table()");
	clear_static_symbols();
	clear_variable_symbols();
}

void clear_static_symbols() {
	StaticSymbol* s;
	while (!static_symbols.empty()) {
		s = static_symbols.back();
		static_symbols.pop_back();
		delete s;
	}
}

void clear_variable_symbols() {
	Symbol* s;
	while (!variable_symbols.empty()) {
		s = variable_symbols.back();
		variable_symbols.pop_back();
		delete s;
	}
}

} // namespace Parsing

