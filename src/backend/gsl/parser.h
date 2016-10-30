/***************************************************************************
    File                 : parser.h
    Project              : LabPlot
    Description          : some definitions for the linker
    --------------------------------------------------------------------
    Copyright            : (C) 2008-2016 by Stefan Gerlach (stefan.gerlach@uni.kn)
    Copyright            : (C) 2014 by Alexander Semke (alexander.semke@web.de)

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

struct con {
	char const *name;
	double value;
};

struct func {
	char const *name;
	double (*fnct)();
};

#define MAX_VARNAME_LENGTH 10

/* variables to pass to parser */
typedef struct parser_var {
	char name[MAX_VARNAME_LENGTH];	/* name of variable */
	double value;			/* value */
} parser_var;

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
	struct symrec *next;	/* next field */
} symrec;

int init_parser();
double parse(const char[]);
double parse_with_vars(const char[], const parser_var[], int nvars);
int parse_errors();
int finish_parser();
symrec* assign_variable(const char* symb_name, double value);
/*symrec* assign_variable(symrec *sym_table, parser_var var);*/

extern struct con _constants[];
extern struct func _functions[];

#endif /* PARSER_H */
