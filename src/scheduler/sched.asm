default rel

global switch_context

%macro save_context 0
    push rbx
    push rsp
    push rbp
    push r12
    push r13
    push r14
    push r15
%endmacro

%macro load_context 0
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbp
    pop rsp
    pop rbx
%endmacro

; Switch context from old to new
; void switch_context(context *old, context* new);
;                       rdi             rsi
switch_context:

    save_context

    mov rdi, rsp
    mov rsp, rsi
    
    load_context
    
    retfq


    







