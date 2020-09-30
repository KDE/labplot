/***************************************************************************
    File                 : parser.y
    Project              : LabPlot
    Description          : Parser for mathematical expressions
    --------------------------------------------------------------------
    Copyright            : (C) 2014 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2014-2020 Stefan Gerlach (stefan.gerlach@uni.kn)

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/

%{
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <locale.h>
#ifdef HAVE_XLOCALE
#include <xlocale.h>
#endif
#include "parser.h"
#include "constants.h"
#include "functions.h"
#if defined(_WIN32)
#define locale_t _locale_t
#define strtod_l _strtod_l
#define freelocale _free_locale
#endif

#ifdef PDEBUG
#include <stdio.h>
#define pdebug(...) fprintf(stderr, __VA_ARGS__)
#else
#define pdebug(...) {}
#endif

#define YYERROR_VERBOSE 1

/* params passed to yylex (and yyerror) */
typedef struct param {
	size_t pos;		/* current position in string */
	char* string;		/* the string to parse */
	const char* locale;	/* name of locale to convert numbers */
} param;

int yyerror(param *p, const char *err);
int yylex(param *p);

double res;
%}

%lex-param {param *p}
%parse-param {param *p}

%union {
double dval;	/* For returning numbers */
symbol *tptr;   /* For returning symbol-table pointers */
}

%token <dval>  NUM 	/* Simple double precision number */
%token <tptr> VAR FNCT	/* VARiable and FuNCTion */
%type  <dval>  expr

%right '='
%left '-' '+'
%left '*' '/' '%'
%left NEG     /* Negation--unary minus */
%right '^' '!'

%%
input:   /* empty */
	| input line
;

line:	'\n'
	| expr '\n'   { res=$1; }
	| error '\n' { yyerrok; }
;

expr:      NUM       { $$ = $1;                            }
| VAR                { $$ = $1->value.var;                 }
| VAR '=' expr       { $$ = $3; $1->value.var = $3;        }
| FNCT '(' ')'       { $$ = (*($1->value.fnctptr))();      }
| FNCT '(' expr ')'  { $$ = (*((func_t1)($1->value.fnctptr)))($3); }
| FNCT '(' expr ',' expr ')'  { $$ = (*((func_t2)($1->value.fnctptr)))($3,$5); }
| FNCT '(' expr ',' expr ','expr ')'  { $$ = (*((func_t3)($1->value.fnctptr)))($3,$5,$7); }
| FNCT '(' expr ',' expr ',' expr ','expr ')'  { $$ = (*((func_t4)($1->value.fnctptr)))($3,$5,$7,$9); }
| expr '+' expr      { $$ = $1 + $3;                       }
| expr '-' expr      { $$ = $1 - $3;                       }
| expr '*' expr      { $$ = $1 * $3;                       }
| expr '/' expr      { $$ = $1 / $3;                       }
| expr '%' expr      { $$ = (int)($1) % (int)($3);         }
| '-' expr  %prec NEG{ $$ = -$2;                           }
| expr '^' expr      { $$ = pow($1, $3);                   }
| expr '*' '*' expr  { $$ = pow($1, $4);                   }
| '(' expr ')'       { $$ = $2;                            }
| '|' expr '|'       { $$ = fabs($2);                      }
| expr '!'           { $$ = gsl_sf_fact((unsigned int)$1); }
/* logical operators (!,&&,||) are not supported */
;

%%

/* global symbol table (as linked list) */
symbol *symbol_table = 0;

int parse_errors(void) {
	return yynerrs;
}

int yyerror(param *p, const char *s) {
	/* remove trailing newline */
	p->string[strcspn(p->string, "\n")] = 0;
	printf("PARSER ERROR: %s @ position %d of string '%s'\n", s, (int)(p->pos), p->string);

	return 0;
}

/* save symbol in symbol table (at start of linked list) */
symbol* put_symbol(const char *symbol_name, int symbol_type) {
/*	pdebug("PARSER: put_symbol(): symbol_name = '%s'\n", symbol_name); */

	symbol *ptr = (symbol *)malloc(sizeof(symbol));
	ptr->name = (char *)malloc(strlen(symbol_name) + 1);
	strcpy(ptr->name, symbol_name);
	ptr->type = symbol_type;
	ptr->value.var = 0;	/* set value to 0 even if fctn */
	ptr->next = (symbol *)symbol_table;
	symbol_table = ptr;
	
/*	pdebug("PARSER: put_symbol() DONE\n"); */
	return ptr;
}

/* remove symbol of name symbol_name from symbol table
   removes only variables of value 0
   returns 0 on success */
int remove_symbol(const char *symbol_name) {
	symbol* ptr = symbol_table;

	/* check if head contains symbol */
	if (ptr && (strcmp(ptr->name, symbol_name) == 0)) {
		if (ptr->type == VAR && ptr->value.var == 0) {
			pdebug("PARSER: REMOVING symbol '%s'\n", symbol_name);
			symbol_table = ptr->next;
			free(ptr->name);
			free(ptr);
		}
		return 0;
	}

	/* search for symbol to be deleted */
	symbol* prev;
	while (ptr && (strcmp(ptr->name, symbol_name) != 0)) {
		prev = ptr;
		ptr = ptr->next;
	}

	/* symbol not found or is not a variable or is not 0 */
	if (!ptr || ptr->type != VAR || ptr->value.var != 0)
		return 1;

	/* remove symbol */
	pdebug("PARSER: REMOVING symbol '%s'\n", symbol_name);
	prev->next = ptr->next;
	free(ptr->name);
	free(ptr);

	return 0;
}

/* get symbol from symbol table
   returns 0 if symbol not found */
symbol* get_symbol(const char *symbol_name) {
	pdebug("PARSER: get_symbol(): symbol_name = '%s'\n", symbol_name);
	
	symbol *ptr;
	for (ptr = symbol_table; ptr != 0; ptr = (symbol *)ptr->next) {
		/* pdebug("%s ", ptr->name); */
		if (strcmp(ptr->name, symbol_name) == 0) {
			pdebug("PARSER:		SYMBOL FOUND\n");
			return ptr;
		}
	}

	pdebug("PARSER:		SYMBOL NOT FOUND\n");
	return 0;
}

/* initialize symbol table with all known functions and constants */
void init_table(void) {
	pdebug("PARSER: init_table()\n");

	symbol *ptr = 0;
	int i;
	/* add functions */
	for (i = 0; _functions[i].name != 0; i++) {
		ptr = put_symbol(_functions[i].name, FNCT);
		ptr->value.fnctptr = _functions[i].fnct;
	}
	/* add constants */
	for (i = 0; _constants[i].name != 0; i++) {
		ptr = put_symbol(_constants[i].name, VAR);
		ptr->value.var = _constants[i].value;
	}

	pdebug("PARSER: init_table() DONE. sym_table = %p\n", ptr);
}

void delete_table(void) {
	pdebug("PARSER: delete_table()\n");
	while(symbol_table) {
		symbol *tmp = symbol_table;
		symbol_table = symbol_table->next;
		free(tmp->name);
		free(tmp);
	}
}

/* add new symbol with value or just set value if symbol is a variable */
symbol* assign_symbol(const char* symbol_name, double value) {
	pdebug("PARSER: assign_symbol() : symbol_name = '%s', value = %g\n", symbol_name, value);

	/* be sure that the symbol table has been initialized */
	if (!symbol_table)
		init_table();

	symbol* ptr = get_symbol(symbol_name);
	if (!ptr) {
		pdebug("PARSER: calling putsymbol(): symbol_name = '%s'\n", symbol_name);
		ptr = put_symbol(symbol_name, VAR);
	} else {
		pdebug("PARSER: Symbol already assigned\n");
	}

	/* do not assign value if symbol already exits as function */
	if (ptr->type == VAR)
		ptr->value.var = value;

	return ptr;
};

static char getcharstr(param *p) {
	pdebug(" getcharstr() pos = %d\n", (int)(p->pos));

	if (p->string[p->pos] == '\0')
		return EOF;
	/* pdebug("PARSER: 	char is %c\n", p->string[p->pos]); */
	return p->string[(p->pos)++];
}

static void ungetcstr(size_t *pos) {
	/* pdebug("PARSER: ungetcstr()\n"); */
	if (*pos > 0)
		(*pos)--;
}

double parse(const char* string, const char* locale) {
	pdebug("\nPARSER: parse('%s') len = %d\n********************************\n", string, (int)strlen(string));

	/* be sure that the symbol table has been initialized */
	if (!symbol_table)
		init_table();

	param p;
	p.pos = 0;
	p.locale = locale;

	/* leave space to terminate string by "\n\0" */
	const size_t slen = strlen(string) + 2;
	p.string = (char *) malloc(slen * sizeof(char));
	if (p.string == NULL) {
		printf("PARSER ERROR: Out of memory for parsing string\n");
		return 0.;
	}

	strcpy(p.string, string);
	p.string[strlen(string)] = '\n';	// end for parsing
	p.string[strlen(string)+1] = '\0';	// end of string
	/* pdebug("PARSER: Call yyparse() for \"%s\" (len = %d)\n", p.string, (int)strlen(p.string)); */

	/* parameter for yylex */
	res = NAN;	/* default value */
	yyparse(&p);

	pdebug("PARSER: parse() DONE (result = %g, errors = %d)\n*******************************\n", res, parse_errors());
	free(p.string);
	p.string = 0;

	return res;
}

double parse_with_vars(const char *str, const parser_var *vars, int nvars, const char* locale) {
	pdebug("\nPARSER: parse_with_var(\"%s\") len = %d\n", str, (int)strlen(str));

	int i;
	for(i = 0; i < nvars; i++) {	/*assign vars */
		pdebug("PARSER: Assign '%s' the value %g\n", vars[i].name, vars[i].value);
		assign_symbol(vars[i].name, vars[i].value);
	}

	return parse(str, locale);
}

int yylex(param *p) {
	pdebug("PARSER: YYLEX()");

	/* get char and skip white space */
	char c;
	while ((c = getcharstr(p)) == ' ' || c == '\t');

	/* finish if reached EOF */
	if (c == EOF) {
		pdebug("PARSER: FINISHED\n");
		return 0;
	}
	/* check for non-ASCII chars */
	if (!isascii(c)) {
		pdebug(" non-ASCII character found. Giving up\n");
		yynerrs++;
		return 0;
	}
	if (c == '\n') {
		pdebug("PARSER: Reached EOL\n");
		return c;
	}

	pdebug("PARSER: PROCESSING character '%c'\n", c);

	/* process numbers */
	if (isdigit(c)) {
		pdebug("PARSER: Found NUMBER (starts with digit)\n");
                ungetcstr(&(p->pos));
                char *s = &(p->string[p->pos]);

		/* convert to double */
		char *remain;
#if defined(_WIN32)
		locale_t locale = _create_locale(LC_NUMERIC, p->locale);
#else
		locale_t locale = newlocale(LC_NUMERIC_MASK, p->locale, (locale_t)0);
#endif
		const double result = strtod_l(s, &remain, locale);
		freelocale(locale);

		pdebug("PARSER:		Reading: '%s' with locale %s\n", s, p->locale);
		pdebug("PARSER:		Remain: '%s'\n", remain);

		/* check conversion */
		if(strlen(s) == strlen(remain))
			return 0;

		pdebug("PARSER:		Result = %g\n", result);
		yylval.dval = result;

                p->pos += strlen(s) - strlen(remain);

		return NUM;
	}

	/* process symbol */
	if (isalpha (c) || c == '.') {
		pdebug("PARSER: Found SYMBOL (starts with alpha)\n");
		static char *symbol_name = 0;
		static int length = 0;
		int i = 0;

		/* Initially make the buffer long enough for a 10-character symbol name */
		if (length == 0) {
			length = 10;
			symbol_name = (char *) malloc(length + 1);
		}

		do {
			pdebug("PARSER: Reading symbol .. ");
			/* If buffer is full, make it bigger */
			if (i == length) {
				length *= 2;
				symbol_name = (char *) realloc(symbol_name, length + 1);
			}
			symbol_name[i++] = c;
			c = getcharstr(p);
			pdebug("PARSER:		got '%c'\n", c);
		}
		while (c != EOF && (isalnum(c) || c == '_' || c == '.'));
		pdebug("PARSER: Reading SYMBOL DONE\n");

		if (c != EOF)
			ungetcstr(&(p->pos));
		symbol_name[i] = '\0';

		symbol *s = get_symbol(symbol_name);
		if(s == 0) {	/* symbol unknown */
			pdebug("PARSER ERROR: Symbol '%s' UNKNOWN\n", symbol_name);
			yynerrs++;
			return 0;
			/* old behavior: add symbol */
			/* s = put_symbol(symbol_name, VAR); */
		}

		yylval.tptr = s;
		return s->type;
	}

	/* else: single operator */
	pdebug("PARSER: Found single operator '%c'\n", c);
	return c;
}
