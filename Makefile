CC := clang
CFLAGS := -I./include -Wall -Wextra -pedantic -Wno-unused-function
DEBUG_CFLAGS := $(CFLAGS) -g -Og
RELEASE_CFLAGS := $(CFLAGS) -O3 -DNDEBUG

SRCS := plreadln.c plreadln_wordmk.c plreadln_intellisense.c
OBJS := $(SRCS:%.c=build/%.o)

.PHONY: all lib test clean

all: CFLAGS := $(RELEASE_CFLAGS)
all: $(OBJS)

lib: all
	ar rv libplreadln.a $(OBJS)

test: CFLAGS := $(DEBUG_CFLAGS)
test: $(OBJS)
	$(CC) $(DEBUG_CFLAGS) example/echo.c -o echo.out $(OBJS)

build/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf build libplreadln.a echo.out
