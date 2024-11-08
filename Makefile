CC := gcc
DEBUG_CFLAGS := -g -Og -I./include
RELEASE_CFLAGS := -O3 -I./include

SRCS := plreadln.c plreadln_wordmk.c plreadln_intellisense.c
OBJS := $(SRCS:%.c=build/%.o)

.PHONY: all lib test clean

all: CFLAGS := $(RELEASE_CFLAGS)
all: $(OBJS)

lib: all
	ar rv libplreadln.a $(OBJS)

test: CFLAGS := $(DEBUG_CFLAGS)
test: $(OBJS)
	$(CC) $(DEBUG_CFLAGS) example/name.c -o name.out -L. -lplreadln

build/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf build libplreadln.a name.out
