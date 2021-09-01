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

/* uncomment to enable parser specific debugging */
/* #define PDEBUG 1 */

struct cons {
	const char* name;
	double value;
};

struct funs {
	const char* name;
#ifdef _MSC_VER	/* MSVC needs void argument */
	double (*fnct)(void);
#else
	double (*fnct)();
#endif
	int argc;
};

/* variables to pass to parser */
#define MAX_VARNAME_LENGTH 10
typedef struct parser_var {
	char name[MAX_VARNAME_LENGTH];
	double value;
} parser_var;

/* Function types */
#ifdef _MSC_VER	/* MSVC needs void argument */
typedef double (*func_t) (void);
#else
typedef double (*func_t) ();
#endif
typedef double (*func_t1) (double);
typedef double (*func_t2) (double, double);
typedef double (*func_t3) (double, double, double);
typedef double (*func_t4) (double, double, double, double);

/* structure for list of symbols */
typedef struct symbol {
	char *name;	/* name of symbol */
	int type;	/* type of symbol: either VAR or FNCT */
	union {
		double var;	/* value of a VAR */
		func_t fnctptr;	/* value of a FNCT */
	} value;
	struct symbol *next;	/* next symbol */
} symbol;

void init_table(void);		/* initialize symbol table */
void delete_table(void);	/* delete symbol table */
int parse_errors(void);
symbol* assign_symbol(const char* symbol_name, double value);
int remove_symbol(const char* symbol_name);
double parse(const char *string, const char *locale);
double parse_with_vars(const char[], const parser_var[], int nvars, const char* locale);

extern struct cons _constants[];
extern struct funs _functions[];


#endif /*PARSER_H*/
