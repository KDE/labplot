#ifndef PARSER_PRIVATE_H
#define PARSER_PRIVATE_H

#include <vector>

extern int yyparse(Parsing::param* p);

namespace Parsing {
struct StaticSymbol;
struct Symbol;

extern std::vector<StaticSymbol*> static_symbols;
extern std::vector<Symbol*> variable_symbols;

Symbol* get_variable_symbol(const char* symbol_name);
StaticSymbol* get_static_symbol(const char* symbol_name);
void init_table(void);
void delete_table(void);
void clear_static_symbols();
void clear_variable_symbols();
int remove_symbol_(const char* symbol_name);
Symbol* put_symbol(const char* symbol_name, int symbol_type);
}

#endif // PARSER_PRIVATE_H
