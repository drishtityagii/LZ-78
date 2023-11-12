#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <unistd.h>
#include <stdbool.h>
#include "io.h"
#include "endian.h"
#include "code.h"

uint64_t total_syms = 0;
uint64_t total_bits = 0;
int read_bytes(int infile, uint8_t *buf, int to_read) {
    int total = 0; //tracking bytes
    while (total < to_read) {
        int bytes = read(infile, buf + total,
            to_read - total); //reading byte to buf while there are still bytes to read
        if (bytes <= 0) {
            break;
        }
        total += bytes; //check third arg
    }
    return total;
}
int write_bytes(int outfile, uint8_t *buf, int to_write) {
    int total = 0; //tracking bytes
    while (total < to_write) {
        int bytes = write(outfile, buf + total,
            to_write - total); //writing bytes to buf while there are still bytes to write
        if (bytes <= 0) {
            break;
        }
        total += bytes;
    }
    return total;
}
void read_header(int infile, FileHeader *header) {
    read_bytes(infile, (uint8_t *) header, sizeof(FileHeader)); //reading to header
    if (!little_endian()) {
        header->protection = swap16(header->protection); //check
        header->magic = swap32(header->magic); //make sure endianness is little endian
    }
    if (header->magic != MAGIC) {
        fprintf(stderr, "Wrong magic number.\n");
        exit(1); //validating magic number
    }
}
void write_header(int outfile, FileHeader *header) {
    write_bytes(outfile, (uint8_t *) header, sizeof(FileHeader)); //writing to header
    if (!little_endian()) {
        header->protection = swap16(header->protection);
        header->magic = swap32(header->magic);
    } //make sure endianness is little endian (CHECK BIT CNT)
    if (header->magic != MAGIC) {
        fprintf(stderr, "Wrong magic number.\n");
        exit(1); //validating magic number
    }
}
static uint8_t pair_buf[BLOCK];
static int pair_index = 0;
static uint8_t word_buf[BLOCK];
static int word_index = 0;

bool read_sym(int infile, uint8_t *sym) {
    static int sym_bytes = 0;
    static uint8_t sym_buf[BLOCK];
    static int sym_index = 0;
    if (sym_index == sym_bytes) { //if buffer is full
        sym_bytes = read_bytes(infile, sym_buf, BLOCK); //read block
        sym_index = 0; //reset index
        if (sym_bytes == sym_index) {
            return false; //if after reading, bytes was 0, return false (nothing read)
        }
    }
    *sym = sym_buf[sym_index]; //get symbol from buffer
    total_syms++;
    sym_index++; //increment symbol
    return true; //return cond. since symbols should've been read
}
void write_pair(int outfile, uint16_t code, uint8_t sym, int bitlen) {
    static int remain = BLOCK * 8; //remaining bits to write- first set to bits in block
    for (int i = 0; i < bitlen; i++) { //iterating through code of length bitlen
        if ((code >> i) & 0x1) {
            pair_buf[pair_index / 8]
                |= (0x1 << (pair_index
                            % 8)); //if bit is one, set the corressponding bit in the buffer to 1
        } else { //else if the bit is zero, set the corresponding bit in the buffer to zero
            pair_buf[pair_index / 8]
                &= ~(0x1 << (pair_index % 8)); /
        }
        pair_index++; //increment index
        remain--; //decrement remaining bits
        total_bits++;
        if (remain == 0) { //if there are no more bits to write
            write_bytes(outfile, pair_buf, (BLOCK)); //write buffer
            remain = BLOCK * 8; //reset remaining bits
            pair_index = 0; //reset index
        }
    }
    for (int i = 0; i < 8; i++) { //8 bits in symbols
        if ((sym >> (i % 8)) & 0x1) {
            pair_buf[pair_index / 8]
                |= (0x1
                    << (pair_index
                           % 8)); //if bit is one, set the corresponding bit in the buffer to 1. mod 8 since symbols are 8 bits long, vs code being 16
        } else {
            pair_buf[pair_index / 8] &= ~(
                0x1
                << (pair_index
                       % 8)); //else if bit is zero, set the corresponding bit in the buffer to zero
        }
        pair_index++; //increment index
        remain--; //decrement remaining bits
        total_bits++;
        if (remain == 0) { //if there are no sym bits to write
            write_bytes(outfile, pair_buf, BLOCK); //write to outfile
            remain = BLOCK * 8; //reset remaining
            pair_index = 0; //reset index
        }
    }
}
void flush_pairs(int outfile) {
    if (pair_index > 0) {
        write_bytes(outfile, pair_buf,
            pair_index
                / 8); //flushes all remaining pairs of byte size index/8 (since index is being incremented by bits)
    }
    pair_index = 0; //resets index
}
bool read_pair(int infile, uint16_t *code, uint8_t *sym, int bitlen) {
    *code = 0;
    *sym = 0;
    static uint8_t read_buf[BLOCK];
    static int read_index = 0;
    static int bytes = 0;
    static int read_remain = 0;
    for (int i = 0; i < bitlen; i++) { //iterating through code of length bitlen
        if (read_remain == 0) { //if the remaining bits to read is zero
            bytes = read_bytes(infile, read_buf, BLOCK); //read a block
            if (bytes == 0) { //if nothing was read, return false
                return false;
            }
            read_remain = bytes * 8; //reset remaining bits to read to the read bits
            read_index = 0; //reset read index
        }
        *code |= ((uint16_t) ((read_buf[read_index / 8] >> (read_index % 8)) & 0x1))
                 << i; 
        total_bits++;
        //*code |= ((uint16_t)((pair_buf[index/8] & (1 << index%8)) >> index) << i);
        //*code |= ((uint16_t)(buf[index/8] >> (index%8)) & 0x1) << i;
        read_index++; //incrementing index
        read_remain--; //decrementing bits to read
    }
    for (int j = 0; j < 8; j++) { //iterating through symbol of length 8 bits
        if (read_remain == 0) { //if no symbols left to read
            bytes = read_bytes(infile, read_buf, BLOCK); //read a new block
            if (bytes == 0) {
                return false; //if nothing was read, return false
            }
            read_remain = bytes * 8; //reset remaining bits to read
            read_index = 0; //reset the index
        }
        *sym |= ((uint8_t) (read_buf[read_index / 8] >> (read_index % 8)) & 0x1)
                << j; //setting sym
        total_bits++;
        total_syms++;
        //*sym |= (pair_buf[index/8]);
        // *sym |= ((uint16_t)(pair_buf[index/8] >> (index%8)) & 0x1) << j;
        read_index++; //incrementing index
        read_remain--; //decrementing remaining bits to read
    }
    if (code == STOP_CODE) {
        return false; //if code reaches STOP_CODE return false
    }
    if (read_remain == 0) {
        bytes = read_bytes(infile, read_buf, BLOCK);
        if (bytes == 0) {
            return false; //if remaining bits to read is zero, read block, if nothing was read, return false
        }
        read_index = 0;
        read_remain = bytes * 8; //reset
    }
    return true;
}
void write_word(int outfile, Word *w) {
    //static uint8_t word_buf[BLOCK];
    //static int word_index = 0;
    for (uint32_t i = 0; i < (w->len); i++) {
        word_buf[word_index] = (w->syms[i]);
        word_index += 1;
        if (word_index == (BLOCK)) {
            write_bytes(outfile, word_buf, BLOCK); //if buf is full write
            word_index = 0; //reset index
        }
    }
}
void flush_words(int outfile) {
    if (word_index != BLOCK) {
        write_bytes(outfile, word_buf, word_index);
    }
    word_index = 0;
}
