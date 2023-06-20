.code
public get_base
get_base proc
    mov rax, qword ptr gs:[18h]
    mov rcx, [rax+38h]
    mov rax, 0fffffffffffff000h
    and rax, [rcx+4h]
    jmp while_begin
    search_begin:
    add rax, 0fffffffffffff000h
    while_begin: 
    xor ecx, ecx
    jmp search_cmp
    search_next: 
    add rcx, 1
    cmp rcx, 0ff9h
    jz search_begin
    search_cmp:  
    cmp byte ptr[rax+rcx], 48h
    jnz search_next
    cmp byte ptr[rax+rcx+1], 8dh
    jnz search_next
    cmp byte ptr[rax+rcx+2], 1dh
    jnz search_next
    cmp byte ptr[rax+rcx+6], 0ffh
    jnz search_next
    mov r8d,[rax+rcx+3]
    lea edx,[rcx+r8]
    add edx, eax
    add edx, 7
    test edx, 0fffh
    jnz search_next
    mov rdx, 0ffffffff00000000h
    and rdx, rax
    add r8d, eax
    lea eax,[rcx+r8]
    add eax, 7
    or rax, rdx
    ret
get_base endp
end