# Builds libgf.a (field arithmetic and code.txt reader), gfdump, gfencode,
# and the test program gftest.
CC = cc
CFLAGS = -O2 -std=c11 -Wall -Wextra
AR = ar

all: libgf.a gfdump gfencode

libgf.a: gf.o codeio.o
	$(AR) rcs $@ gf.o codeio.o

gf.o: gf.c gf.h
	$(CC) $(CFLAGS) -c gf.c -o $@

codeio.o: codeio.c codeio.h gf.h
	$(CC) $(CFLAGS) -c codeio.c -o $@

gfdump: gfdump.c gf.h libgf.a
	$(CC) $(CFLAGS) gfdump.c libgf.a -o $@

gfencode: gfencode.c codeio.h gf.h libgf.a
	$(CC) $(CFLAGS) gfencode.c libgf.a -o $@

gftest: tests/gftest.c codeio.h gf.h libgf.a
	$(CC) $(CFLAGS) tests/gftest.c libgf.a -o $@

.PHONY: all
