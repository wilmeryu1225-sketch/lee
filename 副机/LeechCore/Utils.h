
VOID Util_SplitN(_In_ LPSTR sz, _In_ CHAR chDelimiter, _In_ DWORD cpsz, _Out_writes_(MAX_PATH) PCHAR _szBuf, _Inout_ LPSTR* psz);

VOID Util_Split3(_In_ LPSTR sz, _In_ CHAR chDelimiter, _Out_writes_(MAX_PATH) PCHAR _szBuf, _Out_ LPSTR* psz1, _Out_ LPSTR* psz2, _Out_ LPSTR* psz3);

VOID Util_GenRandom(_Out_ PBYTE pb, _In_ DWORD cb);

ULONG64 Util_GetNumericA(_In_ LPSTR sz);