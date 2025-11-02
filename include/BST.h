#pragma once

#include "String.h"
#include "bit_vector.h"
#include "gen_vector.h"


typedef String* (*to_string_fn)(const u8* elm);


typedef struct {
    genVec* arr;
    bitVec* flags; // for each i: 0 -> empty, 1-> set , // TODO: may be used for size tracking??
    size_t size;
    genVec_compare_fn cmp_fn; // for < or > operations (= can be done by bitcmp)
    to_string_fn to_str;
} __attribute__((aligned(64))) BST; 


BST* bst_create(size_t data_size, genVec_compare_fn cmp, to_string_fn to_str, genVec_delete_fn del);
void bst_destroy(BST* bst);

void bst_insert(BST* bst, const u8* val);
void bst_remove(BST* bst, const u8* val);
int bst_search(const BST* bst, const u8* val);

void bst_balance(BST* bst);     // TODO: how to do this???
String* bst_subtree(const BST* bst, const u8* root);

String* bst_preorder(const BST* bst);
String* bst_inorder(const BST* bst);
String* bst_postorder(const BST* bst);
String* bst_bfs(const BST* bst);

// visual tree
void bst_print(const BST* bst);

/*
class BST {
private:
    std::vector<int> arr;

public:
    BST() {}

    void insert(int val) {
        if (arr.empty()) {
            arr.resize(1, INT_MAX);
            arr[0] = val;
            return;
        }
         
        size_t index = search_helper(val);
        if (index < arr.size() && arr[index] == val) { 
            return; // Value already exists
        }
        
        if (index >= arr.size()) {
            arr.resize(index + 1, INT_MAX);
        }
        arr[index] = val;
    }

    bool search(int val) const {
        if (arr.empty()) { return false; }
        size_t index = search_helper(val);
        return index < arr.size() && arr[index] == val;
    }

    void remove(int val) {
        if (arr.empty()) { return; }
        
        size_t index = search_helper(val);
        if (index >= arr.size() || arr[index] != val) {
            return; // Value not found
        }
        
        remove_helper(index);
    }

    void preorder() {
        std::cout << "Preorder: ";
        preorder_helper(0);
        std::cout << '\n';
    }

    void inorder() {
        std::cout << "Inorder: ";
        inorder_helper(0);
        std::cout << '\n';
    }

    void postorder() {
        std::cout << "Postorder: ";
        postorder_helper(0);
        std::cout << '\n';
    }

private:
    size_t search_helper(int& val, size_t pos = 0) const {
        if (pos >= arr.size() || arr[pos] == INT_MAX) { 
            return pos; 
        }
        
        if (val == arr[pos]) {
            return pos;
        }
        else if (val > arr[pos]) { // right subtree
            return search_helper(val, (2 * pos) + 2);
        } else {  // left subtree
            return search_helper(val, (2 * pos) + 1);
        }
    }

    std::pair<int, size_t> find_min(size_t pos) const {
        size_t current = pos;
        while (current < arr.size() && arr[current] != INT_MAX) {
            size_t left = (2 * current) + 1;
            if (left >= arr.size() || arr[left] == INT_MAX) {
                return {arr[current], current };
            }
            current = left;
        }
        return {INT_MAX, LONG_MAX };
    }

    void remove_helper(size_t pos) {
        size_t left = (2 * pos) + 1;
        size_t right = (2 * pos) + 2;
        
        bool has_left = left < arr.size() && arr[left] != INT_MAX;
        bool has_right = right < arr.size() && arr[right] != INT_MAX;
        
        if (!has_left && !has_right) {
            // Case 1: No children
            arr[pos] = INT_MAX;
        } else if (!has_left) {
            // Case 2: Only right child
            arr[pos] = arr[right];
            remove_helper(right);
        } else if (!has_right) {
            // Case 3: Only left child
            arr[pos] = arr[left];
            remove_helper(left);
        } else {
            // Case 4: Two children
            std::pair<int, size_t> min_val = find_min(right);
            arr[pos] = min_val.first;
            remove_helper(search_helper(min_val.first, min_val.second));
        }
    }

    void preorder_helper(size_t pos) {
        if (pos >= arr.size() || arr[pos] == INT_MAX) {
            return;
        }
        
        std::cout << arr[pos] << " ";  // Visit root
        preorder_helper((2 * pos) + 1);  // Traverse left
        preorder_helper((2 * pos) + 2);  // Traverse right
    }


    void inorder_helper(size_t pos) {
        if (pos >= arr.size() || arr[pos] == INT_MAX) {
            return;
        }
        
        inorder_helper((2 * pos) + 1);   // Traverse left
        std::cout << arr[pos] << " ";  // Visit root
        inorder_helper((2 * pos) + 2);   // Traverse right
    }

    void postorder_helper(size_t pos) {
        if (pos >= arr.size() || arr[pos] == INT_MAX) {
            return;
        }
        
        postorder_helper((2 * pos) + 1);  // Traverse left
        postorder_helper((2 * pos) + 2);  // Traverse right
        std::cout << arr[pos] << " ";   // Visit root
    }
};
*/

