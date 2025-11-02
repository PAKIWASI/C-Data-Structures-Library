#include "BST.h"

#include "String.h"
#include "helper_functions.h"


void insert(BST* bst, int val) {
    bst_insert(bst, cast(val));
}

int main(void)
{
    BST* bst = bst_create(sizeof(int), int_cmp, int_to_str, NULL);

    insert(bst, 1);
    insert(bst, 2);
    insert(bst, 3);

    String* out = bst_preorder(bst);
    string_print(out);
    
    string_destroy(out);
    bst_destroy(bst);
}
