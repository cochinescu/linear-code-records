CC = cc
CFLAGS = -O2 -std=c11 -Wall -Wextra
FIELDS = ../fields
LIBGF = $(FIELDS)/libgf.a

all: libmindist.a mindist

# The field library is built by its own Makefile (which skips the work
# when it is up to date), so a fresh copy builds with `make -C mindist`.
$(LIBGF): FORCE
	$(MAKE) -C $(FIELDS) libgf.a

FORCE:

mindist.o: mindist.c mindist.h $(FIELDS)/gf.h
	$(CC) $(CFLAGS) -c mindist.c -o mindist.o

libmindist.a: mindist.o
	ar rcs libmindist.a mindist.o

mindist: mindist_main.c mindist.h libmindist.a $(LIBGF) $(FIELDS)/gf.h $(FIELDS)/codeio.h
	$(CC) $(CFLAGS) mindist_main.c libmindist.a $(LIBGF) -o mindist

tests/test_mindist: tests/test_mindist.c mindist.h libmindist.a $(LIBGF)
	$(CC) $(CFLAGS) tests/test_mindist.c libmindist.a $(LIBGF) -o tests/test_mindist

check: all tests/test_mindist
	bash run-tests.sh

.PHONY: all check FORCE
