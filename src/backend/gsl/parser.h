/***************************************************************************
    File                 : parser.h
    Project              : LabPlot
    Description          : Parser for mathematical expressions
    --------------------------------------------------------------------
    Copyright            : (C) 2014-2017 Stefan Gerlach  (stefan.gerlach@uni.kn)
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
/* #define PDEBUG 1 */

struct con {
	const char* name;
	double value;
};

struct func {
	const char* name;
#ifdef _MSC_VER
	double (*fnct)(void);
#else
	double (*fnct)();
#endif
};

/* variables to pass to parser */
#define MAX_VARNAME_LENGTH 10
typedef struct parser_var {
	char name[MAX_VARNAME_LENGTH];
	double value;
} parser_var;

/* Functions type */
#ifdef _MSC_VER
typedef double (*func_t) (void);
#else
typedef double (*func_t) ();
#endif
typedef double (*func_t1) (double);
typedef double (*func_t2) (double, double);
typedef double (*func_t3) (double, double, double);
typedef double (*func_t4) (double, double, double, double);

/* structure for list of symbols */
typedef struct symrec {
	char *name;	/* name of symbol */
	int type;	/* type of symbol: either VAR or FNCT */
	union {
		double var;	/* value of a VAR */
		func_t fnctptr;	/* value of a FNCT */
	} value;
	struct symrec *next;	/* next field */
} symrec;

void init_table(void);		/* initialize symbol table */
void delete_table(void);	/* delete symbol table */
int parse_errors(void);
symrec* assign_variable(const char* symb_name, double value);
/*new style: symrec* assign_variable(symrec *sym_table, parser_var var);*/
double parse(const char *str);
double parse_with_vars(const char[], const parser_var[], int nvars);

extern struct con _constants[];
extern struct func _functions[];


#endif /*PARSER_H*/
