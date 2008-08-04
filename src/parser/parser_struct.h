/*LabPlot : parser_struct.h*/

#ifndef PARSER_STRUCT_H
#define PARSER_STRUCT_H

struct con {
	char const *name;
	double value;
};

struct init {
	char const *fname;
#ifdef HAVE_SOLARIS 
	double (*fnct)(double);
#else
	double (*fnct)();
#endif
};

#endif /* PARSER_STRUCT_H */
