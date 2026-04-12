.386
.model flat, stdcall
option casemap:none

include \masm32\include\windows.inc
include \masm32\include\kernel32.inc
includelib \masm32\lib\kernel32.lib

.data
    ; ПІБ та група, які будуть виведені в канал
    msg db "Petrenko Petro Petrovich, Group IO-99", 0
    msgLen equ $ - msg

.data?
    hStdOut dd ?
    bytesWritten dd ?

.code
start:
    ; Отримуємо дескриптор стандартного виводу (який буде перенаправленим каналом)
    invoke GetStdHandle, STD_OUTPUT_HANDLE
    mov hStdOut, eax

    ; Перевірка на помилку (INVALID_HANDLE_VALUE = -1)
    .if eax == INVALID_HANDLE_VALUE
        invoke ExitProcess, 1
    .endif

    ; Виводимо текст у канал
    invoke WriteFile, hStdOut, addr msg, msgLen, addr bytesWritten, NULL

    ; Завершуємо процес
    invoke ExitProcess, 0

end start
