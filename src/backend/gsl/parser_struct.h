#ifndef PARSER_STRUCT_H
#define PARSER_STRUCT_H

struct con {
	char const *name;
	double value;
};

struct func {
	char const *name;
#ifdef HAVE_SOLARIS
	double (*fnct)(double);
#else
	double (*fnct)();
#endif
};

#endif /* PARSER_STRUCT_H */
