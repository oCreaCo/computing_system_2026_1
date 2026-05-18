CC=gcc
CFLAGS= -O0 -g3 -ggdb
LDFLAGS=-lpthread

SRCS=pthread_test.c
OBJS=$(SRCS:.c=.o)
TARGETS=$(SRCS:.c=)

.PHONY: all
all: $(TARGETS)

$(TARGETS): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $@.o $(LDFLAGS)

$(OBJS): $(SRCS)
	$(CC) $(CFLAGS) -c $(SRCS) $(LDFLAGS)

.PHONY: clean
clean:
	rm $(TARGETS) $(OBJS)