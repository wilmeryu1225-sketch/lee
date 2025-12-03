#include "main.h"

void BindToHostConsole() {

    HANDLE hHostConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hHostConsole == INVALID_HANDLE_VALUE) {
        OutputDebugStringA("主程序无有效控制台句柄\n");
        return;
    }

    CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
    if (!GetConsoleScreenBufferInfo(hHostConsole, &consoleInfo)) {
        OutputDebugStringA("无法访问主程序控制台信息\n");
        return;
    }

    const char* testMsg = "成功关联到主程序控制台！\n";
    DWORD bytesWritten;
    if (WriteFile(hHostConsole, testMsg, strlen(testMsg), &bytesWritten, NULL)) {
        OutputDebugStringA("通过控制台 API 输出成功\n");
    }
    else {
        OutputDebugStringA("通过控制台 API 输出失败\n");
    }
}

BOOL WINAPI DllMain(HMODULE hModule, DWORD  fdwReason, LPVOID lpReserved)
{
    if (fdwReason == DLL_PROCESS_ATTACH) {
        BindToHostConsole();
        ZeroMemory(&g_ctx, sizeof(LC_MAIN_CONTEXT));
        InitializeCriticalSection(&g_ctx.Lock);
    }
    if (fdwReason == DLL_PROCESS_DETACH) {
        LcCloseAll();
        DeleteCriticalSection(&g_ctx.Lock);
        ZeroMemory(&g_ctx, sizeof(LC_MAIN_CONTEXT));
    }
    return TRUE;
}

