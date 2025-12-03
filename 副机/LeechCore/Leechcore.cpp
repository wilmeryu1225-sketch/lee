#include "main.h"

ULONG64 LcCallStart()
{
    ULONG64 tmNow;
    QueryPerformanceCounter((PLARGE_INTEGER)&tmNow);
    return tmNow;
}

VOID LcLockAcquire(_In_ PLC_CONTEXT ctxLC)
{
    if (!ctxLC->fMultiThread) { EnterCriticalSection(&ctxLC->Lock); }
}

VOID LcLockRelease(_In_ PLC_CONTEXT ctxLC)
{
    if (!ctxLC->fMultiThread) { LeaveCriticalSection(&ctxLC->Lock); }
}

VOID LcCallEnd(_In_ PLC_CONTEXT ctxLC, _In_ DWORD fId, _In_ ULONG64 tmCallStart)
{
    ULONG64 tmNow;
    QueryPerformanceCounter((PLARGE_INTEGER)&tmNow);
    InterlockedIncrement64((LONG64 volatile*)&ctxLC->CallStat.Call[fId].c);
    InterlockedAdd64((LONG64 volatile*)&ctxLC->CallStat.Call[fId].tm, tmNow - tmCallStart);
}

VOID LcClose(_In_opt_ _Post_ptr_invalid_ HANDLE hLC)
{
    PLC_CONTEXT ctxParent;
    PLC_CONTEXT ctxLC = (PLC_CONTEXT)hLC;
    if (!ctxLC || (ctxLC->version != LC_CONTEXT_VERSION)) { return; }
    EnterCriticalSection(&g_ctx.Lock);
    if (0 == --ctxLC->dwHandleCount) {
        if (g_ctx.FLink == ctxLC) {
            g_ctx.FLink = ctxLC->FLink;
        }
        else {
            ctxParent = (PLC_CONTEXT)g_ctx.FLink;
            while (ctxParent) {
                if (ctxParent->FLink == ctxLC) {
                    ctxParent->FLink = ctxLC->FLink;
                    break;
                }
                ctxParent = (PLC_CONTEXT)ctxParent->FLink;
            }
        }
        LcLockAcquire(ctxLC);
        LcReadContigious_Close(ctxLC);
        if (ctxLC->pfnClose) { ctxLC->pfnClose(ctxLC); }
        LcLockRelease(ctxLC);
        ctxLC->version = 0;
        DeleteCriticalSection(&ctxLC->Lock);
        if (ctxLC->hDeviceModule) { FreeLibrary((HMODULE)ctxLC->hDeviceModule); }
        LocalFree(ctxLC->pMemMap);
        LocalFree(ctxLC);
    }
    LeaveCriticalSection(&g_ctx.Lock);
}

VOID LcCloseAll()
{
    EnterCriticalSection(&g_ctx.Lock);
    while (g_ctx.FLink) {
        LcClose(g_ctx.FLink);
    }
    LeaveCriticalSection(&g_ctx.Lock);
}

VOID LcMemFree(_Frees_ptr_opt_ PVOID pv)
{
    LocalFree(pv);
}

VOID LcCreate_FetchDeviceParameter(_Inout_ PLC_CONTEXT ctxLC)
{
    PLC_DEVICE_PARAMETER_ENTRY pe;
    CHAR szDevice[MAX_PATH] = { 0 };
    LPSTR szDelim, szParameters, szToken, szTokenContext = NULL;
    memcpy(szDevice, ctxLC->Config.szDevice, _countof(szDevice));
    if (!(szParameters = strstr(szDevice, "://"))) { return; }
    szParameters += 3;
    while ((szToken = strtok_s(szParameters, ",;", &szTokenContext)) && (ctxLC->cDeviceParameter < LC_DEVICE_PARAMETER_MAX_ENTRIES)) {
        szParameters = NULL;
        if (!(szDelim = strstr(szToken, "="))) { continue; }
        pe = &ctxLC->pDeviceParameter[ctxLC->cDeviceParameter];
        strncpy_s(pe->szName, _countof(pe->szName), szToken, szDelim - szToken);
        strncpy_s(pe->szValue, _countof(pe->szValue), szDelim + 1, _TRUNCATE);
        pe->qwValue = Util_GetNumericA(pe->szValue);
        if ((0 == pe->qwValue) && !_stricmp(pe->szValue, "true")) {
            pe->qwValue = 1;
        }
        ctxLC->cDeviceParameter++;
    }
}

VOID LcCreate_FetchDevice(_Inout_ PLC_CONTEXT ctx)
{
    CHAR c;
    DWORD cch, cszDevice = 0;
    LPSTR szDeviceSpecial = NULL;
    if (0 == _strnicmp("grpc://", ctx->Config.szRemote, 7)) {
        strncpy_s(ctx->Config.szDeviceName, sizeof(ctx->Config.szDeviceName), "grpc", _TRUNCATE);
        ctx->pfnCreate = LeechRpc_Open;
        return;
    }
    if (0 == _strnicmp("rpc://", ctx->Config.szRemote, 6)) {
        strncpy_s(ctx->Config.szDeviceName, sizeof(ctx->Config.szDeviceName), "rpc", _TRUNCATE);
        ctx->pfnCreate = LeechRpc_Open;
        return;
    }
    return;
}

VOID LcCreate_MemMapInitAddressDetect_AddDefaultRange(_Inout_ PLC_CONTEXT ctxLC, _In_ ULONG64 paMax)
{
    paMax = (paMax + 0xfff) & ~0xfff;
    if (ctxLC->Config.fVolatile) {
        LcMemMap_AddRange(ctxLC, 0, min(paMax, 0x000a0000), 0);
        if (paMax > 0x00100000) {
            LcMemMap_AddRange(ctxLC, 0x00100000, paMax - 0x00100000, 0x00100000);
        }
    }
    else {
        LcMemMap_AddRange(ctxLC, 0, paMax, 0);
    }
}

VOID LcCreate_MemMapInitAddressDetect(_Inout_ PLC_CONTEXT ctxLC)
{
    BOOL fFPGA, fCheckTiny = FALSE;
    PPMEM_SCATTER ppMEMs;
    ULONG64 i, paCurrent = 0x100000000, cbChunk = 0x100000000;
    if (LcMemMap_IsInitialized(ctxLC)) { return; }
    if (ctxLC->Config.paMax) {
        if (ctxLC->Config.paMax > 0x000000fffffff000) {
            ctxLC->Config.paMax = 0x000000fffffff000;
        }
        LcCreate_MemMapInitAddressDetect_AddDefaultRange(ctxLC, ctxLC->Config.paMax);
        return;
    }
    if (!LcAllocScatter1(ADDRDETECT_MAX + 1, &ppMEMs)) { return; }

    while (TRUE) {
        for (i = 0; i < ADDRDETECT_MAX; i++) {
            ppMEMs[i]->qwA = paCurrent + i * cbChunk;
            ppMEMs[i]->f = FALSE;
            ppMEMs[i]->cb = 0x8;
        }
        LcReadScatter(ctxLC, ADDRDETECT_MAX, ppMEMs);
        for (i = 0; i < ADDRDETECT_MAX; i++) {
            if (ppMEMs[i]->f) {
                paCurrent = ppMEMs[i]->qwA;
            }
        }
        if (!ppMEMs[ADDRDETECT_MAX - 1]->f) {
            break;
        }
    }

    fFPGA = (0 == _stricmp("fpga", ctxLC->Config.szDeviceName));
    while (cbChunk > 0x1000) {
        cbChunk = cbChunk >> 4;
        for (i = 0; i < ADDRDETECT_MAX; i++) {
            ppMEMs[i]->qwA = paCurrent + i * cbChunk;
            ppMEMs[i]->f = FALSE;
        }
        if (fFPGA && (cbChunk == 0x1000)) {

            ppMEMs[ADDRDETECT_MAX]->qwA = paCurrent;
            fCheckTiny = TRUE;
            LcReadScatter(ctxLC, ADDRDETECT_MAX + 1, ppMEMs);
        }
        else {
            LcReadScatter(ctxLC, ADDRDETECT_MAX, ppMEMs);
        }
        for (i = 0; i < ADDRDETECT_MAX; i++) {
            if (ppMEMs[i]->f) {
                paCurrent = ppMEMs[i]->qwA;
            }
        }
        if (fCheckTiny && !ppMEMs[ADDRDETECT_MAX]->f) {
            ctxLC->pfnSetOption(ctxLC, LC_OPT_FPGA_ALGO_TINY, 1);
        }
    }
    // 3: finish
    if (paCurrent == 0x100000000) { paCurrent -= 0x1000; }
    LcCreate_MemMapInitAddressDetect_AddDefaultRange(ctxLC, paCurrent + 0x1000);
    LocalFree(ppMEMs);
}

char* to_lower_copy(const char* s) {
    if (!s) return NULL;
    size_t len = strlen(s);
    char* result = (char*)malloc(len + 1);
    if (!result) return NULL;

    for (size_t i = 0; i < len; i++) {
        result[i] = (char)tolower((unsigned char)s[i]);
    }
    result[len] = '\0';
    return result;
}

BOOL isValidIP(const wchar_t* ip) {
    int dots = 0;
    size_t len = wcslen(ip);

    if (len < 7 || len > 15) {
        return FALSE;
    }

    for (size_t i = 0; i < len; i++) {
        if (ip[i] == L'.') {  // 宽字符点号
            dots++;
            if (i == 0 || i == len - 1 || ip[i - 1] == L'.') {
                return FALSE;
            }
        }
        else if (ip[i] < L'0' || ip[i] > L'9') {

            return FALSE;
        }
    }

    return dots == 3;
}

BOOL ReadIPFromConfigFile(wchar_t* ipBuffer, int bufferSize) {
    FILE* file = _wfopen(L"ipconfig.ini", L"r");
    if (!file) {
        printf("无法打开ipconfig.ini文件 \n");
        return FALSE;
    }

    wchar_t line[256] = { 0 };
    BOOL ipFound = FALSE;

    while (fgetws(line, sizeof(line) / sizeof(wchar_t), file)) {

        if (wcsstr(line, L"IP=") || wcsstr(line, L"ip=") || wcsstr(line, L"Address=")) {

            wchar_t* equals = wcschr(line, L'=');
            if (equals) {
                equals++;
                while (*equals == L' ' || *equals == L'\t') equals++;

                wchar_t* end = equals + wcslen(equals) - 1;
                while (end > equals && (*end == L'\n' || *end == L'\r' || *end == L' ' || *end == L'\t')) {
                    *end = L'\0';
                    end--;
                }
                wcsncpy(ipBuffer, equals, bufferSize - 1);
                ipBuffer[bufferSize - 1] = L'\0';
                ipFound = TRUE;
                break;
            }
        }
    }

    fclose(file);

    if (!ipFound) {
        printf("在ipconfig.ini中未找到IP地址配置 \n");
        return FALSE;
    }

    return TRUE;
}

BOOL isIPReachable(const wchar_t* ip) {
    // 1. 校验IP参数有效性
    if (ip == NULL || wcslen(ip) == 0) {
        return FALSE;  // IP为空，直接返回不可达
    }

    // 2. 处理带端口的IP（如 "127.0.0.1:80" → "127.0.0.1"）
    wchar_t cleanIP[64] = { 0 };
    // 找到第一个 ':' 或空格，截断端口部分
    size_t colonPos = wcscspn(ip, L": ");
    wcsncpy_s(cleanIP, sizeof(cleanIP) / sizeof(wchar_t), ip, colonPos);

    // 3. 构建宽字符ping命令（隐藏输出）
    wchar_t pingCmd[128];
    // -n 1：发送1个包；-w 1000：超时1秒；>nul 2>&1：隐藏所有输出
    swprintf_s(pingCmd, sizeof(pingCmd) / sizeof(wchar_t),
        L"ping -n 1 -w 1000 %s > nul 2>&1", cleanIP);

    // 4. 执行宽字符命令（_wsystem 是宽字符版的 system）
    int result = _wsystem(pingCmd);
    // 返回值为0表示ping成功（IP可达）
    return (result == 0);
}

wchar_t user_ip[128];  // 宽字符存储IP
BOOL LianjieIP() {
    int maxRetries = 3;
    int retries = 0;

    if (!ReadIPFromConfigFile(user_ip, sizeof(user_ip) / sizeof(wchar_t))) {

        return FALSE;
    }

    printf("从配置文件读取到IP: %ws \n", user_ip);

    while (retries < maxRetries) {
        // 2. 校验IP格式
        if (!isValidIP(user_ip)) {
            retries++;

            // 如果IP格式错误，尝试重新读取配置文件
            if (!ReadIPFromConfigFile(user_ip, sizeof(user_ip) / sizeof(wchar_t))) {
                return FALSE;
            }
            continue;
        }

        if (isIPReachable(user_ip)) {
            printf("连接成功 \n");
            return TRUE;
        }
        else {
            retries++;

        }
    }

    printf("IP地址链接失败 \n");

    return FALSE;
}

HANDLE LcCreateEx(_Inout_ PLC_CONFIG pLcCreateConfig, _Out_opt_ PPLC_CONFIG_ERRORINFO ppLcCreateErrorInfo)
{
    if (to_lower_copy(pLcCreateConfig->szDevice) != "") {

        LianjieIP();
        pLcCreateConfig->dwVersion = LC_CONFIG_VERSION;
#if DEBUG
        pLcCreateConfig->dwPrintfVerbosity = 3;
        pLcCreateConfig->pfn_printf_opt = printf;
#endif
        RtlZeroMemory(pLcCreateConfig->szDevice, sizeof(pLcCreateConfig->szDevice));
        strncpy_s(pLcCreateConfig->szDevice, sizeof(pLcCreateConfig->szDevice), "pmem", _TRUNCATE);
        RtlZeroMemory(pLcCreateConfig->szRemote, sizeof(pLcCreateConfig->szRemote));
        char user_ip_mb[128];
        wcstombs(user_ip_mb, user_ip, sizeof(user_ip_mb));
        snprintf(pLcCreateConfig->szRemote, sizeof(pLcCreateConfig->szRemote), "rpc://insecure:%s:port=%u", user_ip_mb, 50001);
    }

    PLC_CONTEXT ctxLC = NULL;
    ULONG64 qwExistingHandle = 0, tmStart = LcCallStart();
    if (ppLcCreateErrorInfo) { *ppLcCreateErrorInfo = NULL; }
    if (!pLcCreateConfig || (pLcCreateConfig->dwVersion != LC_CONFIG_VERSION)) { return NULL; }

    if (!pLcCreateConfig->szRemote[0] && (0 == _strnicmp("existing", pLcCreateConfig->szDevice, 8))) {
        if (0 == _strnicmp("existing://", pLcCreateConfig->szDevice, 11)) {
            qwExistingHandle = Util_GetNumericA(pLcCreateConfig->szDevice + 11);
        }
        EnterCriticalSection(&g_ctx.Lock);
        ctxLC = (PLC_CONTEXT)g_ctx.FLink;
        while (qwExistingHandle && ctxLC && (qwExistingHandle != (ULONG64)ctxLC)) {
            ctxLC = (PLC_CONTEXT)ctxLC->FLink;
        }
        if (qwExistingHandle && (qwExistingHandle != (ULONG64)ctxLC)) {
            ctxLC = NULL;
        }
        if (ctxLC) {
            memcpy(pLcCreateConfig, &ctxLC->Config, sizeof(LC_CONFIG));
            InterlockedIncrement(&ctxLC->dwHandleCount);
        }
        LeaveCriticalSection(&g_ctx.Lock);
        return ctxLC;
    }

    if (!(ctxLC = (PLC_CONTEXT)LocalAlloc(LMEM_ZEROINIT, sizeof(LC_CONTEXT)))) { return NULL; }
    pLcCreateConfig->fRemote = FALSE;
    memcpy(&ctxLC->Config, pLcCreateConfig, sizeof(LC_CONFIG));
    InitializeCriticalSection(&ctxLC->Lock);
    ctxLC->version = LC_CONTEXT_VERSION;
    ctxLC->dwHandleCount = 1;
    ctxLC->cMemMapMax = 0x20;
    ctxLC->pMemMap = (PLC_MEMMAP_ENTRY)LocalAlloc(LMEM_ZEROINIT, ctxLC->cMemMapMax * sizeof(LC_MEMMAP_ENTRY));
    ctxLC->fPrintf[0] = (ctxLC->Config.dwPrintfVerbosity & LC_CONFIG_PRINTF_ENABLED) ? TRUE : FALSE;
    ctxLC->fPrintf[1] = (ctxLC->Config.dwPrintfVerbosity & LC_CONFIG_PRINTF_V) ? TRUE : FALSE;
    ctxLC->fPrintf[2] = (ctxLC->Config.dwPrintfVerbosity & LC_CONFIG_PRINTF_VV) ? TRUE : FALSE;
    ctxLC->fPrintf[3] = (ctxLC->Config.dwPrintfVerbosity & LC_CONFIG_PRINTF_VVV) ? TRUE : FALSE;
    LcCreate_FetchDeviceParameter(ctxLC);
    LcCreate_FetchDevice(ctxLC);
    if (!ctxLC->pfnCreate || !ctxLC->pfnCreate(ctxLC, ppLcCreateErrorInfo) || !LcReadContigious_Initialize(ctxLC)) {
        LcClose(ctxLC);
        return NULL;
    }
    if (!ctxLC->Config.fRemote) {
        LcCreate_MemMapInitAddressDetect(ctxLC);
        ctxLC->Config.paMax = LcMemMap_GetMaxAddress(ctxLC);
        ctxLC->Config.fWritable = (ctxLC->pfnWriteScatter != NULL) || (ctxLC->pfnWriteContigious != NULL);
    }
    ctxLC->CallStat.dwVersion = LC_STATISTICS_VERSION;
    QueryPerformanceFrequency((PLARGE_INTEGER)&ctxLC->CallStat.qwFreq);
    memcpy(pLcCreateConfig, &ctxLC->Config, sizeof(LC_CONFIG));
    EnterCriticalSection(&g_ctx.Lock);
    ctxLC->FLink = g_ctx.FLink;
    g_ctx.FLink = ctxLC;
    LeaveCriticalSection(&g_ctx.Lock);
    LcCallEnd(ctxLC, LC_STATISTICS_ID_OPEN, tmStart);
    return ctxLC;
}

HANDLE LcCreate(_Inout_ PLC_CONFIG pLcCreateConfig)
{
    return LcCreateEx(pLcCreateConfig, NULL);
}

VOID LcReadContigious_Close(_In_ PLC_CONTEXT ctxLC)
{
    DWORD i;
    PLC_READ_CONTIGIOUS_CONTEXT ctxRC;
    ctxLC->RC.fActive = FALSE;
    for (i = 0; i < ctxLC->ReadContigious.cThread; i++) {
        if (!ctxLC->RC.ctx[i] || !ctxLC->RC.ctx[i]->hEventWakeup) { break; }
        SetEvent(ctxLC->RC.ctx[i]->hEventWakeup);
    }
    for (i = 0; i < ctxLC->ReadContigious.cThread; i++) {
        if (!ctxLC->RC.ctx[i]) { break; }
        ctxRC = ctxLC->RC.ctx[i];
        ctxLC->RC.ctx[i] = NULL;
        if (ctxRC->hEventWakeup) { SetEvent(ctxRC->hEventWakeup); }
        if (ctxRC->hEventFinish) { WaitForSingleObject(ctxRC->hEventFinish, INFINITE); }
        if (ctxRC->hEventFinish) { CloseHandle(ctxRC->hEventFinish); }
        if (ctxRC->hEventWakeup) { CloseHandle(ctxRC->hEventWakeup); }
        if (ctxRC->hThread) { CloseHandle(ctxRC->hThread); }
        LocalFree(ctxRC);
    }
}

VOID LcReadContigious_DeviceRead(PLC_READ_CONTIGIOUS_CONTEXT ctxRC)
{
    DWORD i, o, cbRead;
    PMEM_SCATTER pMEM;
    ctxRC->ctxLC->pfnReadContigious(ctxRC);
    cbRead = ctxRC->cbRead;
    for (i = 0, o = 0; ((i < ctxRC->cMEMs) && (cbRead >= ctxRC->ppMEMs[i]->cb)); i++) {
        pMEM = ctxRC->ppMEMs[i];
        memcpy(pMEM->pb, ctxRC->pb + o, pMEM->cb);
        pMEM->f = TRUE;
        o += pMEM->cb;
        cbRead -= pMEM->cb;
    }
}

VOID LcReadScatter(_In_ HANDLE hLC, _In_ DWORD cMEMs, _Inout_ PPMEM_SCATTER ppMEMs)
{
    PLC_CONTEXT ctxLC = (PLC_CONTEXT)hLC;
    ULONG64 i, tmStart = LcCallStart();
    if (!ctxLC || ctxLC->version != LC_CONTEXT_VERSION) { return; }
    if (ctxLC->Config.fRemote && ctxLC->pfnReadScatter) {
        ctxLC->pfnReadScatter(ctxLC, cMEMs, ppMEMs);
    }
    else {
        for (i = 0; i < cMEMs; i++) {
            MEM_SCATTER_STACK_PUSH(ppMEMs[i], ppMEMs[i]->qwA);
        }
        LcMemMap_TranslateMEMs(ctxLC, cMEMs, ppMEMs);
        LcLockAcquire(ctxLC);
        if (ctxLC->pfnReadScatter) {
            ctxLC->pfnReadScatter(ctxLC, cMEMs, ppMEMs);
        }
        else if (ctxLC->RC.fActive) {
            LcReadContigious_ReadScatterGather(ctxLC, cMEMs, ppMEMs);
        }
        LcLockRelease(ctxLC);
        for (i = 0; i < cMEMs; i++) {
            ppMEMs[i]->qwA = MEM_SCATTER_STACK_POP(ppMEMs[i]);
        }
    }
    LcCallEnd(ctxLC, LC_STATISTICS_ID_READSCATTER, tmStart);
}

VOID LcWriteScatter(_In_ HANDLE hLC, _In_ DWORD cMEMs, _Inout_ PPMEM_SCATTER ppMEMs)
{
    PLC_CONTEXT ctxLC = (PLC_CONTEXT)hLC;
    ULONG64 i, tmStart = LcCallStart();
    if (!ctxLC || ctxLC->version != LC_CONTEXT_VERSION) { return; }
    if (!ctxLC->pfnWriteScatter && !ctxLC->pfnWriteContigious) { return; }
    if (!cMEMs) { return; }
    if (ctxLC->Config.fRemote && ctxLC->pfnWriteScatter) {
        // REMOTE
        ctxLC->pfnWriteScatter(ctxLC, cMEMs, ppMEMs);
    }
    else {
        // LOCAL LEECHCORE
        // 1: TRANSLATE
        for (i = 0; i < cMEMs; i++) {
            MEM_SCATTER_STACK_PUSH(ppMEMs[i], ppMEMs[i]->qwA);
        }
        LcMemMap_TranslateMEMs(ctxLC, cMEMs, ppMEMs);
        // 2: FETCH
        LcLockAcquire(ctxLC);
        if (ctxLC->pfnWriteScatter) {
            ctxLC->pfnWriteScatter(ctxLC, cMEMs, ppMEMs);
        }
        else {
            LcWriteScatter_GatherContigious(ctxLC, cMEMs, ppMEMs);
        }
        LcLockRelease(ctxLC);
        // 3: RESTORE
        for (i = 0; i < cMEMs; i++) {
            ppMEMs[i]->qwA = MEM_SCATTER_STACK_POP(ppMEMs[i]);
        }
    }
    LcCallEnd(ctxLC, LC_STATISTICS_ID_WRITESCATTER, tmStart);
}

VOID LcReadContigious_Read(_In_ PLC_CONTEXT ctxLC, _In_ DWORD cMEMs, _Inout_ PPMEM_SCATTER ppMEMs, _In_ ULONG64 paBase, _In_ DWORD cb, _In_ BOOL fSingleThreaded)
{
    DWORD i;
    PLC_READ_CONTIGIOUS_CONTEXT ctxRC;
    if (!ctxLC->RC.fActive) { return; }
    if (fSingleThreaded) {
        ctxRC = ctxLC->RC.ctx[0];
    }
    else {
        i = WaitForMultipleObjects(ctxLC->ReadContigious.cThread, ctxLC->RC.hEventFinish, FALSE, INFINITE) - WAIT_OBJECT_0;
        if (!ctxLC->RC.fActive || (i >= ctxLC->ReadContigious.cThread)) { return; }
        ctxRC = ctxLC->RC.ctx[i];
        ResetEvent(ctxRC->hEventFinish);
    }
    ctxRC->cbRead = 0;
    ctxRC->cMEMs = cMEMs;
    ctxRC->ppMEMs = ppMEMs;
    ctxRC->paBase = paBase;
    ctxRC->cb = cb;
    if (fSingleThreaded) {
        LcReadContigious_DeviceRead(ctxRC);
    }
    else {
        SetEvent(ctxRC->hEventWakeup);
    }
}

VOID LcReadContigious_ReadScatterGather(_In_ PLC_CONTEXT ctxLC, _In_ DWORD cMEMs, _Inout_ PPMEM_SCATTER ppMEMs)
{
    PMEM_SCATTER pMEM;
    ULONG64 i, iBase = 0, paBase = 0;
    DWORD cbChunkSizeLimit, c = 0, cbCurrent = 0;
    BOOL fSingleThreaded, fFirst = TRUE;
    fSingleThreaded = (ctxLC->ReadContigious.cThread == 1);
    cbChunkSizeLimit = ctxLC->ReadContigious.cbChunkSize;
    if ((ctxLC->ReadContigious.cThread > 1) && ctxLC->ReadContigious.fLoadBalance) {
        cbChunkSizeLimit = min(cbChunkSizeLimit, max(0x00010000, 0x1000 * (cMEMs / ctxLC->ReadContigious.cThread)));
    }
    for (i = 0; i < cMEMs; i++) {
        pMEM = ppMEMs[i];
        if (!MEM_SCATTER_ADDR_ISVALID(pMEM)) { continue; }
        if (c == 0) {
            if (pMEM->cb && !pMEM->f) {
                c = 1;
                iBase = i;
                paBase = pMEM->qwA;
                cbCurrent = pMEM->cb;
            }
        }
        else if ((paBase + cbCurrent == pMEM->qwA) && (cbCurrent < cbChunkSizeLimit)) {
            c++;
            cbCurrent += pMEM->cb;
        }
        else {
            fFirst = FALSE;
            LcReadContigious_Read(ctxLC, c, ppMEMs + iBase, paBase, cbCurrent, fSingleThreaded);
            c = 0;
            if (pMEM->cb && !pMEM->f) {
                c = 1;
                iBase = i;
                paBase = pMEM->qwA;
                cbCurrent = pMEM->cb;
            }
        }
    }
    fSingleThreaded = fSingleThreaded || fFirst;
    if (c) {
        LcReadContigious_Read(ctxLC, c, ppMEMs + iBase, paBase, cbCurrent, fSingleThreaded);
    }
    if (!fSingleThreaded && ctxLC->RC.fActive) {
        WaitForMultipleObjects(ctxLC->ReadContigious.cThread, ctxLC->RC.hEventFinish, TRUE, INFINITE);
    }
}

VOID LcWriteScatter_GatherContigious(_In_ PLC_CONTEXT ctxLC, _In_ DWORD cMEMs, _Inout_ PPMEM_SCATTER ppMEMs)
{
    DWORD c = 0, cbCurrent;
    ULONG64 i, iBase = 0, paBase;
    PMEM_SCATTER pMEM;
    for (i = 0; i < cMEMs; i++) {
        pMEM = ppMEMs[i];
        if (pMEM->f || !MEM_SCATTER_ADDR_ISVALID(pMEM)) { continue; }
        if (c == 0) {
            c = 1;
            iBase = i;
            paBase = pMEM->qwA;
            cbCurrent = pMEM->cb;
        }
        else if (paBase + cbCurrent == pMEM->qwA) {
            c++;
            cbCurrent += pMEM->cb;
        }
        else {
            LcWriteScatter_GatherContigious2(ctxLC, c, ppMEMs + iBase, cbCurrent);
            c = 1;
            iBase = i;
            paBase = pMEM->qwA;
            cbCurrent = pMEM->cb;
        }
    }
    if (c) {
        LcWriteScatter_GatherContigious2(ctxLC, c, ppMEMs + iBase, cbCurrent);
    }
}

VOID LcWriteScatter_GatherContigious2(_In_ PLC_CONTEXT ctxLC, _In_ DWORD cMEMs, _Inout_ PPMEM_SCATTER ppMEMs, _In_ DWORD cbWrite)
{
    DWORD i;
    if (ctxLC->pfnWriteContigious(ctxLC, ppMEMs[0]->qwA, cbWrite, ppMEMs[0]->pb)) {
        for (i = 0; i < cMEMs; i++) {
            ppMEMs[i]->f = TRUE;
        }
    }
}

DWORD LcReadContigious_ThreadProc(PLC_READ_CONTIGIOUS_CONTEXT ctxRC)
{
    while (ctxRC->ctxLC->RC.fActive) {
        WaitForSingleObject(ctxRC->hEventWakeup, INFINITE);
        if (!ctxRC->ctxLC->RC.fActive) { break; }
        LcReadContigious_DeviceRead(ctxRC);
        SetEvent(ctxRC->hEventFinish);
    }
    SetEvent(ctxRC->hEventFinish);
    return 0;
}

BOOL LcRead(_In_ HANDLE hLC, _In_ ULONG64 pa, _In_ DWORD cb, _Out_writes_(cb) PBYTE pb)
{
    ULONG64 i, o, paBase, cMEMs;
    PPMEM_SCATTER ppMEMs = NULL;
    BOOL fFirst, fLast, f, fResult = FALSE;
    BYTE pbFirst[0x1000] = { 0 }, pbLast[0x1000] = { 0 };
    PLC_CONTEXT ctxLC = (PLC_CONTEXT)hLC;
    ULONG64 tmStart = LcCallStart();
    if (!ctxLC || ctxLC->version != LC_CONTEXT_VERSION) { return FALSE; }
    if (cb == 0) { return TRUE; }
    cMEMs = ((pa & 0xfff) + cb + 0xfff) >> 12;
    if (cMEMs == 0) { return FALSE; }
    fFirst = (pa & 0xfff) || (cb < 0x1000);
    fLast = (cMEMs > 1) && ((pa + cb) & 0xfff);
    f = LcAllocScatter3(
        fFirst ? pbFirst : NULL,
        fLast ? pbLast : NULL,
        cb - (fFirst ? 0x1000 - (pa & 0xfff) : 0) - (fLast ? (pa + cb) & 0xfff : 0),
        pb + ((pa & 0xfff) ? 0x1000 - (pa & 0xfff) : 0),
        (DWORD)cMEMs,
        &ppMEMs
    );
    if (!f) { goto fail; }
    paBase = pa & ~0xfff;
    for (i = 0; i < cMEMs; i++) {
        ppMEMs[i]->qwA = paBase + (i << 12);
    }
    LcReadScatter(hLC, (DWORD)cMEMs, ppMEMs);
    for (i = 0; i < cMEMs; i++) {
        if (!ppMEMs[i]->f) { goto fail; }
    }
    if (fFirst) {
        o = pa & 0xfff;
        memcpy(pb, ppMEMs[0]->pb + o, min(cb, 0x1000 - (SIZE_T)o));
    }
    if (fLast) {
        o = ppMEMs[cMEMs - 1]->qwA;
        memcpy(pb + (SIZE_T)(o - pa), ppMEMs[cMEMs - 1]->pb, (SIZE_T)(pa + cb - o));
    }
    fResult = TRUE;
fail:
    LocalFree(ppMEMs);
    LcCallEnd(ctxLC, LC_STATISTICS_ID_READ, tmStart);
    return fResult;
}

BOOL LcWrite(_In_ HANDLE hLC, _In_ ULONG64 pa, _In_ DWORD cb, _In_reads_(cb) PBYTE pb)
{
    BOOL fResult = FALSE;
    PBYTE pbBuffer = NULL;
    DWORD i = 0, oA = 0, cbP, cMEMs;
    PMEM_SCATTER pMEM, pMEMs, * ppMEMs;
    PLC_CONTEXT ctxLC = (PLC_CONTEXT)hLC;
    ULONG64 tmStart = LcCallStart();
    if (!ctxLC || ctxLC->version != LC_CONTEXT_VERSION) { goto fail; }
    // allocate
    cMEMs = (DWORD)(((pa & 0xfff) + cb + 0xfff) >> 12);
    if (!(pbBuffer = (PBYTE)LocalAlloc(LMEM_ZEROINIT, cMEMs * (sizeof(MEM_SCATTER) + sizeof(PMEM_SCATTER))))) { goto fail; }
    pMEMs = (PMEM_SCATTER)pbBuffer;
    ppMEMs = (PPMEM_SCATTER)(pbBuffer + cMEMs * sizeof(MEM_SCATTER));
    // prepare pages
    while (oA < cb) {
        cbP = 0x1000 - ((pa + oA) & 0xfff);
        cbP = min(cbP, cb - oA);
        ppMEMs[i] = pMEM = pMEMs + i;
        pMEM->version = MEM_SCATTER_VERSION;
        pMEM->qwA = pa + oA;
        pMEM->cb = cbP;
        pMEM->pb = pb + oA;
        oA += cbP;
        i++;
    }

    LcWriteScatter(hLC, cMEMs, ppMEMs);
    for (i = 0; i < cMEMs; i++) {
        if (!ppMEMs[i]->f) {
            break;
        }
    }
    fResult = TRUE;
fail:
    LocalFree(pbBuffer);
    LcCallEnd(ctxLC, LC_STATISTICS_ID_WRITE, tmStart);
    return fResult;
}

BOOL LcAllocScatter1(_In_ DWORD cMEMs, _Out_ PPMEM_SCATTER* pppMEMs)
{
    DWORD i, o = 0;
    PBYTE pb, pbData;
    PMEM_SCATTER pMEMs, * ppMEMs;
    if (!(pb =(PBYTE)LocalAlloc(LMEM_ZEROINIT, cMEMs * (sizeof(PMEM_SCATTER) + sizeof(MEM_SCATTER) + 0x1000)))) { return FALSE; }
    ppMEMs = (PPMEM_SCATTER)pb;
    pMEMs = (PMEM_SCATTER)(pb + cMEMs * (sizeof(PMEM_SCATTER)));
    pbData = pb + cMEMs * (sizeof(PMEM_SCATTER) + sizeof(MEM_SCATTER));
    for (i = 0; i < cMEMs; i++) {
        ppMEMs[i] = pMEMs + i;
        pMEMs[i].version = MEM_SCATTER_VERSION;
        pMEMs[i].cb = 0x1000;
        pMEMs[i].pb = pbData + o;
        o += 0x1000;
    }
    *pppMEMs = ppMEMs;
    return TRUE;
}

BOOL LcAllocScatter2(_In_ DWORD cbData, _Inout_updates_opt_(cbData) PBYTE pbData, _In_ DWORD cMEMs, _Out_ PPMEM_SCATTER* pppMEMs)
{
    DWORD i, o = 0;
    PBYTE pb;
    PMEM_SCATTER pMEMs, * ppMEMs;
    if (cbData > (cMEMs << 12)) { return FALSE; }
    if (!(pb = (PBYTE)LocalAlloc(LMEM_ZEROINIT, cMEMs * (sizeof(PMEM_SCATTER) + sizeof(MEM_SCATTER))))) { return FALSE; }
    ppMEMs = (PPMEM_SCATTER)pb;
    pMEMs = (PMEM_SCATTER)(pb + cMEMs * (sizeof(PMEM_SCATTER)));
    for (i = 0; i < cMEMs; i++) {
        ppMEMs[i] = pMEMs + i;
        pMEMs[i].version = MEM_SCATTER_VERSION;
        pMEMs[i].cb = 0x1000;
        pMEMs[i].pb = pbData + o;
        o += 0x1000;
    }
    *pppMEMs = ppMEMs;
    return TRUE;
}

BOOL LcAllocScatter3(_Inout_updates_opt_(0x1000) PBYTE pbDataFirstPage, _Inout_updates_opt_(0x1000) PBYTE pbDataLastPage, _In_ DWORD cbData, _Inout_updates_opt_(cbData) PBYTE pbData, _In_ DWORD cMEMs, _Out_ PPMEM_SCATTER * pppMEMs)
{
    DWORD i, o = 0;
    PBYTE pb;
    PMEM_SCATTER pMEMs, * ppMEMs;
    if (pbDataFirstPage) { cbData += 0x1000; }
    if (pbDataLastPage) { cbData += 0x1000; }
    if (cbData > (cMEMs << 12)) { return FALSE; }
    if (!(pb = (PBYTE)LocalAlloc(LMEM_ZEROINIT, cMEMs * (sizeof(PMEM_SCATTER) + sizeof(MEM_SCATTER))))) { return FALSE; }
    ppMEMs = (PPMEM_SCATTER)pb;
    pMEMs = (PMEM_SCATTER)(pb + cMEMs * (sizeof(PMEM_SCATTER)));
    for (i = 0; i < cMEMs; i++) {
        ppMEMs[i] = pMEMs + i;
        pMEMs[i].version = MEM_SCATTER_VERSION;
        pMEMs[i].cb = 0x1000;
        if (pbDataFirstPage && (i == 0)) {
            pMEMs[i].pb = pbDataFirstPage;
        }
        else if (pbDataLastPage && (i == cMEMs - 1)) {
            pMEMs[i].pb = pbDataLastPage;
        }
        else {
            pMEMs[i].pb = pbData + o;
            o += 0x1000;
        }
    }
    *pppMEMs = ppMEMs;
    return TRUE;
}

BOOL LcReadContigious_Initialize(_In_ PLC_CONTEXT ctxLC)
{
    DWORD i;
    PLC_READ_CONTIGIOUS_CONTEXT ctxRC;
    if (!ctxLC->pfnReadContigious) { return TRUE; }
    if (!ctxLC->ReadContigious.cThread) { ctxLC->ReadContigious.cThread = 1; }                   
    if (!ctxLC->ReadContigious.cbChunkSize) { ctxLC->ReadContigious.cbChunkSize = 0x01000000; } 
    ctxLC->ReadContigious.cThread = min(8, ctxLC->ReadContigious.cThread);                     
    ctxLC->ReadContigious.cbChunkSize = min(0x01000000, ctxLC->ReadContigious.cbChunkSize);   
    ctxLC->RC.fActive = TRUE;
    for (i = 0; i < ctxLC->ReadContigious.cThread; i++) {
        if (!(ctxRC = ctxLC->RC.ctx[i] = (PLC_READ_CONTIGIOUS_CONTEXT)LocalAlloc(0, sizeof(LC_READ_CONTIGIOUS_CONTEXT) + ctxLC->ReadContigious.cbChunkSize + 0x1000))) { goto fail; }
        ZeroMemory(ctxRC, sizeof(LC_READ_CONTIGIOUS_CONTEXT));
        ctxRC->ctxLC = ctxLC;
        if (ctxLC->ReadContigious.cThread > 1) {
            ctxRC->iRL = i;
            if (!(ctxRC->hEventWakeup = CreateEvent(NULL, FALSE, FALSE, FALSE))) { goto fail; }
            if (!(ctxRC->hEventFinish = ctxLC->RC.hEventFinish[i] = CreateEvent(NULL, TRUE, TRUE, FALSE))) { goto fail; }
            if (!(ctxRC->hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)LcReadContigious_ThreadProc, ctxRC, 0, NULL))) { goto fail; }
        }
    }
    return TRUE;
fail:
    LcReadContigious_Close(ctxLC);
    return FALSE;
}

BOOL LcGetOption_DoWork(_In_ PLC_CONTEXT ctxLC, _In_ ULONG64 fOption, _Out_ PULONG64 pqwValue)
{
    *pqwValue = 0;
    switch (fOption & 0xffffffff00000000) {
    case LC_OPT_CORE_PRINTF_ENABLE:
        *pqwValue = ctxLC->fPrintf[LC_PRINTF_ENABLE] ? 1 : 0;
        return TRUE;
    case LC_OPT_CORE_VERBOSE:
        *pqwValue = ctxLC->fPrintf[LC_PRINTF_V] ? 1 : 0;
        return TRUE;
    case LC_OPT_CORE_VERBOSE_EXTRA:
        *pqwValue = ctxLC->fPrintf[LC_PRINTF_VV] ? 1 : 0;
        return TRUE;
    case LC_OPT_CORE_VERBOSE_EXTRA_TLP:
        *pqwValue = ctxLC->fPrintf[LC_PRINTF_VVV] ? 1 : 0;
        return TRUE;
    case LC_OPT_CORE_VERSION_MAJOR:
        *pqwValue = VERSION_MAJOR;
        return TRUE;
    case LC_OPT_CORE_VERSION_MINOR:
        *pqwValue = VERSION_MINOR;
        return TRUE;
    case LC_OPT_CORE_VERSION_REVISION:
        *pqwValue = VERSION_REVISION;
        return TRUE;
    case LC_OPT_CORE_ADDR_MAX:
        *pqwValue = LcMemMap_GetMaxAddress(ctxLC);
        return TRUE;
    case LC_OPT_CORE_STATISTICS_CALL_COUNT:
        if ((DWORD)fOption > LC_STATISTICS_ID_MAX) { return FALSE; }
        *pqwValue = ctxLC->CallStat.Call[(DWORD)fOption].c;
        return TRUE;
    case LC_OPT_CORE_STATISTICS_CALL_TIME:
        if ((DWORD)fOption > LC_STATISTICS_ID_MAX) { return FALSE; }
        *pqwValue = ctxLC->CallStat.Call[(DWORD)fOption].tm;
        return TRUE;
    case LC_OPT_CORE_VOLATILE:
        *pqwValue = ctxLC->Config.fVolatile ? 1 : 0;
        return TRUE;
    case LC_OPT_CORE_READONLY:
        *pqwValue = ctxLC->Config.fWritable ? 0 : 1;
        return TRUE;
    }
    if (ctxLC->pfnGetOption) {
        return ctxLC->pfnGetOption(ctxLC, fOption, pqwValue);
    }
    return FALSE;
}

BOOL LcSetOption_DoWork(_In_ PLC_CONTEXT ctxLC, _In_ ULONG64 fOption, _In_ ULONG64 qwValue)
{
    switch (fOption) {
    case LC_OPT_CORE_PRINTF_ENABLE:
        ctxLC->fPrintf[LC_PRINTF_ENABLE] = qwValue ? TRUE : FALSE;
        return TRUE;
    case LC_OPT_CORE_VERBOSE:
        ctxLC->fPrintf[LC_PRINTF_V] = qwValue ? TRUE : FALSE;
        return TRUE;
    case LC_OPT_CORE_VERBOSE_EXTRA:
        ctxLC->fPrintf[LC_PRINTF_VV] = qwValue ? TRUE : FALSE;
        return TRUE;
    case LC_OPT_CORE_VERBOSE_EXTRA_TLP:
        ctxLC->fPrintf[LC_PRINTF_VVV] = qwValue ? TRUE : FALSE;
        return TRUE;
    }
    if (ctxLC->pfnSetOption) {
        return ctxLC->pfnSetOption(ctxLC, fOption, qwValue);
    }
    return FALSE;
}

BOOL LcCommand_DoWork(_In_ PLC_CONTEXT ctxLC, _In_ ULONG64 fOption, _In_ DWORD cbDataIn, _In_reads_opt_(cbDataIn) PBYTE pbDataIn, _Out_opt_ PBYTE* ppbDataOut, _Out_opt_ PDWORD pcbDataOut)
{
    if (ppbDataOut) { *ppbDataOut = NULL; }
    if (pcbDataOut) { *pcbDataOut = 0; }
    switch (fOption) {
    case LC_CMD_STATISTICS_GET:
        if (!ppbDataOut) { return FALSE; }
        if (!(*ppbDataOut = (PBYTE)LocalAlloc(0, sizeof(LC_STATISTICS)))) { return FALSE; }
        if (pcbDataOut) { *pcbDataOut = sizeof(LC_STATISTICS); }
        memcpy(*ppbDataOut, &ctxLC->CallStat, sizeof(LC_STATISTICS));
        return TRUE;
    case LC_CMD_MEMMAP_GET_STRUCT:
        if (!ppbDataOut) { return FALSE; }
        return LcMemMap_GetRangesAsStruct(ctxLC, ppbDataOut, pcbDataOut);
    case LC_CMD_MEMMAP_SET_STRUCT:
        if (!cbDataIn || !pbDataIn) { return FALSE; }
        return LcMemMap_SetRangesFromStruct(ctxLC, (PLC_MEMMAP_ENTRY)pbDataIn, cbDataIn / sizeof(LC_MEMMAP_ENTRY));
    case LC_CMD_MEMMAP_GET:
        if (!ppbDataOut) { return FALSE; }
        return LcMemMap_GetRangesAsText(ctxLC, ppbDataOut, pcbDataOut);
    case LC_CMD_MEMMAP_SET:
        if (!pbDataIn || !cbDataIn) { return FALSE; }
        return LcMemMap_SetRangesFromText(ctxLC, pbDataIn, cbDataIn);
    }
    if (ctxLC->pfnCommand) {
        return ctxLC->pfnCommand(ctxLC, fOption, cbDataIn, pbDataIn, ppbDataOut, pcbDataOut);
    }
    return FALSE;
}

BOOL LcGetOption(_In_ HANDLE hLC, _In_ ULONG64 fOption, _Out_ PULONG64 pqwValue)
{
    PLC_CONTEXT ctxLC = (PLC_CONTEXT)hLC;
    ULONG64 tmStart = LcCallStart();
    BOOL fResult;
    if (!ctxLC || ctxLC->version != LC_CONTEXT_VERSION) { return FALSE; }
    LcLockAcquire(ctxLC);
    fResult = ctxLC->Config.fRemote ?
        ctxLC->pfnGetOption(ctxLC, fOption, pqwValue) :
        LcGetOption_DoWork(ctxLC, fOption, pqwValue);
    LcLockRelease(ctxLC);
    LcCallEnd(ctxLC, LC_STATISTICS_ID_GETOPTION, tmStart);
    return fResult;
}

BOOL LcSetOption(_In_ HANDLE hLC, _In_ ULONG64 fOption, _In_ ULONG64 qwValue)
{
    PLC_CONTEXT ctxLC = (PLC_CONTEXT)hLC;
    ULONG64 tmStart = LcCallStart();
    BOOL fResult;
    if (!ctxLC || ctxLC->version != LC_CONTEXT_VERSION) { return FALSE; }
    LcLockAcquire(ctxLC);
    fResult = ctxLC->Config.fRemote ?
        ctxLC->pfnSetOption(ctxLC, fOption, qwValue) :
        LcSetOption_DoWork(ctxLC, fOption, qwValue);
    LcLockRelease(ctxLC);
    LcCallEnd(ctxLC, LC_STATISTICS_ID_SETOPTION, tmStart);
    return fResult;
}

BOOL LcCommand(_In_ HANDLE hLC, _In_ ULONG64 fCommand, _In_ DWORD cbDataIn, _In_reads_opt_(cbDataIn) PBYTE pbDataIn, _Out_opt_ PBYTE* ppbDataOut, _Out_opt_ PDWORD pcbDataOut)
{
    PLC_CONTEXT ctxLC = (PLC_CONTEXT)hLC;
    ULONG64 tmStart = LcCallStart();
    BOOL fResult;
    if (!ctxLC || ctxLC->version != LC_CONTEXT_VERSION) { return FALSE; }
    LcLockAcquire(ctxLC);
    fResult = ctxLC->Config.fRemote ?
        ctxLC->pfnCommand(ctxLC, fCommand, cbDataIn, pbDataIn, ppbDataOut, pcbDataOut) :
        LcCommand_DoWork(ctxLC, fCommand, cbDataIn, pbDataIn, ppbDataOut, pcbDataOut);
    LcLockRelease(ctxLC);
    LcCallEnd(ctxLC, LC_STATISTICS_ID_COMMAND, tmStart);
    return fResult;
}

ULONG64 LcDeviceParameterGetNumeric(_In_ PLC_CONTEXT ctxLC, _In_ LPSTR szName)
{
    PLC_DEVICE_PARAMETER_ENTRY p = LcDeviceParameterGet(ctxLC, szName);
    return p ? p->qwValue : 0;
}

PLC_DEVICE_PARAMETER_ENTRY LcDeviceParameterGet(_In_ PLC_CONTEXT ctxLC, _In_ LPSTR szName)
{
    for (DWORD i = 0; i < ctxLC->cDeviceParameter; i++) {
        if (!_stricmp(szName, ctxLC->pDeviceParameter[i].szName)) {
            return &ctxLC->pDeviceParameter[i];
        }
    }
    return NULL;
}