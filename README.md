# **NeuroDSL — A Domain Specific Language for Defining Neural Networks**
A compiler written in C that parses a custom deep learning language, builds an AST, and generates Python/TensorFlow code.

---

## **Overview**

**NeuroDSL** is a small compiler that allows users to define neural networks using a simple, human-readable domain-specific language (DSL).

The compiler performs the following steps:

- Tokenizes the DSL input  
- Parses it using a hand-written recursive descent parser  
- Builds a typed Abstract Syntax Tree (AST)  
- Generates Python code that constructs and trains a neural network using TensorFlow/Keras  
- Supports real datasets (currently MNIST)  
- Can be easily extended with additional layers, datasets, and language features  

This project demonstrates:

- Compiler design fundamentals  
- LL(1)-style recursive descent parsing  
- AST construction using C structures  
- Code generation targeting Python/TensorFlow  
- DSL design for machine learning workflows  

---

## **Example DSL Program**
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
---

## **Features Supported**

### **Layers**

- `input (channels, height, width)`
- `conv2d filters=…, kernel=…, activation=…`
- `maxpool2d size=…`
- `flatten`
- `dense units=…, activation=…`
- `output units=…, activation=…`

### **Training Options**

- Optimizer (e.g., `adam`, `sgd`)
- Loss function (e.g., `categorical_crossentropy`)
- Number of epochs  
- Dataset selection  
  - `mnist` (built-in)  
  - Easily extendable for more datasets  

---

## **Build Instructions**

### **1. Clone the Repository**

git clone https://github.com/<your-username>/neurodsl.git
cd neurodsl

## **2. Compile the Compiler (GCC)**

gcc -Iinclude src/main.c src/lexer.c src/parser.c src/codegen.c -o neurodsl
3. Run the Compiler

./neurodsl examples/example.nn
This generates the Python model at:
generated/model.py
4. Execute the Generated Model

python generated/model.py
Output Example
vbnet

Model: "sequential"
---------------------------------------------------------------------
conv2d ...
max_pooling2d ...
flatten ...
dense ...
dense ...
---------------------------------------------------------------------
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
│   ├── ast.h
│   ├── lexer.h
│   ├── parser.h
│   └── codegen.h
│── src/
    ├── main.c
    ├── lexer.c
    ├── parser.c
    ├── codegen.c
    └── ast.c
File: examples/example.nn

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

