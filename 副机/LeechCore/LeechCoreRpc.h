
 VOID LeechRPC_RpcClose(PLEECHRPC_CLIENT_CONTEXT ctx);

 VOID LeechRPC_Close(_Inout_ PLC_CONTEXT ctxLC);

 VOID LeechRPC_ReadScatter(_In_ PLC_CONTEXT ctxLC, _In_ DWORD cMEMs, _Inout_ PPMEM_SCATTER ppMEMs);

 VOID LeechRPC_WriteScatter(_In_ PLC_CONTEXT ctxLC, _In_ DWORD cMEMs, _Inout_ PPMEM_SCATTER ppMEMs);

 VOID LeechRPC_ReadScatter_Impl(_In_ PLC_CONTEXT ctxLC, _In_ DWORD cMEMs, _Inout_ PPMEM_SCATTER ppMEMs);

 VOID LeechRPC_WriteScatter_Impl(_In_ PLC_CONTEXT ctxLC, _In_ DWORD cMEMs, _Inout_ PPMEM_SCATTER ppMEMs);

 BOOL LeechRPC_Ping(_In_ PLC_CONTEXT ctxLC);

 BOOL LeechRpc_Open(_Inout_ PLC_CONTEXT ctxLC, _Out_opt_ PPLC_CONFIG_ERRORINFO ppLcCreateErrorInfo);

 BOOL LeechRPC_GetOption(_In_ PLC_CONTEXT ctxLC, _In_ ULONG64 fOption, _Out_ PULONG64 pqwValue);

 BOOL LeechRPC_SetOption(_In_ PLC_CONTEXT ctxLC, _In_ ULONG64 fOption, _In_ ULONG64 qwValue);

 BOOL LeechRPC_Command(_In_ PLC_CONTEXT ctxLC, _In_ ULONG64 fCMD, _In_ DWORD cbDataIn, _In_reads_opt_(cbDataIn) PBYTE pbDataIn, _Out_opt_ PBYTE* ppbDataOut, _Out_opt_ PDWORD pcbDataOut);

 BOOL LeechRPC_Command_VerifyUntrustedVfsRsp(_In_ ULONG64 fCMD, _In_ PLEECHRPC_MSG_BIN pMsgRsp);

 BOOL LeechRPC_RpcInitialize_NtlmWithUserCreds(_In_ PLC_CONTEXT ctxLC, _In_ PLEECHRPC_CLIENT_CONTEXT ctx);

 BOOL LeechRPC_RpcInitialize(_In_ PLC_CONTEXT ctxLC, _In_ PLEECHRPC_CLIENT_CONTEXT ctx);

 BOOL LeechRPC_GRpcInitialize(_In_ PLC_CONTEXT ctxLC, _In_ PLEECHRPC_CLIENT_CONTEXT ctx);

 BOOL LeechRPC_SubmitCommand(_In_ PLC_CONTEXT ctxLC, _In_ PLEECHRPC_MSG_HDR pMsgIn, _In_ LEECHRPC_MSGTYPE tpMsgRsp, _Out_ PPLEECHRPC_MSG_HDR ppMsgOut);