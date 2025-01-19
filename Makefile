# Build directory and tools
BUILD_DIR=build
CC=x86_64-elf-gcc
AS=nasm
LD=x86_64-elf-ld

# Compiler and linker flags
CFLAGS+=-Wall -Wextra -std=gnu11 -ffreestanding -fno-stack-protector -fno-stack-check \
        -fno-lto -fPIE -m64 -march=x86-64 -mno-80387 -mno-mmx -mno-sse -mno-sse2 \
        -mno-red-zone -I src/include -O0 -g
LDFLAGS+=-m elf_x86_64 -nostdlib -static -pie --no-dynamic-linker -z text \
        -z max-page-size=0x1000 -T linker.ld
NASMFLAGS=-f elf64

# Source files
UACPI_SRC=src/uacpi/default_handlers.c \
          src/uacpi/event.c \
          src/uacpi/interpreter.c \
          src/uacpi/io.c \
          src/uacpi/mutex.c \
          src/uacpi/namespace.c \
          src/uacpi/notify.c \
          src/uacpi/opcodes.c \
          src/uacpi/opregion.c \
          src/uacpi/osi.c \
          src/uacpi/registers.c \
          src/uacpi/resources.c \
          src/uacpi/shareable.c \
          src/uacpi/sleep.c \
          src/uacpi/stdlib.c \
          src/uacpi/tables.c \
          src/uacpi/types.c \
          src/uacpi/uacpi.c \
          src/uacpi/utilities.c

# Transform source files to object files
UACPI_O=$(UACPI_SRC:src/uacpi/%.c=$(BUILD_DIR)/uacpi_%.o)

# Other source files
OBJS=$(BUILD_DIR)/main.o \
     $(BUILD_DIR)/flanterm.o \
     $(BUILD_DIR)/fb.o \
     $(BUILD_DIR)/string.o \
     $(BUILD_DIR)/stdio.o \
     $(BUILD_DIR)/io.o \
     $(BUILD_DIR)/spinlock.o \
     $(BUILD_DIR)/gdt.o \
     $(BUILD_DIR)/gdt_asm.o \
     $(BUILD_DIR)/idt.o \
     $(BUILD_DIR)/idt_asm.o \
     $(BUILD_DIR)/apic.o \
     $(BUILD_DIR)/ioapic.o \
     $(BUILD_DIR)/timer.o \
     $(BUILD_DIR)/smp.o \
     $(BUILD_DIR)/pmm.o \
     $(BUILD_DIR)/vmm.o \
     $(BUILD_DIR)/kmalloc.o \
     $(BUILD_DIR)/acpi.o \
     $(BUILD_DIR)/pci.o \
     $(BUILD_DIR)/serial.o \
     $(BUILD_DIR)/pmt.o \
     $(BUILD_DIR)/ahci.o

# All object files
ALL_OBJS=$(UACPI_O) $(OBJS)

# Default target
all: $(BUILD_DIR) $(ALL_OBJS)
	$(LD) -o $(BUILD_DIR)/lord84.elf $(ALL_OBJS) $(LDFLAGS)
	mkdir -p iso_root
	cp -v $(BUILD_DIR)/lord84.elf limine.conf limine/limine-bios.sys \
	    limine/limine-bios-cd.bin limine/limine-uefi-cd.bin iso_root/
	mkdir -p iso_root/EFI/BOOT

	cp -v limine/BOOTX64.EFI iso_root/EFI/BOOT/
	cp -v limine/BOOTIA32.EFI iso_root/EFI/BOOT/
	
	xorriso -as mkisofs -b limine-bios-cd.bin \
	    -no-emul-boot -boot-load-size 4 -boot-info-table \
	    --efi-boot limine-uefi-cd.bin \
	    -efi-boot-part --efi-boot-image --protective-msdos-label \
	    iso_root -o $(BUILD_DIR)/lord84.iso
	./limine/limine bios-install $(BUILD_DIR)/lord84.iso

# Build directory creation
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Rule to build UACPI object files
$(BUILD_DIR)/uacpi_%.o: src/uacpi/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Rule to build other object files
$(BUILD_DIR)/%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Rule for assembly files
$(BUILD_DIR)/%.o: src/%.asm
	$(AS) $< -o $@ $(NASMFLAGS)

# Dependencies setup
dependencies:
	rm -rf limine
	git clone https://github.com/limine-bootloader/limine.git --branch=v8.x-binary --depth=1
	make -C limine
	rm -rf src/flanterm
	git clone https://github.com/mintsuki/flanterm src/flanterm

# Create a blank disk
disk:
	dd if=/dev/zero of=disk.img bs=1M count=128

# Clean build
clean:
	rm -rf $(BUILD_DIR) iso_root disk.img
	rm -rf limine
