CC=gcc
CFLAGS=-std=c99 -Wall
LIBS=-lm
OUTDIR=out
OBJS=main.o vc.o myvc.o utils.o shape.o color.o sign.o edges.o

.PHONY: all debug release format tidy clean clean-all

all: roadrunner clean
	mkdir -p $(OUTDIR)

debug: CFLAGS += -DDEBUG -g
debug: all

release: CFLAGS += -O2
release: all

roadrunner: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o roadrunner $(LIBS)

.c.o: vc.h
	$(CC) $(CFLAGS) -c $<

format:
	clang-format -style=LLVM -i *.c

tidy:
	clang-tidy *.[ch] -checks=* -- -std=c99

clean:
	rm -f $(OBJS)

clean-all:
	rm -f $(OBJS) roadrunner $(OUTDIR)/*
