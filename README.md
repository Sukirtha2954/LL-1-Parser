# LL-1-Parser

# LL(1) Parser in C

This project implements a complete LL(1) parser in C that includes:

- FIRST and FOLLOW set computation  
- LL(1) parsing table construction  
- Stack-based predictive parsing  
- A tokenizer that converts a sample input into tokenized form  

---

##  Grammar Used

This LL(1) parser uses the following grammar:

E → T X
X → + E | ε
T → int Y | ( E )
Y → * T | ε

### Terminals
- `int` (integer literals)
- `+` (addition)
- `*` (multiplication)
- `(` and `)`
- `$` (end of input)

### Non-Terminals
- `E`, `X`, `T`, `Y`

---

## 🛠 How It Works

### 1. Tokenization
The input string (e.g., `int + int * int`) is converted into a sequence of tokens:
Input: int + int * int
Tokens: INT_TOK + INT_TOK * INT_TOK $

Each token corresponds to a terminal in the grammar.

### 2. FIRST Set Computation
- For each terminal: `FIRST(terminal) = { terminal }`
- For each non-terminal:
  - Examine each production.
  - For a sequence A → X Y ..., compute `FIRST(X)`:
    - If `FIRST(X)` contains ε, continue to `Y`, and so on.
    - Add all terminals found (excluding ε unless needed).

### 3. FOLLOW Set Computation
- FOLLOW(Start Symbol) includes `$`
- For each non-terminal A in production B → α A β:
  - Add FIRST(β) to FOLLOW(A)
  - If β can derive ε, also add FOLLOW(B) to FOLLOW(A)

### 4. LL(1) Parsing Table Construction
- For each production A → α:
  - For each terminal `t` in FIRST(α), add `A → α` to table[A][t]
  - If ε ∈ FIRST(α), then for each terminal `t` in FOLLOW(A), add `A → ε` to table[A][t]

### 5. Stack-Based Predictive Parsing
- Stack is initialized with `$` and the start symbol (`E`)
- The input token stream ends with `$`
- On each iteration:
  - Match the top of the stack with the current input token
  - If top is non-terminal: use the parsing table to get production and push RHS in reverse
  - If top is terminal: match and pop
  - If mismatch: reject
- Accept if both input and stack reach `$`

---
## Features
- Supports ε-productions
- Uses arrays for the parsing table
- Prints FIRST and FOLLOW sets
- Prints the parsing table in human-readable form
- Shows detailed parsing trace

---

##  How to Run

1. Clone the repository
2. Compile using GCC:
```bash
gcc parser.c -o parser
