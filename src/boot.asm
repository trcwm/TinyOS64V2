format pe64 dll efi
entry main

include 'uefi.asm'

section '.text' code executable readable

; UEFI calling conventions are: RCX, RDX, R8, R9

disable_watchdog:
    mov rcx, [system_table]
    mov rcx, [rcx + EFI_SYSTEM_TABLE.BootServices]
    mov rbx, [rcx + EFI_BOOT_SERVICES.SetWatchdogTimer]
    xor rdx,rdx ; timeout = 0
    xor r8,r8   ; watchdog code = 0
    xor r9,r9   ; data size = 0
    push r9     ; *watchdogdata = null
    sub rsp, 32-8
    call rbx
    add rsp, 32
    ret

clear_screen:
    push rcx
    push rbx
    mov rcx, [system_table]
    mov rcx, [rcx + EFI_SYSTEM_TABLE.ConOut]
    mov rbx, [rcx + EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.ClearScreen]
    sub rsp, 32
    call rbx
    add rsp, 32
    pop rbx
    pop rcx
    ret

enable_cursor:
    push rdx
    push rcx
    push rbx
    mov rdx, 1
    mov rcx, [system_table]
    mov rcx, [rcx + EFI_SYSTEM_TABLE.ConOut]
    mov rbx, [rcx + EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.EnableCursor]
    sub rsp, 32
    call rbx
    add rsp, 32
    pop rbx
    pop rcx
    pop rdx
    ret

; call with null-terminated unicode string ptr in rdx
print:
    push rcx
    push rbx
    mov rcx, [system_table]
    mov rcx, [rcx + EFI_SYSTEM_TABLE.ConOut]
    mov rbx, [rcx + EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString]
    sub rsp, 32
    call rbx
    add rsp, 32
    pop rbx
    pop rcx
    ret

; call with unicode char in ax
putch:
    push rdx
    lea  rdx, [putchbuf]
    mov  [rdx], ax 
    call print
    pop  rdx
    ret

getch:
.read_key:
    mov rcx, [system_table]                                       ; EFI_SYSTEM_TABLE* rcx
    mov rcx, [rcx + EFI_SYSTEM_TABLE.ConIn]                       ; EFI_SIMPLE_TEXT_INPUT_PROTOCOL* rcx
    mov rbx, [rcx + EFI_SIMPLE_TEXT_INPUT_PROTOCOL.ReadKeyStroke] ; EFI_INPUT_READ_KEY rbx
    mov rdx, input_key                                            ; EFI_INPUT_KEY* rdx
    sub rsp, 32
    call rbx
    add rsp, 32

    mov r8, EFI_NOT_READY
    cmp rax, r8
    je .read_key    

    movzx rax, word [input_key.UnicodeChar]
    ret

main:
    cld
    mov rbp, return_stack_top  ; Initialize return stack
    mov [system_table], rdx

    call disable_watchdog
    call clear_screen
    call enable_cursor

    lea rdx, [TinyOS64_txt]
    call print
.again:
    call getch
    cmp  al,13
    jne .no_newline
    call putch
    mov  al,10
    call putch
.no_newline:
    call putch
    jmp .again

section '.data' readable writable
system_table dq ? ; EFI_SYSTEM_TABLE*


input_key EFI_INPUT_KEY
putchbuf  du 0,0

; strings
TinyOS64_txt du "Hello from the 64-bit UEFI world!",0,0

; stack space
rq $2000
return_stack_top: