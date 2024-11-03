#include "idt.h"
#include <stdio.h>
idt_descriptor idt[256] = {0};

idt_register idtr = {sizeof(idt), (uint64_t)(&idt)};

extern void s_isr0();
extern void s_isr1();
extern void s_isr2();
extern void s_isr3();
extern void s_isr4();
extern void s_isr5();
extern void s_isr6();
extern void s_isr7();
extern void s_isr8();
extern void s_isr9();
extern void s_isr10();
extern void s_isr11();
extern void s_isr12();
extern void s_isr13();
extern void s_isr14();
extern void s_isr15();
extern void s_isr16();
extern void s_isr17();
extern void s_isr18();
extern void s_isr19();
extern void s_isr20();
extern void s_isr21();
extern void s_isr22();
extern void s_isr23();
extern void s_isr24();
extern void s_isr25();
extern void s_isr26();
extern void s_isr27();
extern void s_isr28();
extern void s_isr29();
extern void s_isr30();
extern void s_isr31();

extern void s_isr44();

extern void s_isr255();

extern void s_load_idt();

void set_idt_descriptor(uint8_t vector, void *base, uint8_t flags){
    idt[vector].offset_low = ((uint64_t)base & 0xffff);

    idt[vector].segment_sel = 0x08; // kernel code segment

    idt[vector].ist = 0;
    idt[vector].attributes = flags;

    idt[vector].offset_high = ((uint64_t)base >> 16) & 0xffff;
    idt[vector].offset_higher = ((uint64_t)base >> 32) & 0xffffffff;

    idt[vector].reserved = 0;
}

void set_idt(void){
    
    set_idt_descriptor(0, s_isr0, 0x8E);
    set_idt_descriptor(1, s_isr1, 0x8E);
    set_idt_descriptor(2, s_isr2, 0x8E);
    set_idt_descriptor(3, s_isr3, 0x8E);
    set_idt_descriptor(4, s_isr4, 0x8E);
    set_idt_descriptor(5, s_isr5, 0x8E);
    set_idt_descriptor(6, s_isr6, 0x8E);
    set_idt_descriptor(7, s_isr7, 0x8E);
    set_idt_descriptor(8, s_isr8, 0x8E);
    set_idt_descriptor(9, s_isr9, 0x8E);
    set_idt_descriptor(10, s_isr10, 0x8E);
    set_idt_descriptor(11, s_isr11, 0x8E);
    set_idt_descriptor(12, s_isr12, 0x8E);
    set_idt_descriptor(13, s_isr13, 0x8E);
    set_idt_descriptor(14, s_isr14, 0x8E);
    set_idt_descriptor(15, s_isr15, 0x8E);
    set_idt_descriptor(16, s_isr16, 0x8E);
    set_idt_descriptor(17, s_isr17, 0x8E);
    set_idt_descriptor(18, s_isr18, 0x8E);
    set_idt_descriptor(19, s_isr19, 0x8E);
    set_idt_descriptor(20, s_isr20, 0x8E);
    set_idt_descriptor(21, s_isr21, 0x8E);
    set_idt_descriptor(22, s_isr22, 0x8E);
    set_idt_descriptor(23, s_isr23, 0x8E);
    set_idt_descriptor(24, s_isr24, 0x8E);
    set_idt_descriptor(25, s_isr25, 0x8E);
    set_idt_descriptor(26, s_isr26, 0x8E);
    set_idt_descriptor(27, s_isr27, 0x8E);
    set_idt_descriptor(28, s_isr28, 0x8E);
    set_idt_descriptor(29, s_isr29, 0x8E);
    set_idt_descriptor(30, s_isr30, 0x8E);
    set_idt_descriptor(31, s_isr31, 0x8E);

    set_idt_descriptor(44, s_isr44, 0x8E);
    set_idt_descriptor(255, s_isr255, 0x8E);

    s_load_idt();
}

char *exception_messages[] =
{
    "Division Error",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "Device not available",

    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "x87 FPU Floating-point error",

    "Alignment Check",
    "Machine Check",
    "SIMD Floating-point exception",
    "Virtualization exception",
    "Control Protection",
    "Reserved",
    "Reserved",

    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};

void interrupt_handler(interrupt_frame *r){
    if(r->int_no < 32){
        kprintf("\nOh no! Received interrupt {d}, '{s}'. Below is the provided stack frame{n}{n}", r->int_no, exception_messages[r->int_no]);
        kprintf("error code 0x{xn}", r->err);
        kprintf("rax 0x{x} | rbx 0x{x} | rcx 0x{x} | rdx 0x{xn}", r->rax, r->rbx, r->rcx, r->rdx);
        kprintf("rdi 0x{x} | rsi 0x{x} | rbp 0x{xn}", r->rdi, r->rsi, r->rbp);
        kprintf("r8 0x{x} | r9 0x{x} | r10 0x{x} | r11 0x{x} | r12 0x{x} | r13 0x{x} | r14 0x{x} | r15 0x{xn}", r->r8, r->r9, r->r10, r->r11, r->r12, r->r13, r->r14, r->r15);
        kprintf("rip 0x{x} | cs 0x{x} | ss 0x{x} | rsp 0x{x} | rflags 0x{xn}", r->rip, r->cs, r->ss, r->rsp, r->rflags);
        asm("cli; hlt");
        for(;;);
    }

    if(r->int_no == 255){
        kprintf("The kernel has been killed. :(\n\n");
        kprintf("rax 0x{x} | rbx 0x{x} | rcx 0x{x} | rdx 0x{xn}", r->rax, r->rbx, r->rcx, r->rdx);
        kprintf("rdi 0x{x} | rsi 0x{x} | rbp 0x{xn}", r->rdi, r->rsi, r->rbp);
        kprintf("r8 0x{x} | r9 0x{x} | r10 0x{x} | r11 0x{x} | r12 0x{x} | r13 0x{x} | r14 0x{x} | r15 0x{xn}", r->r8, r->r9, r->r10, r->r11, r->r12, r->r13, r->r14, r->r15);
        kprintf("rip 0x{x} | cs 0x{x} | ss 0x{x} | rsp 0x{x} | rflags 0x{xn}", r->rip, r->cs, r->ss, r->rsp, r->rflags);

        asm("cli; hlt");
        for(;;);
    }

    if(r->int_no == 44){
        kprintf("serial\n");
    }
}