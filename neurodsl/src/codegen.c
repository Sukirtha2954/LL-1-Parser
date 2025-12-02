#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/ast.h"

static const char *out_path = "generated/model.py";

void generate_python(ModelAST *m, TrainAST *t) {
    FILE *f = fopen(out_path, "w");
    if (!f) { perror("fopen"); return; }

    fprintf(f, "import os\n");
    fprintf(f, "os.environ['TF_CPP_MIN_LOG_LEVEL']='2'\n");
    fprintf(f, "import numpy as np\n");
    fprintf(f, "import tensorflow as tf\n");
    fprintf(f, "from tensorflow.keras import layers, models\n\n");

    fprintf(f, "def build_model():\n");
    fprintf(f, "    model = models.Sequential()\n");

    // find input shape if present first layer is input
    Layer *p = m->layers;
    if (p && p->type == LAYER_INPUT) {
        fprintf(f, "    model.add(layers.Input(shape=(%d, %d, %d)))\n", p->i_h, p->i_w, p->i_ch);
        p = p->next;
    }

    for (; p; p = p->next) {
        switch (p->type) {
            case LAYER_CONV2D:
                {
                    int filt = p->filters? p->filters : 32;
                    int k = p->kernel? p->kernel : 3;
                    const char *act = p->activation[0]? p->activation : "relu";
                    fprintf(f, "    model.add(layers.Conv2D(%d, (%d, %d), activation='%s', padding='same'))\n", filt, k, k, act);
                }
                break;
            case LAYER_MAXPOOL2D:
                {
                    int s = p->pool_size? p->pool_size : 2;
                    fprintf(f, "    model.add(layers.MaxPooling2D(pool_size=(%d,%d)))\n", s, s);
                } break;
            case LAYER_FLATTEN:
                fprintf(f, "    model.add(layers.Flatten())\n"); break;
            case LAYER_DENSE:
                {
                    int u = p->units? p->units : 64;
                    const char *act = p->activation[0]? p->activation : "relu";
                    fprintf(f, "    model.add(layers.Dense(%d, activation='%s'))\n", u, act);
                } break;
            case LAYER_OUTPUT:
                {
                    int u = p->units? p->units : 10;
                    const char *act = p->activation[0]? p->activation : "softmax";
                    fprintf(f, "    model.add(layers.Dense(%d, activation='%s'))\n", u, act);
                } break;
            default: break;
        }
    }

    fprintf(f, "    return model\n\n");

    // compile and training block
    fprintf(f, "if __name__ == '__main__':\n");
    fprintf(f, "    model = build_model()\n");
    fprintf(f, "    model.summary()\n");
    const char *opt = t->optimizer[0]? t->optimizer : "adam";
    const char *loss = t->loss[0]? t->loss : "categorical_crossentropy";
    int epochs = t->epochs? t->epochs : 1;
    fprintf(f, "    model.compile(optimizer='%s', loss='%s', metrics=['accuracy'])\n", opt, loss);

    // If dataset == "mnist", emit MNIST loader
    if (t->dataset[0] && strcasecmp(t->dataset, "mnist") == 0) {
        fprintf(f, "    from tensorflow.keras.datasets import mnist\n");
        fprintf(f, "    (x_train, y_train), (x_test, y_test) = mnist.load_data()\n");
        // detect input shape
        Layer *inp = m->layers;
        int h=28,w=28,ch=1;
        if (inp && inp->type==LAYER_INPUT) { ch = inp->i_ch; h = inp->i_h; w = inp->i_w; }
        // reshape/pad single-channel if needed
        if (ch == 1) {
            fprintf(f, "    x_train = x_train.reshape(-1, %d, %d, 1).astype('float32') / 255.0\n", h, w);
            fprintf(f, "    x_test  = x_test.reshape(-1, %d, %d, 1).astype('float32') / 255.0\n", h, w);
        } else {
            fprintf(f, "    x_train = x_train.reshape(-1, %d, %d, %d).astype('float32') / 255.0\n", h, w, ch);
            fprintf(f, "    x_test  = x_test.reshape(-1, %d, %d, %d).astype('float32') / 255.0\n", h, w, ch);
        }
        fprintf(f, "    y_train = tf.keras.utils.to_categorical(y_train, 10)\n");
        fprintf(f, "    y_test  = tf.keras.utils.to_categorical(y_test, 10)\n");
        fprintf(f, "    model.fit(x_train, y_train, epochs=%d, batch_size=64, validation_split=0.1)\n", epochs);
        fprintf(f, "    loss, acc = model.evaluate(x_test, y_test)\n");
        fprintf(f, "    print('Test loss:', loss, 'Test accuracy:', acc)\n");
    } else {
        // fallback: random data (existing behavior)
        Layer *inp = m->layers;
        int h=28,w=28,ch=1;
        if (inp && inp->type==LAYER_INPUT) { ch = inp->i_ch; h = inp->i_h; w = inp->i_w; }
        fprintf(f, "    x = np.random.rand(100, %d, %d, %d).astype(np.float32)\n", h, w, ch);
        fprintf(f, "    y = tf.keras.utils.to_categorical(np.random.randint(0,10,size=(100,)), num_classes=10)\n");
        fprintf(f, "    model.fit(x,y, epochs=%d, batch_size=16)\n", epochs);
    }

    fclose(f);
    printf("Generated Python model at %s\n", out_path);
}
