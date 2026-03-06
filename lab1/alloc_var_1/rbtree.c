#include <stdio.h>
#include <stdlib.h>
#include "rbtree.h"

// Initialize the sentinel TNULL node
void initialize_TNULL(RBTree *tree, Node *node) {
    node->key = 0;
    node->color = BLACK;

    node->left = node;
    node->right = node;
    node->parent = node;

    node->dup_prev = NULL;
    node->dup_next = NULL;

    tree->TNULL = node;
    tree->root = tree->TNULL;
}

// Initialize new node
void init_node(Node *node, size_t key) {
    node->key = key;
    node->color = RED;
    node->left = NULL;
    node->right = NULL;
    node->parent = NULL;
    node->dup_prev = NULL;
    node->dup_next = NULL;
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
    tree->TNULL->color = BLACK;
}

// Find tree min node
Node* tree_minimum(RBTree *tree, Node *node) {
    while (node->left != tree->TNULL) {
        node = node->left;
    }
    return node;
}

// Fix up the tree to maintain Red-Black properties after deletion
void rb_delete_fixup(RBTree *tree, Node *x) {
    Node *w;
    while (x != tree->root && x->color == BLACK) {
        if (x == x->parent->left) {
            w = x->parent->right; // w - sibling of node x

            // Case 1: sibling of w is red
            if (w->color == RED) {
                w->color = BLACK;
                x->parent->color = RED;
                left_rotate(tree, x->parent);
                w = x->parent->right;
            }

            // Case 2: sibling of w is black, its both children are black
            if (w->left->color == BLACK && w->right->color == BLACK) {
                w->color = RED;
                x = x->parent;
            } else {
                // Case 3: sibling of w is black, its left child red, right black
                if (w->right->color == BLACK) {
                    w->left->color = BLACK;
                    w->color = RED;
                    right_rotate(tree, w);
                    w = x->parent->right;
                }

                // Case 4: sibling of w is black, its right child red
                w->color = x->parent->color;
                x->parent->color = BLACK;
                w->right->color = BLACK;
                left_rotate(tree, x->parent);
                x = tree->root;
            }
        } else { // Symmetrical for right subtree
            w = x->parent->left;

            if (w->color == RED) {
                w->color = BLACK;
                x->parent->color = RED;
                right_rotate(tree, x->parent);
                w = x->parent->left;
            }

            if (w->right->color == BLACK && w->left->color == BLACK) {
                w->color = RED;
                x = x->parent;
            } else {
                if (w->left->color == BLACK) {
                    w->right->color = BLACK;
                    w->color = RED;
                    left_rotate(tree, w);
                    w = x->parent->left;
                }

                w->color = x->parent->color;
                x->parent->color = BLACK;
                w->left->color = BLACK;
                right_rotate(tree, x->parent);
                x = tree->root;
            }
        }
    }
    x->color = BLACK;
    tree->TNULL->color = BLACK;
}

// Insert a node into the Red-Black Tree
Node* insert_node(RBTree *tree, Node *new_node) {
    new_node->left = tree->TNULL;
    new_node->right = tree->TNULL;

    Node *y = NULL;
    Node *x = tree->root;

    while (x != tree->TNULL) {
        y = x;
        if (new_node->key == x->key) {
            // Found dublicate
            Node *curr = x;
            while (curr->dup_next != NULL) {
                curr = curr->dup_next;
            }
            curr->dup_next = new_node;
            new_node->dup_prev = curr;
            new_node->color = x->color; // Same color
            return new_node; // No balancing needed
        }
        if (new_node->key < x->key) x = x->left;
        else x = x->right;
    }

    new_node->parent = y;
    if (y == NULL) tree->root = new_node;
    else if (new_node->key < y->key) y->left = new_node;
    else y->right = new_node;

    if (new_node->parent == NULL) {
        new_node->color = BLACK;
        return new_node;
    }
    if (new_node->parent->parent == NULL) return new_node;

    insert_fixup(tree, new_node);
    return new_node;
}
// Change root node
void transplant(RBTree *tree, Node *u, Node *v) {
    if (u->parent == NULL) tree->root = v;
    else if (u == u->parent->left) u->parent->left = v;
    else u->parent->right = v;
    v->parent = u->parent;
}

Node* delete_node(RBTree *tree, Node *z) {
    if (z == tree->TNULL) return NULL;

    // Case 1: It is dublicate (not in the tree)
    if (z->parent == NULL && z != tree->root) {
        if (z->dup_prev) z->dup_prev->dup_next = z->dup_next;
        if (z->dup_next) z->dup_next->dup_prev = z->dup_prev;
        z->dup_next = NULL;
        z->dup_prev = NULL;
        return z;
    }

    // Case 2: It is dublicate (is in the tree)
    if (z->dup_next != NULL) {
        Node *repl = z->dup_next;

        repl->dup_prev = NULL;

        repl->parent = z->parent;
        repl->left = z->left;
        repl->right = z->right;
        repl->color = z->color;

        if (z->parent == NULL) tree->root = repl;
        else if (z == z->parent->left) z->parent->left = repl;
        else z->parent->right = repl;

        if (repl->left != tree->TNULL) repl->left->parent = repl;
        if (repl->right != tree->TNULL) repl->right->parent = repl;

        z->dup_next = NULL;
        z->dup_prev = NULL;
        z->parent = NULL; z->left = NULL; z->right = NULL;
        return z; // No balancing needed
    }

    // Case 3: Standart deletion in Black and Red Tree + possible fix up
    Node *x, *y = z;
    enum Color y_original_color = y->color;

    if (z->left == tree->TNULL) {
        x = z->right;
        transplant(tree, z, z->right);
    } else if (z->right == tree->TNULL) {
        x = z->left;
        transplant(tree, z, z->left);
    } else {
        // Node has two children = finding min in right subtree (y)
        y = tree_minimum(tree, z->right);
        y_original_color = y->color;
        x = y->right; // x - child of y and will take its place

        if (y->parent == z) {
            x->parent = y;
        } else {
            transplant(tree, y, y->right);
            y->right = z->right;
            y->right->parent = y;
        }

        transplant(tree, z, y);
        y->left = z->left;
        y->left->parent = y;
        y->color = z->color;
    }

    if (y_original_color == BLACK) {
        rb_delete_fixup(tree, x);
    }

    // Clean up after deleted node
    z->parent = NULL;
    z->left = NULL;
    z->right = NULL;
    z->dup_next = NULL;
    z->dup_prev = NULL;

    return z;
}

void inorder_traverse(RBTree *tree, Node *node, void (*visit)(Node*)) {
    if (node != tree->TNULL) {
        inorder_traverse(tree, node->left, visit);

        Node *curr = node;
        while (curr != NULL) {
            visit(curr);
            curr = curr->dup_next;
        }

        inorder_traverse(tree, node->right, visit);
    }
}

Node* rbtree_find_best(RBTree *tree, size_t size) {
    Node *current = tree->root;
    Node *best = NULL;

    while (current != tree->TNULL) {
        if (current->key == size) {
            return current; // Ideal
        } else if (current->key > size) {
            best = current; // Left is better fit
            current = current->left;
        } else {
            current = current->right; // Too small, go right
        }
    }
    return best;
}
