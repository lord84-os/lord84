# lord84

lord84 is an operating system for the x86_64 architecture. It is written in C. Licensed under MIT


## How to build

First run `make dependencies` to build Limine and Flanterm

Then run `make all` - make sure to adjust the `CC`, `AS` and `LD` flags to match your cross-compiling toolchain

in the `build` folder you should have a `lord84.iso` file.

To try out lord84 you can use QEMU:

`qemu-system-x86_64 build/lord84.iso -m 512M`



## External projects

- [Limine bootloader](https://github.com/limine-bootloader/limine) for the bootloader
- [Flanterm](https://github.com/mintsuki/flanterm) for the terminal