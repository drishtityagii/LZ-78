#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <unistd.h>
#include <stdbool.h>
#include "word.h"
#include "trie.h"
#include "io.h"
#include "code.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

int bit_length(uint16_t x) {
    int count = 0;
    while (x != 0) {
        x >>= 1;
        count += 1; //function that gets the bit length of a code
    }
    return count;
}

int main(int argc, char *argv[]) {
    bool verbose = false;
    int opt;
    int infile = STDIN_FILENO;
    int outfile = STDOUT_FILENO;
    //    int infile1 = STDIN_FILENO;
    while ((opt = getopt(argc, argv, "i:o:v")) != -1) {
        switch (opt) {
        case 'i':
            infile = open(optarg, O_RDONLY);
            //              infile1 = open(optarg, O_RDONLY);
            break;
        case 'o':
            outfile = open(optarg, O_WRONLY | O_CREAT | O_TRUNC); //Lev helped with the O_XX
            break;
        case 'v': verbose = true; break;
        }
    }
    FileHeader out;
    struct stat x;
    fstat(infile, &x);
    fchmod(outfile, x.st_mode);
    out.magic = MAGIC;
    out.protection = x.st_mode;
    write_header(outfile, &out); //setting permissions, Nishant helped with this
    TrieNode *root = trie_create();
    TrieNode *curr_node = root; //setting current node to root
    TrieNode *prev_node = NULL;
    uint8_t curr_sym = 0;
    uint8_t prev_sym = 0;
    uint16_t next_code = START_CODE;
    while (read_sym(infile, &curr_sym) == true) { //while there are still symbols to read
        TrieNode *next_node = trie_step(curr_node, curr_sym);
        if (next_node != NULL) {
            prev_node = curr_node;
            curr_node = next_node;
        } else {
            write_pair(outfile, curr_node->code, curr_sym, bit_length(next_code)); //write pair
            curr_node->children[curr_sym] = trie_node_create(next_code);
            curr_node = root;
            next_code = next_code + 1; //increment next code
        }
        if (next_code == MAX_CODE) { //once next code reaches max, reset
            trie_reset(root);
            curr_node = root;
            next_code = START_CODE;
        }
        prev_sym = curr_sym; //go to next sym
    }
    if (curr_node != root) {
        write_pair(outfile, prev_node->code, prev_sym, bit_length(next_code));
        next_code = (next_code + 1) % MAX_CODE;
    }
    write_pair(outfile, STOP_CODE, 0, bit_length(next_code));
    flush_pairs(outfile); //write and flush all pairs
    // struct stat in_stats;
    // fstat(infile, &in_stats);
    //s//truct stat out_stats;
    // fstat(outfile, &out_stats);
    if (verbose == true) {
        uint64_t compressed_size = total_bits / 8;
        uint64_t orig_size = total_syms;
        double ratio = (100 * (1 - ((double) (compressed_size / orig_size))));

        printf("Compressed File Size: %ld bytes\nUncompressed File Size: %ld bytes\nCompression "
               "Ratio: %0.2f%%\n",
            compressed_size, orig_size, ratio);
    }
}
