/*LabPlot : parser.h*/

#ifndef PARSER_H
#define PARSER_H

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef HAVE_SOLARIS
#include <strings.h>
#endif

#ifdef HAVE_GSL
#include <gsl/gsl_math.h>
#include <gsl/gsl_sf.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_const_num.h>
#ifdef HAVE_GSL14
#include <gsl/gsl_const_mksa.h>
#include <gsl/gsl_const_cgsm.h>
#else
#include <gsl/gsl_const_mks.h>
#include <gsl/gsl_const_cgs.h>
#endif
#endif

/*#include "../cephes/cephes.h"*/
#include "constants.h"
#include "functions.h"
#include "parser_struct.h"

/* Functions type.                                   */
#ifdef HAVE_SOLARIS 
typedef double (*func_t) (double);
#else
typedef double (*func_t) ();
#endif

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

double parse(char *str);
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
