CC=gcc
CFLAGS=-std=c99 -Wall
LIBS=-lm
OUTDIR=out

.PHONY: all debug release format clean clean-all

all: roadrunner clean
	mkdir -p $(OUTDIR)

debug: CFLAGS += -DDEBUG -g
debug: all

release: CFLAGS += -O2
release: all

roadrunner: main.o vc.o myvc.o
	$(CC) $(CFLAGS) main.o vc.o myvc.o -o roadrunner $(LIBS)

.c.o: vc.h
	$(CC) $(CFLAGS) -c $<

format:
	clang-format -style=LLVM -i *.c

clean:
	rm -f *.o

clean-all:
	rm -f *.o roadrunner $(OUTDIR)/*
