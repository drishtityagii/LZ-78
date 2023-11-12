#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>
#include "word.h"
#include "trie.h"
#include "code.h"
#include "io.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
int bit_length(uint16_t x) {
    int count = 0;
    while (x != 0) {
        x >>= 1;
        count++;
    }
    return count; //same bitcount function as in encode
}

int main(int argc, char *argv[]) {
    int infile = STDIN_FILENO;
    int outfile = STDOUT_FILENO;
    bool verbose = false;
    int opt;
    while ((opt = getopt(argc, argv, "i:o:v")) != -1) {
        switch (opt) {
        case 'i': infile = open(optarg, O_RDONLY); break;
        case 'o': outfile = open(optarg, O_WRONLY | O_CREAT | O_TRUNC | S_IWUSR); break;
        case 'v': verbose = true; break;
        }
    }
    FileHeader in;
    struct stat x;
    fstat(infile, &x); //setting file permissions
    read_header(infile, &in); //reading in the header file
    fchmod(outfile, in.protection);

    WordTable *table = wt_create();
    uint8_t curr_sym = 0;
    uint16_t curr_code = 0;
    uint16_t next_code = START_CODE;
    while (read_pair(infile, &curr_code, &curr_sym, bit_length(next_code))
           == true) { //while there are pairs to read
        table[next_code] = word_append_sym(table[curr_code], curr_sym);
        write_word(outfile, table[next_code]);
        next_code = next_code + 1; //increment code
        if (next_code == MAX_CODE) { //once next code reaches max code, reset
            wt_reset(table);
            next_code = START_CODE;
        }
    }
    flush_words(outfile); //flush words
    struct stat out_stats;
    fstat(outfile, &out_stats);
    struct stat in_stats;
    fstat(infile, &in_stats);
    if (verbose == true) {
        size_t comp_size = in_stats.st_size;
        size_t uncomp_size = out_stats.st_size;
        printf("Compressed File Size: %ld bytes.\nUncompressed File Size: %ld bytes.\n", comp_size,
            uncomp_size);
    }
}
