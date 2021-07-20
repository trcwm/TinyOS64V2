#!/bin/sh
mkdir -p ./dist/EFI/BOOT
fasm ./src/boot.asm ./dist/EFI/BOOT/BOOTX64.EFI

dd if=/dev/zero of=fat.img bs=1k count=1440
mformat -i fat.img -f 1440 ::
mmd -i fat.img ::/EFI
mmd -i fat.img ::/EFI/BOOT
mcopy -i fat.img ./dist/EFI/BOOT/BOOTX64.EFI ::/EFI/BOOT
mkgpt -o hdimage.img --image-size 4096 --part fat.img --type system
