#ifndef TREE_H
#define TREE_H

typedef struct {
    int freq;
    char symbol;
}node;

typedef struct Tree {
    node Node;
    struct Tree *left;
    struct Tree *right;
} Tree;

Tree* new_node(Tree *left, Tree *right);
Tree* new_leaf(char symbol, int freq);
void print_tree(Tree *t);
void free_tree(Tree *t);

#endif
