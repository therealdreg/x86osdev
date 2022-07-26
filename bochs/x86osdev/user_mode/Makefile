# Makefile for JamesM's kernel tutorials.
# The C and C++ rules are already setup by default.
# The only one that needs changing is the assembler 
# rule, as we use nasm instead of GNU as.
# -
# modified by David Reguera Garcia aka Dreg dreg@fr33project.org
# https://github.com/therealdreg https://www.fr33project.org @therealdreg

SOURCES=make_initrd

CC = gcc
CFLAGS=-m32 

all: $(SOURCES)

clean:
	-rm -f *.o make_initrd
