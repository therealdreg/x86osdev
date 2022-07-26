#!/bin/bash
# -
# by David Reguera Garcia aka Dreg dreg@fr33project.org
# https://github.com/therealdreg https://www.fr33project.org @therealdreg

rm -f floppy.img
dd if=/dev/zero of=floppy.img bs=512 count=2880
dd if=src/multi_core_startup.bin of=floppy.img bs=512 conv=notrunc
