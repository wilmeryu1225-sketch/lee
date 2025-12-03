
 ULONG64 LcMemMap_GetMaxAddress(_In_ PLC_CONTEXT ctxLC);

 BOOL LcMemMap_IsInitialized(_In_ PLC_CONTEXT ctxLC);

 VOID LcMemMap_TranslateMEMs(_In_ PLC_CONTEXT ctxLC, _In_ DWORD cMEMs, _Inout_ PPMEM_SCATTER ppMEMs);

 BOOL LcMemMap_AddRange(_In_ PLC_CONTEXT ctxLC, _In_ ULONG64 pa, _In_ ULONG64 cb, _In_opt_ ULONG64 paRemap);

 BOOL LcMemMap_GetRangesAsStruct(_In_ PLC_CONTEXT ctxLC, _Out_ PBYTE* ppbDataOut, _Out_opt_ PDWORD pcbDataOut);

 BOOL LcMemMap_SetRangesFromStruct(_In_ PLC_CONTEXT ctxLC, _In_ PLC_MEMMAP_ENTRY pMemMap, _In_ DWORD cMemMap);

 BOOL LcMemMap_GetRangesAsText(_In_ PLC_CONTEXT ctxLC, _Out_ PBYTE* ppbDataOut, _Out_opt_ PDWORD pcbDataOut);

 BOOL LcMemMap_SetRangesFromText(_In_ PLC_CONTEXT ctxLC, _In_ PBYTE pb, _In_ DWORD cb);