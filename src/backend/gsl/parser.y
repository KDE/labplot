/***************************************************************************
    File                 : parser.y
    Project              : LabPlot
    Description          : Parser for mathematical expressions
    --------------------------------------------------------------------
    Copyright            : (C) 2014 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2014-2016 Stefan Gerlach  (stefan.gerlach@uni.kn)

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
#include <strings.h>	/* bzero */
#include <ctype.h>
#include <locale.h>
#include "parser_struct.h"
#include "constants.h"
#include "functions.h"

/* Enable Parser DEBUGGING */
/*
#define PDEBUG
*/

#ifdef PDEBUG
#define pdebug(...) fprintf(stderr, __VA_ARGS__)
#else
#define pdebug(...) do {} while (0)
#endif

/* Functions type */
typedef double (*func_t) ();

/* Data type for links in the chain of symbols */
typedef struct symrec {
	char *name;	/* name of symbol */
	int type;	/* type of symbol: either VAR or FNCT */
	union {
		double var;	/* value of a VAR */
		int intvar;
		func_t fnctptr;	/* value of a FNCT */
	} value;
	struct symrec *next;	/* link field */
} symrec;

/* params passed to yylex (and yyerror) */
typedef struct param {
	unsigned int pos;
	char *string;
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

#define PARSE_STRING_SIZE 500	/* big enough? */
/* The symbol table: a chain of `struct symrec'.  */
symrec *sym_table = (symrec *) 0;

int parse_errors(void) {
	return yynerrs;
}

int yyerror(param *p, const char *s) {
	printf ("PARSER ERROR: %s @ position %d of string %s", s, p->pos, p->string);
	return 0;
}

symrec* putsym (const char *sym_name, int sym_type) {
	pdebug("PARSER: putsym(): sym_name = %s\n", sym_name);

	symrec *ptr = (symrec *) malloc(sizeof(symrec));
	ptr->name = (char *) malloc(strlen(sym_name) + 1);
	strcpy(ptr->name, sym_name);
	ptr->type = sym_type;
	ptr->value.var = 0;	/* set value to 0 even if fctn. */
	ptr->next = (struct symrec *)sym_table;
	sym_table = ptr;

	pdebug("PARSER: putsym() DONE\n");
	return ptr;
}

symrec* getsym (const char *sym_name) {
	pdebug("PARSER: getsym(): sym_name = %s\n", sym_name);

	symrec *ptr;
	for (ptr = sym_table; ptr != (symrec *) 0; ptr = (symrec *)ptr->next) {
		if (strcmp (ptr->name, sym_name) == 0) {
			pdebug("PARSER: symbol \'%s\' found\n", sym_name);
			return ptr;
		}
	}

	pdebug("PARSER: symbol \'%s\' not found\n", sym_name);
	return 0;
}

/* put arithmetic functions in table. */
void init_table (void) {
	pdebug("PARSER: init_table()\n");

	symrec *ptr;
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
	pdebug("PARSER: init_table() DONE\n");
}

void delete_table(void) {
	symrec *tmp;
	while(sym_table) {
		tmp = sym_table;
		sym_table = sym_table->next;
		free(tmp->name);
		free(tmp);
	}
}

symrec* assign_variable(const char* symb_name, double value) {
	pdebug("PARSER: assign_variable(): symb_name = %s value=%g\n", symb_name, value);

	symrec* ptr = getsym(symb_name);
	if (!ptr) {
		pdebug("PARSER: calling putsym(): symb_name = %s\n", symb_name);
		ptr = putsym(symb_name, VAR);
	}
	ptr->value.var = value;

	return ptr;
};

static int getcharstr(param *p) {
	pdebug("PARSER: getcharstr() pos = %d\n", p->pos);

	if (p->string[p->pos] == '\0')
		return EOF;
	return (int)p->string[(p->pos)++];
}

static void ungetcstr(unsigned int *pos) {
    if (*pos > 0)
        (*pos)--;
}

double parse(const char *str) {
	pdebug("\nPARSER: parse(\"%s\")\n", str);

	/* be sure that the symbol table has been initialized */
	if (!sym_table)
		init_table();

	/* create parameter for yylex */
	param p;
	p.pos = 0;
	p.string = (char *)malloc(PARSE_STRING_SIZE*sizeof(char));
	bzero(p.string, PARSE_STRING_SIZE);

	/* leave space to terminate string by "\n\0" */
	strncpy(p.string, str, PARSE_STRING_SIZE - 2);
	p.string[strlen(p.string)] = '\n';

	yyparse(&p);

	pdebug("PARSER: parse() DONE\n");
	free(p.string);
	return res;
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

		symrec *s = getsym(symbuf);
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
