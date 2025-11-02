#include "BST.h"
#include "String.h"
#include "bit_vector.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>


#define PARENT(i)  (((i) - 1) / 2)
#define L_CHILD(i) ((2 * (i)) + 1)
#define R_CHILD(i) ((2 * (i)) + 2)


// PRIVATE HELPER FUNCTIONS
static void bst_insert_helper(BST* bst, const u8* val, size_t i);
static void bst_preorder_helper(const BST* bst, size_t i, String* out);
static void bst_inorder_helper(const BST* bst, size_t i, String* out);
static void bst_postorder_helper(const BST* bst, size_t i, String* out);
static size_t bst_search_helper(const BST* bst, const u8* val, size_t pos);



BST* bst_create(size_t data_size, genVec_compare_fn cmp, to_string_fn to_str, genVec_delete_fn del)
{
    if (!data_size || !cmp || !to_str) {
        printf("bst create: invalid parameters\n");
        return NULL;
    }

    BST* bst = malloc(sizeof(BST));
    if (!bst) {
        printf("bst create: malloc failed\n");
        return NULL;
    }

    bst->arr = genVec_init(0, data_size, del);
    if (!bst->arr) {
        printf("bst create: genvec init failed\n");
        free(bst);
        return NULL;
    }


    bst->flags = bitVec_create(); 
    if (!bst->flags) {
        printf("bst create: genvec init failed\n");
        free(bst);
        free(bst->arr);
        return NULL;
    }

    bst->size = 0;
    bst->cmp_fn = cmp;
    bst->to_str = to_str;
    
    return bst;
}

void bst_destroy(BST* bst)
{
    if (!bst) { return; }

    if (bst->arr) { genVec_destroy(bst->arr); }

    if (bst->flags) { bitVec_destroy(bst->flags); }

    free(bst);
}

void bst_insert(BST* bst, const u8* val)
{
    if (!bst || !val) {
        printf("bst insert: parameters null\n");
        return;
    }
    
    bst_insert_helper(bst, val, 0);

    bst->size++;
}

String* bst_preorder(const BST* bst)
{
    if (!bst) {
        printf("bst preorder: parameters null\n");
        return NULL;
    }

    String* out = string_create();
    
    bst_preorder_helper(bst, 0, out);

    return out;
}

String* bst_inorder(const BST* bst)
{
    if (!bst) {
        printf("bst preorder: parameters null\n");
        return NULL;
    }

    String* out = string_create();
    
    bst_inorder_helper(bst, 0, out);

    return out;
}

String* bst_postorder(const BST* bst)
{
    if (!bst) {
    }

    String* out = string_create();
    
    bst_postorder_helper(bst, 0, out);

    return out;
}


// PRIVATE FUNCTION IMPLEMENTATION

static void bst_insert_helper(BST* bst, const u8* val, size_t i)
{
    // Check if index is beyond current array size
    if (i >= bst->arr->size) {
        genVec_push(bst->arr, val);
        bitVec_push(bst->flags);
        bitVec_set(bst->flags, i);
        return;
    }
    
    // Check if slot is empty
    if (!bitVec_test(bst->flags, i)) {
        genVec_replace(bst->arr, i, val);  
        bitVec_set(bst->flags, i);
        return;
    }
    
    // Slot occupied - compare and recurse
    u8 cmp = bst->cmp_fn(val, genVec_get_ptr(bst->arr, i));
    
    if (cmp == 0) {
        bst_insert_helper(bst, val, L_CHILD(i));
    } else if (cmp == 1) {
        bst_insert_helper(bst, val, R_CHILD(i));
    }
    // If cmp == 255 (equal), do nothing (no duplicates)

    // REWRITE
    size_t index = bst_search_helper(bst, val, 0);

    if (index < bst->size && bst->cmp_fn())
}

static void bst_preorder_helper(const BST* bst, size_t i, String* out)
{       // end of arr or root i not set
    if (i >= bst->size || !bitVec_test(bst->flags, i)) {
        return;
    }

    String* str = bst->to_str(genVec_get_ptr(bst->arr, i));
    string_append_string(out, str);
    string_destroy(str);

    bst_preorder_helper(bst, L_CHILD(i), out);
    bst_preorder_helper(bst, R_CHILD(i), out);
}

static void bst_inorder_helper(const BST* bst, size_t i, String* out)
{       // end of arr or root i not set
    if (i >= bst->size || !bitVec_test(bst->flags, i)) {
        return;
    }


    bst_inorder_helper(bst, L_CHILD(i), out);

    String* str = bst->to_str(genVec_get_ptr(bst->arr, i));
    string_append_string(out, str);
    string_destroy(str);

    bst_inorder_helper(bst, R_CHILD(i), out);
}

static void bst_postorder_helper(const BST* bst, size_t i, String* out)
{       // end of arr or root i not set
    if (i >= bst->size || !bitVec_test(bst->flags, i)) {
        return;
    }

    bst_postorder_helper(bst, L_CHILD(i), out);
    bst_postorder_helper(bst, R_CHILD(i), out);

    String* str = bst->to_str(genVec_get_ptr(bst->arr, i));
    string_append_string(out, str);
    string_destroy(str);
}

static size_t search_helper(const BST* bst, const u8* val, size_t pos) 
{
    // Check if index is beyond current array size
    if (pos >= bst->arr->size) {
        return pos;
    }
    
    // Slot occupied - compare and recurse
    u8 cmp = bst->cmp_fn(val, genVec_get_ptr(bst->arr, pos));  // 1 if a > b, 0 if a < b, 255 if a = b
    
    if (cmp == 0) { // a < b 
        return search_helper(bst, val, L_CHILD(pos));
    } else if (cmp == 1) { // a > b
        return search_helper(bst, val, R_CHILD(pos));
    } else { // cmp = 255 // a = b
        return pos; 
    }
}


