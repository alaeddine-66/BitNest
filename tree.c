#include <stdio.h>
#include "tree.h"
#include <stdlib.h>

Tree* new_node(Tree *left, Tree *right) {
    Tree *t = malloc(sizeof(Tree));
    if (!t) return NULL;
    t->Node.freq = left->Node.freq + right->Node.freq;
    t->Node.symbol = 0;
    t->left = left;
    t->right = right;
    return t;
}

Tree* new_leaf(char symbol, int freq) {
    Tree *t = malloc(sizeof(Tree));
    if (!t) return NULL;
    t->Node.symbol = symbol;
    t->Node.freq = freq;
    t->left = NULL;
    t->right = NULL;
    return t;
}


void print_tree(Tree *t){
    if (!t) return;

    if (t->left == NULL && t->right == NULL)
        printf("Leaf %c : %d\n", t->Node.symbol, t->Node.freq);
    else
        printf("Node : %d\n", t->Node.freq);

    print_tree(t->left);
    print_tree(t->right);
}


void free_tree(Tree *t){
    if (t==NULL) return;

    free_tree(t->left);
    free_tree(t->right);
    free(t);
}
