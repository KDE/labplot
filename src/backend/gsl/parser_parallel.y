/***************************************************************************
    File                 : parser.y
    Project              : LabPlot
    Description          : Parser for mathematical expressions
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2014 Alexander Semke <alexander.semke@web.de>
    SPDX-FileCopyrightText: 2014-2016 Stefan Gerlach <stefan.gerlach@uni.kn>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

/* TODO: not working yet! */


%{
#include <string.h>
#include <strings.h>	/* bzero */
#include <ctype.h>
#include <stdlib.h>
#include <locale.h>
#ifndef HAVE_WINDOWS
#include <xlocale.h>
#endif
#include "constants.h"
#include "functions.h"
#include "parser.h"

/* Enable Parser DEBUGGING */
/*
#define PDEBUG
*/

#ifdef PDEBUG
#define pdebug(...) fprintf(stderr, __VA_ARGS__)
#else
/*#define pdebug(...) do {} while (0)*/
#define pdebug(...) {}
#endif

#define YYERROR_VERBOSE 1

/* params passed to yylex (and yyerror) */
typedef struct param {
	unsigned int pos;	/* current position in string */
	char *string;		/* the string to parse */
	symrec *sym_table;	/* the symbol table */
} param;

int yyerror(param *p, const char *err);
int yylex(param *p);

double res;
%}

%lex-param {param *p}
%parse-param {param *p}

%union {
double dval;  /* For returning numbers */
symrec *tptr;   /* For returning symbol-table pointers */
}

%token <dval> NUM 	/* Simple double precision number */
%token <tptr> VAR FNCT	/* Variable and Function */
%type  <dval> expr

%right '='
%left '-' '+'
%left '*' '/'
%left NEG     /* Negation--unary minus */
%right '^'    /* Exponential */

%%
input:   /* empty */
	| input line
;

line:	'\n'
	| expr '\n'   { res=$1; }
	| error '\n' { yyerrok; }
;

expr:      NUM       { $$ = $1;                         }
| VAR                { $$ = $1->value.var;              }
| VAR '=' expr       { $$ = $3; $1->value.var = $3;     }
| FNCT '(' ')'       { $$ = (*($1->value.fnctptr))();   }
| FNCT '(' expr ')'  { $$ = (*($1->value.fnctptr))($3); }
| FNCT '(' expr ',' expr ')'  { $$ = (*($1->value.fnctptr))($3,$5); }
| FNCT '(' expr ',' expr ','expr ')'  { $$ = (*($1->value.fnctptr))($3,$5,$7); }
| FNCT '(' expr ',' expr ',' expr ','expr ')'  { $$ = (*($1->value.fnctptr))($3,$5,$7,$9); }
| expr '+' expr      { $$ = $1 + $3;                    }
| expr '-' expr      { $$ = $1 - $3;                    }
| expr '*' expr      { $$ = $1 * $3;                    }
| expr '/' expr      { $$ = $1 / $3;                    }
| '-' expr  %prec NEG{ $$ = -$2;                        }
| expr '^' expr      { $$ = pow ($1, $3);               }
| expr '*' '*' expr  { $$ = pow ($1, $4);               }
| '(' expr ')'       { $$ = $2;                         }
;

%%

/* global symbol table */
symrec *symbol_table = 0;

int parse_errors(void) {
	return yynerrs;
}

int yyerror(param *p, const char *s) {
	printf("PARSER ERROR: %s @ position %d of string %s", s, p->pos, p->string);
	return 0;
}

symrec* putsym(symrec *sym_table, const char *sym_name, int sym_type) {
	/*pdebug("PARSER: putsym(): sym_name = %s\n", sym_name);*/

	symrec *ptr = (symrec *) malloc(sizeof(symrec));
	ptr->name = (char *) malloc(strlen(sym_name) + 1);
	strcpy(ptr->name, sym_name);
	ptr->type = sym_type;
	ptr->value.var = 0;	/* set value to 0 even if fctn. */
	ptr->next = (struct symrec *)sym_table;
	sym_table = ptr;

	/*pdebug("PARSER: putsym() DONE sym_table = %p\n", sym_table);*/
	return ptr;
}

symrec* getsym(symrec *sym_table, const char *sym_name) {
	/*pdebug("PARSER: getsym(): sym_name = %s, sym_table = %p\n", sym_name, sym_table);*/

	symrec *ptr;
	for (ptr = sym_table; ptr != 0; ptr = (symrec *)ptr->next) {
		/*pdebug("%s ", ptr->name);*/
		if (strcmp(ptr->name, sym_name) == 0) {
			/*pdebug("PARSER: symbol \'%s\' found\n", sym_name);*/
			return ptr;
		}
	}

	/*pdebug("PARSER: symbol \'%s\' not found\n", sym_name);*/
	return 0;
}

/* put arithmetic functions in table. */
symrec* init_table() {
	pdebug("PARSER: init_table()\n");

	symrec *ptr = 0;
	int i;
	/* add functions */
	for (i = 0; _functions[i].name != 0; i++) {
		ptr = putsym(ptr, _functions[i].name, FNCT);
		ptr->value.fnctptr = _functions[i].fnct;
	}
	/* add constants */
	for (i = 0; _constants[i].name != 0; i++) {
		ptr = putsym(ptr, _constants[i].name, VAR);
		ptr->value.var = _constants[i].value;
	}
	pdebug("PARSER: init_table() DONE sym_table = %p\n", ptr);
	return ptr;
}

void delete_table(symrec *sym_table) {
	symrec *tmp;
	while(sym_table) {
		tmp = sym_table;
		sym_table = sym_table->next;
		free(tmp->name);
		free(tmp);
	}
}

symrec* assign_variable(symrec *sym_table, parser_var var) {
	/*pdebug("PARSER: assign_variable(): symb_name = %s value = %g sym_table = %p\n", var.name, var.value, sym_table);*/

	symrec* ptr = getsym(sym_table, var.name);
	if (!ptr) {
		/*pdebug("PARSER: calling putsym(): symb_name = %s\n", var.name);*/
		ptr = putsym(sym_table, var.name, VAR);
	}
	ptr->value.var = var.value;

	return ptr;
};

static int getcharstr(param *p) {
	/*pdebug("PARSER: getcharstr() pos = %d\n", p->pos);*/

	if (p->string[p->pos] == '\0')
		return EOF;
	return (int) p->string[(p->pos)++];
}

static void ungetcstr(unsigned int *pos) {
    if (*pos > 0)
        (*pos)--;
}

int init_parser() {
	symbol_table = init_table();	
	return 0;
}

int finish_parser() {
	if (symbol_table != 0)
		delete_table(symbol_table);
	symbol_table = 0;

	return 0;
}

double parse_with_vars(const char *str, const parser_var *vars, int nvars) {
	pdebug("\nPARSER: parse(\"%s\") len=%zu\n", str, strlen(str));
	int i;
	for(i = 0; i < nvars; i++) 	/*assign vars */
		pdebug("var %d of name %s with value %g\n", i, vars[i].name, vars[i].value);

	/* parameter for yylex */
	param p;
	p.pos = 0;
	/* leave space to terminate string by "\n\0" */
	size_t slen = strlen(str) + 2;
	p.string = (char *)malloc(slen*sizeof(char));
	/* If there is no global symbol table: create own */
	if (symbol_table == 0)
		p.sym_table = init_table();

	for(i = 0; i < nvars; i++) {	/*assign vars */
		pdebug("assign %s the value %g\n", vars[i].name, vars[i].value);
		if (symbol_table == 0)
			p.sym_table = assign_variable(p.sym_table, vars[i]);
		else
			symbol_table = assign_variable(symbol_table, vars[i]);
	}

	strncpy(p.string, str, slen);
	p.string[strlen(p.string)] = '\n';
	/*pdebug("	slen = %zu, strlen(str) = %zu, strlen(p.string) = %zu\n", slen, strlen(str), strlen(p.string));*/

	yyparse(&p);

	/*pdebug("PARSER: parse() DONE (res = %g, parse errors = %d)\n", res, parse_errors());*/
	free(p.string);
	if (symbol_table == 0)
		delete_table(p.sym_table);

	return res;
}

double parse(const char *str) {
	return parse_with_vars(str, 0, 0);
}

int yylex(param *p) {
	pdebug("PARSER: yylex()\n");
	int c;

	/* skip white spaces */
	while ((c = getcharstr(p)) == ' ' || c == '\t');

	/* finish if reached EOF */
	if (c == EOF) {
		pdebug("FINISHED\n");
		return 0;
	}

	pdebug("PARSER: reading character: %c\n", c);

	/* process numbers */
	if (isdigit(c)) {
		pdebug("PARSER: reading number (starts with digit)\n");
                ungetcstr(&(p->pos));
                char *s = &p->string[p->pos];

		/* convert to double */
		char *remain;
#if defined(_WIN32) || defined(__APPLE__)
		double result = strtod(s, &remain);
#else
		/* use same locale for all languages: '.' as decimal point */
		locale_t locale = newlocale(LC_NUMERIC_MASK, "C", NULL);

		double result = strtod_l(s, &remain, locale);
		freelocale(locale);
#endif
		pdebug("PARSER: reading: %s", s);
		pdebug("PARSER: remain = %s", remain);

		/* check conversion */
		if(strlen(s) == strlen(remain))
			return 0;

		pdebug("PARSER: result = %g\n", result);

		yylval.dval = result;
                p->pos += strlen(s) - strlen(remain);

		return NUM;
	}

	if (isalpha (c) || c == '.') {
		pdebug("PARSER: reading identifier (starts with alpha)\n");
		static char *symbuf = 0;
		static int length = 0;
		int i=0;

		/* Initially make the buffer long enough for a 4-character symbol name */
		if (length == 0)
			length = 20, symbuf = (char *)malloc(length + 1);

		do {
			/* If buffer is full, make it bigger */
			if (i == length) {
				length *= 2;
				symbuf = (char *)realloc(symbuf, length + 1);
			}
			/* Add this character to the buffer */
			symbuf[i++] = c;
			/* Get another character */
			c = getcharstr(p);
		}
		while (c != EOF && (isalnum(c) || c == '_' || c == '.'));

		ungetcstr(&(p->pos));
		symbuf[i] = '\0';

		/*pdebug("PARSER: search for symbol: sym_table = %p\n", p->sym_table);*/
		symrec *s;
		if (symbol_table != 0)
			s = getsym(symbol_table, symbuf);
		else
			s = getsym(p->sym_table, symbuf);
		if(s == 0) {	/* symbol unknown */
			pdebug("PARSER: ERROR: symbol \"%s\" UNKNOWN\n", symbuf);
			return 0;
		}
		/* old behavior */
		/* if (s == 0)
			 s = putsym (symbuf, VAR); */
		yylval.tptr = s;
		return s->type;
	}

	/* else: single operator */
	pdebug("PARSER: single operator\n");
	return c;
}
