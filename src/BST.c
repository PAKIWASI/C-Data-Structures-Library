#include "BST.h"
#include "String.h"
#include "bit_vector.h"
#include "gen_vector.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

// TODO:  is this correct?
#define OUT_CREATE(bst) (malloc(sizeof(bst->arr->data_size)))
#define L_CHILD(i) ((2 * i) + 1)
#define R_CHILD(i) ((2 * i) + 2)

// PRIVATE HELPER FUNCTIONS
static void bst_insert_helper(BST* bst, const u8* val, size_t i);
void bst_preorder_helper(const BST* bst, size_t i,String* out, genVec_to_string to_str);
static size_t bst_search_helper(const BST* bst, const u8* val, size_t i);



BST* bst_create(size_t data_size, genVec_compare_fn cmp, genVec_delete_fn del)
{
    if (!data_size || !cmp || !del) {
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
    
    if (bst->size == bst->arr->size) {
        // TODO: bst_grow(BST* bst);
    }

    bst_insert_helper(bst, val, 0);
}


String* bst_preorder(const BST* bst, genVec_to_string to_str)
{
    if (!bst || !to_str) {
        printf("bst preorder: parameters null\n");
        return NULL;
    }

    String* out = string_create();
    
    bst_preorder_helper(bst, 0, out, to_str);

    return out;
}

// PRIVATE FUNCTION IMPLEMENTATION

static void bst_insert_helper(BST* bst, const u8* val, size_t i)
{
    u8* out = NULL;
    switch (bitVec_test(bst->flags, i)) {
        // i >= size
        case 255: 
            genVec_push(bst->arr, val); // push val
            bitVec_push(bst->flags);         // set flag to 1
            break; 
        // not set
        case 0:
            genVec_replace(bst->arr, i, val);  
            bitVec_set(bst->flags, i);
            break;
        // already set
        case 1: // recursive case
            out = (u8*)genVec_get_ptr(bst->arr, i);
            switch (bst->cmp_fn(val, out)) { // 1 if a > b, 0 if a < b, 255 if a = b
                case 255: break; // set with same value... nothing to do 
                case 0: bst_insert_helper(bst, val, L_CHILD(i)); break; // a < b 
                case 1: bst_insert_helper(bst, val, R_CHILD(i)); break; // a > b
                default: break;
            }
        default: break;
    }
}

void bst_preorder_helper(const BST* bst, size_t i,String* out, genVec_to_string to_str)
{

}

static size_t search_helper(const BST* bst, const u8* val, size_t i) 
{
    if (i >= bst->arr->size) {
        return i;
    }

    u8* out = OUT_CREATE(bst);
    genVec_get(bst->arr, i, out);
    if (bst->cmp_fn(val, out)) { 

    }
}


