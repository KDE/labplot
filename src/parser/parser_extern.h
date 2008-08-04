/*LabPlot : parser_extern.h*/

#ifndef PARSER_EXTERN_H
#define PARSER_EXTERN_H

extern "C" double parse(char[]);
extern "C" double old_parse(char[]);
extern "C" int parse_errors();
extern "C" void init_table();
extern "C" void delete_table();
extern "C" void* assign_variable(char* variable, double value);

#endif /* PARSER_EXTERN_H */
