#include "main.h"

VOID LeechRPC_KeepaliveThreadClient(_In_ PLC_CONTEXT ctxLC)
{
    PLEECHRPC_CLIENT_CONTEXT ctx = (PLEECHRPC_CLIENT_CONTEXT)ctxLC->hDevice;
    LEECHRPC_MSG_HDR MsgReq = { 0 };
    PLEECHRPC_MSG_HDR pMsgRsp = NULL;
    DWORD c = 0;
    ctx->fHousekeeperThread = TRUE;
    ctx->fHousekeeperThreadIsRunning = TRUE;
    while (ctx->fHousekeeperThread) {
        c++;
        if (0 == (c % (10 * 15))) { 
            ZeroMemory(&MsgReq, sizeof(LEECHRPC_MSG_HDR));
            MsgReq.tpMsg = LEECHRPC_MSGTYPE_KEEPALIVE_REQ;
            LeechRPC_SubmitCommand(ctxLC, (PLEECHRPC_MSG_HDR)&MsgReq, LEECHRPC_MSGTYPE_KEEPALIVE_RSP, &pMsgRsp);
            LocalFree(pMsgRsp);
            pMsgRsp = NULL;
        }
        Sleep(100);
    }
    ctx->fHousekeeperThreadIsRunning = FALSE;
}

VOID LeechRPC_CompressClose(_Inout_ PLEECHRPC_COMPRESS ctxCompress)
{
    DWORD i;
    for (i = 0; i < LEECHRPC_COMPRESS_MAXTHREADS; i++) {
        if (ctxCompress->fValid) {
            DeleteCriticalSection(&ctxCompress->Compress[i].Lock);
            LocalFree(ctxCompress->Compress[i].pbWorkspace);
        }
    }
    if (ctxCompress->hDll) { FreeLibrary((HMODULE)ctxCompress->hDll); }
    ZeroMemory(ctxCompress, sizeof(LEECHRPC_COMPRESS));
}

VOID LeechRPC_Compress(_In_ PLEECHRPC_COMPRESS ctxCompress, _Inout_ PLEECHRPC_MSG_BIN pMsg, _In_ BOOL fCompressDisable)
{
    NTSTATUS nt;
    PBYTE pb;
    ULONG cb;
    DWORD i;
    if (ctxCompress->fValid && (pMsg->cb > 0x1800) && !fCompressDisable) {
        if (!(pb = (PBYTE)LocalAlloc(0, pMsg->cb))) { return; }
        do {
            i = InterlockedIncrement(&ctxCompress->iCompress) % LEECHRPC_COMPRESS_MAXTHREADS;
        } while (!TryEnterCriticalSection(&ctxCompress->Compress[i].Lock));
        nt = ctxCompress->fn.pfnRtlCompressBuffer(COMPRESSION_FORMAT_XPRESS, pMsg->pb, pMsg->cb, pb, pMsg->cb, 4096, &cb, ctxCompress->Compress[i].pbWorkspace);
        LeaveCriticalSection(&ctxCompress->Compress[i].Lock);
        if (((nt == 0x00000000L) || (nt == 0x00000117L)) && (cb <= pMsg->cb)) {
            memcpy(pMsg->pb, pb, cb);
            pMsg->cbDecompress = pMsg->cb;
            pMsg->cb = (DWORD)cb;
            pMsg->cbMsg = sizeof(LEECHRPC_MSG_BIN) + (DWORD)cb;
        }
        LocalFree(pb);
    }
}

BOOL LeechRPC_Decompress(_In_ PLEECHRPC_COMPRESS ctxCompress, _In_ PLEECHRPC_MSG_BIN pMsgIn, _Out_ PLEECHRPC_MSG_BIN* ppMsgOut)
{
    NTSTATUS nt;
    ULONG cb;
    PLEECHRPC_MSG_BIN pMsgOut = NULL;
    *ppMsgOut = NULL;
    if (!pMsgIn->cbDecompress) { return FALSE; }
    if (!ctxCompress->fValid || (pMsgIn->cbDecompress > 0x04000000)) { return FALSE; }
    if (!(pMsgOut = (PLEECHRPC_MSG_BIN)LocalAlloc(0, sizeof(LEECHRPC_MSG_BIN) + pMsgIn->cbDecompress))) { return FALSE; }
    memcpy(pMsgOut, pMsgIn, sizeof(LEECHRPC_MSG_BIN));
    nt = ctxCompress->fn.pfnRtlDecompressBuffer(COMPRESSION_FORMAT_XPRESS, pMsgOut->pb, pMsgOut->cbDecompress, pMsgIn->pb, pMsgIn->cb, &cb);
    if ((nt != 0x00000000L) || (cb != pMsgIn->cbDecompress)) {
        LocalFree(pMsgOut);
        return FALSE;
    }
    pMsgOut->cb = (DWORD)cb;
    pMsgOut->cbMsg = sizeof(LEECHRPC_MSG_BIN) + pMsgOut->cb;
    pMsgOut->cbDecompress = 0;
    *ppMsgOut = pMsgOut;
    return TRUE;
}

BOOL LeechRPC_CompressInitialize(_Inout_ PLEECHRPC_COMPRESS ctxCompress)
{
    DWORD i;
    LeechRPC_CompressClose(ctxCompress);
    ctxCompress->hDll = LoadLibraryA("ntdll.dll");
    if (!ctxCompress->hDll) { return FALSE; }
    if (!(ctxCompress->fn.pfnRtlCompressBuffer = (PFN_RtlCompressBuffer*)GetProcAddress((HMODULE)ctxCompress->hDll, "RtlCompressBuffer"))) { goto fail; }
    if (!(ctxCompress->fn.pfnRtlDecompressBuffer = (PFN_RtlDecompressBuffer*)GetProcAddress((HMODULE)ctxCompress->hDll, "RtlDecompressBuffer"))) { goto fail; }
    ctxCompress->fValid = TRUE;
    for (i = 0; i < LEECHRPC_COMPRESS_MAXTHREADS; i++) {
        InitializeCriticalSection(&ctxCompress->Compress[i].Lock);
        ctxCompress->fValid = ctxCompress->fValid && (ctxCompress->Compress[i].pbWorkspace = LocalAlloc(0, 0x00100000));
    }
fail:
    if (!ctxCompress->fValid) {
        LeechRPC_CompressClose(ctxCompress);
    }
    return ctxCompress->fValid;
}