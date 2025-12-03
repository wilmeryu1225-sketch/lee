
 VOID LeechRPC_KeepaliveThreadClient(_In_ PLC_CONTEXT ctxLC);

 VOID LeechRPC_CompressClose(_Inout_ PLEECHRPC_COMPRESS ctxCompress);

 VOID LeechRPC_Compress(_In_ PLEECHRPC_COMPRESS ctxCompress, _Inout_ PLEECHRPC_MSG_BIN pMsg, _In_ BOOL fCompressDisable);

 BOOL LeechRPC_Decompress(_In_ PLEECHRPC_COMPRESS ctxCompress, _In_ PLEECHRPC_MSG_BIN pMsgIn, _Out_ PLEECHRPC_MSG_BIN* ppMsgOut);

 BOOL LeechRPC_CompressInitialize(_Inout_ PLEECHRPC_COMPRESS ctxCompress);