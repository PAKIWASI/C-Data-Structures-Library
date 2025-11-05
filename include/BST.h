#pragma once

#include "String.h"
#include "bit_vector.h"
#include "gen_vector.h"


typedef String* (*to_string_fn)(const u8* elm);


typedef struct {
    genVec* arr;
    bitVec* flags; // for each i: 0 -> empty, 1-> set ,
    size_t size;
    genVec_compare_fn cmp_fn; // for < or > operations (= can be done by bitcmp)
    to_string_fn to_str;
} __attribute__((aligned(64))) BST; 


BST* bst_create(size_t data_size, genVec_compare_fn cmp, to_string_fn to_str, genVec_delete_fn del);
void bst_destroy(BST* bst);

void bst_insert(BST* bst, const u8* val);
void bst_remove(BST* bst, const u8* val);
u8 bst_search(const BST* bst, const u8* val);
void bst_find_min(const BST* bst, u8* min);
void bst_find_max(const BST* bst, u8* max);

void bst_balance(BST* bst);     // TODO: how to do this???
String* bst_subtree(const BST* bst, const u8* root);

String* bst_preorder(const BST* bst);
String* bst_inorder(const BST* bst);
String* bst_postorder(const BST* bst);
String* bst_bfs(const BST* bst);

// visual tree
void bst_print(const BST* bst);

/*              balance
    1. collect_inorder()     → values = [1,2,3,4,5,6,7]
    2. Clear flags           → tree is now empty
    3. Insert values[mid=3]  → inserts 4 (the middle value)
    4. Recurse left [0..2]   → inserts 2 (middle of [1,2,3])
    5. Recurse right [4..6]  → inserts 6 (middle of [5,6,7])
    6. Continue recursively  → fills in 1,3,5,7
*/
