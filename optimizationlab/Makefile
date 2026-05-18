CC=gcc
CFLAGS= -O0 -g3 -ggdb -fno-omit-frame-pointer -w
LDFLAGS=-L/usr/lib/x86_64-linux-gnu -ljemalloc -lpthread
# LDFLAGS=-lpthread

SRCS= grep-log.c log-gen.c
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