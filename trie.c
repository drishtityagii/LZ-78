#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include "trie.h"
#include <stdlib.h>
#include "code.h"
#include <stdbool.h>

TrieNode *trie_node_create(
    uint16_t code) { 
    TrieNode *node = (TrieNode *) calloc(1, sizeof(TrieNode)); //allocating memory for trie
    if (node == NULL) {
        printf("Couldn't allocate memory for node :(\n");
        return NULL;
    }
    node->code = code; //setting code to code
    for (int i = 0; i < 256; i++) { //looping through all indexes
        node->children[i] = NULL; //setting each node to null
    }
    return node;
}

void trie_node_delete(TrieNode *n) {
    if (n == NULL) { //to not free memory that's alr been freed (trauma)
        return;
    }
    for (int i = 0; i < 256; ++i) {
        if (n->children[i] != NULL) {
            trie_node_delete(n->children[i]); //recursively calling to delete all the babies
        }
    }
    free(n);
}
TrieNode *trie_create(void) {
    TrieNode *root = malloc(sizeof(TrieNode)); //allocating memory for root
    if (root == NULL) {
        printf("Couldn't allocate memory for root :(\n");
        return NULL;
    }
    (root->code) = EMPTY_CODE; //setting the code to EMPTY_CODE
    for (int i = 0; i < 256; ++i) { //going thru all vals
        root->children[i] = NULL; //initializing all the babies
    }
    return root; //returns null (from malloc) if not successful, if it is then it returns set root
}
void trie_reset(TrieNode *root) {
    if (root == NULL) {
        return;
    }
    for (int i = 0; i < 256; ++i) { //setting all children to null- need to check how to delete
        if (root->children[i] != NULL) {
            trie_node_delete(root->children[i]); //traversing through children and deleting
            root->children[i] = NULL; //setting to null
        }
    }
    root->code = EMPTY_CODE; //setting root to EMPTY_CODE to indicate that its empty
}
void trie_delete(TrieNode *n) {
    for (int i = 0; i < 256; ++i) { //add check above for null
        if (n->children[i] != NULL) {
            trie_delete(n->children[i]); //deleting children - add check if children is null
            n->children[i] = NULL; //setting to null
        }
    }
    trie_node_delete(n); //freeing node
}
TrieNode *trie_step(TrieNode *n, uint8_t sym) {
    return n->children
        [sym]; //returns the pointer to child node [sym], and if it doesn't exist,  itll be null
}
