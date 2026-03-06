#ifndef TREE_H_INCLUDED
#define TREE_H_INCLUDED

#include "rbtree.h"

// Синоніми
typedef RBTree block_tree;
typedef Node   block_node;

// Макроси-обгортки для функцій
#define block_tree_init(t, null_node) initialize_TNULL((t), (null_node))
#define block_tree_insert(t, n)       insert_node((t), (n))
#define block_tree_remove(t, n)       delete_node((t), (n))
#define block_tree_find_best(t, size) rbtree_find_best((t), (size))
#define block_tree_traverse(t, visit) inorder_traverse((t), (t)->root, (visit))

#endif // TREE_H_INCLUDED
