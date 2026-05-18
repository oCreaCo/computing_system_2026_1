CC=gcc
CFLAGS= -O0 -g3 -ggdb
LDFLAGS=-lpthread

SRC=malloc_test.c

.PHONY: all
all: malloc_pt malloc_je

malloc_pt: $(SRC)
	$(CC) $(CFLAGS) -o $@.o -c $< $(LDFLAGS)
	$(CC) $(CFLAGS) -o $@ $@.o $(LDFLAGS)

malloc_je: $(SRC)
	$(CC) $(CFLAGS) -o $@.o -c $< -L/usr/lib/x86_64-linux-gnu/libjemalloc.so -ljemalloc $(LDFLAGS)
	$(CC) $(CFLAGS) -o $@ $@.o -L/usr/lib/x86_64-linux-gnu/libjemalloc.so -ljemalloc $(LDFLAGS)

.PHONY: clean
clean:
	rm malloc_pt malloc_pt.o malloc_je malloc_je.o