/***************************************************************************
    File                 : parser.h
    Project              : LabPlot
    Description          : Parser for mathematical expressions
    --------------------------------------------------------------------
    Copyright            : (C) 2014-2020 Stefan Gerlach  (stefan.gerlach@uni.kn)
    Copyright            : (C) 2014 Alexander Semke (alexander.semke@web.de)

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

#ifndef PARSER_H
#define PARSER_H

/* uncomment to enable parser specific debugging */
#define PDEBUG 1

struct cons {
	const char* name;
	double value;
};

struct funs {
	const char* name;
	/* MSVC needs void argument */
	double (*fnct)(void);
};

/* variables to pass to parser */
#define MAX_VARNAME_LENGTH 10
typedef struct parser_var {
	char name[MAX_VARNAME_LENGTH];
	double value;
} parser_var;

/* Function types */
/* MSVC needs void argument */
typedef double (*func_t) (void);
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
double parse(const char *str);
double parse_with_vars(const char[], const parser_var[], int nvars);

extern struct cons _constants[];
extern struct funs _functions[];


#endif /*PARSER_H*/
