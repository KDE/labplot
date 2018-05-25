/***************************************************************************
    File                 : parser.y
    Project              : LabPlot
    Description          : Parser for mathematical expressions
    --------------------------------------------------------------------
    Copyright            : (C) 2014 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2014-2016 Stefan Gerlach (stefan.gerlach@uni.kn)

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

#ifdef PDEBUG
#include <stdio.h>
#define pdebug(...) fprintf(stderr, __VA_ARGS__)
#else
#define pdebug(...) {}
#endif

#define YYERROR_VERBOSE 1

/* params passed to yylex (and yyerror) */
typedef struct param {
	size_t pos;	/* current position in string */
	char *string;		/* the string to parse */
/*	symrec *sym_table;	the symbol table (not used) */
} param;

int yyerror(param *p, const char *err);
int yylex(param *p);

double res;
%}

%lex-param {param *p}
%parse-param {param *p}

%union {
double dval;	/* For returning numbers */
symrec *tptr;   /* For returning symbol-table pointers */
}

%token <dval>  NUM 	/* Simple double precision number */
%token <tptr> VAR FNCT	/* VARiable and FuNCTion */
%type  <dval>  expr

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
| FNCT '(' expr ')'  { $$ = (*((func_t1)($1->value.fnctptr)))($3); }
| FNCT '(' expr ',' expr ')'  { $$ = (*((func_t2)($1->value.fnctptr)))($3,$5); }
| FNCT '(' expr ',' expr ','expr ')'  { $$ = (*((func_t3)($1->value.fnctptr)))($3,$5,$7); }
| FNCT '(' expr ',' expr ',' expr ','expr ')'  { $$ = (*((func_t4)($1->value.fnctptr)))($3,$5,$7,$9); }
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
symrec *sym_table = 0;

int parse_errors(void) {
	return yynerrs;
}

int yyerror(param *p, const char *s) {
	/* remove trailing newline */
	p->string[strcspn(p->string, "\n")] = 0;
	printf("PARSER ERROR: %s @ position %d of string \'%s\'\n", s, (int)(p->pos), p->string);

	return 0;
}

/* save symbol in symbol table */
symrec* putsym(const char *sym_name, int sym_type) {
	pdebug("PARSER: putsym(): sym_name = %s\n", sym_name);

	symrec *ptr = (symrec *) malloc(sizeof (symrec));
	ptr->name = (char *) malloc(strlen (sym_name) + 1);
	strcpy(ptr->name, sym_name);
	ptr->type = sym_type;
	ptr->value.var = 0;	/* set value to 0 even if fctn */
	ptr->next = (struct symrec *)sym_table;
	sym_table = ptr;
	
	pdebug("PARSER: putsym() DONE\n");
	return ptr;
}

/* get symbol from symbol table */
symrec* getsym(const char *sym_name) {
	pdebug("PARSER: getsym(): sym_name = %s\n", sym_name);
	
	symrec *ptr;
	for (ptr = sym_table; ptr != 0; ptr = (symrec *)ptr->next) {
		/* pdebug("%s ", ptr->name); */
		if (strcmp(ptr->name, sym_name) == 0) {
			pdebug("PARSER: symbol \'%s\' found\n", sym_name);
			return ptr;
		}
	}

	pdebug("PARSER: symbol \'%s\' not found\n", sym_name);
	return 0;
}

void init_table(void) {
	pdebug("PARSER: init_table()\n");

	symrec *ptr = 0;
	int i;
	/* add functions */
	for (i = 0; _functions[i].name != 0; i++) {
		ptr = putsym (_functions[i].name, FNCT);
		ptr->value.fnctptr = _functions[i].fnct;
	}
	/* add constants */
	for (i = 0; _constants[i].name != 0; i++) {
		ptr = putsym (_constants[i].name, VAR);
		ptr->value.var = _constants[i].value;
	}

	pdebug("PARSER: init_table() DONE sym_table = %p\n", ptr);
}

void delete_table(void) {
	while(sym_table) {
		symrec *tmp = sym_table;
		sym_table = sym_table->next;
		free(tmp->name);
		free(tmp);
	}
}

symrec* assign_variable(const char* symb_name, double value) {
	/* new style: pdebug("PARSER: assign_variable(): symb_name = %s value = %g sym_table = %p\n", var.name, var.value, sym_table);*/
	pdebug("PARSER: assign_variable() : symb_name = %s value=%g\n", symb_name, value);

	/* be sure that the symbol table has been initialized */
	if (!sym_table)
		init_table();

	symrec* ptr = getsym(symb_name);
	if (!ptr) {
		pdebug("PARSER: calling putsym(): symb_name = %s\n", symb_name);
		ptr = putsym(symb_name, VAR);
	}
	ptr->value.var = value;

	return ptr;
};

static char getcharstr(param *p) {
	pdebug("PARSER: getcharstr() pos = %d\n", (int)(p->pos));

	if (p->string[p->pos] == '\0')
		return EOF;
	pdebug("next char is %c\n", p->string[p->pos]);
	return p->string[(p->pos)++];
}

static void ungetcstr(size_t *pos) {
    if (*pos > 0)
        (*pos)--;
}

double parse(const char *str) {
	pdebug("\nPARSER: parse(\"%s\") len=%d\n", str, (int)strlen(str));

	/* be sure that the symbol table has been initialized */
	if (!sym_table)
		init_table();

	param p;
	p.pos = 0;
	/* leave space to terminate string by "\n\0" */
	size_t slen = strlen(str) + 2;
	p.string = (char *) malloc(slen * sizeof(char));
	if (p.string == NULL) {
		printf("ERROR: out of memory for parsing string\n");
		return 0.;
	}

	strncpy(p.string, str, slen);	// fills with '\0'
	p.string[strlen(str)] = '\n';
	pdebug("\nPARSER: yyparse(\"%s\") len=%d\n", p.string, (int)strlen(p.string));

	/* parameter for yylex */
	yyparse(&p);

	pdebug("PARSER: parse() DONE (res = %g, parse errors = %d)\n", res, parse_errors());
	free(p.string);
	p.string = 0;

	return res;
}

double parse_with_vars(const char *str, const parser_var *vars, int nvars) {
	pdebug("\nPARSER: parse_with_var(\"%s\") len=%d\n", str, (int)strlen(str));

	int i;
	for(i = 0; i < nvars; i++) {	/*assign vars */
		pdebug("assign %s the value %g\n", vars[i].name, vars[i].value);
		assign_variable(vars[i].name, vars[i].value);
	}

	return parse(str);
}

int yylex(param *p) {
	pdebug("PARSER: yylex()\n");
	char c;

	/* skip white space  */
	while ((c = getcharstr(p)) == ' ' || c == '\t');

	/* finish if reached EOF */
	if (c == EOF) {
		pdebug("FINISHED\n");
		return 0;
	}
	/* check for non-ASCII chars */
	if (!isascii(c)) {
		pdebug("non-ASCII character found. Giving up\n");
		yynerrs++;
		return 0;
	}

	pdebug("PARSER: reading character: %c\n", c);

	/* process numbers */
	if (isdigit(c)) {
		pdebug("PARSER: reading number (starts with digit)\n");
                ungetcstr(&(p->pos));
                char *s = &(p->string[p->pos]);

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
		pdebug("PARSER: reading identifier (starts with alpha: %c)\n", c);
		static char *symbuf = 0;
		static int length = 0;
		int i = 0;

		/* Initially make the buffer long enough for a 10-character symbol name */
		if (length == 0)
			length = 10, symbuf = (char *) malloc(length + 1);

		do {
			pdebug("reading symbol .. ");
			/* If buffer is full, make it bigger */
			if (i == length) {
				length *= 2;
				symbuf = (char *) realloc(symbuf, length + 1);
			}
			symbuf[i++] = c;
			c = getcharstr(p);
			pdebug("got %c\n", c);
		}
		while (c != EOF && (isalnum(c) || c == '_' || c == '.'));
		pdebug("reading symbol done\n");

		if (c != EOF)
			ungetcstr(&(p->pos));
		symbuf[i] = '\0';

		symrec *s = getsym(symbuf);
		if(s == 0) {	/* symbol unknown */
			pdebug("PARSER: ERROR: symbol \"%s\" UNKNOWN\n", symbuf);
			yynerrs++;
			return 0;
		}
		/* old behavior */
		/* if (s == 0)
			 s = putsym (symbuf, VAR);
		*/
		yylval.tptr = s;
		return s->type;
	}

	/* else: single operator */
	pdebug("PARSER: single operator\n");
	return c;
}
