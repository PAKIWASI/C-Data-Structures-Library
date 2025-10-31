#include "String.h"

#include <ctype.h>
#include <stdbool.h>
#include <string.h>

#define ALPHABET_SIZE 29


typedef struct {
    struct Node* children[29];
    bool isword;
} __attribute__((aligned(128))) Node;


typedef struct {
    Node* root;
} Trie;

// TODO: modify clean() from parse.h to use here (must allow - . )
char* normalize(const char* token, char* output, size_t output_size) {
    if (!token || !output || output_size == 0) { return NULL; }

    size_t len = strlen(token);
    size_t out_idx = 0;
    bool has_letters = false;

    // Normalize curly quotes → ASCII apostrophe
    char tmp[256];
    size_t tmp_idx = 0;
    for (size_t i = 0; i < len && tmp_idx < sizeof(tmp) - 1; i++) {
        unsigned char c = token[i];
        if (c == 0xE2 && i + 2 < len &&
            (unsigned char)token[i + 1] == 0x80 &&
            (unsigned char)token[i + 2] == 0x99) {
            tmp[tmp_idx++] = '\'';
            i += 2;
        } else {
            tmp[tmp_idx++] = c;
        }
    }
    tmp[tmp_idx] = '\0';
    token = tmp;
    len = tmp_idx;

    for (size_t i = 0; i < len && out_idx < output_size - 1; i++) {
        unsigned char c = (unsigned char)token[i];

        if (isdigit(c)) continue; // remove digits

        if (c == '\'' || c == '-' || c == '.') {
            if (has_letters && i + 1 < len && isalpha((unsigned char)token[i + 1])) {
                output[out_idx++] = c;
            }
            continue;
        }

        if (isalpha(c)) {
            c = tolower(c);
            output[out_idx++] = c;
            has_letters = true;
        }
    }

    // Trim trailing apostrophes
    while (out_idx > 0 && output[out_idx - 1] == '\'') out_idx--;

    output[out_idx] = '\0';
    return (out_idx > 0 && has_letters) ? output : NULL;
}

static inline int char_index(char c) {
    if (c >= 'A' && c <= 'Z') { c = (char)tolower(c); }
    if (c >= 'a' && c <= 'z') { return c - 'a'; }
    if (c == '\'') { return 26; }
    if (c == '-') {  return 27; }
    if (c == '.') {  return 28; }
    return -1;  // ignore other chars
}

Trie* trie_create(void);
void trie_destroy(Trie* trie);


void trie_insert_cstr(const char* cword);
void trie_insert_string(String word);


