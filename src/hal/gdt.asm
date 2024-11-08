[bits 64]

default rel

extern gdtr

global s_load_gdt

s_load_gdt:

    lgdt [gdtr]

    ; move kernel data offset into data registers
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax 

    ; zero the optional data registers
    xor ax, ax
    mov fs, ax
    mov gs, ax

    ; pop the return instruction pointer from the stack
    pop rax

    ; first push the segment selector we will far return to (0x08 is the code segment)
    push 0x08

    ; then push the return instruction pointer
    push rax

    ; and finally far return
    retfq