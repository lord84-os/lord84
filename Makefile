BUILD_DIR=build
CC = x86_64-elf-gcc
AS = nasm
LD = x86_64-elf-ld
CFLAGS += -Wall \
    -Wextra \
    -std=gnu11 \
    -ffreestanding \
    -fno-stack-protector \
    -fno-stack-check \
    -fno-lto \
    -fPIE \
    -m64 \
    -march=x86-64 \
    -mno-80387 \
    -mno-mmx \
    -mno-sse \
    -mno-sse2 \
    -mno-red-zone \
	-I src/include \
	-O0 \
	-ggdb3 \
	-g
CDEBUG = -g
LDFLAGS += -m elf_x86_64 \
    -nostdlib \
    -static \
    -pie \
    --no-dynamic-linker \
    -z text \
    -z max-page-size=0x1000 \
    -T linker.ld
NASMFLAGS = -f elf64
dependencies:
	# build limine
	rm -rf limine
	git clone https://github.com/limine-bootloader/limine.git --branch=v8.x-binary --depth=1
	make -C limine
	# clone flanterm
	rm -rf src/flanterm
	
	git clone https://github.com/mintsuki/flanterm src/flanterm
all:
	# make build directory
	mkdir -p $(BUILD_DIR) || true
	# build & link boot and kernel files
	$(CC) -c src/main.c -o $(BUILD_DIR)/main.o $(CFLAGS)
	$(CC) -c src/flanterm/flanterm.c -o $(BUILD_DIR)/flanterm.o $(CFLAGS)
	$(CC) -c src/flanterm/backends/fb.c -o $(BUILD_DIR)/fb.o $(CFLAGS)
	$(CC) -c src/lib/string.c -o $(BUILD_DIR)/string.o $(CFLAGS)
	$(CC) -c src/lib/stdio.c -o $(BUILD_DIR)/stdio.o $(CFLAGS)
	$(CC) -c src/lib/io.c -o $(BUILD_DIR)/io.o $(CFLAGS)
	$(CC) -c src/lib/spinlock.c -o $(BUILD_DIR)/spinlock.o $(CFLAGS)

	$(CC) -c src/hal/gdt.c -o $(BUILD_DIR)/gdt.o $(CFLAGS)
	$(AS) 	 src/hal/gdt.asm -o $(BUILD_DIR)/gdt_asm.o $(NASMFLAGS)
	$(CC) -c src/hal/idt.c -o $(BUILD_DIR)/idt.o $(CFLAGS)
	$(AS) 	 src/hal/idt.asm -o $(BUILD_DIR)/idt_asm.o $(NASMFLAGS)
	$(AS) 	 src/hal/stack_frame.asm -o $(BUILD_DIR)/stack_frame_asm.o $(NASMFLAGS)
	$(CC) -c src/hal/apic.c -o $(BUILD_DIR)/apic.o $(CFLAGS)
	$(CC) -c src/hal/ioapic.c -o $(BUILD_DIR)/ioapic.o $(CFLAGS)
	$(CC) -c src/hal/timer.c -o $(BUILD_DIR)/timer.o $(CFLAGS)
	$(CC) -c src/hal/smp.c -o $(BUILD_DIR)/smp.o $(CFLAGS)
	$(CC) -c src/hal/tsc.c -o $(BUILD_DIR)/tsc.o $(CFLAGS)
	$(CC) -c src/mm/pmm.c -o $(BUILD_DIR)/pmm.o $(CFLAGS)
	$(CC) -c src/mm/vmm.c -o $(BUILD_DIR)/vmm.o $(CFLAGS)
	$(CC) -c src/mm/kmalloc.c -o $(BUILD_DIR)/kmalloc.o $(CFLAGS)
	$(CC) -c src/sys/acpi.c -o $(BUILD_DIR)/acpi.o $(CFLAGS)
	$(CC) -c src/sys/pci.c -o $(BUILD_DIR)/pci.o $(CFLAGS)

	$(CC) -c src/drivers/serial.c -o $(BUILD_DIR)/serial.o $(CFLAGS)
	$(CC) -c src/drivers/pmt.c -o $(BUILD_DIR)/pmt.o $(CFLAGS)
	$(CC) -c src/drivers/ahci.c -o $(BUILD_DIR)/ahci.o $(CFLAGS)

	$(CC) -c src/scheduler/sched.c -o $(BUILD_DIR)/sched.o $(CFLAGS)
	$(AS) 	 src/scheduler/sched.asm -o $(BUILD_DIR)/sched_asm.o $(NASMFLAGS)




	# link everything to an elf
	$(LD) -o $(BUILD_DIR)/lord84.elf  $(BUILD_DIR)/*.o $(LDFLAGS)
	# Create a directory which will be our ISO root.
	mkdir -p iso_root
	# Copy the relevant files over.
	cp -v $(BUILD_DIR)/lord84.elf limine.conf limine/limine-bios.sys \
		limine/limine-bios-cd.bin limine/limine-uefi-cd.bin iso_root/
	# Create the EFI boot tree and copy Limine's EFI executables over.
	mkdir -p iso_root/EFI/BOOT
	cp -v limine/BOOTX64.EFI iso_root/EFI/BOOT/
	cp -v limine/BOOTIA32.EFI iso_root/EFI/BOOT/
	# Create the bootable ISO.
	xorriso -as mkisofs -b limine-bios-cd.bin \
			-no-emul-boot -boot-load-size 4 -boot-info-table \
			--efi-boot limine-uefi-cd.bin \
			-efi-boot-part --efi-boot-image --protective-msdos-label \
			iso_root -o $(BUILD_DIR)/lord84.iso
	# Install Limine stage 1 and 2 for legacy BIOS boot.
	./limine/limine bios-install $(BUILD_DIR)/lord84.iso
disk:
	dd if=/dev/zero of=disk.img bs=1M count=128
elftest:
	$(CC) src/elf/elftest.c -o $(BUILD_DIR)/elftest -ffreestanding -Isrc/include -static -fPIE -nostdlib
