/***************************************************************************
    File                 : parser.h
    Project              : LabPlot
    Description          : Parser for mathematical expressions
    --------------------------------------------------------------------
    Copyright            : (C) 2014 Stefan Gerlach  (stefan.gerlach@uni-konstanz.de)
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

#include "constants.h"
#include "functions.h"
#include "parser_struct.h"

#include <string.h>
#include <strings.h>
#include <ctype.h>

/* Functions type.                                   */
/*#ifdef HAVE_SOLARIS
typedef double (*func_t) (double);
#else*/
typedef double (*func_t) ();
/*#endif*/

/* Data type for links in the chain of symbols.      */
struct symrec {
	char *name;  /* name of symbol                     */
	int type;    /* type of symbol: either VAR or FNCT */
	union {
		double var;                  /* value of a VAR   */
		int intvar;
		func_t fnctptr;              /* value of a FNCT  */
	} value;
	struct symrec *next;    /* link field              */
};

typedef struct symrec symrec;

double parse(const char *str);
int parse_errors();
symrec *putsym (const char *, int);
symrec *getsym (const char *);
void init_table(void);
int yyerror (const char*);
int yylex (void);

#define PARSE_STRING_SIZE       500
double res;
int pos;
char string[PARSE_STRING_SIZE];


#endif /*PARSER_H*/
