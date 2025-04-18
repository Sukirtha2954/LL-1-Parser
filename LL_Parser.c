#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_FIRST_FOLLOW 10
#define MAX_STACK 100
#define NUM_NON_TERMINALS 4
#define NUM_TOKENS 6   
#define NUM_PRODUCTIONS 7

// Terminals defn
enum { INT_TOK, PLUS, STAR, LPAREN, RPAREN, DOLLAR, EPSILON = -1 };
const char non_terminals[NUM_NON_TERMINALS] = {'E', 'X', 'T', 'Y'};

//Production structure//
typedef struct {
    char lhs;         
    char* rhs[10];    
    int rhs_count;    
} Production;

// Grammar

Production productions[NUM_PRODUCTIONS] = {
    { 'E', {"T", "X"}, 2 },
    { 'X', {"+", "E"}, 2 },
    { 'X', {"eps"}, 1 },
    { 'T', {"int", "Y"}, 2 },
    { 'T', {"(", "E", ")"}, 3 },
    { 'Y', {"*", "T"}, 2 },
    { 'Y', {"eps"}, 1 }
};

/* Global arrays for FIRST and FOLLOW sets.
   For each non-terminal (indexed by the position in non_terminals[]),
   we store up to MAX_FIRST_FOLLOW token values.
*/
int first[NUM_NON_TERMINALS][MAX_FIRST_FOLLOW];
int first_count[NUM_NON_TERMINALS] = {0};

int follow[NUM_NON_TERMINALS][MAX_FIRST_FOLLOW];
int follow_count[NUM_NON_TERMINALS] = {0};

/* The parse table is a 2D array
*/
int parse_table[NUM_NON_TERMINALS][NUM_TOKENS];

/* Utility function to check if a token is already in a set.
   Returns 1 if found, 0 else.
*/
int contains(int set[], int count, int token) {
    for (int i = 0; i < count; i++) {
        if (set[i] == token)
            return 1;
    }
    return 0;
}

/* Adds token to the set.
   Returns true if added.
*/
bool add_to_set(int set[], int *count, int token) {
    if (!contains(set, *count, token)) {
        if (*count < MAX_FIRST_FOLLOW) {
            set[(*count)++] = token;
            return true;
        }
    }
    return false;
}

/* Checks whether a string represents a non-terminal.
   If yes, returns its index (in non_terminals), else returns -1.
*/
int is_non_terminal_string(const char* s) {
    if (strlen(s) == 1) {
        char c = s[0];
        for (int i = 0; i < NUM_NON_TERMINALS; i++) {
            if (non_terminals[i] == c)
                return i;
        }
    }
    return -1;
}

/* Maps a terminal string to its token value.
   (For “eps”, we return EPSILON.)
*/
int terminal_token_value(const char* s) {
    if (strcmp(s, "int") == 0) return INT_TOK;
    if (strcmp(s, "+") == 0) return PLUS;
    if (strcmp(s, "*") == 0) return STAR;
    if (strcmp(s, "(") == 0) return LPAREN;
    if (strcmp(s, ")") == 0) return RPAREN;
    if (strcmp(s, "$") == 0) return DOLLAR;
    if (strcmp(s, "eps") == 0) return EPSILON;
    return -2; 
}

/* Returns a string for a given token value. */
const char* token_to_string(int token) {
    switch(token) {
        case INT_TOK:  return "int";
        case PLUS:     return "+";
        case STAR:     return "*";
        case LPAREN:   return "(";
        case RPAREN:   return ")";
        case DOLLAR:   return "$";
        case EPSILON:  return "eps";
        default:       return "?";
    }
}

/* 
   Computes FIRST sets dynamically.
   This helps in iterating over all productions and adds to the FIRST set
   of the left-hand side non-terminal the FIRST of the production’s right-hand side.
   Iteration continues until no set changes.
*/
void compute_first() {
    bool changed = true;
    while(changed) {
        changed = false;
        // For every production in the grammar
        for (int i = 0; i < NUM_PRODUCTIONS; i++) {
            int lhs_index = -1;
            for (int j = 0; j < NUM_NON_TERMINALS; j++) {
                if (non_terminals[j] == productions[i].lhs) {
                    lhs_index = j; break;
                }
            }
            if (lhs_index == -1)
                continue; 

            bool derives_epsilon = true;
            // Process each symbol in the rhs 
            for (int j = 0; j < productions[i].rhs_count; j++) {
                const char* sym = productions[i].rhs[j];
                int nt_index = is_non_terminal_string(sym);
                if (nt_index == -1) {  // Terminal symbol
                    int t = terminal_token_value(sym);
                    if (t != EPSILON) { 
                        if(add_to_set(first[lhs_index], &first_count[lhs_index], t))
                            changed = true;
                    } else { // the production rule has epsilon in it
                        if(add_to_set(first[lhs_index], &first_count[lhs_index], EPSILON))
                            changed = true;
                    }
                    derives_epsilon = (t == EPSILON);
                    break; // terminal halts further processing in 
                } else {
                    // Symbol is a non-terminal; add its FIRST (except eps) into lhs FIRST
                    for (int k = 0; k < first_count[nt_index]; k++) {
                        int tok = first[nt_index][k];
                        if (tok != EPSILON) {
                            if(add_to_set(first[lhs_index], &first_count[lhs_index], tok))
                                changed = true;
                        }
                    }
                    // If FIRST of this non-terminal contains EPSILON, continue;
                    // otherwise, stop processing.
                    if (!contains(first[nt_index], first_count[nt_index], EPSILON)) {
                        derives_epsilon = false;
                        break;
                    }
                }
            }
            // If every symbol in the rhs can derive epsilon, add epsilon to FIRST(lhs)
            if (derives_epsilon) {
                if(add_to_set(first[lhs_index], &first_count[lhs_index], EPSILON))
                    changed = true;
            }
        }
    }
}

/* 
   Computes the FIRST set for a sequence of symbols.
 
*/
void compute_first_of_sequence(char* symbols[], int count, int result[], int *result_count) {
    *result_count = 0;
    bool all_epsilon = true;
    for (int i = 0; i < count; i++) {
        int temp_count = 0;
        int temp[MAX_FIRST_FOLLOW] = {0};
        int nt_index = is_non_terminal_string(symbols[i]);
        if (nt_index == -1) { // Terminal
            int t = terminal_token_value(symbols[i]);
            temp[temp_count++] = t;
        } else {
            for (int j = 0; j < first_count[nt_index]; j++) {
                temp[temp_count++] = first[nt_index][j];
            }
        }
        bool contains_epsilon = false;
        for (int j = 0; j < temp_count; j++) {
            if (temp[j] == EPSILON)
                contains_epsilon = true;
            else if (!contains(result, *result_count, temp[j]))
                result[(*result_count)++] = temp[j];
        }
        if (!contains_epsilon) {
            all_epsilon = false;
            break;
        }
    }
    if(all_epsilon) {
        if (!contains(result, *result_count, EPSILON))
            result[(*result_count)++] = EPSILON;
    }
}

/* 
   Computes FOLLOW sets dynamically.
   Initially, add $ (DOLLAR) to the FOLLOW set of the start symbol ( 'E').
   Then for each production, add to FOLLOW(B) all tokens in FIRST(beta) for every occurrence
   of a non-terminal B in the right-hand side. If beta can derive epsilon, add FOLLOW(A) to FOLLOW(B).
*/
void compute_follow() {
    // Initialize: add $ to FOLLOW
    int start_index = -1;
    for (int i = 0; i < NUM_NON_TERMINALS; i++) {
        if (non_terminals[i] == 'E') {
            start_index = i;
            break;
        }
    }
    if (start_index != -1)
        add_to_set(follow[start_index], &follow_count[start_index], DOLLAR);

    bool changed = true;
    while(changed) {
        changed = false;
        for (int i = 0; i < NUM_PRODUCTIONS; i++) {
            int lhs_index = -1;
            for (int j = 0; j < NUM_NON_TERMINALS; j++) {
                if(non_terminals[j] == productions[i].lhs) { lhs_index = j; break; }
            }
            if(lhs_index == -1) continue;
            
            for (int j = 0; j < productions[i].rhs_count; j++) {
                int B_index = is_non_terminal_string(productions[i].rhs[j]);
                if (B_index == -1)
                    continue; // skip terminals
                
                int first_beta[MAX_FIRST_FOLLOW];
                int first_beta_count = 0;
                if (j + 1 < productions[i].rhs_count)
                    compute_first_of_sequence(&productions[i].rhs[j+1],
                                              productions[i].rhs_count - (j+1),
                                              first_beta, &first_beta_count);
                // Add FIRST(beta) except EPSILON to FOLLOW(B)
                for (int k = 0; k < first_beta_count; k++) {
                    if (first_beta[k] != EPSILON) {
                        if(add_to_set(follow[B_index], &follow_count[B_index], first_beta[k]))
                            changed = true;
                    }
                }
                // If beta is empty or can derive epsilon, add FOLLOW(lhs) to FOLLOW(B)
                bool beta_has_epsilon = (first_beta_count == 0) || contains(first_beta, first_beta_count, EPSILON);
                if (beta_has_epsilon) {
                    for (int k = 0; k < follow_count[lhs_index]; k++) {
                        if(add_to_set(follow[B_index], &follow_count[B_index], follow[lhs_index][k]))
                            changed = true;
                    }
                }
            }
        }
    }
}

/*
   Builds the LL(1) parsing table.
   For every production A -> α, for every token a in FIRST(α) (except epsilon)
   the table entry (A, a) is set to that production number.
   If FIRST(α) contains epsilon, then for every token b in FOLLOW(A) add the production.
*/
void build_parse_table() {

    for (int i = 0; i < NUM_NON_TERMINALS; i++)
        for (int j = 0; j < NUM_TOKENS; j++)
            parse_table[i][j] = -1;

    for (int i = 0; i < NUM_PRODUCTIONS; i++) {
        // Find non-terminal index for production lhs
        int nt_index = -1;
        for (int j = 0; j < NUM_NON_TERMINALS; j++) {
            if(non_terminals[j] == productions[i].lhs) { nt_index = j; break; }
        }
        if (nt_index == -1)
            continue;

        int first_alpha[MAX_FIRST_FOLLOW];
        int first_alpha_count = 0;
        compute_first_of_sequence(productions[i].rhs, productions[i].rhs_count,
                                  first_alpha, &first_alpha_count);
        bool has_epsilon = false;
        for (int j = 0; j < first_alpha_count; j++) {
            if (first_alpha[j] == EPSILON)
                has_epsilon = true;
            else
                parse_table[nt_index][first_alpha[j]] = i;
        }
        if (has_epsilon) {
            // For every terminal in FOLLOW(A), add the production to the table.
            for (int j = 0; j < follow_count[nt_index]; j++) {
                int term = follow[nt_index][j];
                parse_table[nt_index][term] = i;
            }
        }
    }
}

/* 
   Fn to print the FIRST and FOLLOW sets.
*/
void print_first_follow_sets() {
    printf("FIRST Sets:\n");
    for (int i = 0; i < NUM_NON_TERMINALS; i++) {
        printf("FIRST(%c): { ", non_terminals[i]);
        for (int j = 0; j < first_count[i]; j++) {
            printf("%s", token_to_string(first[i][j]));
            if (j != first_count[i] - 1)
                printf(", ");
        }
        printf(" }\n");
    }
    printf("\nFOLLOW Sets:\n");
    for (int i = 0; i < NUM_NON_TERMINALS; i++) {
        printf("FOLLOW(%c): { ", non_terminals[i]);
        for (int j = 0; j < follow_count[i]; j++) {
            printf("%s", token_to_string(follow[i][j]));
            if (j != follow_count[i] - 1)
                printf(", ");
        }
        printf(" }\n");
    }
    printf("\n");
}

/*
   Prints the LL(1) parsing table.
   The table is printed with non-terminals as rows and tokens as columns.
*/
void print_parse_table() {
    printf("LL(1) Parsing Table:\n\n");
    
    // Print header row for tokens
    printf("%-8s", "");  
    for (int j = 0; j < NUM_TOKENS; j++) {
        printf("%-20s", token_to_string(j));
    }
    printf("\n");
    
    // Print each row corresponding to a non-terminal
    for (int i = 0; i < NUM_NON_TERMINALS; i++) {
        printf("%-8c", non_terminals[i]);
        for (int j = 0; j < NUM_TOKENS; j++) {
            if (parse_table[i][j] != -1) {
                // Prepare the production string (ex: E -> T X)
                char prod_str[100] = "";
                int prod_index = parse_table[i][j];
                sprintf(prod_str, "%c ->", productions[prod_index].lhs);
                for (int k = 0; k < productions[prod_index].rhs_count; k++) {
                    strcat(prod_str, " ");
                    strcat(prod_str, productions[prod_index].rhs[k]);
                }
                printf("%-20s", prod_str);
            } else {
                printf("%-20s", "-");
            }
        }
        printf("\n");
    }
}



void print_production(int index) {
    Production *p = &productions[index];
    printf("%c ->", p->lhs);
    for (int i = 0; i < p->rhs_count; i++)
        printf(" %s", p->rhs[i]);
}

/*
   The LL(1) parser.
   It uses a stack (initialized with the end marker and start symbol). For non-terminals, it looks up the production; for terminals,
   it matches the input. 
*/
void parse_input(int input[]) {
    int stack[MAX_STACK];
    int top = -1;
    
    // Push end marker and start symbol 
    stack[++top] = DOLLAR;
    stack[++top] = 'E';
    
    int ip = 0;
    printf("\nParsing steps:\n");
    
    while(top >= 0) {
        int top_symbol = stack[top--];
        
        
        if (top_symbol < 128 && is_non_terminal_string((char[]){(char)top_symbol, '\0'}) == -1) {
            if (top_symbol == input[ip]) {
                printf("Matched terminal: %s\n", token_to_string(top_symbol));
                ip++;
            } else {
                printf("Error: Unexpected token '%s'\n", token_to_string(input[ip]));
                return;
            }
        } else if (top_symbol == DOLLAR) {
            if (input[ip] == DOLLAR) {
                printf("Parsing successful!\n");
            } else {
                printf("Error: Input remaining after parsing.\n");
            }
            return;
        } else {
            int nt_index = is_non_terminal_string((char[]){(char)top_symbol, '\0'});
            int token = input[ip];
            int prod_index = parse_table[nt_index][token];
            if (prod_index == -1) {
                printf("Error: No rule for %c on input '%s'\n", top_symbol, token_to_string(token));
                return;
            }
            printf("Apply production: ");
            print_production(prod_index);
            printf("\n");
            Production *p = &productions[prod_index];
            // Push the production’s right-hand side in reverse order
            for (int i = p->rhs_count - 1; i >= 0; i--) {
                if (strcmp(p->rhs[i], "eps") == 0)
                    continue;
                int nt_check = is_non_terminal_string(p->rhs[i]);
                if (nt_check == -1) {
                    int t = terminal_token_value(p->rhs[i]);
                    stack[++top] = t;
                } else {
                    stack[++top] = p->rhs[i][0];
                }
            }
        }
    }
}
//main fn
int main() {
    
    compute_first();
    compute_follow();
    build_parse_table();
    
 
    print_first_follow_sets();
    print_parse_table();
    
    // Example input:int + int * int $
    int input[] = { INT_TOK, PLUS, INT_TOK, STAR, INT_TOK, DOLLAR };
    parse_input(input);
    
    return 0;
}
