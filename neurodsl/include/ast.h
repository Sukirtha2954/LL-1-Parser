#ifndef AST_H
#define AST_H

typedef enum {
    LAYER_INPUT,
    LAYER_CONV2D,
    LAYER_MAXPOOL2D,
    LAYER_FLATTEN,
    LAYER_DENSE,
    LAYER_OUTPUT
} LayerType;

typedef struct Layer {
    LayerType type;
    // parameters
    int i_ch, i_h, i_w;     // input shape (channels, height, width)
    int filters;
    int kernel;
    int pool_size;
    int units;
    char activation[32];
    struct Layer *next;
} Layer;

typedef struct {
    char name[64];
    Layer *layers;
} ModelAST;

typedef struct {
    char optimizer[32];
    char loss[64];
    int epochs;
    char dataset[64];     // NEW: dataset name, e.g., "mnist"
} TrainAST;

typedef struct {
    ModelAST *model;
    TrainAST train;
} ProgramAST;

// helpers
Layer *layer_new();
void layer_free_all(Layer *head);
void model_free(ModelAST *m);

#endif
