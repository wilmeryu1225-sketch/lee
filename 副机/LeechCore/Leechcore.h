
#ifndef __LEECHCORE_H__
#define __LEECHCORE_H__
#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32

#include <Windows.h>
#define EXPORTED_FUNCTION                   __declspec(dllexport)
#endif 
#if defined(LINUX) || defined(MACOS)

#include <inttypes.h>
#include <stdlib.h>
#define EXPORTED_FUNCTION                   __attribute__((visibility("default")))
	typedef void                                VOID, * PVOID, * HANDLE, ** PHANDLE, * HMODULE;
	typedef long long unsigned int              QWORD, * PQWORD, ULONG64, * PULONG64;
	typedef size_t                              SIZE_T, * PSIZE_T;
	typedef uint64_t                            FILETIME, * PFILETIME;
	typedef uint32_t                            DWORD, * PDWORD, * LPDWORD, BOOL, * PBOOL, NTSTATUS;
	typedef uint16_t                            WORD, * PWORD;
	typedef uint8_t                             BYTE, * PBYTE, * LPBYTE, UCHAR;
	typedef char                                CHAR, * PCHAR, * LPSTR;
	typedef const char* LPCSTR;
	typedef uint16_t                            WCHAR, * PWCHAR, * LPWSTR;
	typedef const uint16_t* LPCWSTR;
#define MAX_PATH                            260
#define _In_
#define _In_z_
#define _In_opt_
#define _In_reads_(x)
#define _In_reads_bytes_(x)
#define _In_reads_bytes_opt_(x)
#define _In_reads_opt_(x)
#define _Inout_
#define _Inout_bytecount_(x)
#define _Inout_opt_
#define _Inout_updates_opt_(x)
#define _Out_
#define _Out_opt_
#define _Out_writes_(x)
#define _Out_writes_bytes_opt_(x)
#define _Out_writes_opt_(x)
#define _Out_writes_to_(x,y)
#define _When_(x,y)
#define _Frees_ptr_opt_
#define _Post_ptr_invalid_
#define _Check_return_opt_
#define _Printf_format_string_
#define _Success_(x)

#endif

	ULONG64 LcCallStart();

	VOID LcLockAcquire(_In_ PLC_CONTEXT ctxLC);

	VOID LcLockRelease(_In_ PLC_CONTEXT ctxLC);

	VOID LcCallEnd(_In_ PLC_CONTEXT ctxLC, _In_ DWORD fId, _In_ ULONG64 tmCallStart);

	__declspec(dllexport) VOID LcClose(_In_opt_ _Post_ptr_invalid_ HANDLE hLC);

	VOID LcCloseAll();

	__declspec(dllexport) VOID LcMemFree(_Frees_ptr_opt_ PVOID pv);

	VOID LcCreate_FetchDeviceParameter(_Inout_ PLC_CONTEXT ctxLC);

	VOID LcCreate_FetchDevice(_Inout_ PLC_CONTEXT ctx);

	VOID LcCreate_MemMapInitAddressDetect_AddDefaultRange(_Inout_ PLC_CONTEXT ctxLC, _In_ ULONG64 paMax);

	VOID LcCreate_MemMapInitAddressDetect(_Inout_ PLC_CONTEXT ctxLC);

	__declspec(dllexport) HANDLE LcCreateEx(_Inout_ PLC_CONFIG pLcCreateConfig, _Out_opt_ PPLC_CONFIG_ERRORINFO ppLcCreateErrorInfo);

	__declspec(dllexport) HANDLE LcCreate(_Inout_ PLC_CONFIG pLcCreateConfig);

	VOID LcReadContigious_Close(_In_ PLC_CONTEXT ctxLC);

	VOID LcReadContigious_DeviceRead(PLC_READ_CONTIGIOUS_CONTEXT ctxRC);

	__declspec(dllexport) VOID LcReadScatter(_In_ HANDLE hLC, _In_ DWORD cMEMs, _Inout_ PPMEM_SCATTER ppMEMs);

	__declspec(dllexport) VOID LcWriteScatter(_In_ HANDLE hLC, _In_ DWORD cMEMs, _Inout_ PPMEM_SCATTER ppMEMs);

	VOID LcReadContigious_Read(_In_ PLC_CONTEXT ctxLC, _In_ DWORD cMEMs, _Inout_ PPMEM_SCATTER ppMEMs, _In_ ULONG64 paBase, _In_ DWORD cb, _In_ BOOL fSingleThreaded);

	VOID LcReadContigious_ReadScatterGather(_In_ PLC_CONTEXT ctxLC, _In_ DWORD cMEMs, _Inout_ PPMEM_SCATTER ppMEMs);

	VOID LcWriteScatter_GatherContigious(_In_ PLC_CONTEXT ctxLC, _In_ DWORD cMEMs, _Inout_ PPMEM_SCATTER ppMEMs);

	VOID LcWriteScatter_GatherContigious2(_In_ PLC_CONTEXT ctxLC, _In_ DWORD cMEMs, _Inout_ PPMEM_SCATTER ppMEMs, _In_ DWORD cbWrite);

	DWORD LcReadContigious_ThreadProc(PLC_READ_CONTIGIOUS_CONTEXT ctxRC);

	__declspec(dllexport) BOOL LcRead(_In_ HANDLE hLC, _In_ ULONG64 pa, _In_ DWORD cb, _Out_writes_(cb) PBYTE pb);

	__declspec(dllexport) BOOL LcWrite(_In_ HANDLE hLC, _In_ ULONG64 pa, _In_ DWORD cb, _In_reads_(cb) PBYTE pb);

	__declspec(dllexport) BOOL LcAllocScatter1(_In_ DWORD cMEMs, _Out_ PPMEM_SCATTER* pppMEMs);

	__declspec(dllexport) BOOL LcAllocScatter2(_In_ DWORD cbData, _Inout_updates_opt_(cbData) PBYTE pbData, _In_ DWORD cMEMs, _Out_ PPMEM_SCATTER* pppMEMs);

	__declspec(dllexport) BOOL LcAllocScatter3(_Inout_updates_opt_(0x1000) PBYTE pbDataFirstPage, _Inout_updates_opt_(0x1000) PBYTE pbDataLastPage, _In_ DWORD cbData, _Inout_updates_opt_(cbData) PBYTE pbData, _In_ DWORD cMEMs, _Out_ PPMEM_SCATTER* pppMEMs);

	BOOL LcReadContigious_Initialize(_In_ PLC_CONTEXT ctxLC);

	BOOL LcGetOption_DoWork(_In_ PLC_CONTEXT ctxLC, _In_ ULONG64 fOption, _Out_ PULONG64 pqwValue);

	BOOL LcSetOption_DoWork(_In_ PLC_CONTEXT ctxLC, _In_ ULONG64 fOption, _In_ ULONG64 qwValue);

	BOOL LcCommand_DoWork(_In_ PLC_CONTEXT ctxLC, _In_ ULONG64 fOption, _In_ DWORD cbDataIn, _In_reads_opt_(cbDataIn) PBYTE pbDataIn, _Out_opt_ PBYTE* ppbDataOut, _Out_opt_ PDWORD pcbDataOut);

	__declspec(dllexport) BOOL LcGetOption(_In_ HANDLE hLC, _In_ ULONG64 fOption, _Out_ PULONG64 pqwValue);

	__declspec(dllexport) BOOL LcSetOption(_In_ HANDLE hLC, _In_ ULONG64 fOption, _In_ ULONG64 qwValue);

	__declspec(dllexport) BOOL LcCommand(_In_ HANDLE hLC, _In_ ULONG64 fCommand, _In_ DWORD cbDataIn, _In_reads_opt_(cbDataIn) PBYTE pbDataIn, _Out_opt_ PBYTE* ppbDataOut, _Out_opt_ PDWORD pcbDataOut);

	ULONG64 LcDeviceParameterGetNumeric(_In_ PLC_CONTEXT ctxLC, _In_ LPSTR szName);

	PLC_DEVICE_PARAMETER_ENTRY LcDeviceParameterGet(_In_ PLC_CONTEXT ctxLC, _In_ LPSTR szName);

#ifdef __cplusplus
}
#endif
#endif
