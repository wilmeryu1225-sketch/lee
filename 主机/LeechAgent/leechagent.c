//	leechagent.c : Implementation the LeechAgent service related functionality.
//
// (c) Ulf Frisk, 2018-2025
// Author: Ulf Frisk, pcileech@frizk.net
//
#include "leechagent.h"
#include "leechagent_svc.h"
#include "leechagent_rpc.h"
#include "leechagent_proc.h"
#include "leechrpc.h"
#include "util.h"
#include <stdio.h>
#include <strsafe.h>
#define SECURITY_WIN32
#include <security.h>
#include <windows.h>
#include <stdio.h>
#include "dll.h"
#include "peloader.h"

int Main()
{
    //AllocConsole();

    //freopen("CONOUT$", "w+t", stdout);

    //freopen("CONIN$", "r+t", stdin);

    HMODULE Module = PeLoader_LoadLibrary(DllData, sizeof(DllData), 0, NULL, NULL);

    if (!Module)
    {
        MessageBoxA(NULL, "Leechcore Initialization failed", "", MB_OK | MB_ICONHAND);
        ExitProcess(0);
    }

    LcCreateEx = (LCCREATEEX)PeLoader_GetProcAddress(Module, "LcCreateEx");
    if (!LcCreateEx)
    {
        MessageBoxA(NULL, "LcCreateEx failed", "", MB_OK | MB_ICONHAND);
        ExitProcess(0);
    }
    LcReadScatter = (LCREADSCATTER)PeLoader_GetProcAddress(Module, "LcReadScatter");
    if (!LcReadScatter)
    {
        MessageBoxA(NULL, "LcReadScatter failed", "", MB_OK | MB_ICONHAND);
        ExitProcess(0);
    }
    LcWriteScatter = (LCWRITESCATTER)PeLoader_GetProcAddress(Module, "LcWriteScatter");
    if (!LcWriteScatter)
    {
        MessageBoxA(NULL, "LcWriteScatter failed", "", MB_OK | MB_ICONHAND);
        ExitProcess(0);
    }
    LcClose = (LCCLOSE)PeLoader_GetProcAddress(Module, "LcClose");
    if (!LcClose)
    {
        MessageBoxA(NULL, "LcClose failed", "", MB_OK | MB_ICONHAND);
        ExitProcess(0);
    }
    LcSetOption = (LCSETOPTION)PeLoader_GetProcAddress(Module, "LcSetOption");
    if (!LcSetOption)
    {
        MessageBoxA(NULL, "LcSetOption failed", "", MB_OK | MB_ICONHAND);
        ExitProcess(0);
    }
    LcGetOption = (LCGETOPTION)PeLoader_GetProcAddress(Module, "LcGetOption");
    if (!LcGetOption)
    {
        MessageBoxA(NULL, "LcGetOption failed", "", MB_OK | MB_ICONHAND);
        ExitProcess(0);
    }
    LcCommand = (LCCOMMAND)PeLoader_GetProcAddress(Module, "LcCommand");
    if (!LcCommand)
    {
        MessageBoxA(NULL, "LcCommand failed", "", MB_OK | MB_ICONHAND);
        ExitProcess(0);
    }

    LEECHSVC_CONFIG cfg = { 0 };

    g_LeechAgent_IsService = FALSE;

    cfg.fInteractive = TRUE;

    cfg.fInsecure = TRUE;

    cfg.fMSRPC = TRUE;

    cfg.fgRPC = FALSE;

    LeechSvc_Interactive(&cfg);

    return 0;
}

BOOL DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        HANDLE hThread = CreateThread(NULL, 0, Main, NULL, 0, NULL);
    }

    return TRUE;
}
