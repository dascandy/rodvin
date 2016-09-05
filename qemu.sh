#!/bin/sh

#qemu-system-arm -kernel out/rpi/kernel.img -initrd disk2.img -cpu arm1176 -m 512 -M raspi -serial stdio
qemu-system-arm -kernel out/rpi/kernel.img -drive if=sd,file=disk.img,format=raw -cpu arm1176 -m 512 -M raspi -serial stdio


