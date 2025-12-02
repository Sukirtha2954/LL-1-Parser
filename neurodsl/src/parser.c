#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "../include/ast.h"
#include "../include/parser.h"

// helper prototypes
static int expect(TokenType t, Token *out);
static int accept(TokenType t, Token *out);

// AST helpers
Layer *layer_new() {
    Layer *l = (Layer*)calloc(1, sizeof(Layer));
    l->next = NULL;
    l->i_ch = l->i_h = l->i_w = l->filters = l->kernel = l->pool_size = l->units = 0;
    l->activation[0] = '\0';
    return l;
}
void layer_free_all(Layer *head) {
    Layer *p = head;
    while (p) { Layer *n = p->next; free(p); p = n; }
}
void model_free(ModelAST *m) {
    if (!m) return;
    layer_free_all(m->layers);
    free(m);
}

// accept/expect implementations
static int accept(TokenType t, Token *out) {
    Token la = lexer_peek();
    if (la.type == t) {
        Token got = lexer_next();
        if (out) *out = got;
        return 1;
    }
    return 0;
}
static int expect(TokenType t, Token *out) {
    if (accept(t,out)) return 1;
    Token la = lexer_peek();
    fprintf(stderr,"Parse error: expected token type %d but got '%s'\n", t, la.text);
    return 0;
}

// Parsing functions
int parse_program(ProgramAST *prog) {
    Token t;
    // network <id> { layers... }
    if (!accept(TOK_NETWORK, &t)) {
        fprintf(stderr, "Error: expected 'network' at start\n"); return 1;
    }
    Token idtok;
    if (!accept(TOK_IDENTIFIER, &idtok)) { fprintf(stderr,"Error: expected network name\n"); return 2; }
    ModelAST *model = (ModelAST*)calloc(1,sizeof(ModelAST));
    strncpy(model->name, idtok.text, sizeof(model->name)-1);
    model->layers = NULL;
    if (!accept(TOK_LBRACE, &t)) { fprintf(stderr,"Error: expected '{' after model name\n"); free(model); return 3; }

    Layer *last = NULL;
    // parse layers until RBRACE
    while (1) {
        Token la = lexer_peek();
        if (la.type == TOK_RBRACE) { lexer_next(); break; }
        if (la.type == TOK_EOF) { fprintf(stderr,"Unexpected EOF in model\n"); free(model); return 4; }

        // input
        if (la.type == TOK_INPUT) {
            lexer_next(); // consume input
            if (!accept(TOK_LPAREN, &t)) { fprintf(stderr,"Error: expected '(' after input\n"); free(model); return 5; }
            Token n1,n2,n3;
            if (!accept(TOK_NUMBER, &n1)) { fprintf(stderr,"Error: input needs numbers\n"); free(model); return 6; }
            accept(TOK_COMMA, NULL);
            if (!accept(TOK_NUMBER, &n2)) { fprintf(stderr,"Error: input needs 3 numbers\n"); free(model); return 7; }
            accept(TOK_COMMA, NULL);
            if (!accept(TOK_NUMBER, &n3)) { fprintf(stderr,"Error: input needs 3 numbers\n"); free(model); return 8; }
            accept(TOK_RPAREN, NULL);
            Layer *L = layer_new();
            L->type = LAYER_INPUT;
            L->i_ch = atoi(n1.text);
            L->i_h  = atoi(n2.text);
            L->i_w  = atoi(n3.text);
            if (!last) model->layers = L; else last->next = L;
            last = L;
            continue;
        }

        // conv2d
        if (la.type == TOK_CONV2D) {
            lexer_next();
            Layer *L = layer_new();
            L->type = LAYER_CONV2D;
            while (1) {
                Token nxt = lexer_peek();
                if (nxt.type == TOK_IDENTIFIER) {
                    Token id = lexer_next();
                    if (accept(TOK_EQUALS, NULL)) {
                        Token val = lexer_next();
                        if (val.type == TOK_NUMBER) {
                            if (strcmp(id.text,"filters")==0) L->filters = atoi(val.text);
                            else if (strcmp(id.text,"kernel")==0) L->kernel = atoi(val.text);
                            else if (strcmp(id.text,"size")==0) L->pool_size = atoi(val.text);
                            else if (strcmp(id.text,"units")==0) L->units = atoi(val.text);
                        } else if (val.type == TOK_IDENTIFIER) {
                            if (strcmp(id.text,"activation")==0) strncpy(L->activation, val.text, sizeof(L->activation)-1);
                        }
                        accept(TOK_COMMA, NULL);
                        continue;
                    } else {
                        continue;
                    }
                }
                break;
            }
            if (!last) model->layers = L; else last->next = L; last = L; continue;
        }

        // maxpool2d
        if (la.type == TOK_MAXPOOL2D) {
            lexer_next();
            Layer *L = layer_new(); L->type = LAYER_MAXPOOL2D;
            if (accept(TOK_IDENTIFIER, NULL) && accept(TOK_EQUALS, NULL)) {
                Token v = lexer_next();
                if (v.type == TOK_NUMBER) L->pool_size = atoi(v.text);
            } else {
                L->pool_size = 2;
            }
            if (!last) model->layers = L; else last->next = L; last = L; continue;
        }

        // flatten
        if (la.type == TOK_FLATTEN) {
            lexer_next();
            Layer *L = layer_new(); L->type = LAYER_FLATTEN;
            if (!last) model->layers = L; else last->next = L; last = L; continue;
        }

        // dense
        if (la.type == TOK_DENSE) {
            lexer_next();
            Layer *L = layer_new(); L->type = LAYER_DENSE;
            while (1) {
                Token nxt = lexer_peek();
                if (nxt.type == TOK_IDENTIFIER) {
                    Token id = lexer_next();
                    if (accept(TOK_EQUALS,NULL)) {
                        Token val = lexer_next();
                        if (val.type == TOK_NUMBER && strcmp(id.text,"units")==0) L->units = atoi(val.text);
                        else if (val.type == TOK_IDENTIFIER && strcmp(id.text,"activation")==0) strncpy(L->activation, val.text, sizeof(L->activation)-1);
                        accept(TOK_COMMA,NULL);
                        continue;
                    } else {
                        continue;
                    }
                }
                break;
            }
            if (!last) model->layers = L; else last->next = L; last = L; continue;
        }

        // output
        if (la.type == TOK_OUTPUT) {
            lexer_next();
            Layer *L = layer_new(); L->type = LAYER_OUTPUT;
            while (1) {
                Token nxt = lexer_peek();
                if (nxt.type == TOK_IDENTIFIER) {
                    Token id = lexer_next();
                    if (accept(TOK_EQUALS,NULL)) {
                        Token val = lexer_next();
                        if (val.type == TOK_NUMBER && strcmp(id.text,"units")==0) L->units = atoi(val.text);
                        else if (val.type == TOK_IDENTIFIER && strcmp(id.text,"activation")==0) strncpy(L->activation, val.text, sizeof(L->activation)-1);
                        accept(TOK_COMMA,NULL);
                        continue;
                    }
                }
                break;
            }
            if (!last) model->layers = L; else last->next = L; last = L; continue;
        }

        // unknown - warn and skip
        fprintf(stderr, "Warning: unexpected token '%s' in model\n", la.text);
        lexer_next();
    } // end layers parse

    // parse optional train block
    Token p = lexer_peek();
    TrainAST train;
    train.optimizer[0]=0; train.loss[0]=0; train.epochs = 1; train.dataset[0]=0;
    if (p.type == TOK_TRAIN) {
        lexer_next();
        if (!accept(TOK_LBRACE,NULL)) { fprintf(stderr,"Expected '{' after train\n"); }
        // parse until }
        while (1) {
            Token nx = lexer_peek();
            if (nx.type == TOK_RBRACE) { lexer_next(); break; }
            if (nx.type == TOK_IDENTIFIER) {
                Token id = lexer_next();
                // accept either colon or equals after identifier
                if (accept(TOK_COLON,NULL) || accept(TOK_EQUALS,NULL)) {
                    Token v = lexer_next();
                    if (strcmp(id.text,"optimizer")==0 && v.type==TOK_IDENTIFIER) strncpy(train.optimizer, v.text, sizeof(train.optimizer)-1);
                    else if (strcmp(id.text,"loss")==0 && v.type==TOK_IDENTIFIER) strncpy(train.loss, v.text, sizeof(train.loss)-1);
                    else if (strcmp(id.text,"epochs")==0 && v.type==TOK_NUMBER) train.epochs = atoi(v.text);
                    else if (strcmp(id.text,"dataset")==0 && v.type==TOK_IDENTIFIER) strncpy(train.dataset, v.text, sizeof(train.dataset)-1);
                    accept(TOK_COMMA,NULL);
                    continue;
                }
            } else {
                // skip token
                lexer_next();
            }
        }
    }

    prog->model = model;
    prog->train = train;
    return 0;
}
