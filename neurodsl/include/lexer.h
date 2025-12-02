#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>

typedef enum {
    TOK_NETWORK,
    TOK_LBRACE,     // {
    TOK_RBRACE,     // }
    TOK_INPUT,
    TOK_CONV2D,
    TOK_MAXPOOL2D,
    TOK_FLATTEN,
    TOK_DENSE,
    TOK_OUTPUT,
    TOK_TRAIN,
    TOK_IDENTIFIER,
    TOK_NUMBER,
    TOK_EQUALS,
    TOK_COMMA,
    TOK_LPAREN,     // (
    TOK_RPAREN,     // )
    TOK_COLON,      // :
    TOK_EOF,
    TOK_UNKNOWN
} TokenType;

typedef struct {
    TokenType type;
    char text[128];
} Token;

void lexer_init_file(const char *path);
Token lexer_peek();      // lookahead (one token)
Token lexer_next();      // consume and return next token
void lexer_free();

#endif
