#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "word.h"
#include "code.h"
#include <string.h>

Word *word_create(uint8_t *syms, uint32_t len) {
    //len is lenght of array of symbols
    //syms is the array of symbols
    //returns word* if successful, NULL otherwise
    Word *w = malloc(sizeof(Word)); //allocate memory for word
    if (w == NULL) {
        return NULL;
    }
    uint8_t *ws = malloc(sizeof(uint8_t) * len); //allocate memory for symbol array
    if (ws == NULL) {
        return NULL;
    }
    for (uint32_t i = 0; i < len; ++i) {
        ws[i] = syms[i]; //setting symbol array to word array
    }
    w->syms = ws; //setting syms array to this initialized array- might have to do this for len?
    w->len = len;
    return w;
}
Word *word_append_sym(Word *w, uint8_t sym) {
    //if word empty, word only contains sym
    //else, append sym to word
    uint32_t nlen = (w->len) + 1; //adding one to account for new sym
    uint8_t *nsym
        = (uint8_t *) malloc(sizeof(uint8_t) * nlen); //allocating memory for array to append to
    if (nsym == NULL) {
        return NULL;
    }
    memcpy(nsym, w->syms, w->len); //copying old sym with space for one more
    nsym[w->len] = sym; //new array last index = sym
    Word *newword = word_create(nsym, nlen); //creating new word
    free(nsym); //freeing array since it's stored in word
    return newword;
}
void word_delete(Word *w) {
    if (w->syms != NULL) {
        free(w->syms); //free syms array
    }
    if (w != NULL) {
        free(w); //free word
    }
}
WordTable *wt_create(void) {
    //create array of words (calloc)
    //size of MAX_CODE, val UINT16_t
    //initialize single Word at index EMPTY_CODE- represents empty word- str len 0
    WordTable *wt = (WordTable *) malloc(sizeof(WordTable) * MAX_CODE); //allocating memory for word
    for (uint32_t i = 0; i < MAX_CODE; ++i) {
        wt[i] = NULL; //for each index onwards, set each word to null
    }
    wt[EMPTY_CODE] = word_create(NULL, 0); //initializing single word
    return wt; //return word
}
void wt_reset(WordTable *wt) {
    if (wt != NULL) {
        //resets wt to contain only empty Word
        //all other words in wt are NULL
        for (uint32_t i = 0; i < MAX_CODE; i++) {
            if (wt[i] != NULL) {
                word_delete(wt[i]);
                wt[i] = NULL;
            }
        }
        wt[EMPTY_CODE] = word_create(NULL, 0); //initializing single word
    }
}
void wt_delete(WordTable *wt) {
    for (uint32_t i = 0; i < MAX_CODE; i++) {
        if (wt[i] != NULL) {
            word_delete(wt[i]);
        }
    }
    free(wt);
}
