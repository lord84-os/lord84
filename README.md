# SFB25

SFB25 is an operating system for the x86_64 architecture. It is written in C. Licensed under MIT


## How to build

First run `make dependencies` to clone and build Limine and Flanterm

Then run `make all` - make sure to adjust the `CC`, `AS` and `LD` flags to match your cross-compiling toolchain

in the `build` folder you should have a `SFB25.iso` file.

To try out SFB25 you can use QEMU:

`qemu-system-x86_64 build/SFB25.iso -m 512M`



## External projects

- [Limine bootloader](https://github.com/limine-bootloader/limine) for the bootloader
- [Flanterm](https://github.com/mintsuki/flanterm) for the terminal
- [uACPI](https://github.com/uacpi/uacpi) for the AML interpreter