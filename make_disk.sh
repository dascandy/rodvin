#!/bin/bash

rm -f disk.img
rm -f disk.img_
dd if=/dev/zero of=disk.img_ bs=1M count=256
mkfs.vfat -F 32 disk.img_ 
out/tools/writebootsect disk.img_ out/boot/bs.bin 
sudo mount -o loop disk.img_ mnt/
sudo cp out/boot/bootldr.bin mnt/
sudo cp out/x8664/kernel.elf mnt/kernel64.elf
sudo cp out/i386/kernel.elf mnt/kernel32.elf
sudo cp out/rpi/kernel.img mnt/kernel.img
sudo cp staging/* mnt/
sleep 1
sudo umount mnt
mv disk.img_ disk.img


