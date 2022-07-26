#!/bin/bash
# -
# modified by David Reguera Garcia aka Dreg dreg@fr33project.org
# https://github.com/therealdreg https://www.fr33project.org @therealdreg

sudo mkdir /mnt2
sudo /sbin/losetup /dev/loop0 floppy.img
sudo mount /dev/loop0 /mnt2
sudo cp src/kernel /mnt2/kernel
sudo umount /dev/loop0
sudo /sbin/losetup -d /dev/loop0
