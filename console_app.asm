.386
.model flat, stdcall
option casemap:none

; --- WinAPI declarations (without MASM32) ---
STD_OUTPUT_HANDLE   EQU -11
NULL                EQU 0

ExitProcess  PROTO STDCALL :DWORD
GetStdHandle PROTO STDCALL :DWORD
WriteFile    PROTO STDCALL :DWORD, :DWORD, :DWORD, :DWORD, :DWORD

includelib kernel32.lib

.data
    msg     db "Tymchuck Petro Oleksandrovych, Group SE-22", 13, 10, 0
    msgLen  equ ($ - msg - 1)

.data?
    hStdOut     dd ?
    bytesWritten dd ?

.code
start:
    invoke GetStdHandle, STD_OUTPUT_HANDLE
    mov hStdOut, eax

    cmp eax, -1
    je  done

    invoke WriteFile, hStdOut, OFFSET msg, msgLen, OFFSET bytesWritten, NULL

done:
    invoke ExitProcess, 0

end start
