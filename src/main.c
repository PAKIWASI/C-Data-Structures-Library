#include "BST.h"

#include "String.h"
#include "helper_functions.h"


void insert(BST* bst, int val) {
    bst_insert(bst, cast(val));
}

void rm(BST* bst, int val) {
    bst_remove(bst, cast(val));
}

u8 search(BST* bst, int val) {
    return bst_search(bst, cast(val));
}

int main(void)
{
    BST* bst = bst_create(sizeof(int), int_cmp, int_to_str, NULL);

    insert(bst, 4);
    insert(bst, 2);
    insert(bst, 6);
    insert(bst, 1);
    insert(bst, 3);
    insert(bst, 5);
    insert(bst, 7);

    String* str;
    str = bst_preorder(bst);
    string_print(str);
    string_destroy(str);
    printf("\n");

    str = bst_inorder(bst);
    string_print(str);
    string_destroy(str);
    printf("\n");

    str = bst_postorder(bst);
    string_print(str);
    string_destroy(str);
    printf("\n");

    str = bst_bfs(bst);
    string_print(str);
    string_destroy(str);
    printf("\n");

    printf("%d\n", search(bst, 5));
    printf("%d\n", search(bst, 10));

    rm(bst, 4);

    str = bst_bfs(bst);
    string_print(str);
    string_destroy(str);
    printf("\n");

    bst_balance(bst);

    str = bst_bfs(bst);
    string_print(str);
    string_destroy(str);
    printf("\n");


    bst_destroy(bst);
}


