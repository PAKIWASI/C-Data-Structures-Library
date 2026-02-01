#include "BST.h"
#include "Queue.h"
#include "common.h"


// WARN: this shit is not right, needs a compelete re-write (but will i ever need this ?)
// not a priority rn

#define PARENT(i)  (((i) - 1) / 2)
#define L_CHILD(i) ((2 * (i)) + 1)
#define R_CHILD(i) ((2 * (i)) + 2)


// PRIVATE HELPER FUNCTIONS
static void bst_preorder_helper(const BST* bst, u32 i, String* out);
static void bst_inorder_helper(const BST* bst, u32 i, String* out);
static void bst_postorder_helper(const BST* bst, u32 i, String* out);
static void bst_inorder_arr(const BST* bst, u32 i, genVec* out);
static void bst_bfs_helper(const BST* bst, String* out);
static u32  bst_search_helper(const BST* bst, const u8* val, u32 pos, u8* flags);
static void bst_remove_helper(BST* bst, const u32* index);
static u32  bst_find_min_helper(const BST* bst, u32 index);
static u32  bst_find_max_helper(const BST* bst, u32 index);
static void bst_balance_helper(BST* bst, genVec* inorder, u32 l, u32 r);


BST* bst_create(u16 data_size, genVec_compare_fn cmp, to_string_fn to_str, genVec_delete_fn del)
{
    CHECK_FATAL(data_size == 0, "data_size can't be 0");
    CHECK_FATAL(!cmp, "cmp is null");
    CHECK_FATAL(!to_str, "to_str is null");

    BST* bst = malloc(sizeof(BST));
    CHECK_FATAL(!bst, "bst malloc failed");

    bst->arr = genVec_init(0, data_size, NULL, NULL, del);
    //CHECK_FATAL(!bst->arr, "bst arr init failed");


    bst->flags = bitVec_create();
    //CHECK_FATAL(!bst->flags, "bst flag init failed");

    bst->size   = 0;
    bst->cmp_fn = cmp;
    bst->to_str = to_str;

    return bst;
}

void bst_destroy(BST* bst)
{
    CHECK_FATAL(!bst, "bst is null");

    genVec_destroy(bst->arr);

    bitVec_destroy(bst->flags);

    free(bst);
}

void bst_insert(BST* bst, const u8* val)
{
    CHECK_FATAL(!bst, "bst is null");
    CHECK_FATAL(!val, "val is null");

    b8  found = 0; // 1 -> a = b
    u32 index = bst_search_helper(bst, val, 0, &found);

    if (found) { // value exits
        return;
    }

    genVec_reserve_val(bst->arr, index + 1, val); // reserve size with values = val
    // the ith pos will have the correct val and flag
    bitVec_set(bst->flags, index); // set ith index to 1 while prev (unallocated) set to 0

    bst->size++;
}

void bst_remove(BST* bst, const u8* val)
{
    CHECK_FATAL(!bst, "bst is null");
    CHECK_FATAL(!val, "val is null");


    b8  found = 0;
    u32 index = bst_search_helper(bst, val, 0, &found);

    if (!found) { return; }

    bst_remove_helper(bst, &index);

    bst->size--;
}


u8 bst_search(const BST* bst, const u8* val)
{
    CHECK_FATAL(!bst, "bst is null");
    CHECK_FATAL(!val, "val is null");


    b8 found = 0;
    bst_search_helper(bst, val, 0, &found);

    return found;
}

void bst_find_min(const BST* bst, u8* min)
{
    CHECK_FATAL(!bst, "bst is null");
    CHECK_FATAL(!min, "min is null");

    u32 index = bst_find_min_helper(bst, 0);

    genVec_get(bst->arr, index, min);
}

void bst_find_max(const BST* bst, u8* max)
{
    CHECK_FATAL(!bst, "bst is null");
    CHECK_FATAL(!max, "max is null");

    u32 index = bst_find_max_helper(bst, 0);

    genVec_get(bst->arr, index, max);
}

/*
// BUG: this is wrong
void bst_balance(BST* bst)
{
    CHECK_FATAL(!bst, "bst is null");

    if (bst->size == 0) { return; }

    // Collect inorder traversal
    genVec inorder;
    bst_inorder_arr(bst, 0, &inorder);

    // COMPLETELY clear the current tree
    genVec_destroy(bst->arr);
    bitVec_destroy(bst->flags);

    // Create new empty structures
    bst->arr   = genVec_init(0, inorder.data_size, NULL, inorder.del_fn);
    bst->flags = bitVec_create();
    bst->size  = 0;

    // Build balanced tree
    bst_balance_helper(bst, &inorder, 0, inorder.size);

    genVec_destroy_stk(&inorder);
}
*/


String* bst_preorder(const BST* bst)
{
    CHECK_FATAL(!bst, "bst is null");

    String* out = string_create();

    bst_preorder_helper(bst, 0, out);

    return out;
}

String* bst_inorder(const BST* bst)
{
    CHECK_FATAL(!bst, "bst is null");

    String* out = string_create();

    bst_inorder_helper(bst, 0, out);

    return out;
}

String* bst_postorder(const BST* bst)
{
    CHECK_FATAL(!bst, "bst is null");

    String* out = string_create();

    bst_postorder_helper(bst, 0, out);

    return out;
}

String* bst_bfs(const BST* bst)
{
    CHECK_FATAL(!bst, "bst is null");

    String* out = string_create();
    bst_bfs_helper(bst, out);
    return out;
}

//                      PRIVATE FUNCTION IMPLEMENTATION

static void bst_preorder_helper(const BST* bst, u32 i, String* out)
{ // end of arr or root i not set
    if (i >= bst->arr->size || !bitVec_test(bst->flags, i)) { return; }

    String* temp = bst->to_str(genVec_get_ptr(bst->arr, i));
    string_append_string(out, temp);
    string_append_char(out, ' ');
    string_destroy(temp);

    bst_preorder_helper(bst, L_CHILD(i), out);
    bst_preorder_helper(bst, R_CHILD(i), out);
}

static void bst_inorder_helper(const BST* bst, u32 i, String* out)
{ // end of arr or root i not set
    if (i >= bst->arr->size || !bitVec_test(bst->flags, i)) { return; }

    bst_inorder_helper(bst, L_CHILD(i), out);

    String* temp = bst->to_str(genVec_get_ptr(bst->arr, i));
    string_append_string(out, temp);
    string_append_char(out, ' ');
    string_destroy(temp);

    bst_inorder_helper(bst, R_CHILD(i), out);
}

static void bst_postorder_helper(const BST* bst, u32 i, String* out)
{ // end of arr or root i not set
    if (i >= bst->arr->size || !bitVec_test(bst->flags, i)) { return; }

    bst_postorder_helper(bst, L_CHILD(i), out);
    bst_postorder_helper(bst, R_CHILD(i), out);

    String* temp = bst->to_str(genVec_get_ptr(bst->arr, i));
    string_append_string(out, temp);
    string_append_char(out, ' ');
    string_destroy(temp);
}

static void bst_inorder_arr(const BST* bst, u32 i, genVec* out)
{
    if (i >= bst->arr->size || !bitVec_test(bst->flags, i)) { return; }

    bst_inorder_arr(bst, L_CHILD(i), out);
    genVec_push(out, genVec_get_ptr(bst->arr, i));
    bst_inorder_arr(bst, R_CHILD(i), out);
}

static void bst_bfs_helper(const BST* bst, String* out)
{
    if (bst->size == 0) { return; }
    // we can store the index of the bst node
    Queue* q     = queue_create(bst->size, sizeof(u32), NULL);
    u32    index = 0;
    enqueue(q, cast(index));

    while (!queue_empty(q)) {
        u32 i = 0;
        dequeue(q, cast(i));

        String* temp = bst->to_str(genVec_get_ptr(bst->arr, i));
        string_append_string(out, temp);
        string_append_char(out, ' ');
        string_destroy(temp);

        u32 l = L_CHILD(i);
        if (l < bst->arr->size && bitVec_test(bst->flags, l)) { enqueue(q, cast(l)); }
        u32 r = R_CHILD(i);
        if (r < bst->arr->size && bitVec_test(bst->flags, r)) { enqueue(q, cast(r)); }
    }

    queue_destroy(q);
}

static u32 bst_search_helper(const BST* bst, const u8* val, u32 pos, u8* flags)
{
    // Check if slot is empty (either beyond array or flag not set)
    if (pos >= bst->arr->size || !bitVec_test(bst->flags, pos)) {
        *flags = 0;
        return pos; // found empty slot for insertion
    }

    // Slot occupied - compare and recurse
    u8 cmp =
        bst->cmp_fn(val, genVec_get_ptr(bst->arr, pos)); // 1 if a > b, 0 if a < b, 255 if a = b

    if (cmp == 0) { // a < b
        return bst_search_helper(bst, val, L_CHILD(pos), flags);
    } else if (cmp == 1) { // a > b
        return bst_search_helper(bst, val, R_CHILD(pos), flags);
    } else { // cmp = 255 // a = b
        *flags = 1;
        return pos;
    }
}

static void bst_remove_helper(BST* bst, const u32* index)
{
    u32 l = L_CHILD(*index);
    u32 r = R_CHILD(*index);

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
        u32 min_r = bst_find_min_helper(bst, r);
        genVec_replace(bst->arr, *index, genVec_get_ptr(bst->arr, min_r));
        bst_remove_helper(bst, &min_r);
    }
}

static u32 bst_find_min_helper(const BST* bst, u32 index)
{
    while (index < bst->arr->size && bitVec_test(bst->flags, index)) {
        if (L_CHILD(index) >= bst->arr->size || !bitVec_test(bst->flags, L_CHILD(index))) {
            return index;
        }

        index = L_CHILD(index);
    }
    return -1; // LONG_MAX returned -> error
}

static u32 bst_find_max_helper(const BST* bst, u32 index)
{
    while (index < bst->arr->size && bitVec_test(bst->flags, index)) {
        if (R_CHILD(index) >= bst->arr->size || !bitVec_test(bst->flags, R_CHILD(index))) {
            return index;
        }

        index = R_CHILD(index);
    }
    return -1; // LONG_MAX returned -> error
}

// BUG: this is wrong???
static void bst_balance_helper(BST* bst, genVec* inorder, u32 l, u32 r)
{
    if (l >= r) {
        return; // base case: no elements in range [l, r)
    }

    u32 m = l + ((r - l) / 2);

    // Insert middle element
    bst_insert(bst, genVec_get_ptr(inorder, m));

    // Recursively build left subtree [l, m)
    bst_balance_helper(bst, inorder, l, m);

    // Recursively build right subtree [m + 1, r)
    bst_balance_helper(bst, inorder, m + 1, r);
}


