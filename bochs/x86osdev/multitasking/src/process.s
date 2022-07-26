; https://wiki.osdev.org/James_Molloy%27s_Tutorial_Known_Bugs
; Here we:
; * Stop interrupts so we don't get interrupted.
; * Temporarily put the new EIP location in ECX.
; * Temporarily put the new page directory's physical address in EAX.
; * Set the base and stack pointers
; * Set the page directory
; * Put a dummy value (0x12345) in EAX so that above we can recognize that we've just
;   switched task.
; * Restart interrupts. The STI instruction has a delay - it doesn't take effect until after
;   the next instruction.
; * Jump to the location in ECX (remember we put the new EIP in there).

[GLOBAL perform_task_switch]
perform_task_switch:
     cli;
     mov ecx, [esp+4]   ; EIP
     mov eax, [esp+8]   ; physical address of current directory
     mov ebp, [esp+12]  ; EBP
     mov esp, [esp+16]  ; ESP
     mov cr3, eax       ; set the page directory
     mov eax, 0x12345   ; magic number to detect a task switch
     sti;
     jmp ecx
	 
[GLOBAL read_eip]
read_eip:
    pop eax                     ; Get the return address
    jmp eax                     ; Return. Can't use RET because return
                                ; address popped off the stack. 

[GLOBAL copy_page_physical]
copy_page_physical:
    push ebx              ; According to __cdecl, we must preserve the contents of EBX.
    pushf                 ; push EFLAGS, so we can pop it and reenable interrupts
                          ; later, if they were enabled anyway.
    cli                   ; Disable interrupts, so we aren't interrupted.
                          ; Load these in BEFORE we disable paging!
    mov ebx, [esp+12]     ; Source address
    mov ecx, [esp+16]     ; Destination address
  
    mov edx, cr0          ; Get the control register...
    and edx, 0x7fffffff   ; and...
    mov cr0, edx          ; Disable paging.
  
    mov edx, 1024         ; 1024*4bytes = 4096 bytes
  
.loop:
    mov eax, [ebx]        ; Get the word at the source address
    mov [ecx], eax        ; Store it at the dest address
    add ebx, 4            ; Source address += sizeof(word)
    add ecx, 4            ; Dest address += sizeof(word)
    dec edx               ; One less word to do
    jnz .loop             
  
    mov edx, cr0          ; Get the control register again
    or  edx, 0x80000000   ; and...
    mov cr0, edx          ; Enable paging.
  
    popf                  ; Pop EFLAGS back.
    pop ebx               ; Get the original value of EBX back.
    ret
