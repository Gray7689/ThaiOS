; ThaiOS System Call Entry (x86_64)
; ===================================
; Handler per syscall via MSR (syscall/sysret).
; Riceve: rax=syscall_num, rdi..r15=arg0..arg5

[GLOBAL syscall_entry]
[EXTERN syscall_dispatch]

section .text
syscall_entry:
    ; Salva i registri che useremo
    swapgs
    mov [gs:0x8], rsp          ; Salva user stack
    mov rsp, [gs:0x10]         ; Carica kernel stack

    ; Alloca frame per i 6 argomenti + salvataggi
    push r15
    push r14
    push r13
    push r12
    push rbx

    ; Chiama il dispatcher C: syscall_dispatch(num, arg0..arg5)
    ; rdi=rax(r9), rsi=rdi, rdx=rsi, rcx=rdx, r8=r10, r9=r8
    mov r9, rax                ; syscall_num
    mov rdi, r9                ; arg0
    mov rsi, rdi               ; arg1
    mov rdx, rsi               ; arg2
    mov rcx, rdx               ; arg3
    mov r8, r10                ; arg4
    mov r9, r8                 ; arg5

    call syscall_dispatch

    ; Ripristina
    pop rbx
    pop r12
    pop r13
    pop r14
    pop r15

    ; Ritorna in user-space
    mov rsp, [gs:0x8]
    swapgs
    sysretq
