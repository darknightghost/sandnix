# What is sandnix

Sandix is a simple unix-like kernel. It has not been finished. Just for learning.
Arm has not been support now, it will be supported in the future.

# How to compile
- Use `./skconfig` to change compile options.<br />
- Use `./configure` to get make file.<br />
- Use `make all` to compile.<br />

>Multiboot 2 header only support X86 architecture.
Uboot header only support arm architecture.

# How to install

## X86

- 1.Install grub2.
- 2.Copy multiboot2 image(`./bin/x86/sandnix/kernel.multiboot2`) to `/boot/` then make a ramdisk image(VFS is not finished so you can use any file as an ramdis file) and copy it to `/boot/`.
- 3.Add a menuentery into grub/cfg, here is an example(`sandnix` is the kernel file, `sandnix-initrd` is the ramdisk file.):

```
menuentry 'Sandnix 0.0.2' {
        multiboot2 (hd0,3)/sandnix root=/dev/sda1 driver_init=/bin/dinit debug=on
        module2 (hd0,3)/sandnix-initrd
}
```

# Something else
- [Project plan](./doc/project-plan.md)
- [Document for kernel developer](./doc/kernel-devel-doc/kernel-devel-doc.md)
