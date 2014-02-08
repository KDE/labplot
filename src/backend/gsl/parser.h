#ifndef PARSER_H
#define PARSER_H

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
