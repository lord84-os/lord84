default rel

global switch_context

%macro save_context 0
    push rsp
    push rbp
    push rbx
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
    pop rbx
    pop rbp
    pop rsp
%endmacro

; Switch context from old to new
; void switch_context(context *old, context* new);
;                       rdi             rsi
switch_context:

    save_context

    mov rsp, rdi
    mov rsi, rsp
    
    load_context
    
    retfq


    







