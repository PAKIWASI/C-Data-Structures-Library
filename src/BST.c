#include "BST.h"
#include "Queue.h"
#include "bit_vector.h"
#include "gen_vector.h"
#include "helper_functions.h"

#include <stddef.h>
#include <stdlib.h>



#define PARENT(i)  (((i) - 1) / 2)
#define L_CHILD(i) ((2 * (i)) + 1)
#define R_CHILD(i) ((2 * (i)) + 2)


// PRIVATE HELPER FUNCTIONS
static void bst_preorder_helper(const BST* bst, size_t i, String* out);
static void bst_inorder_helper(const BST* bst, size_t i, String* out);
static void bst_postorder_helper(const BST* bst, size_t i, String* out);
static void bst_bfs_helper(const BST* bst, String* out);
static size_t bst_search_helper(const BST* bst, const u8* val, size_t pos, u8* flags);
static void bst_remove_helper(BST* bst, const size_t* index);
static size_t bst_find_min_helper(const BST* bst, size_t index);
static size_t bst_find_max_helper(const BST* bst, size_t index);



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

    u8 found = 0; // 1 -> a = b
    size_t index = bst_search_helper(bst, val, 0, &found);

    if (found) {   // value exits
        return;
    }

    genVec_reserve_val(bst->arr, index + 1, val); // reserve size with values = val
    // the ith pos will have the correct val and flag
    bitVec_set(bst->flags, index); // set ith index to 1 while prev (unallocated) set to 0
    
    bst->size++;
}

void bst_remove(BST* bst, const u8* val)
{
    if (!bst || !val) {
        printf("bst val: parameters null\n");
        return;
    }

    u8 found = 0;
    size_t index = bst_search_helper(bst, val, 0, &found);
    
    if (!found) { return; }

    bst_remove_helper(bst, &index);

    bst->size--;
}


u8 bst_search(const BST* bst, const u8* val)
{
    if (!bst || !val) {
        printf("bst search: parameters null\n");
        return -1; // 255 error
    }

    u8 found = 0;
    bst_search_helper(bst, val, 0, &found);
    return found;
}

void bst_find_min(const BST* bst, u8* min)
{
    if (!bst || !min) {
        printf("bst find min: parameters null\n");
        return;
    }

    size_t index = bst_find_min_helper(bst, 0);

    genVec_get(bst->arr, index, min);
}

void bst_find_max(const BST* bst, u8* max)
{
    if (!bst || !max) {
        printf("bst find min: parameters null\n");
        return;
    }

    size_t index = bst_find_max_helper(bst, 0);

    genVec_get(bst->arr, index, max);
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
        printf("bst inorder: parameters null\n");
        return NULL;
    }

    String* out = string_create();
    
    bst_inorder_helper(bst, 0, out);

    return out;
}

String* bst_postorder(const BST* bst)
{
    if (!bst) {
        printf("bst postorder: parameters null\n");
        return NULL;
    }

    String* out = string_create();
    
    bst_postorder_helper(bst, 0, out);

    return out;
}

String* bst_bfs(const BST* bst)
{
    if (!bst) {
        printf("bst bfs: bst is null\n");
        return NULL;
    }

    String* out = string_create();
    bst_bfs_helper(bst, out);
    return out;
}

// PRIVATE FUNCTION IMPLEMENTATION

static void bst_preorder_helper(const BST* bst, size_t i, String* out)
{       // end of arr or root i not set
    if (i >= bst->arr->size || !bitVec_test(bst->flags, i)) {
        return;
    }

    String* temp = bst->to_str(genVec_get_ptr(bst->arr, i));
    string_append_string(out, temp);
    string_destroy(temp);

    bst_preorder_helper(bst, L_CHILD(i), out);
    bst_preorder_helper(bst, R_CHILD(i), out);
}

static void bst_inorder_helper(const BST* bst, size_t i, String* out)
{       // end of arr or root i not set
    if (i >= bst->arr->size || !bitVec_test(bst->flags, i)) {
        return;
    }


    bst_inorder_helper(bst, L_CHILD(i), out);

    String* temp = bst->to_str(genVec_get_ptr(bst->arr, i));
    string_append_string(out, temp);
    string_destroy(temp);

    bst_inorder_helper(bst, R_CHILD(i), out);
}

static void bst_postorder_helper(const BST* bst, size_t i, String* out)
{       // end of arr or root i not set
    if (i >= bst->arr->size || !bitVec_test(bst->flags, i)) {
        return;
    }

    bst_postorder_helper(bst, L_CHILD(i), out);
    bst_postorder_helper(bst, R_CHILD(i), out);

    String* temp = bst->to_str(genVec_get_ptr(bst->arr, i));
    string_append_string(out, temp);
    string_destroy(temp);
}


static void bst_bfs_helper(const BST* bst, String* out)
{
    if (bst->size == 0) { return; }
    // we can store the index of the bst node
    Queue* q = queue_create(bst->size, sizeof(size_t), NULL);
    size_t index = 0;
    enqueue(q, cast(index));

    while (!queue_empty(q)) {
        size_t i = 0;
        dequeue(q, cast(i));

        String* temp = bst->to_str(genVec_get_ptr(bst->arr, i));
        string_append_string(out, temp);
        string_destroy(temp);

        size_t l = L_CHILD(i);
        if (l < bst->arr->size && bitVec_test(bst->flags, l)) {
            enqueue(q, cast(l));
        }
        size_t r = R_CHILD(i);
        if (r < bst->arr->size && bitVec_test(bst->flags, r)) {
            enqueue(q, cast(r));
        }
    }

    queue_destroy(q);
}

static size_t bst_search_helper(const BST* bst, const u8* val, size_t pos, u8* flags) 
{
    // Check if slot is empty (either beyond array or flag not set)
    if (pos >= bst->arr->size || !bitVec_test(bst->flags, pos)) {
        *flags = 0;
        return pos; // found empty slot for insertion
    }
    
    // Slot occupied - compare and recurse
    u8 cmp = bst->cmp_fn(val, genVec_get_ptr(bst->arr, pos));  // 1 if a > b, 0 if a < b, 255 if a = b
    
    if (cmp == 0) { // a < b 
        return bst_search_helper(bst, val, L_CHILD(pos), flags);
    } else if (cmp == 1) { // a > b
        return bst_search_helper(bst, val, R_CHILD(pos), flags);
    } else { // cmp = 255 // a = b
        *flags = 1;
        return pos; 
    }
}

static void bst_remove_helper(BST* bst, const size_t* index)
{
    size_t l = L_CHILD(*index);
    size_t r = R_CHILD(*index);

    u8 has_l = l < bst->arr->size && bitVec_test(bst->flags, l);
    u8 has_r = r < bst->arr->size && bitVec_test(bst->flags, r);

    if (!has_l && !has_r) { // no children
        bitVec_clear(bst->flags, *index); 
    } else if (!has_l) { // only right child
        genVec_replace(bst->arr, *index, genVec_get_ptr(bst->arr, r));   
        bst_remove_helper(bst, &r);
    } else if (!has_r) { // only left child
        genVec_replace(bst->arr, *index, genVec_get_ptr(bst->arr, l));   
        bst_remove_helper(bst, &l);
    } else { // has both left and right children
        // get the min in the right subtree
        size_t min_r = bst_find_min_helper(bst, r);
        genVec_replace(bst->arr, *index, genVec_get_ptr(bst->arr, min_r));
        bst_remove_helper(bst, &min_r);   
    }
}

static size_t bst_find_min_helper(const BST* bst, size_t index)
{
    while (index < bst->arr->size && bitVec_test(bst->flags, index)) {
        if (L_CHILD(index) >= bst->arr->size || !bitVec_test(bst->flags, L_CHILD(index))) 
            { return index; }

        index = L_CHILD(index);
    }     
    return -1; // LONG_MAX returned -> error
}

static size_t bst_find_max_helper(const BST* bst, size_t index)
{
    while (index < bst->arr->size && bitVec_test(bst->flags, index)) {
        if (R_CHILD(index) >= bst->arr->size || !bitVec_test(bst->flags, R_CHILD(index))) 
            { return index; }

        index = R_CHILD(index);
    }     
    return -1; // LONG_MAX returned -> error
}

