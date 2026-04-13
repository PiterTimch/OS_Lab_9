#include <windows.h>
#include <stdio.h>
#include <time.h>

#define MENU_CHOICE_1 '1'
#define MENU_CHOICE_2 '2'
#define MENU_CHOICE_3 '3'
#define MENU_EXIT '0'

const char* USER_NAME = "Tymchuck Petro Oleksandrovych";
const char* USER_GROUP = "Group SE-22";
const char* ARG_READER = "reader_mode";

void RunTask1();
void RunTask2();
void RunTask3(bool isReader);
void ShowErrorMessage(const char* action);

void ConsolePrint(const char* text) {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD written;
        WriteConsoleA(hOut, text, (DWORD)lstrlenA(text), &written, NULL);
    }
}

int main(int argc, char* argv[]) {
    if (argc > 1 && lstrcmpA(argv[1], ARG_READER) == 0) {
        RunTask3(true);
        return 0;
    }

    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    if (hIn == INVALID_HANDLE_VALUE) return 1;

    char choice = 0;
    while (choice != MENU_EXIT) {
        ConsolePrint("\n--- WINAPI LAB 9 MENU ---\n");
        ConsolePrint("1. Task 1: Global Atoms\n");
        ConsolePrint("2. Task 2: Anonymous Pipes (console_app.exe)\n");
        ConsolePrint("3. Task 3: Synchronization (Mutex & File)\n");
        ConsolePrint("0. Exit\n");
        ConsolePrint("Your choice: ");

        INPUT_RECORD ir;
        DWORD read;
        while (ReadConsoleInputA(hIn, &ir, 1, &read) && read > 0) {
            if (ir.EventType == KEY_EVENT && ir.Event.KeyEvent.bKeyDown) {
                choice = ir.Event.KeyEvent.uChar.AsciiChar;
                ConsolePrint(&choice);
                ConsolePrint("\n");
                break;
            }
        }

        switch (choice) {
        case MENU_CHOICE_1: RunTask1(); break;
        case MENU_CHOICE_2: RunTask2(); break;
        case MENU_CHOICE_3: RunTask3(false); break;
        case MENU_EXIT: ConsolePrint("Exiting...\n"); break;
        default: ConsolePrint("Invalid choice!\n"); break;
        }
    }

    return 0;
}

void RunTask1() {
    ATOM atom = GlobalAddAtomA(USER_NAME);
    if (atom == 0) {
        ShowErrorMessage("GlobalAddAtomA");
        return;
    }

    ATOM foundAtom = GlobalFindAtomA(USER_NAME);
    if (foundAtom == 0) {
        ShowErrorMessage("GlobalFindAtomA");
        return;
    }

    char msg[256];
    wsprintfA(msg, "Global Atom ID for '%s': %u", USER_NAME, foundAtom);
    MessageBoxA(NULL, msg, "Task 1 (60 points)", MB_YESNOCANCEL);

    GlobalDeleteAtom(atom);
}

void RunTask2() {
    HANDLE hReadPipe, hWritePipe;
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0)) {
        ShowErrorMessage("CreatePipe");
        return;
    }

    if (!SetHandleInformation(hReadPipe, HANDLE_FLAG_INHERIT, 0)) {
        ShowErrorMessage("SetHandleInformation");
        return;
    }

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(STARTUPINFOA));
    si.cb = sizeof(STARTUPINFOA);
    si.hStdError = hWritePipe;
    si.hStdOutput = hWritePipe;
    si.dwFlags |= STARTF_USESTDHANDLES;

    ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

    if (!CreateProcessA("console_app.exe", NULL, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
        char err[256];
        wsprintfA(err, "Failed to start console_app.exe! Error: %lu\nMake sure console_app.exe is in the current directory.", GetLastError());
        MessageBoxA(NULL, err, "Task 2 Error", MB_YESNOCANCEL);
        CloseHandle(hReadPipe);
        CloseHandle(hWritePipe);
        return;
    }

    CloseHandle(hWritePipe);

    char buffer[1024];
    DWORD bytesRead;
    RtlZeroMemory(buffer, 1024);
    if (ReadFile(hReadPipe, buffer, 1024 - 1, &bytesRead, NULL)) {
        MessageBoxA(NULL, buffer, "Task 2 (75 points) - Output", MB_YESNOCANCEL);
    }
    else {
        ShowErrorMessage("ReadFile");
    }

    CloseHandle(hReadPipe);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

void RunTask3(bool isReader) {
    const char* mutexName = "Global\\MyLab9Mutex";
    const char* fileName = "data.txt";

    if (!isReader) {
        HANDLE hMutex = CreateMutexA(NULL, FALSE, mutexName);
        if (hMutex == NULL) {
            ShowErrorMessage("CreateMutexA");
            return;
        }

        char cmdLine[MAX_PATH + 20];
        GetModuleFileNameA(NULL, cmdLine, MAX_PATH);
        lstrcatA(cmdLine, " ");
        lstrcatA(cmdLine, ARG_READER);

        STARTUPINFOA si = { sizeof(si) };
        PROCESS_INFORMATION pi;
        if (!CreateProcessA(NULL, cmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
            ShowErrorMessage("CreateProcessA (Reader)");
            CloseHandle(hMutex);
            return;
        }

        WaitForSingleObject(hMutex, INFINITE);

        HANDLE hFile = CreateFileA(fileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            SYSTEMTIME st;
            GetLocalTime(&st);

            const char* months[] = {
                "January", "February", "March", "April", "May", "June",
                "July", "August", "September", "October", "November", "December"
            };

            const char* monthName = (st.wMonth >= 1 && st.wMonth <= 12) ? months[st.wMonth - 1] : "Unknown";

            DWORD written;
            WriteFile(hFile, monthName, (DWORD)lstrlenA(monthName), &written, NULL);
            CloseHandle(hFile);
        }
        else {
            ShowErrorMessage("CreateFile (Writer)");
        }

        ReleaseMutex(hMutex);

        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        CloseHandle(hMutex);

        MessageBoxA(NULL, "Writer finished its job and child closed.", "Task 3 (Writer)", MB_YESNOCANCEL);

    }
    else {
        HANDLE hMutex = OpenMutexA(MUTEX_ALL_ACCESS, FALSE, mutexName);
        if (hMutex == NULL) {
            Sleep(500);
            hMutex = OpenMutexA(MUTEX_ALL_ACCESS, FALSE, mutexName);
        }

        if (hMutex != NULL) {
            WaitForSingleObject(hMutex, INFINITE);

            HANDLE hFile = CreateFileA(fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if (hFile != INVALID_HANDLE_VALUE) {
                char buf[100];
                DWORD read;
                RtlZeroMemory(buf, 100);
                if (ReadFile(hFile, buf, 99, &read, NULL)) {
                    char display[256];
                    wsprintfA(display, "Reader read month from file: %s", buf);
                    MessageBoxA(NULL, display, "Task 3 (95 points) - Reader", MB_YESNOCANCEL);
                }
                CloseHandle(hFile);
            }

            ReleaseMutex(hMutex);
            CloseHandle(hMutex);
        }
    }
}

void ShowErrorMessage(const char* action) {
    char err[256];
    wsprintfA(err, "WinAPI Error during %s! Code: %lu", action, GetLastError());
    MessageBoxA(NULL, err, "Error Notification", MB_YESNOCANCEL);
}
