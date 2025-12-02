#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "../include/parser.h"
#include "../include/ast.h"

// forward of codegen (declare)
void generate_python(ModelAST *m, TrainAST *t);

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s <dsl-file>\nExample: %s examples/example.nn\n", argv[0], argv[0]);
        return 1;
    }
    const char *path = argv[1];
    lexer_init_file(path);

    ProgramAST prog;
    if (parse_program(&prog) != 0) {
        fprintf(stderr, "Parsing failed.\n");
        lexer_free();
        return 1;
    }

    printf("Parsing succeeded. Model name: %s\n", prog.model->name);
    // generate code
    generate_python(prog.model, &prog.train);

    // free ast
    model_free(prog.model);
    lexer_free();

    printf("Done. Run: python generated/model.py (needs tensorflow installed).\n");
    return 0;
}
