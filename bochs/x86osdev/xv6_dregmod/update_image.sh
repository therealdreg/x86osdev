#!/bin/bash
# -
# modified by David Reguera Garcia aka Dreg dreg@fr33project.org
# https://github.com/therealdreg https://www.fr33project.org @therealdreg

sudo rm -f fs.img
sudo rm -f xv6.img
sudo rm -f kernel.sym
sudo cp src/fs.img .
sudo cp src/xv6.img .
sudo cp src/kernel.sym .
echo "just copy xv6.img and fs.img to bochs/ dir, after that remove bochs/floppy.img and run bochs"
