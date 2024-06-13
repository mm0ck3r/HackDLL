#include "pch.h"
#include <windows.h>
#include <tchar.h>
#include <chrono>
#include <thread>
#include <atomic>
#include <sstream>
#include <iomanip>

HHOOK hHook = NULL;
std::atomic<int> keyPressCount(0);
std::atomic<bool> stopThread(false);
HWND hwndStatus = NULL;

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION)
    {
        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)
        {
            keyPressCount++;
        }
    }
    return CallNextHookEx(hHook, nCode, wParam, lParam);
}

//LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
//{
//    if (nCode == HC_ACTION)
//    {
//        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)
//        {
//            KBDLLHOOKSTRUCT* pKeyboard = (KBDLLHOOKSTRUCT*)lParam;
//
//            // 입력받은 키 값을 문자로 변환
//            BYTE keyboardState[256];
//            GetKeyboardState(keyboardState);
//            WCHAR buffer[2];
//            int result = ToUnicode(pKeyboard->vkCode, pKeyboard->scanCode, keyboardState, buffer, 2, 0);
//
//            // 문자를 DebugView에 출력
//            if (result > 0)
//            {
//                buffer[result] = L'\0'; // null-terminate the string
//                std::wstring debugMessage = L"Key Pressed: ";
//                debugMessage += buffer;
//                OutputDebugString(debugMessage.c_str());
//            }
//
//            keyPressCount++;
//        }
//    }
//    return CallNextHookEx(hHook, nCode, wParam, lParam);
//}


DWORD WINAPI APMThread(LPVOID lpParameter)
{
    using namespace std::chrono;
    auto start = steady_clock::now();
    int totalKeyPressCount = 0;
    int totalSeconds = 0;

    while (!stopThread)
    {
        std::this_thread::sleep_for(seconds(1));
        auto end = steady_clock::now();
        duration<double> elapsed = end - start;
        totalKeyPressCount += keyPressCount.load();
        keyPressCount = 0;
        totalSeconds++;

        // Calculate elapsed time in hh:mm:ss format
        auto elapsed_seconds = duration_cast<seconds>(end - start).count();
        int hours = elapsed_seconds / 3600;
        int minutes = (elapsed_seconds % 3600) / 60;
        int seconds = elapsed_seconds % 60;

        // Calculate average APM
        int apm = (totalKeyPressCount * 60) / totalSeconds;

        // Create the status message
        std::wstringstream ss;
        ss << L"APM: " << apm << L" Time: " << std::setw(2) << std::setfill(L'0') << hours << L":"
            << std::setw(2) << std::setfill(L'0') << minutes << L":" << std::setw(2) << std::setfill(L'0') << seconds;
        std::wstring statusMessage = ss.str();

        SendMessage(hwndStatus, WM_SETTEXT, 0, (LPARAM)statusMessage.c_str());
    }
    return 0;
}

DWORD WINAPI ThreadProc(LPVOID lpParameter)
{
    // Notepad++의 상태 표시줄에 접근하기 위해 핸들 검색
    HWND hwnd = FindWindow(_T("Notepad++"), NULL);
    if (hwnd)
    {
        hwndStatus = FindWindowEx(hwnd, NULL, _T("msctls_statusbar32"), NULL);
        if (hwndStatus)
        {
            // 키보드 훅 설정
            hHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, GetModuleHandle(NULL), 0);
            if (hHook)
            {
                // APM 계산 스레드 시작
                CreateThread(NULL, 0, APMThread, NULL, 0, NULL);
            }
            else
            {
                MessageBox(NULL, _T("Failed to install keyboard hook!"), _T("Error"), MB_ICONERROR);
            }

            // 메시지 루프 시작
            MSG msg;
            while (GetMessage(&msg, NULL, 0, 0))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    }
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        CreateThread(NULL, 0, ThreadProc, NULL, 0, NULL);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        stopThread = true;
        if (hHook)
        {
            UnhookWindowsHookEx(hHook);
        }
        break;
    }
    return TRUE;
}
