CC = clang
CFLAGS = -Wall -Wextra -Werror -Wpedantic -Wstrict-prototypes
LDFLAGS = -lm
EXEC = encode decode
OBJS = encode.o decode.o word.o trie.o io.o

all: $(EXEC)

encode: encode.o word.o trie.o io.o
		$(CC) -o encode encode.o word.o trie.o io.o $(LDFLAGS)
decode: decode.o word.o trie.o io.o
		$(CC) -o decode decode.o word.o trie.o io.o $(LDFLAGS)
encode.o: encode.c
		$(CC) $(CFLAGS) -c encode.c
decode.o: decode.c
		$(CC) $(CFLAGS) -c decode.c
word.o: word.c
		$(CC) $(CFLAGS) -c word.c
trie.o: trie.c
		$(CC) $(CFLAGS) -c trie.c
io.o: io.c
		$(CC) $(CFLAGS) -c io.c
clean:
		rm -f encode decode *.o
format:
		clang-format -i -style=file *.[ch]
