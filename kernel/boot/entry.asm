; ThaiOS Kernel Entry Point (x86_64)
; ==================================
; Chiamato dal bootloader (Limine) in long mode con paging attivo.
; Stack pointer non ancora configurato.

[BITS 64]
[GLOBAL _start]
[EXTERN kmain]

; Limine specific revision
LIMINE_ENTRY_POINT equ 0xdead

section .text

_start:
    ; Disabilita interrupts
    cli

    ; Setup stack pointer temporaneo
    mov rsp, stack_top

    ; Salva Boot info pointer (passato da Limine in rdi)
    mov rdi, rbx                 ; Limine passa boot info in rbx
    mov [boot_info_ptr], rbx

    ; Abilita SSE (opzionale, per floating point)
    mov rax, cr0
    and ax, 0xFFFB               ; Clear EM bit
    or ax, 0x02                  ; Set MP bit
    mov cr0, rax
    mov rax, cr4
    or ax, 3 << 9                ; Set OSFXSR and OSXMMEXCPT
    mov cr4, rax

    ; Chiama il kernel C
    call kmain

    ; Se kmain torna, halt
.halt:
    hlt
    jmp .halt

section .bss
align 16
stack_bottom:
    RESB 16384                   ; 16KB stack per bootstrap
stack_top:

section .data
boot_info_ptr:
    DQ 0
