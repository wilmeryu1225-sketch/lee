#include "main.h"

VOID LeechRPC_RpcClose(PLEECHRPC_CLIENT_CONTEXT ctx)
{
    if (ctx->grpc.hGRPC) {
        ctx->grpc.pfn_leechgrpc_client_free(ctx->grpc.hGRPC);
        ctx->grpc.hGRPC = NULL;
    }
    if (ctx->grpc.hDll) {
        FreeLibrary(ctx->grpc.hDll);
    }
    ZeroMemory(&ctx->grpc, sizeof(ctx->grpc));
#ifdef _WIN32
    if (ctx->hRPC) {
        RpcBindingFree((RPC_BINDING_HANDLE*)ctx->hRPC);
        ctx->hRPC = NULL;
    }
    if (ctx->szStringBinding) {
        RpcStringFreeA(&ctx->szStringBinding);
        ctx->szStringBinding = NULL;
    }
#endif /* _WIN32 */
}

VOID LeechRPC_Close(_Inout_ PLC_CONTEXT ctxLC)
{
    PLEECHRPC_CLIENT_CONTEXT ctx = (PLEECHRPC_CLIENT_CONTEXT)ctxLC->hDevice;
    LEECHRPC_MSG_HDR Msg = { 0 };
    PLEECHRPC_MSG_HDR pMsgRsp = NULL;
    if (!ctx) { return; }
    ctx->fHousekeeperThread = FALSE;
    Msg.tpMsg = LEECHRPC_MSGTYPE_CLOSE_REQ;
    if (LeechRPC_SubmitCommand(ctxLC, (PLEECHRPC_MSG_HDR)&Msg, LEECHRPC_MSGTYPE_CLOSE_RSP, &pMsgRsp)) {
        LocalFree(pMsgRsp);
    }
    while (ctx->fHousekeeperThreadIsRunning) {
        SwitchToThread();
    }
    LeechRPC_RpcClose(ctx);
    LeechRPC_CompressClose(&ctx->Compress);
    if (ctx->hHousekeeperThread) { CloseHandle(ctx->hHousekeeperThread); }
    LocalFree(ctx);
    ctxLC->hDevice = 0;
}

VOID LeechRPC_ReadScatter(_In_ PLC_CONTEXT ctxLC, _In_ DWORD cMEMs, _Inout_ PPMEM_SCATTER ppMEMs)
{
    DWORD cMEMsChunk;
    while (cMEMs) {     // read max 16MB at a time.
        cMEMsChunk = min(cMEMs, 0x1000);
        LeechRPC_ReadScatter_Impl(ctxLC, cMEMsChunk, ppMEMs);
        ppMEMs += cMEMsChunk;
        cMEMs -= cMEMsChunk;
    }
}

VOID LeechRPC_WriteScatter(_In_ PLC_CONTEXT ctxLC, _In_ DWORD cMEMs, _Inout_ PPMEM_SCATTER ppMEMs)
{
    DWORD cMEMsChunk;
    while (cMEMs) {     // read max 16MB at a time.
        cMEMsChunk = min(cMEMs, 0x1000);
        LeechRPC_WriteScatter_Impl(ctxLC, cMEMsChunk, ppMEMs);
        ppMEMs += cMEMsChunk;
        cMEMs -= cMEMsChunk;
    }
}

VOID LeechRPC_ReadScatter_Impl(_In_ PLC_CONTEXT ctxLC, _In_ DWORD cMEMs, _Inout_ PPMEM_SCATTER ppMEMs)
{
    BOOL result;
    DWORD i, cValidMEMs = 0;
    PLEECHRPC_MSG_BIN pMsgReq = NULL;
    PLEECHRPC_MSG_BIN pMsgRsp = NULL;
    DWORD cbOffset, cbTotal = 0;
    PMEM_SCATTER pMEM_Src, pMEM_Dst;
    // 0: sanity check incoming data and count valid non-already finished MEMs
    for (i = 0; i < cMEMs; i++) {
        pMEM_Src = ppMEMs[i];
        if ((pMEM_Src->version != MEM_SCATTER_VERSION) || (pMEM_Src->cb > 0x1000)) { goto fail; }
        if (!pMEM_Src->f && MEM_SCATTER_ADDR_ISVALID(pMEM_Src)) {
            cValidMEMs++;
        }
    }

    if (!(pMsgReq = (PLEECHRPC_MSG_BIN)LocalAlloc(LMEM_ZEROINIT, sizeof(LEECHRPC_MSG_BIN) + cValidMEMs * sizeof(MEM_SCATTER)))) { return; }
    pMsgReq->tpMsg = LEECHRPC_MSGTYPE_READSCATTER_REQ;
    pMsgReq->cb = cValidMEMs * sizeof(MEM_SCATTER);
    pMEM_Dst = (PMEM_SCATTER)pMsgReq->pb;
    for (i = 0; i < cMEMs; i++) {
        pMEM_Src = ppMEMs[i];
        if (!pMEM_Src->f && MEM_SCATTER_ADDR_ISVALID(pMEM_Src)) {
            cbTotal += pMEM_Src->cb;
            memcpy(pMEM_Dst, pMEM_Src, sizeof(MEM_SCATTER));
            pMEM_Dst = pMEM_Dst + 1;
        }
    }
    pMsgReq->qwData[0] = cValidMEMs;
    pMsgReq->qwData[1] = cbTotal;
    // 2: transmit & get result
    result = LeechRPC_SubmitCommand(ctxLC, (PLEECHRPC_MSG_HDR)pMsgReq, LEECHRPC_MSGTYPE_READSCATTER_RSP, (PPLEECHRPC_MSG_HDR)&pMsgRsp);
    if (!result) { goto fail; }
    if ((pMsgRsp->qwData[0] != cValidMEMs) || (pMsgRsp->cb < cValidMEMs * sizeof(MEM_SCATTER))) { goto fail; }
    cbOffset = cValidMEMs * sizeof(MEM_SCATTER);
    pMEM_Src = (PMEM_SCATTER)pMsgRsp->pb;
    for (i = 0; i < cMEMs; i++) {
        pMEM_Dst = ppMEMs[i];
        if (pMEM_Dst->f || MEM_SCATTER_ADDR_ISINVALID(pMEM_Dst)) { continue; }
        // sanity check
        if ((pMEM_Src->version != MEM_SCATTER_VERSION) || (pMEM_Src->qwA != pMEM_Dst->qwA) || (pMEM_Dst->cb > pMsgRsp->cb - cbOffset)) { break; }
        pMEM_Dst->f = pMEM_Src->f;
        if (pMEM_Src->f) {
            memcpy(pMEM_Dst->pb, pMsgRsp->pb + cbOffset, pMEM_Dst->cb);
            cbOffset += pMEM_Dst->cb;
        }
        pMEM_Src = pMEM_Src + 1;
    }
fail:
    LocalFree(pMsgReq);
    LocalFree(pMsgRsp);
}

VOID LeechRPC_WriteScatter_Impl(_In_ PLC_CONTEXT ctxLC, _In_ DWORD cMEMs, _Inout_ PPMEM_SCATTER ppMEMs)
{
    PBOOL pfRsp;
    DWORD i, cbReqData;
    PLEECHRPC_MSG_BIN pMsgReq = NULL;
    PLEECHRPC_MSG_BIN pMsgRsp = NULL;
    PMEM_SCATTER pMEM, pReqWrMEM;
    PBYTE pbReqWrData;
    // 1: prepare message to send
    cbReqData = cMEMs * (sizeof(MEM_SCATTER) + 0x1000);
    if (!(pMsgReq = (PLEECHRPC_MSG_BIN)LocalAlloc(0, sizeof(LEECHRPC_MSG_BIN) + cbReqData))) { goto fail; }
    ZeroMemory(pMsgReq, sizeof(LEECHRPC_MSG_BIN));
    pMsgReq->tpMsg = LEECHRPC_MSGTYPE_WRITESCATTER_REQ;
    pMsgReq->qwData[0] = cMEMs;
    pMsgReq->cb = cbReqData;
    pReqWrMEM = (PMEM_SCATTER)pMsgReq->pb;
    pbReqWrData = pMsgReq->pb + cMEMs * sizeof(MEM_SCATTER);
    for (i = 0; i < cMEMs; i++) {
        pMEM = ppMEMs[i];
        if (pMEM->cb > 0x1000) { goto fail; }
        memcpy(pReqWrMEM + i, pMEM, sizeof(MEM_SCATTER));
        memcpy(pbReqWrData, pMEM->pb, pMEM->cb);
        pbReqWrData += pMEM->cb;
    }
    // 2: transmit
    if (!LeechRPC_SubmitCommand(ctxLC, (PLEECHRPC_MSG_HDR)pMsgReq, LEECHRPC_MSGTYPE_WRITESCATTER_RSP, (PPLEECHRPC_MSG_HDR)&pMsgRsp)) { goto fail; }
    // 3: parse result (1 BOOL per cMEM)
    if (pMsgRsp->cb < cMEMs * sizeof(BOOL)) { goto fail; }
    pfRsp = (PBOOL)pMsgRsp->pb;
    for (i = 0; i < cMEMs; i++) {
        ppMEMs[i]->f = pfRsp[i] ? TRUE : FALSE;
    }
fail:
    LocalFree(pMsgReq);
    LocalFree(pMsgRsp);
}

BOOL LeechRPC_Ping(_In_ PLC_CONTEXT ctxLC)
{
    BOOL result;
    LEECHRPC_MSG_HDR MsgReq = { 0 };
    PLEECHRPC_MSG_HDR pMsgRsp = NULL;
    MsgReq.tpMsg = LEECHRPC_MSGTYPE_PING_REQ;
    result = LeechRPC_SubmitCommand(ctxLC, (PLEECHRPC_MSG_HDR)&MsgReq, LEECHRPC_MSGTYPE_PING_RSP, &pMsgRsp);
    LocalFree(pMsgRsp);
    return result;
}

BOOL LeechRpc_Open(_Inout_ PLC_CONTEXT ctxLC, _Out_opt_ PPLC_CONFIG_ERRORINFO ppLcCreateErrorInfo)
{
    BOOL f;
    PLEECHRPC_CLIENT_CONTEXT ctx;
    CHAR _szBufferArg[MAX_PATH], _szBufferOpt[MAX_PATH];
    LEECHRPC_MSG_OPEN MsgReq = { 0 };
    PLEECHRPC_MSG_OPEN pMsgRsp = NULL;
    LPSTR szArg1, szArg2, szArg3;
    LPSTR aszOpt[6];
    DWORD i, dwPort = 0;
    int(*pfn_printf_opt_tmp)(_In_z_ _Printf_format_string_ char const* const _Format, ...);
    if (ppLcCreateErrorInfo) { *ppLcCreateErrorInfo = NULL; }
    ctx = (PLEECHRPC_CLIENT_CONTEXT)LocalAlloc(LMEM_ZEROINIT, sizeof(LEECHRPC_CLIENT_CONTEXT));
    if (!ctx) { return FALSE; }
    ctxLC->hDevice = (HANDLE)ctx;
    if (!_stricmp(ctxLC->Config.szDeviceName, "grpc")) { ctx->fIsProtoGRpc = TRUE; }
    if (!_stricmp(ctxLC->Config.szDeviceName, "rpc")) { ctx->fIsProtoRpc = TRUE; }
    if (!_stricmp(ctxLC->Config.szDeviceName, "smb")) { ctx->fIsProtoSmb = TRUE; }
    if (!ctx->fIsProtoGRpc && !ctx->fIsProtoRpc && !ctx->fIsProtoSmb) {
        printf("ERROR: No valid remote transport protocol specified.\n");
        goto fail;
    }
    if (ctx->fIsProtoRpc || ctx->fIsProtoSmb) {
        ctxLC->Rpc.fCompress = !ctxLC->Config.fRemoteDisableCompress;
        Util_Split3(ctxLC->Config.szRemote + 6, ':', _szBufferArg, &szArg1, &szArg2, &szArg3);
        if (!szArg1 || !szArg1[0] || !szArg2 || !szArg2[0]) { goto fail; }
        if (!_stricmp("insecure", szArg1)) {
            ctx->fIsAuthInsecure = TRUE;
        }
        else if (!_stricmp("ntlm", szArg1)) {
            ctx->fIsAuthNTLM = TRUE;
        }
        else {
            strncpy_s(ctx->szRemoteSPN, _countof(ctx->szRemoteSPN), szArg1, MAX_PATH);
            ctx->fIsAuthKerberos = TRUE;
        }

        strncpy_s(ctx->szTcpAddr, _countof(ctx->szTcpAddr), szArg2, MAX_PATH);
        if (szArg3[0]) {
            Util_SplitN(szArg3, ',', 5, _szBufferOpt, aszOpt);
            for (i = 0; i < 5; i++) {
                if (0 == _stricmp("nocompress", aszOpt[i])) {
                    ctxLC->Rpc.fCompress = FALSE;
                }
                if (0 == _strnicmp("port=", aszOpt[i], 5)) {
                    dwPort = atoi(aszOpt[i] + 5);
                }
                if (0 == _stricmp("logon", aszOpt[i])) {
                    ctx->fIsAuthNTLMCredPrompt = ctx->fIsAuthNTLM;
                }
                if (0 == _strnicmp("user=", aszOpt[i], 5)) {
                    ctx->szAuthNtlmUserInitOnly = aszOpt[i] + 5;
                }
                if (0 == _strnicmp("password=", aszOpt[i], 9)) {
                    ctx->szAuthNtlmPasswordInitOnly = aszOpt[i] + 9;
                }
            }
        }
        ctx->fIsAuthNTLMCredPrompt = ctx->fIsAuthNTLMCredPrompt || ctx->szAuthNtlmUserInitOnly || ctx->szAuthNtlmPasswordInitOnly;
        if (dwPort == 0) {
            dwPort = 50001; // default port
        }
        _itoa_s(dwPort, ctx->szTcpPort, 6, 10);
        // initialize rpc connection and ping
        f = LeechRPC_RpcInitialize(ctxLC, ctx);
        ctx->szAuthNtlmUserInitOnly = NULL;
        ctx->szAuthNtlmPasswordInitOnly = NULL;
        SecureZeroMemory(_szBufferOpt, sizeof(_szBufferOpt));
        if (!f) {
            printf("ERROR: Unable to connect to remote service.\n");
            goto fail;
        }
        if (!LeechRPC_Ping(ctxLC)) {
            printf("ERROR: Unable to ping remote service.\n");
            goto fail;
        }
    }
    if (ctx->fIsProtoGRpc) {
        // RPC SPECIFIC INITIALIZATION BELOW:
        ctxLC->Rpc.fCompress = !ctxLC->Config.fRemoteDisableCompress;
        // parse arguments
        Util_Split3(ctxLC->Config.szRemote + 7, ':', _szBufferArg, &szArg1, &szArg2, &szArg3);
        if (!szArg1 || !szArg2 || !szArg2[0]) { goto fail; }
        // Argument1 : Auth method, insecure
        if (!_stricmp("insecure", szArg1)) {
            ctx->fIsAuthInsecure = TRUE;
        }
        else if (szArg1[0]) {
            strncpy_s(ctx->grpc.szServerCertHostnameOverride, _countof(ctx->grpc.szServerCertHostnameOverride), szArg1, _TRUNCATE);
        }
        // Argument2 : Tcp Address.
        strncpy_s(ctx->szTcpAddr, _countof(ctx->szTcpAddr), szArg2, MAX_PATH);
        // Argument3 : Options.
        if (szArg3[0]) {
            Util_SplitN(szArg3, ',', 6, _szBufferOpt, aszOpt);
            for (i = 0; i < 6; i++) {
                if (0 == _stricmp("nocompress", aszOpt[i])) {
                    ctxLC->Rpc.fCompress = FALSE;
                }
                if (0 == _strnicmp("port=", aszOpt[i], 5)) {
                    dwPort = atoi(aszOpt[i] + 5);
                }
                if (0 == _strnicmp("client-cert-p12-password=", aszOpt[i], 25)) {
                    strncpy_s(ctx->grpc.szClientTlsP12Password, _countof(ctx->grpc.szClientTlsP12Password), aszOpt[i] + 25, _TRUNCATE);
                }
                if (0 == _strnicmp("client-cert-p12=", aszOpt[i], 16)) {
                    strncpy_s(ctx->grpc.szClientTlsP12Path, _countof(ctx->grpc.szClientTlsP12Path), aszOpt[i] + 16, _TRUNCATE);
                }
                if (0 == _strnicmp("server-cert=", aszOpt[i], 12)) {
                    strncpy_s(ctx->grpc.szServerCertCaPath, _countof(ctx->grpc.szServerCertCaPath), aszOpt[i] + 12, _TRUNCATE);
                }
                if (0 == _strnicmp("server-cert-host-override=", aszOpt[i], 26)) {
                    strncpy_s(ctx->grpc.szServerCertHostnameOverride, _countof(ctx->grpc.szServerCertHostnameOverride), aszOpt[i] + 26, _TRUNCATE);
                }
            }
        }
        if (dwPort == 0) {
            dwPort = 28474; // default port
        }
        _itoa_s(dwPort, ctx->szTcpPort, 6, 10);
        // initialize rpc connection and ping
        f = LeechRPC_GRpcInitialize(ctxLC, ctx);
        SecureZeroMemory(_szBufferOpt, sizeof(_szBufferOpt));
        if (!f) {
            printf("ERROR: Unable to connect to remote gRPC service.'\n");
            goto fail;
        }
        if (!LeechRPC_Ping(ctxLC)) {
            printf("ERROR: Unable to ping remote gRPC service.\n");
            goto fail;
        }
    }
    if (0 == _strnicmp(ctxLC->Config.szDevice, "existingremote", 14)) {
        for (i = 14; i < _countof(ctxLC->Config.szDevice); i++) {
            ctxLC->Config.szDevice[i - 6] = ctxLC->Config.szDevice[i];
            if (0 == ctxLC->Config.szDevice[i]) { break; }
        }
    }
    // try enable compression (if required)
    ctxLC->Rpc.fCompress = ctxLC->Rpc.fCompress && LeechRPC_CompressInitialize(&ctx->Compress);
    ctxLC->Config.fRemoteDisableCompress = ctxLC->Config.fRemoteDisableCompress && !ctxLC->Rpc.fCompress;
    // call open on the remote service
    Util_GenRandom((PBYTE)&ctxLC->Rpc.dwRpcClientId, sizeof(DWORD));
    MsgReq.tpMsg = LEECHRPC_MSGTYPE_OPEN_REQ;
    memcpy(&MsgReq.cfg, &ctxLC->Config, sizeof(LC_CONFIG));
    ZeroMemory(MsgReq.cfg.szRemote, _countof(MsgReq.cfg.szRemote));
    MsgReq.cfg.pfn_printf_opt = 0;
    if (!LeechRPC_SubmitCommand(ctxLC, (PLEECHRPC_MSG_HDR)&MsgReq, LEECHRPC_MSGTYPE_OPEN_RSP, (PPLEECHRPC_MSG_HDR)&pMsgRsp)) {
        printf("ERROR: Unable to open remote device #1.'\n");
        goto fail;
    }
    if (!pMsgRsp->fValidOpen) {
        if ((pMsgRsp->errorinfo.dwVersion == LC_CONFIG_ERRORINFO_VERSION) && (pMsgRsp->errorinfo.cbStruct < pMsgRsp->cbMsg) && ((pMsgRsp->errorinfo.cwszUserText * 2ULL) + sizeof(LC_CONFIG_ERRORINFO) < pMsgRsp->errorinfo.cbStruct)) {
            if ((*ppLcCreateErrorInfo = (PLC_CONFIG_ERRORINFO)LocalAlloc(LMEM_ZEROINIT, pMsgRsp->errorinfo.cbStruct))) {
                pMsgRsp->errorinfo.wszUserText[pMsgRsp->errorinfo.cwszUserText] = 0;
                memcpy(*ppLcCreateErrorInfo, &pMsgRsp->errorinfo, pMsgRsp->errorinfo.cbStruct);
            }
        }
        printf("ERROR: Unable to open remote device #2.'\n");
        goto fail;
    }
    // sanity check positive result from remote service
    if (pMsgRsp->cfg.dwVersion != LC_CONFIG_VERSION) {
        printf("ERROR: Invalid message received from remote service.\n");
        goto fail;
    }
    if (ctxLC->Rpc.fCompress && pMsgRsp->cfg.fRemoteDisableCompress) {
        ctxLC->Config.fRemoteDisableCompress = TRUE;
        ctxLC->Rpc.fCompress = FALSE;
    }
    // all ok - initialize this rpc device stub.
    ctx->hHousekeeperThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)LeechRPC_KeepaliveThreadClient, ctxLC, 0, NULL);
    strncpy_s(pMsgRsp->cfg.szRemote, sizeof(pMsgRsp->cfg.szRemote), ctxLC->Config.szRemote, _TRUNCATE); // ctx from remote doesn't contain remote info ...
    pfn_printf_opt_tmp = ctxLC->Config.pfn_printf_opt;
    memcpy(&ctxLC->Config, &pMsgRsp->cfg, sizeof(LC_CONFIG));
    ctxLC->Config.pfn_printf_opt = pfn_printf_opt_tmp;
    ctxLC->Config.fRemote = TRUE;
    ctxLC->fMultiThread = TRUE;
    ctxLC->pfnClose = LeechRPC_Close;
    ctxLC->pfnReadScatter = LeechRPC_ReadScatter;
    ctxLC->pfnWriteScatter = LeechRPC_WriteScatter;
    ctxLC->pfnGetOption = LeechRPC_GetOption;
    ctxLC->pfnSetOption = LeechRPC_SetOption;
    ctxLC->pfnCommand = LeechRPC_Command;
    LocalFree(pMsgRsp);
    return TRUE;
fail:
    LeechRPC_Close(ctxLC);
    LocalFree(pMsgRsp);
    return FALSE;
}

BOOL LeechRPC_GetOption(_In_ PLC_CONTEXT ctxLC, _In_ ULONG64 fOption, _Out_ PULONG64 pqwValue)
{
    BOOL result;
    LEECHRPC_MSG_DATA MsgReq = { 0 };
    PLEECHRPC_MSG_DATA pMsgRsp = NULL;
    MsgReq.tpMsg = LEECHRPC_MSGTYPE_GETOPTION_REQ;
    MsgReq.qwData[0] = fOption;
    result = LeechRPC_SubmitCommand(ctxLC, (PLEECHRPC_MSG_HDR)&MsgReq, LEECHRPC_MSGTYPE_GETOPTION_RSP, (PPLEECHRPC_MSG_HDR)&pMsgRsp);
    *pqwValue = result ? pMsgRsp->qwData[0] : 0;
    LocalFree(pMsgRsp);
    return result;
}

BOOL LeechRPC_SetOption(_In_ PLC_CONTEXT ctxLC, _In_ ULONG64 fOption, _In_ ULONG64 qwValue)
{
    BOOL result;
    LEECHRPC_MSG_DATA MsgReq = { 0 };
    PLEECHRPC_MSG_HDR pMsgRsp = NULL;
    // 1: prepare message to send
    MsgReq.tpMsg = LEECHRPC_MSGTYPE_SETOPTION_REQ;
    MsgReq.qwData[0] = fOption;
    MsgReq.qwData[1] = qwValue;
    // 2: transmit & get result
    result = LeechRPC_SubmitCommand(ctxLC, (PLEECHRPC_MSG_HDR)&MsgReq, LEECHRPC_MSGTYPE_SETOPTION_RSP, (PPLEECHRPC_MSG_HDR)&pMsgRsp);
    LocalFree(pMsgRsp);
    return result;
}

BOOL LeechRPC_Command(_In_ PLC_CONTEXT ctxLC, _In_ ULONG64 fCMD, _In_ DWORD cbDataIn, _In_reads_opt_(cbDataIn) PBYTE pbDataIn, _Out_opt_ PBYTE* ppbDataOut, _Out_opt_ PDWORD pcbDataOut)
{
    BOOL result;
    PLEECHRPC_MSG_BIN pMsgReq = NULL;
    PLEECHRPC_MSG_BIN pMsgRsp = NULL;
    if (!pbDataIn && cbDataIn) { return FALSE; }
    if (fCMD & 0x2000000000000000) { return FALSE; }   
    if (!(pMsgReq = (PLEECHRPC_MSG_BIN)LocalAlloc(0, sizeof(LEECHRPC_MSG_BIN) + cbDataIn))) { return FALSE; }
    ZeroMemory(pMsgReq, sizeof(LEECHRPC_MSG_BIN));
    pMsgReq->tpMsg = LEECHRPC_MSGTYPE_COMMAND_REQ;
    pMsgReq->cb = cbDataIn;
    pMsgReq->qwData[0] = fCMD;
    pMsgReq->qwData[1] = 0;
    if (pbDataIn) {
        memcpy(pMsgReq->pb, pbDataIn, cbDataIn);
    }
    // 2: transmit & get result
    result = LeechRPC_SubmitCommand(ctxLC, (PLEECHRPC_MSG_HDR)pMsgReq, LEECHRPC_MSGTYPE_COMMAND_RSP, (PPLEECHRPC_MSG_HDR)&pMsgRsp);
    if (result && ((fCMD == LC_CMD_AGENT_VFS_LIST) || (fCMD == LC_CMD_AGENT_VFS_READ) || (fCMD == LC_CMD_AGENT_VFS_WRITE))) {
        result = LeechRPC_Command_VerifyUntrustedVfsRsp(fCMD, pMsgRsp);
    }
    if (result) {
        if (pcbDataOut) { *pcbDataOut = pMsgRsp->cb; }
        if (ppbDataOut) {
            if ((*ppbDataOut = (PBYTE)LocalAlloc(0, pMsgRsp->cb))) {
                memcpy(*ppbDataOut, pMsgRsp->pb, pMsgRsp->cb);
            }
            else {
                result = FALSE;
            }
        }
    }
    if (!result && pcbDataOut) { *pcbDataOut = 0; }
    LocalFree(pMsgReq);
    LocalFree(pMsgRsp);
    return result;
}

BOOL LeechRPC_Command_VerifyUntrustedVfsRsp(_In_ ULONG64 fCMD, _In_ PLEECHRPC_MSG_BIN pMsgRsp)
{
    PLC_CMD_AGENT_VFS_RSP pRsp;
    P__VFS_FILELISTBLOB pVfs;
    DWORD i;
    // 1: general
    if (pMsgRsp->cb < sizeof(LC_CMD_AGENT_VFS_RSP)) { return FALSE; }
    pRsp = (PLC_CMD_AGENT_VFS_RSP)pMsgRsp->pb;
    if (pRsp->dwVersion != LC_CMD_AGENT_VFS_RSP_VERSION) { return FALSE; }
    if (pMsgRsp->cb != sizeof(LC_CMD_AGENT_VFS_RSP) + pRsp->cb) { return FALSE; }
    // 2: specific
    if (fCMD == LC_CMD_AGENT_VFS_READ) {
        return (pRsp->cbReadWrite == pRsp->cb);
    }
    if (fCMD == LC_CMD_AGENT_VFS_WRITE) {
        return (0 == pRsp->cb);
    }
    if (fCMD == LC_CMD_AGENT_VFS_LIST) {
        if (pRsp->cb < sizeof(__VFS_FILELISTBLOB)) { return FALSE; }
        if (pRsp->pb[pRsp->cb - 1] != 0) { return FALSE; }
        pVfs = (P__VFS_FILELISTBLOB)pRsp->pb;
        if ((pVfs->dwVersion != __VFS_FILELISTBLOB_VERSION) || (pRsp->cb != pVfs->cbStruct) || (pVfs->cbMultiText == 0)) { return FALSE; }
        if (pRsp->cb != sizeof(__VFS_FILELISTBLOB) + pVfs->cFileEntry * sizeof(__VFS_FILELISTBLOB_ENTRY) + pVfs->cbMultiText) { return FALSE; }
        if (pRsp->pb[sizeof(__VFS_FILELISTBLOB) + pVfs->cFileEntry * sizeof(__VFS_FILELISTBLOB_ENTRY)] != 0) { return FALSE; }
        pVfs->uszMultiText = (LPSTR)(sizeof(__VFS_FILELISTBLOB) + pVfs->cFileEntry * sizeof(__VFS_FILELISTBLOB_ENTRY));
        for (i = 0; i < pVfs->cFileEntry; i++) {
            if (pVfs->FileEntry[i].ouszName >= pVfs->cbMultiText) { return FALSE; }
        }
        return TRUE;
    }
    return FALSE;
}

BOOL LeechRPC_RpcInitialize_NtlmWithUserCreds(_In_ PLC_CONTEXT ctxLC, _In_ PLEECHRPC_CLIENT_CONTEXT ctx)
{
    BOOL fResult = FALSE;
    RPC_STATUS status;
    WCHAR wszTcpAddr[MAX_PATH];
    WCHAR wszAuthIdentityUser[MAX_PATH] = { 0 };
    WCHAR wszAuthIdentityDomain[MAX_PATH] = { 0 };
    WCHAR wszAuthIdentityPassword[MAX_PATH] = { 0 };
    LPVOID pvAuthBuffer = NULL;
    ULONG cbAuthBuffer = 0;
    ULONG AuthenticationPackage = 0;
    HANDLE LsaHandle = NULL;
    SEC_WINNT_AUTH_IDENTITY_W AuthIdentity = { 0 };
    RPC_SECURITY_QOS RpcSecurityQOS = { 0 };
    LSA_STRING PackageName = { 0 };
    CREDUI_INFOW credui = { 0 };
    RpcSecurityQOS.Version = RPC_C_SECURITY_QOS_VERSION;
    RpcSecurityQOS.Capabilities = RPC_C_QOS_CAPABILITIES_DEFAULT;
    RpcSecurityQOS.IdentityTracking = RPC_C_QOS_IDENTITY_DYNAMIC;
    RpcSecurityQOS.ImpersonationType = RPC_C_IMP_LEVEL_IDENTIFY;
    swprintf_s(wszTcpAddr, _countof(wszTcpAddr), L"Enter your credentials to connect to: %S", ctx->szTcpAddr);
    credui.cbSize = sizeof(credui);
    credui.hwndParent = NULL;
    credui.pszMessageText = wszTcpAddr;
    credui.pszCaptionText = L"Enter network credentials";
    credui.hbmBanner = NULL;
    if (ERROR_SUCCESS != LsaConnectUntrusted(&LsaHandle)) { goto fail; }
    PackageName.Buffer = (PCHAR)MICROSOFT_KERBEROS_NAME_A;
    PackageName.Length = (USHORT)strlen(PackageName.Buffer);
    PackageName.MaximumLength = (USHORT)strlen(PackageName.Buffer);
    if (ERROR_SUCCESS != LsaLookupAuthenticationPackage(LsaHandle, &PackageName, &AuthenticationPackage)) { goto fail; }
    // get user creds via credprompt (unless user already set both user & password by command line):
    AuthIdentity.Domain = (USHORT*)wszAuthIdentityDomain;
    AuthIdentity.DomainLength = (DWORD)_countof(wszAuthIdentityDomain);
    AuthIdentity.User = (USHORT*)wszAuthIdentityUser;
    AuthIdentity.UserLength = (DWORD)_countof(wszAuthIdentityUser);
    AuthIdentity.Password = (USHORT*)wszAuthIdentityPassword;
    AuthIdentity.PasswordLength = (DWORD)_countof(wszAuthIdentityPassword);
    AuthIdentity.Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;
    if (ctx->szAuthNtlmUserInitOnly && ctx->szAuthNtlmPasswordInitOnly) {
        AuthIdentity.DomainLength = 0;
    }
    else {
        if (ERROR_SUCCESS != CredUIPromptForWindowsCredentialsW(&credui, 0, &AuthenticationPackage, NULL, 0, &pvAuthBuffer, &cbAuthBuffer, NULL, CREDUIWIN_GENERIC)) { goto fail; }
        // unpack user creds:
        if (FALSE == CredUnPackAuthenticationBufferW(CRED_PACK_PROTECTED_CREDENTIALS, pvAuthBuffer, cbAuthBuffer, (LPWSTR)AuthIdentity.User, &AuthIdentity.UserLength, (LPWSTR)AuthIdentity.Domain, &AuthIdentity.DomainLength, (LPWSTR)AuthIdentity.Password, &AuthIdentity.PasswordLength)) { goto fail; }
        if (AuthIdentity.UserLength && (wszAuthIdentityUser[AuthIdentity.UserLength - 1] == 0)) { AuthIdentity.UserLength--; }
        if (AuthIdentity.DomainLength && (wszAuthIdentityDomain[AuthIdentity.DomainLength - 1] == 0)) { AuthIdentity.DomainLength--; }
        if (AuthIdentity.PasswordLength && (wszAuthIdentityPassword[AuthIdentity.PasswordLength - 1] == 0)) { AuthIdentity.PasswordLength--; }
    }
    if (ctx->szAuthNtlmUserInitOnly) {
        AuthIdentity.UserLength = _snwprintf_s(wszAuthIdentityUser, _countof(wszAuthIdentityUser), _TRUNCATE, L"%S", ctx->szAuthNtlmUserInitOnly);
    }
    if (ctx->szAuthNtlmPasswordInitOnly) {
        AuthIdentity.PasswordLength = _snwprintf_s(wszAuthIdentityPassword, _countof(wszAuthIdentityPassword), _TRUNCATE, L"%S", ctx->szAuthNtlmPasswordInitOnly);
    }
    status = RpcBindingSetAuthInfoExW(
        ctx->hRPC,
        NULL,
        RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
        RPC_C_AUTHN_WINNT,
        &AuthIdentity,
        RPC_C_AUTHZ_DEFAULT,
        &RpcSecurityQOS);
    if (status) {
        printf("Failed to set connection security for connection, Error code: 0x%08x\n", status);
        LeechRPC_RpcClose(ctx);
        goto fail;
    }
    fResult = TRUE;
fail:
    SecureZeroMemory(pvAuthBuffer, cbAuthBuffer);
    SecureZeroMemory(wszAuthIdentityPassword, sizeof(wszAuthIdentityPassword));
    if (LsaHandle) { LsaDeregisterLogonProcess(LsaHandle); LsaHandle = NULL; }
    if (pvAuthBuffer) { CoTaskMemFree(pvAuthBuffer); pvAuthBuffer = NULL; }
    return fResult;
}

BOOL LeechRPC_RpcInitialize(_In_ PLC_CONTEXT ctxLC, _In_ PLEECHRPC_CLIENT_CONTEXT ctx)
{
    LPSTR szTcpAddr;
    RPC_STATUS status;
    RPC_SECURITY_QOS RpcSecurityQOS = { 0 };
    LeechRPC_RpcClose(ctx);
    if (ctx->fIsProtoSmb) {
        if ((ctx->szTcpAddr[0] == 0) || !_stricmp("localhost", ctx->szTcpAddr) || !_stricmp("127.0.0.1", ctx->szTcpAddr)) {
            szTcpAddr = NULL;
        }
        else {
            szTcpAddr = ctx->szTcpAddr;
        }
        status = RpcStringBindingComposeA(
            (RPC_CSTR)CLSID_BINDING_INTERFACE_LEECHRPC,
            (RPC_CSTR)"ncacn_np",
            (RPC_CSTR)szTcpAddr,
            (RPC_CSTR)"\\pipe\\PIPE_1",
            NULL,
            &ctx->szStringBinding);
        if (status) {
            printf("Failed compose binding: Error code: 0x%08x\n", status);
            LeechRPC_RpcClose(ctx);
            return FALSE;
        }
    }
    if (ctx->fIsProtoRpc) {
        status = RpcStringBindingComposeA(
            (RPC_CSTR)CLSID_BINDING_INTERFACE_LEECHRPC,
            (RPC_CSTR)"ncacn_ip_tcp",
            (RPC_CSTR)ctx->szTcpAddr,
            (RPC_CSTR)ctx->szTcpPort,
            NULL,
            &ctx->szStringBinding);
        if (status) {
            printf("Failed compose binding: Error code: 0x%08x\n", status);
            LeechRPC_RpcClose(ctx);
            return FALSE;
        }
    }
    // create binding:
    status = RpcBindingFromStringBindingA(ctx->szStringBinding, &ctx->hRPC);
    if (status) {
        printf("Failed create binding: Error code: 0x%08x\n", status);
        LeechRPC_RpcClose(ctx);
        return FALSE;
    }
    // set connection security (if any):
    if (ctx->fIsAuthNTLM) {
        if (ctx->fIsAuthNTLMCredPrompt) {
            if (!LeechRPC_RpcInitialize_NtlmWithUserCreds(ctxLC, ctx)) { return FALSE; }
        }
        else {
            // NTLM - use default credentials (current user) or user-supplied credentials via prompt:
            RpcSecurityQOS.Version = RPC_C_SECURITY_QOS_VERSION;
            RpcSecurityQOS.Capabilities = RPC_C_QOS_CAPABILITIES_DEFAULT;
            RpcSecurityQOS.IdentityTracking = RPC_C_QOS_IDENTITY_DYNAMIC;
            RpcSecurityQOS.ImpersonationType = RPC_C_IMP_LEVEL_IDENTIFY;
            status = RpcBindingSetAuthInfoExA(
                ctx->hRPC,
                NULL,
                RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
                RPC_C_AUTHN_WINNT,
                NULL,
                RPC_C_AUTHZ_DEFAULT,
                &RpcSecurityQOS);
            if (status) {
                printf("Failed to set connection security for connection, Error code: 0x%08x\n", status);
                LeechRPC_RpcClose(ctx);
                return FALSE;
            }
        }
    }
    if (ctx->fIsAuthKerberos) {
        RpcSecurityQOS.Version = RPC_C_SECURITY_QOS_VERSION;
        RpcSecurityQOS.Capabilities = RPC_C_QOS_CAPABILITIES_MUTUAL_AUTH;
        RpcSecurityQOS.IdentityTracking = RPC_C_QOS_IDENTITY_DYNAMIC;
        RpcSecurityQOS.ImpersonationType = RPC_C_IMP_LEVEL_IDENTIFY;
        status = RpcBindingSetAuthInfoExA(
            ctx->hRPC,
            (RPC_CSTR)ctx->szRemoteSPN,
            RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
            RPC_C_AUTHN_GSS_KERBEROS,
            NULL,
            RPC_C_AUTHZ_NONE,
            &RpcSecurityQOS);
        if (status) {
            printf("Failed to set connection security: SPN: '%s', Error code: 0x%08x\n", ctx->szRemoteSPN, status);
            LeechRPC_RpcClose(ctx);
            return FALSE;
        }
    }

    return TRUE;
}

BOOL LeechRPC_GRpcInitialize(_In_ PLC_CONTEXT ctxLC, _In_ PLEECHRPC_CLIENT_CONTEXT ctx)
{
    DWORD dwTcpPort = strtol(ctx->szTcpPort, NULL, 10);
    ctx->grpc.hDll = LoadLibraryA("libleechgrpc.dll");
    if (!ctx->grpc.hDll) {
        printf("Failed to load libleechgrpc.dll\n");
        return FALSE;
    }
    ctx->grpc.pfn_leechgrpc_client_create_insecure = (pfn_leechgrpc_client_create_insecure)GetProcAddress(ctx->grpc.hDll, "leechgrpc_client_create_insecure");
    ctx->grpc.pfn_leechgrpc_client_create_secure_p12 = (pfn_leechgrpc_client_create_secure_p12)GetProcAddress(ctx->grpc.hDll, "leechgrpc_client_create_secure_p12");
    ctx->grpc.pfn_leechgrpc_client_free = (pfn_leechgrpc_client_free)GetProcAddress(ctx->grpc.hDll, "leechgrpc_client_free");
    ctx->grpc.pfn_leechgrpc_client_submit_command = (pfn_leechgrpc_client_submit_command)GetProcAddress(ctx->grpc.hDll, "leechgrpc_client_submit_command");
    if (!ctx->grpc.pfn_leechgrpc_client_create_insecure || !ctx->grpc.pfn_leechgrpc_client_create_secure_p12 || !ctx->grpc.pfn_leechgrpc_client_free || !ctx->grpc.pfn_leechgrpc_client_submit_command) {
        printf("Failed to load functions from libleechgrpc.dll\n");
        return FALSE;
    }
    if (ctx->fIsAuthInsecure) {
        ctx->grpc.hGRPC = ctx->grpc.pfn_leechgrpc_client_create_insecure(ctx->szTcpAddr, dwTcpPort);
    }
    else {
        ctx->grpc.hGRPC = ctx->grpc.pfn_leechgrpc_client_create_secure_p12(
            ctx->szTcpAddr,
            dwTcpPort,
            ctx->grpc.szServerCertHostnameOverride[0] ? ctx->grpc.szServerCertHostnameOverride : NULL,
            ctx->grpc.szServerCertCaPath[0] ? ctx->grpc.szServerCertCaPath : NULL,
            ctx->grpc.szClientTlsP12Path,
            ctx->grpc.szClientTlsP12Password);
    }
    if (!ctx->grpc.hGRPC) {
        printf("Failed to create gRPC client connection\n");
        return FALSE;
    }
    return TRUE;
}

BOOL LeechRPC_SubmitCommand(_In_ PLC_CONTEXT ctxLC, _In_ PLEECHRPC_MSG_HDR pMsgIn, _In_ LEECHRPC_MSGTYPE tpMsgRsp, _Out_ PPLEECHRPC_MSG_HDR ppMsgOut)
{
    PLEECHRPC_CLIENT_CONTEXT ctx = (PLEECHRPC_CLIENT_CONTEXT)ctxLC->hDevice;
    error_status_t error;
    BOOL fOK;
    DWORD cbMsgOut = 0;
    SIZE_T cbMsgOutSize = 0;
    PLEECHRPC_MSG_BIN pMsgOutDecompress = NULL;
    pMsgIn->dwMagic = LEECHRPC_MSGMAGIC;
    pMsgIn->fMsgResult = TRUE;
    switch (pMsgIn->tpMsg) {
    case LEECHRPC_MSGTYPE_PING_REQ:
    case LEECHRPC_MSGTYPE_CLOSE_REQ:
    case LEECHRPC_MSGTYPE_KEEPALIVE_REQ:
        pMsgIn->cbMsg = sizeof(LEECHRPC_MSG_HDR);
        break;
    case LEECHRPC_MSGTYPE_OPEN_REQ:
        pMsgIn->cbMsg = sizeof(LEECHRPC_MSG_OPEN);
        break;
    case LEECHRPC_MSGTYPE_GETOPTION_REQ:
    case LEECHRPC_MSGTYPE_SETOPTION_REQ:
        pMsgIn->cbMsg = sizeof(LEECHRPC_MSG_DATA);
        break;
    case LEECHRPC_MSGTYPE_READSCATTER_REQ:
    case LEECHRPC_MSGTYPE_WRITESCATTER_REQ:
    case LEECHRPC_MSGTYPE_COMMAND_REQ:
        pMsgIn->cbMsg = sizeof(LEECHRPC_MSG_BIN) + ((PLEECHRPC_MSG_BIN)pMsgIn)->cb;
        LeechRPC_Compress(&ctx->Compress, (PLEECHRPC_MSG_BIN)pMsgIn, !ctxLC->Rpc.fCompress);
        break;
    default:
        return FALSE;
    }
    // submit message to RPC server:
    *ppMsgOut = NULL;
    pMsgIn->dwRpcClientID = ctxLC->Rpc.dwRpcClientId;
    pMsgIn->flags = ctxLC->Rpc.fCompress ? 0 : LEECHRPC_FLAG_NOCOMPRESS;
    if (ctx->fIsProtoRpc || ctx->fIsProtoSmb) {
        // RPC (over tcp or smb) connection methods:
        error = E_FAIL;
#ifdef _WIN32
        __try {
            error = LeechRpc_ReservedSubmitCommand(ctx->hRPC, pMsgIn->cbMsg, (PBYTE)pMsgIn, (long*)&cbMsgOut, (PBYTE*)ppMsgOut);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) { error = E_FAIL; }
#endif /* _WIN32 */
        if (error) {
            *ppMsgOut = NULL;
            return FALSE;
        }
    }
    else if (ctx->fIsProtoGRpc) {
        // gRPC (over tcp) connection method:
        fOK = ctx->grpc.hGRPC && ctx->grpc.pfn_leechgrpc_client_submit_command(ctx->grpc.hGRPC, (PBYTE)pMsgIn, pMsgIn->cbMsg, (PBYTE*)ppMsgOut, &cbMsgOutSize);
        if (!fOK) {
            *ppMsgOut = NULL;
            return FALSE;
        }
        cbMsgOut = (DWORD)cbMsgOutSize;
    }
    // sanity check non-trusted incoming message from RPC server.
    fOK = (cbMsgOut >= sizeof(LEECHRPC_MSG_HDR)) && *ppMsgOut && ((*ppMsgOut)->dwMagic == LEECHRPC_MSGMAGIC);
    fOK = fOK && ((*ppMsgOut)->tpMsg <= LEECHRPC_MSGTYPE_MAX) && ((*ppMsgOut)->cbMsg == cbMsgOut) && (cbMsgOut < 0x10000000);
    fOK = fOK && (*ppMsgOut)->fMsgResult && ((*ppMsgOut)->tpMsg == tpMsgRsp);
    if (fOK) {
        switch ((*ppMsgOut)->tpMsg) {
        case LEECHRPC_MSGTYPE_PING_RSP:
        case LEECHRPC_MSGTYPE_CLOSE_RSP:
        case LEECHRPC_MSGTYPE_KEEPALIVE_RSP:
        case LEECHRPC_MSGTYPE_SETOPTION_RSP:
            fOK = (*ppMsgOut)->cbMsg == sizeof(LEECHRPC_MSG_HDR);
            break;
        case LEECHRPC_MSGTYPE_OPEN_RSP:
            fOK = (*ppMsgOut)->cbMsg >= sizeof(LEECHRPC_MSG_OPEN);
            break;
        case LEECHRPC_MSGTYPE_GETOPTION_RSP:
            fOK = (*ppMsgOut)->cbMsg == sizeof(LEECHRPC_MSG_DATA);
            break;
        case LEECHRPC_MSGTYPE_READSCATTER_RSP:
        case LEECHRPC_MSGTYPE_WRITESCATTER_RSP:
        case LEECHRPC_MSGTYPE_COMMAND_RSP:
            fOK = (*ppMsgOut)->cbMsg == sizeof(LEECHRPC_MSG_BIN) + ((PLEECHRPC_MSG_BIN)*ppMsgOut)->cb;
            if (fOK && ((PLEECHRPC_MSG_BIN)*ppMsgOut)->cbDecompress) {
                if (!LeechRPC_Decompress(&ctx->Compress, (PLEECHRPC_MSG_BIN)*ppMsgOut, &pMsgOutDecompress)) { goto fail; }
                LocalFree(*ppMsgOut);
                *ppMsgOut = (PLEECHRPC_MSG_HDR)pMsgOutDecompress;
            }
            break;
        default:
            fOK = FALSE;
            break;
        }
        if (!fOK) { goto fail; }
        return TRUE;
    }
fail:
    LocalFree(*ppMsgOut);
    *ppMsgOut = NULL;
    return FALSE;
}