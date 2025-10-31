#include "BST.h"
#include "gen_vector.h"

#include <stdio.h>
#include <stdlib.h>

// TODO:  is this correct?
#define OUT_CREATE(bst) (malloc(sizeof(bst->arr->data_size)))


// PRIVATE HELPER FUNCTIONS
static void bst_insert_helper(BST* bst, const u8* val, size_t i);
static size_t bst_search_helper(const BST* bst, const u8* val, size_t i);



BST* bst_create(size_t data_size, genVec_compare_fn cmp, genVec_delete_fn del)
{
    if (!data_size || !cmp || !del) {
        printf("bst create: invalid parameters\n");
        return NULL;
    }
    BST* bst = malloc(sizeof(BST));
    bst->arr = genVec_init(1, data_size, del);
    if (!bst->arr) {
        printf("bst create: genvec init failed\n");
        return NULL;
    }

    bst->flags = genVec_init(1, sizeof(u8), NULL);
    if (!bst->flags) {
        printf("bst create: genvec init failed\n");
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

    if (bst->flags) { genVec_destroy(bst->flags); }

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

// PRIVATE FUNCTION IMPLEMENTATION

static void bst_insert_helper(BST* bst, const u8* val, size_t i)
{
    if (bst->flags);
    

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


