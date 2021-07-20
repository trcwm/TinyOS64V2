#!/bin/sh
qemu-system-x86_64 -bios /usr/share/ovmf/OVMF.fd -hda hdimage.img -net none
#qemu-system-x86_64 -bios /usr/share/OVMF/OVMF_CODE.fd -hda fat:rw:dist -net none