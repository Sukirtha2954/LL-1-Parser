NeuroDSL — A Domain Specific Language for Defining Neural Networks
A compiler written in C that parses a custom deep learning language, builds an AST, and generates Python/TensorFlow code.
<p align="center"> <img src="https://img.shields.io/badge/language-C-blue" /> <img src="https://img.shields.io/badge/python-3.9+-green" /> <img src="https://img.shields.io/badge/tensorflow-2.x-orange" /> <img src="https://img.shields.io/badge/project-type-compiler%20%2F%20DSL-lightgrey" /> </p>
 Overview

NeuroDSL is a small compiler that lets you define neural networks using a simple, human-readable domain-specific language (DSL).

The compiler:

✔ Tokenizes the DSL input
✔ Parses it using recursive descent
✔ Builds a typed Abstract Syntax Tree (AST)
✔ Generates Python code that constructs + trains the model using TensorFlow/Keras
✔ Supports real datasets (MNIST)
✔ Can be extended easily

This project demonstrates:

Compiler design fundamentals

LL(1)-style recursive descent parsing

AST construction

Code generation

DSL design for machine learning

 Example DSL Program

examples/example.nn:

network SimpleCNN {
    input (1, 28, 28)
    conv2d filters=32, kernel=3, activation=relu
    maxpool2d size=2
    flatten
    dense units=64, activation=relu
    output units=10, activation=softmax
}

train {
    optimizer: adam
    loss: categorical_crossentropy
    epochs: 2
    dataset: mnist
}

 Features Supported
 Layers

input (channels, height, width)

conv2d filters=…, kernel=…, activation=…

maxpool2d size=…

flatten

dense units=…, activation=…

output units=…, activation=…

 Training Options

Optimizer (adam, sgd, etc.)

Loss function (categorical_crossentropy)

Epochs

Dataset:

mnist

(More datasets can be added easily)

 Build Instructions
1. Clone
git clone https://github.com/<your-username>/neurodsl.git
cd neurodsl

2. Compile (GCC)
gcc -Iinclude src/main.c src/lexer.c src/parser.c src/codegen.c -o neurodsl

3. Run
./neurodsl examples/example.nn


This generates:

generated/model.py

4. Execute the generated TensorFlow model
python generated/model.py

 Output Example
Model: "sequential"
_________________________________________________________________
conv2d ...
max_pooling2d ...
flatten ...
dense ...
dense ...
_________________________________________________________________
Epoch 1/2
accuracy: 0.8851 - val_accuracy: 0.9802
Epoch 2/2
accuracy: 0.9776 - val_accuracy: 0.9843
Test accuracy: 0.9829

 Project Structure
neurodsl/
│── README.md
│── LICENSE
│── examples/
│   └── example.nn
│── generated/
│── include/
│── src/
