#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"

static FILE *infile = NULL;
static Token lookahead;
static int lookahead_valid = 0;

static void token_set(Token *t, TokenType tp, const char *txt) {
    t->type = tp;
    if (txt) strncpy(t->text, txt, sizeof(t->text)-1);
    else t->text[0] = '\0';
}

void lexer_init_file(const char *path) {
    if (infile) fclose(infile);
    infile = fopen(path, "r");
    if (!infile) {
        perror("fopen");
        exit(1);
    }
    lookahead_valid = 0;
}

void lexer_free() {
    if (infile) { fclose(infile); infile = NULL; }
    lookahead_valid = 0;
}

// helper to read next raw token string (skips whitespace, comments)
static int read_char() { return fgetc(infile); }
static void unread_char(int c) { if (c != EOF) ungetc(c, infile); }

static void skip_ws_and_comments() {
    int c;
    while ((c = read_char()) != EOF) {
        if (isspace(c)) continue;
        if (c == '#') { // comment to line end
            while ((c = read_char()) != EOF && c != '\n');
            continue;
        }
        unread_char(c);
        break;
    }
}

static Token tokenize_next() {
    Token tok;
    token_set(&tok, TOK_EOF, "");
    if (!infile) return tok;
    skip_ws_and_comments();
    int c = read_char();
    if (c == EOF) { token_set(&tok, TOK_EOF, "EOF"); return tok; }

    // single char tokens
    if (c == '{') { token_set(&tok, TOK_LBRACE, "{"); return tok; }
    if (c == '}') { token_set(&tok, TOK_RBRACE, "}"); return tok; }
    if (c == '(') { token_set(&tok, TOK_LPAREN, "("); return tok; }
    if (c == ')') { token_set(&tok, TOK_RPAREN, ")"); return tok; }
    if (c == ',') { token_set(&tok, TOK_COMMA, ","); return tok; }
    if (c == '=') { token_set(&tok, TOK_EQUALS, "="); return tok; }
    if (c == ':') { token_set(&tok, TOK_COLON, ":"); return tok; }

    // identifier or keyword
    if (isalpha(c)) {
        char buf[128]; int i = 0;
        buf[i++] = (char)c;
        while ((c = read_char()) != EOF && (isalnum(c) || c == '_' )) {
            if (i < (int)sizeof(buf)-1) buf[i++] = (char)c;
        }
        unread_char(c);
        buf[i] = '\0';
        // keywords
        if (strcmp(buf, "network") == 0) token_set(&tok, TOK_NETWORK, buf);
        else if (strcmp(buf, "input") == 0) token_set(&tok, TOK_INPUT, buf);
        else if (strcmp(buf, "conv2d") == 0) token_set(&tok, TOK_CONV2D, buf);
        else if (strcmp(buf, "maxpool2d") == 0) token_set(&tok, TOK_MAXPOOL2D, buf);
        else if (strcmp(buf, "flatten") == 0) token_set(&tok, TOK_FLATTEN, buf);
        else if (strcmp(buf, "dense") == 0) token_set(&tok, TOK_DENSE, buf);
        else if (strcmp(buf, "output") == 0) token_set(&tok, TOK_OUTPUT, buf);
        else if (strcmp(buf, "train") == 0) token_set(&tok, TOK_TRAIN, buf);
        else { token_set(&tok, TOK_IDENTIFIER, buf); }
        return tok;
    }

    // number
    if (isdigit(c)) {
        char buf[64]; int i = 0;
        buf[i++] = (char)c;
        while ((c = read_char()) != EOF && isdigit(c)) {
            if (i < (int)sizeof(buf)-1) buf[i++] = (char)c;
        }
        unread_char(c);
        buf[i] = '\0';
        token_set(&tok, TOK_NUMBER, buf);
        return tok;
    }

    // fallback
    char s[2] = {(char)c,'\0'};
    token_set(&tok, TOK_UNKNOWN, s);
    return tok;
}

Token lexer_peek() {
    if (!lookahead_valid) {
        lookahead = tokenize_next();
        lookahead_valid = 1;
    }
    return lookahead;
}

Token lexer_next() {
    if (lookahead_valid) {
        lookahead_valid = 0;
        return lookahead;
    }
    return tokenize_next();
}
