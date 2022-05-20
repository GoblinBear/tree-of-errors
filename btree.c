#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct _Btree_node Btree_node;
struct _Btree_node {
    char value;
    Btree_node *left_child;
    Btree_node *right_child;
};

typedef struct _Stack_node Stack_node;
struct _Stack_node {
    Btree_node *value;
    Stack_node *next;
};

typedef struct _Pair_score_node Pair_score_node;
struct _Pair_score_node {
    int value;
    Pair_score_node *next;
};

int is_valid_input_format(const char *input)
{
    const char *current = input;
    int counter = 0;
    int node_amount = 0;

    while (*current) {
        // Check if it is formattd as (A,B) (B,C) and
        // all values are single uppercase letters
        if (counter % 6 == 0) {
            if (*current != '(') {
                return 0;
            }
        } else if (counter % 6 == 1) {
            if (*current < 65 && *current > 90) {
                return 0;
            }
        } else if (counter % 6 == 2) {
            if (*current != ',') {
                return 0;
            }
        } else if (counter % 6 == 3) {
            if (*current < 65 && *current > 90) {
                return 0;
            }
        } else if (counter % 6 == 4) {
            if (*current != ')') {
                return 0;
            }
            node_amount++;
        } else {
            if (*current != ' ') {
                return 0;
            }
        }

        current++;
        counter++;
    }

    // Check the trailing character
    current--;
    if (*current != ')') {
        return 0;
    }

    return node_amount;
}

Stack_node *create_stack_node(Btree_node *value)
{
    Stack_node *new_stack_node = (Stack_node*)calloc(1, sizeof(Stack_node));
    new_stack_node->value = value;
    new_stack_node->next = NULL;

    return new_stack_node;
}

void stack_push(Stack_node **stack_top, Btree_node *value)
{
    Stack_node *new_stack_node = create_stack_node(value);
    if (stack_top && *stack_top) {
        new_stack_node->next = *stack_top;
    } else {
        new_stack_node->next = NULL;
    }

    *stack_top = new_stack_node;
}

Btree_node *stack_pop(Stack_node **stack_top)
{
    if (!stack_top || !(*stack_top)) {
        return NULL;
    }

    Btree_node *value = (*stack_top)->value;
    Stack_node *previous_stack_top = *stack_top;
    *stack_top = (*stack_top)->next;
    free(previous_stack_top);

    return value;
}

void add_pair_score(Pair_score_node **pair_record, Pair_score_node **pair_record_tail, int score)
{
    Pair_score_node *new_pair_score_node = (Pair_score_node*)calloc(1, sizeof(Pair_score_node));
    new_pair_score_node->value = score;
    new_pair_score_node->next = NULL;

    if (*pair_record_tail) {
        (*pair_record_tail)->next = new_pair_score_node;
        (*pair_record_tail) = new_pair_score_node;
    } else {
        (*pair_record) = new_pair_score_node;
        (*pair_record_tail) = new_pair_score_node;
    }
}

void free_pair_score(Pair_score_node *pair_record)
{
    while (pair_record) {
        Pair_score_node *previos_pair_score_node = pair_record;
        pair_record = pair_record->next;
        free(previos_pair_score_node);
    }
}

Btree_node *create_btree_node(char value)
{
    Btree_node *new_btree_node = (Btree_node*)calloc(1, sizeof(Btree_node));
    new_btree_node->value = value;
    new_btree_node->left_child = NULL;
    new_btree_node->right_child = NULL;

    return new_btree_node;
}

Btree_node *search_btree_node(Btree_node *btree_root, char value)
{
    Btree_node *current_btree_node = btree_root;
    Stack_node *stack = NULL;

    while (1) {
        while (current_btree_node) {
            stack_push(&stack, current_btree_node);
            current_btree_node = current_btree_node->left_child;
        }

        current_btree_node = stack_pop(&stack);
        if (!current_btree_node) {
            break;
        }

        if (current_btree_node->value == value) {
            return current_btree_node;
        }

        current_btree_node = current_btree_node->right_child;
    }

    return NULL;
}

void free_btree(Btree_node *btree_root)
{
    if (btree_root) {
        free_btree(btree_root->left_child);
        free_btree(btree_root->right_child);
        free(btree_root);
    }
}

Btree_node *create_btree(char *input)
{
    const char *current_input = input;
    Btree_node *btree_root = NULL;
    Pair_score_node *pair_record = NULL;
    Pair_score_node *pair_record_tail = NULL;

    // Create b-tree root node
    btree_root = create_btree_node(*(current_input + 1));
    
    while (*current_input) {
        // Find parent node
        char parent_value = *(current_input + 1);
        char child_value = *(current_input + 3);
        int pair_score = parent_value * 100 + child_value;
        
        Btree_node *btree_parent_node = search_btree_node(btree_root, parent_value);

        // E2: Check - Duplicate pair
        if (pair_record) {
            Pair_score_node *current_pair_record_node = pair_record;
            while (current_pair_record_node) {
                if (current_pair_record_node->value == pair_score) {
                    printf("E2");

                    free(input);
                    free_btree(btree_root);
                    free_pair_score(pair_record);
                    exit(EXIT_SUCCESS);
                }

                current_pair_record_node = current_pair_record_node->next;
            }
        }

        // Add score of new pair
        add_pair_score(&pair_record, &pair_record_tail, pair_score);

        // E4: Check - Multiple roots
        if (!btree_parent_node) {
            printf("E4");

            free(input);
            free_btree(btree_root);
            free_pair_score(pair_record);
            exit(EXIT_SUCCESS);
        }

        // Add new pair to the B-tree
        if (!btree_parent_node->left_child) { // Child status: XX
            Btree_node *new_child_node = create_btree_node(child_value);
            btree_parent_node->left_child = new_child_node;
        } else {
            if (!btree_parent_node->right_child) { // Child status: OX
                Btree_node *new_child_node = create_btree_node(child_value);
                btree_parent_node->right_child = new_child_node;
            } else { // Child status: OO
                // E3: Check - Parent Has More than Two Children
                printf("E3");

                free(input);
                free_btree(btree_root);
                free_pair_score(pair_record);
                exit(EXIT_SUCCESS);
            }
        }

        current_input = current_input + 6;
    }

    free_pair_score(pair_record);

    return btree_root;
}

bool btree_has_cycle(Btree_node *btree_root)
{
    bool is_access[26] = {0};
    Btree_node *current_btree_node = btree_root;
    Stack_node *stack = NULL;

    while (1) {
        while (current_btree_node) {
            stack_push(&stack, current_btree_node);
            current_btree_node = current_btree_node->left_child;
        }

        current_btree_node = stack_pop(&stack);
        if (!current_btree_node) {
            break;
        }

        if (is_access[current_btree_node->value - 65]) {
            return true;
        } else {
            is_access[current_btree_node->value - 65] = true;
        }

        current_btree_node = current_btree_node->right_child;
    }

    return false;
}

void string_push_back(char *target, char ch)
{
    char *current_target = target;
    while (*current_target) {
        current_target++;
    }

    *current_target = ch;
    *(current_target + 1) = '\0';
} 
  
void display_btree_s_expression(Btree_node *btree_root, char **s_expression)
{
    if (!btree_root) {
        return;
    }

    Btree_node *current_btree_node = btree_root;
    string_push_back(*s_expression, current_btree_node->value);

    if (!current_btree_node->left_child && !current_btree_node->right_child) {
        return;
    }

    string_push_back(*s_expression, '(');
    display_btree_s_expression(current_btree_node->left_child, s_expression);
    string_push_back(*s_expression, ')');

    if (current_btree_node->right_child) {
        string_push_back(*s_expression, '(');
        display_btree_s_expression(current_btree_node->right_child, s_expression);
        string_push_back(*s_expression, ')');
    }
}

int main()
{
    char *input = (char*)calloc(1000, sizeof(char));
    fgets(input, 1000, stdin);

    // Delete on hackerrank
    char *current_input = input;
    while (current_input) {
        if (*current_input == '\n') {
            *current_input = '\0';
            break;
        }
        current_input++;
    }
    // Delete on hackerrank

    /* E1: Check - Invalid Input Format */
    int node_amount = is_valid_input_format(input);
    if (node_amount == 0) {
        printf ("E1");
        free(input);
        exit(EXIT_SUCCESS);
    }

    /* E2 - E5 */
    Btree_node *btree_root = create_btree(input);

    bool has_cycle = btree_has_cycle(btree_root);
    if (has_cycle) {
        printf ("E5");
        free(input);
        free_btree(btree_root);
        exit(EXIT_SUCCESS);
    }

    // S-Expression
    char *s_expression = (char *)calloc(1000, sizeof(char));
    display_btree_s_expression(btree_root, &s_expression);
    printf ("(%s)", s_expression);

    free(input);
    free_btree(btree_root);
    free(s_expression);

    return 0;
}
