
%macro pushaq_mem 1
    mov qword [rdi + 0], cs
    mov qword [rdi + 8], rsp
    mov qword [rdi + 16], ss

    mov qword [rdi + 24], rax
    mov qword [rdi + 32], rbx
    mov qword [rdi + 40], rcx
    mov qword [rdi + 48], rdx
    mov qword [rdi + 56], rbp
    mov qword [rdi + 64], rsi
    mov qword [rdi + 72], rdi
    mov qword [rdi + 80], r8
    mov qword [rdi + 88], r9
    mov qword [rdi + 96], r10
    mov qword [rdi + 104], r11
    mov qword [rdi + 112], r12
    mov qword [rdi + 120], r13
    mov qword [rdi + 128], r14
    mov qword [rdi + 136], r15

    lea rax, [rip]
    mov qword [rdi + 144], rax

    mov rax, cr2
    mov qword [rdi + 152], rax
%endmacro

global get_stack_frame

; void get_stack_frame(stack_frame *r)
get_stack_frame:
    mov word [rdi + 0], cs
    mov qword [rdi + 8], rsp
    mov word [rdi + 16], ss

    mov qword [rdi + 24], rax
    mov qword [rdi + 32], rbx
    mov qword [rdi + 40], rcx
    mov qword [rdi + 48], rdx
    mov qword [rdi + 56], rbp
    mov qword [rdi + 64], rsi
    mov qword [rdi + 72], rdi
    mov qword [rdi + 80], r8
    mov qword [rdi + 88], r9
    mov qword [rdi + 96], r10
    mov qword [rdi + 104], r11
    mov qword [rdi + 112], r12
    mov qword [rdi + 120], r13
    mov qword [rdi + 128], r14
    mov qword [rdi + 136], r15

    pop rax ; pop IP off the stack
    mov qword [rdi + 144], rax
    push rax

    mov rax, cr2
    mov qword [rdi + 152], rax

    ret


