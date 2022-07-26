# Makefile for JamesM's kernel tutorials.
# The C and C++ rules are already setup by default.
# The only one that needs changing is the assembler 
# rule, as we use nasm instead of GNU as.
# -
# modified by David Reguera Garcia aka Dreg dreg@fr33project.org
# https://github.com/therealdreg https://www.fr33project.org @therealdreg

SOURCES=boot.o main.o monitor.o common.o descriptor_tables.o isr.o interrupt.o gdt.o timer.o

CC = gcc
CFLAGS=-m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector
LDFLAGS=-m elf_i386 -Tlink.ld
ASFLAGS=-f elf32 -felf

all: $(SOURCES) link

clean:
	-rm -f *.o kernel

link:
	ld $(LDFLAGS) -o kernel $(SOURCES)

.s.o:
	nasm $(ASFLAGS) $<
