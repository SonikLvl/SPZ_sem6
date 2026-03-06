#include <stdio.h>
#include <stdlib.h>

enum Color { RED, BLACK };

typedef struct Node {
    int data;
    enum Color color;
    struct Node *left, *right, *parent;
} Node;

typedef struct RBTree {
    Node *root;
    Node *TNULL;
} RBTree;

// Initialize the sentinel TNULL node
void initialize_TNULL(RBTree *tree, Node *node) {
    node->data = 0;
    node->color = BLACK;
    node->left = NULL;
    node->right = NULL;
}

// Create an empty Red-Black Tree
RBTree* create_tree() {
    RBTree *tree = (RBTree *)malloc(sizeof(RBTree));
    tree->TNULL = (Node *)malloc(sizeof(Node));
    initialize_TNULL(tree, tree->TNULL);
    tree->root = tree->TNULL;
    return tree;
}

// Perform a Left Rotation
void left_rotate(RBTree *tree, Node *x) {
    Node *y = x->right;
    x->right = y->left;
    if (y->left != tree->TNULL) {
        y->left->parent = x;
    }
    y->parent = x->parent;
    if (x->parent == NULL) {
        tree->root = y;
    } else if (x == x->parent->left) {
        x->parent->left = y;
    } else {
        x->parent->right = y;
    }
    y->left = x;
    x->parent = y;
}

// Perform a Right Rotation
void right_rotate(RBTree *tree, Node *x) {
    Node *y = x->left;
    x->left = y->right;
    if (y->right != tree->TNULL) {
        y->right->parent = x;
    }
    y->parent = x->parent;
    if (x->parent == NULL) {
        tree->root = y;
    } else if (x == x->parent->right) {
        x->parent->right = y;
    } else {
        x->parent->left = y;
    }
    y->right = x;
    x->parent = y;
}

// Fix up the tree to maintain Red-Black properties after insertion
void insert_fixup(RBTree *tree, Node *k) {
    Node *u;
    while (k->parent != NULL && k->parent->color == RED) {
        if (k->parent == k->parent->parent->left) {
            u = k->parent->parent->right;
            if (u->color == RED) {
                k->parent->color = BLACK;
                u->color = BLACK;
                k->parent->parent->color = RED;
                k = k->parent->parent;
            } else {
                if (k == k->parent->right) {
                    k = k->parent;
                    left_rotate(tree, k);
                }
                k->parent->color = BLACK;
                k->parent->parent->color = RED;
                right_rotate(tree, k->parent->parent);
            }
        } else {
            u = k->parent->parent->left;
            if (u->color == RED) {
                k->parent->color = BLACK;
                u->color = BLACK;
                k->parent->parent->color = RED;
                k = k->parent->parent;
            } else {
                if (k == k->parent->left) {
                    k = k->parent;
                    right_rotate(tree, k);
                }
                k->parent->color = BLACK;
                k->parent->parent->color = RED;
                left_rotate(tree, k->parent->parent);
            }
        }
    }
    tree->root->color = BLACK;
}

// Insert a node into the Red-Black Tree
void insert_node(RBTree *tree, int key) {
    Node *node = (Node *)malloc(sizeof(Node));
    node->parent = NULL;
    node->data = key;
    node->left = tree->TNULL;
    node->right = tree->TNULL;
    node->color = RED;

    Node *y = NULL;
    Node *x = tree->root;

    while (x != tree->TNULL) {
        y = x;
        if (node->data < x->data) {
            x = x->left;
        } else {
            x = x->right;
        }
    }

    node->parent = y;
    if (y == NULL) {
        tree->root = node;
    } else if (node->data < y->data) {
        y->left = node;
    } else {
        y->right = node;
    }

    if (node->parent == NULL) {
        node->color = BLACK;
        return;
    }
    if (node->parent->parent == NULL) {
        return;
    }

    insert_fixup(tree, node);
}

// --- HIERARCHICAL PRINTING LOGIC ---

// Helper function to print the tree in a 2D format (Rotated 90 degrees CCW)
void print_hierarchical_helper(RBTree *tree, Node *root, int space) {
    int COUNT = 5; // Distance between levels
    if (root == tree->TNULL) {
        return;
    }

    // Increase distance between levels
    space += COUNT;

    // Process right child first (so it appears at the top of the display)
    print_hierarchical_helper(tree, root->right, space);

    // Print current node after space
    printf("\n");
    for (int i = COUNT; i < space; i++) {
        printf(" ");
    }

    // Use ANSI escape codes to print colors: \033[1;31m is Red, \033[0m resets color
    if (root->color == RED) {
        printf("\033[1;31m%d(R)\033[0m\n", root->data);
    } else {
        // Standard text color for black nodes to ensure they are visible on dark terminals
        printf("%d(B)\n", root->data);
    }

    // Process left child
    print_hierarchical_helper(tree, root->left, space);
}

void print_tree_hierarchical(RBTree *tree) {
    printf("\nHierarchical Tree Structure (Rotated 90 degrees left):\n");
    printf("Right children are UP, Left children are DOWN.\n");
    printf("------------------------------------------------------\n");
    print_hierarchical_helper(tree, tree->root, 0);
    printf("------------------------------------------------------\n");
}

int main() {
    RBTree *tree = create_tree();

    // Testing insertion with enough values to trigger rotations and recoloring
    int values_to_insert[] = {55, 40, 65, 60, 75, 57, 30, 45};
    int num_values = sizeof(values_to_insert) / sizeof(values_to_insert[0]);

    printf("Inserting values: ");
    for (int i = 0; i < num_values; i++) {
        printf("%d ", values_to_insert[i]);
        insert_node(tree, values_to_insert[i]);
    }
    printf("\n");

    // Print the tree graphically
    print_tree_hierarchical(tree);

    return 0;
}
