#ifndef RBTREE_H_INCLUDED
#define RBTREE_H_INCLUDED

enum Color { RED, BLACK };

typedef struct Node {
    size_t key;
    enum Color color;
    struct Node *left, *right, *parent;
    struct Node *dup_prev, *dup_next;
} Node;

typedef struct RBTree {
    Node *root;
    Node *TNULL;
} RBTree;

void initialize_TNULL(RBTree *tree, Node *node);
void init_node(Node *node, size_t key);
void left_rotate(RBTree *tree, Node *x);
void right_rotate(RBTree *tree, Node *x);
void insert_fixup(RBTree *tree, Node *k);
Node* tree_minimum(RBTree *tree, Node *node);
void rb_delete_fixup(RBTree *tree, Node *x);
Node* insert_node(RBTree *tree, Node *new_node);
Node* delete_node(RBTree *tree, Node *z);
void inorder_traverse(RBTree *tree, Node *node, void (*visit)(Node*));
void transplant(RBTree *tree, Node *u, Node *v);
Node* rbtree_find_best(RBTree *tree, size_t size);

#endif // RBTREE_H_INCLUDED
