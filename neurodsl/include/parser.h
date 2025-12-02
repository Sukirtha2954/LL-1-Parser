#ifndef PARSER_H
#define PARSER_H

#include "ast.h"

int parse_program(ProgramAST *prog); // returns 0 on success, nonzero on error

#endif
