#include "main.h"

VOID Util_SplitN(_In_ LPSTR sz, _In_ CHAR chDelimiter, _In_ DWORD cpsz, _Out_writes_(MAX_PATH) PCHAR _szBuf, _Inout_ LPSTR* psz)
{
    DWORD i, j;
    strcpy_s(_szBuf, MAX_PATH, sz);
    psz[0] = _szBuf;
    for (i = 1; i < cpsz; i++) {
        psz[i] = (LPSTR)"";
    }
    for (i = 0, j = 0; i < MAX_PATH; i++) {
        if ('\0' == _szBuf[i]) {
            return;
        }
        if (chDelimiter == _szBuf[i]) {
            j++;
            if (j >= cpsz) {
                return;
            }
            _szBuf[i] = '\0';
            psz[j] = _szBuf + i + 1;
        }
    }
}

VOID Util_Split3(_In_ LPSTR sz, _In_ CHAR chDelimiter, _Out_writes_(MAX_PATH) PCHAR _szBuf, _Out_ LPSTR* psz1, _Out_ LPSTR* psz2, _Out_ LPSTR* psz3)
{
    LPSTR psz[3] = { 0 };
    Util_SplitN(sz, chDelimiter, 3, _szBuf, psz);
    *psz1 = psz[0];
    *psz2 = psz[1];
    *psz3 = psz[2];
}

VOID Util_GenRandom(_Out_ PBYTE pb, _In_ DWORD cb)
{
    DWORD i = 0;
    srand((unsigned int)GetTickCount64());
    if (cb % 2) {
        *(PBYTE)(pb) = (BYTE)rand();
        i++;
    }
    for (; i <= cb - 2; i += 2) {
        *(PWORD)(pb + i) = (WORD)rand();
    }
}


ULONG64 Util_GetNumericA(_In_ LPSTR sz)
{
    BOOL fhex = sz[0] && sz[1] && (sz[0] == '0') && ((sz[1] == 'x') || (sz[1] == 'X'));
    return strtoull(sz, NULL, fhex ? 16 : 10);
}
