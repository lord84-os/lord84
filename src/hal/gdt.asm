[bits 64]

default rel

extern gdtr

global s_load_gdt

s_load_gdt:

    cli

    lgdt [gdtr]

    ; move kernel code offset into cs register
    mov ax, 0x08
    mov cs, ax

    ; move kernel data offset into data registers
    mov ax, 0x10
    mov ds, ax
    mov es, ax 

    ; zero the optional data registers
    xor ax, ax
    mov fs, ax
    mov gs, ax

    sti

    ret