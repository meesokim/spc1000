#!/bin/bash
qemu-system-x86_64  -kernel buildroot/output/images/bzImage -initrd buildroot/output/images/rootfs.cpio -append "root=/dev/ram0 rdinit=/sbin/init rw ramdisk_size=131072 console=ttyS0 quiet video=1024x768-32 virtio-gpu.modeset=1" -net nic,model=virtio -net user -display sdl,gl=on -serial stdio -m 1G
