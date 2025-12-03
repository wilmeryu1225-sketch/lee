
#define CLSID_BINDING_INTERFACE_LEECHRPC           "906B0DC2-1337-0666-0001-0000657A63DD"
#define ADDRDETECT_MAX                              0x10
#define LC_CONFIG_VERSION                           0xc0fd0002
#define LC_CONFIG_ERRORINFO_VERSION                 0xc0fe0002
#define LC_CONTEXT_VERSION                          0xc0e10004
#define MEM_SCATTER_VERSION                         0xc0fe0002
#define LC_STATISTICS_ID_MAX                        0x07
#define LC_DEVICE_PARAMETER_MAX_ENTRIES             0x10
#define MEM_SCATTER_STACK_SIZE                      12
#define LC_STATISTICS_VERSION                       0xe1a10002
#define LC_STATISTICS_ID_OPEN                       0x00
#define LC_STATISTICS_ID_READ                       0x01
#define LC_STATISTICS_ID_READSCATTER                0x02
#define LC_STATISTICS_ID_WRITE                      0x03
#define LC_STATISTICS_ID_WRITESCATTER               0x04
#define LC_STATISTICS_ID_GETOPTION                  0x05
#define LC_STATISTICS_ID_SETOPTION                  0x06
#define LC_STATISTICS_ID_COMMAND                    0x07
#define LC_STATISTICS_ID_MAX                        0x07
#define LC_CONFIG_PRINTF_ENABLED                    0x01
#define LC_CONFIG_PRINTF_V                          0x02
#define LC_CONFIG_PRINTF_VV                         0x04
#define LC_CONFIG_PRINTF_VVV                        0x08
#define LEECHRPC_MSGMAGIC                           0xd05a2667
#define LEECHRPC_FLAG_NOCOMPRESS                    0x0010
#define LEECHRPC_COMPRESS_MAXTHREADS                8
#define LC_PRINTF_ENABLE                            0
#define LC_PRINTF_V                                 1
#define LC_PRINTF_VV                                2
#define LC_PRINTF_VVV                               3
#define VERSION_MAJOR                               2
#define VERSION_MINOR                               22
#define VERSION_REVISION                            3
#define VERSION_BUILD                               89
#define __VFS_FILELISTBLOB_VERSION                  0xf88f0001
#define LC_CMD_AGENT_VFS_REQ_VERSION                0xfeed0001
#define LC_CMD_AGENT_VFS_RSP_VERSION                0xfeee0001
#define LC_CMD_AGENT_VFS_LIST                       0x8000000300000000  
#define LC_CMD_AGENT_VFS_READ                       0x8000000400000000  
#define LC_CMD_AGENT_VFS_WRITE                      0x8000000500000000  
#define LC_MEMMAP_FORCE_OFFSET                      0x8000000000000000
#define LC_OPT_FPGA_ALGO_TINY                       0x0300008400000000  
#define LC_OPT_CORE_PRINTF_ENABLE                   0x4000000100000000  
#define LC_OPT_CORE_VERBOSE                         0x4000000200000000  
#define LC_OPT_CORE_VERBOSE_EXTRA                   0x4000000300000000  
#define LC_OPT_CORE_VERBOSE_EXTRA_TLP               0x4000000400000000  
#define LC_OPT_CORE_VERSION_MAJOR                   0x4000000500000000 
#define LC_OPT_CORE_VERSION_MINOR                   0x4000000600000000 
#define LC_OPT_CORE_VERSION_REVISION                0x4000000700000000  
#define LC_OPT_CORE_ADDR_MAX                        0x1000000800000000  
#define LC_OPT_CORE_STATISTICS_CALL_COUNT           0x4000000900000000 
#define LC_OPT_CORE_STATISTICS_CALL_TIME            0x4000000a00000000 
#define LC_OPT_CORE_VOLATILE                        0x1000000b00000000 
#define LC_OPT_CORE_READONLY                        0x1000000c00000000  
#define LC_CMD_STATISTICS_GET                       0x4000010000000000 
#define LC_CMD_MEMMAP_GET                           0x4000020000000000 
#define LC_CMD_MEMMAP_SET                           0x4000030000000000 
#define LC_CMD_MEMMAP_GET_STRUCT                    0x4000040000000000  
#define LC_CMD_MEMMAP_SET_STRUCT                    0x4000050000000000  
#define MEM_SCATTER_ADDR_INVALID            ((ULONG64)-1)
#define MEM_SCATTER_ADDR_ISINVALID(pMEM)    (pMEM->qwA == (ULONG64)-1)
#define MEM_SCATTER_ADDR_ISVALID(pMEM)      (pMEM->qwA != (ULONG64)-1)
#define MEM_SCATTER_STACK_PUSH(pMEM, v)     (pMEM->vStack[pMEM->iStack++] = (ULONG64)(v))
#define MEM_SCATTER_STACK_PEEK(pMEM, i)     (pMEM->vStack[pMEM->iStack - i])
#define MEM_SCATTER_STACK_SET(pMEM, i, v)   (pMEM->vStack[pMEM->iStack - i] = (ULONG64)(v))
#define MEM_SCATTER_STACK_ADD(pMEM, i, v)   (pMEM->vStack[pMEM->iStack - i] += (ULONG64)(v))
#define MEM_SCATTER_STACK_POP(pMEM)         (pMEM->vStack[--pMEM->iStack])

typedef struct tdLC_CONTEXT LC_CONTEXT, * PLC_CONTEXT;
typedef void* LEECHGRPC_CLIENT_HANDLE, * LEECHGRPC_SERVER_HANDLE;
typedef NTSTATUS WINAPI PFN_RtlCompressBuffer(USHORT, PUCHAR, ULONG, PUCHAR, ULONG, ULONG, PULONG, PVOID);
typedef NTSTATUS WINAPI PFN_RtlDecompressBuffer(USHORT, PUCHAR, ULONG, PUCHAR, ULONG, PULONG);
typedef LEECHGRPC_CLIENT_HANDLE(*pfn_leechgrpc_client_create_insecure)(LPCSTR, DWORD);
typedef LEECHGRPC_CLIENT_HANDLE(*pfn_leechgrpc_client_create_secure_p12)(LPCSTR, DWORD, LPCSTR, LPCSTR, LPCSTR, LPCSTR);
typedef VOID(*pfn_leechgrpc_client_free)(LEECHGRPC_CLIENT_HANDLE hGRPC);
typedef BOOL(*pfn_leechgrpc_client_submit_command)(LEECHGRPC_CLIENT_HANDLE, PBYTE, SIZE_T, PBYTE*, SIZE_T*);

typedef enum {
    LEECHRPC_MSGTYPE_NA = 0,
    LEECHRPC_MSGTYPE_PING_REQ = 1,
    LEECHRPC_MSGTYPE_PING_RSP = 2,
    LEECHRPC_MSGTYPE_OPEN_REQ = 3,
    LEECHRPC_MSGTYPE_OPEN_RSP = 4,
    LEECHRPC_MSGTYPE_CLOSE_REQ = 5,
    LEECHRPC_MSGTYPE_CLOSE_RSP = 6,
    LEECHRPC_MSGTYPE_READSCATTER_REQ = 7,
    LEECHRPC_MSGTYPE_READSCATTER_RSP = 8,
    LEECHRPC_MSGTYPE_WRITESCATTER_REQ = 9,
    LEECHRPC_MSGTYPE_WRITESCATTER_RSP = 10,
    LEECHRPC_MSGTYPE_GETOPTION_REQ = 11,
    LEECHRPC_MSGTYPE_GETOPTION_RSP = 12,
    LEECHRPC_MSGTYPE_SETOPTION_REQ = 13,
    LEECHRPC_MSGTYPE_SETOPTION_RSP = 14,
    LEECHRPC_MSGTYPE_COMMAND_REQ = 15,
    LEECHRPC_MSGTYPE_COMMAND_RSP = 16,
    LEECHRPC_MSGTYPE_KEEPALIVE_REQ = 17,
    LEECHRPC_MSGTYPE_KEEPALIVE_RSP = 18,
    LEECHRPC_MSGTYPE_MAX = 18,
} LEECHRPC_MSGTYPE;

typedef struct LC_CONFIG {
    DWORD dwVersion;                       
    DWORD dwPrintfVerbosity;              
    CHAR szDevice[MAX_PATH];               
    CHAR szRemote[MAX_PATH];                
    _Check_return_opt_ int(*pfn_printf_opt)(_In_z_ _Printf_format_string_ char const* const _Format, ...);
    ULONG64 paMax;                           
    BOOL fVolatile;
    BOOL fWritable;
    BOOL fRemote;
    BOOL fRemoteDisableCompress;
    CHAR szDeviceName[MAX_PATH];           
} LC_CONFIG, * PLC_CONFIG;


typedef struct tdLC_STATISTICS {
    DWORD dwVersion;
    DWORD _Reserved;
    ULONG64 qwFreq;
    struct {
        ULONG64 c;
        ULONG64 tm;  
    } Call[LC_STATISTICS_ID_MAX + 1];
} LC_STATISTICS, * PLC_STATISTICS;

typedef struct tdLC_CONFIG_ERRORINFO {
    DWORD dwVersion;                      
    DWORD cbStruct;
    DWORD _FutureUse[16];
    BOOL fUserInputRequest;
    DWORD cwszUserText;
    WCHAR wszUserText[];
} LC_CONFIG_ERRORINFO, * PLC_CONFIG_ERRORINFO, ** PPLC_CONFIG_ERRORINFO;

typedef struct tdLC_DEVICE_PARAMETER_ENTRY {
    CHAR szName[MAX_PATH];
    CHAR szValue[MAX_PATH];
    ULONG64 qwValue;
} LC_DEVICE_PARAMETER_ENTRY, * PLC_DEVICE_PARAMETER_ENTRY;

typedef struct tdMEM_SCATTER {
    DWORD version;                         
    BOOL f;                                 
    ULONG64 qwA;                             
    union {
        PBYTE pb;                          
        ULONG64 _Filler;
    };
    DWORD cb;                              
    DWORD iStack;                          
    ULONG64 vStack[MEM_SCATTER_STACK_SIZE];  
} MEM_SCATTER, * PMEM_SCATTER, ** PPMEM_SCATTER;

typedef struct tdLC_READ_CONTIGIOUS_CONTEXT {
    PLC_CONTEXT ctxLC;
    HANDLE hEventWakeup;
    HANDLE hEventFinish;
    HANDLE hThread;
    DWORD iRL;
    DWORD cMEMs;
    PPMEM_SCATTER ppMEMs;
    ULONG64 paBase;
    DWORD cbRead;
    DWORD cb;
    BYTE pb[0];
} LC_READ_CONTIGIOUS_CONTEXT, * PLC_READ_CONTIGIOUS_CONTEXT;

typedef struct tdLC_MEMMAP_ENTRY {
    ULONG64 pa;
    ULONG64 cb;
    ULONG64 paRemap;
} LC_MEMMAP_ENTRY, * PLC_MEMMAP_ENTRY;

typedef struct tdLC_CONTEXT {
    DWORD version;     
    DWORD dwHandleCount;
    HANDLE FLink;
    union {
        CRITICAL_SECTION Lock;
        BYTE _PadLinux[48];
    };
    ULONG64 cReadScatterMEM;
    LC_STATISTICS CallStat;
    HANDLE hDeviceModule;
    BOOL(*pfnCreate)(_Inout_ PLC_CONTEXT ctxLC, _Out_opt_ PPLC_CONFIG_ERRORINFO ppLcCreateErrorInfo);
    LC_CONFIG Config;
    DWORD cDeviceParameter;
    LC_DEVICE_PARAMETER_ENTRY pDeviceParameter[LC_DEVICE_PARAMETER_MAX_ENTRIES];
    BOOL fWritable;        
    BOOL fPrintf[4];
    HANDLE hDevice;
    BOOL fMultiThread;
    VOID(*pfnReadScatter)(_In_ PLC_CONTEXT ctxLC, _In_ DWORD cpMEMs, _Inout_ PPMEM_SCATTER ppMEMs);
    VOID(*pfnReadContigious)(_Inout_ PLC_READ_CONTIGIOUS_CONTEXT ctxReadContigious);
    VOID(*pfnWriteScatter)(_In_ PLC_CONTEXT ctxLC, _In_ DWORD cpMEMs, _Inout_ PPMEM_SCATTER ppMEMs);
    BOOL(*pfnWriteContigious)(_In_ PLC_CONTEXT ctxLC, _In_ ULONG64 pa, _In_ DWORD cb, _In_reads_(cb) PBYTE pb);
    BOOL(*pfnGetOption)(_In_ PLC_CONTEXT ctxLC, _In_ ULONG64 fOption, _Out_ PULONG64 pqwValue);
    BOOL(*pfnSetOption)(_In_ PLC_CONTEXT ctxLC, _In_ ULONG64 fOption, _In_ ULONG64 qwValue);
    BOOL(*pfnCommand)(_In_ PLC_CONTEXT ctxLC, _In_ ULONG64 fOption, _In_ DWORD cbDataIn, _In_reads_opt_(cbDataIn) PBYTE pbDataIn, _Out_opt_ PBYTE* ppbDataOut, _Out_opt_ PDWORD pcbDataOut);
    VOID(*pfnClose)(_Inout_ PLC_CONTEXT ctxLC);
    struct {
        DWORD cThread;
        DWORD cbChunkSize;
        BOOL fLoadBalance;
    } ReadContigious;
    struct {
        BOOL fActive;
        HANDLE hEventFinish[8];
        PLC_READ_CONTIGIOUS_CONTEXT ctx[8];
    } RC;
    DWORD cMemMap;
    DWORD cMemMapMax;
    PLC_MEMMAP_ENTRY pMemMap;
    struct {
        BOOL fCompress;
        DWORD dwRpcClientId;
    } Rpc;
} LC_CONTEXT, * PLC_CONTEXT;

typedef struct tdLC_MAIN_CONTEXT {
    CRITICAL_SECTION Lock;
    HANDLE FLink;
} LC_MAIN_CONTEXT, * PLC_MAIN_CONTEXT;

typedef struct tdLEECHRPC_COMPRESS {
    BOOL fValid;
    HANDLE hDll;
    DWORD iCompress;
    struct {
        CRITICAL_SECTION Lock;
        PVOID pbWorkspace;
    } Compress[LEECHRPC_COMPRESS_MAXTHREADS];
    struct {
        PFN_RtlCompressBuffer* pfnRtlCompressBuffer;
        PFN_RtlDecompressBuffer* pfnRtlDecompressBuffer;
    } fn;
} LEECHRPC_COMPRESS, * PLEECHRPC_COMPRESS;

typedef struct LEECHRPC_GRPC {
    HMODULE hDll;
    HANDLE hGRPC;
    pfn_leechgrpc_client_create_insecure pfn_leechgrpc_client_create_insecure;
    pfn_leechgrpc_client_create_secure_p12 pfn_leechgrpc_client_create_secure_p12;
    pfn_leechgrpc_client_free pfn_leechgrpc_client_free;
    pfn_leechgrpc_client_submit_command pfn_leechgrpc_client_submit_command;
    CHAR szClientTlsP12Path[MAX_PATH];
    CHAR szClientTlsP12Password[MAX_PATH];
    CHAR szServerCertCaPath[MAX_PATH];
    CHAR szServerCertHostnameOverride[MAX_PATH];
} LEECHRPC_GRPC, * PLEECHRPC_GRPC;

typedef struct tdLEECHRPC_CLIENT_CONTEXT {
    BOOL fIsProtoRpc;               
    BOOL fIsProtoSmb;              
    BOOL fIsProtoGRpc;             
    BOOL fHousekeeperThread;
    BOOL fHousekeeperThreadIsRunning;
    HANDLE hHousekeeperThread;
    BOOL fIsAuthInsecure;           
    BOOL fIsAuthNTLM;             
    BOOL fIsAuthKerberos;          
    BOOL fIsAuthNTLMCredPrompt;   
    CHAR szRemoteSPN[MAX_PATH];
    CHAR szTcpAddr[MAX_PATH];
    CHAR szTcpPort[6];
    LPSTR szAuthNtlmUserInitOnly;      
    LPSTR szAuthNtlmPasswordInitOnly;  
    RPC_BINDING_HANDLE hRPC;
    RPC_CSTR szStringBinding;
    LEECHRPC_COMPRESS Compress;
    LEECHRPC_GRPC grpc;
} LEECHRPC_CLIENT_CONTEXT, * PLEECHRPC_CLIENT_CONTEXT;

typedef struct tdLEECHRPC_MSG_OPEN {
    DWORD dwMagic;
    DWORD cbMsg;
    LEECHRPC_MSGTYPE tpMsg;
    BOOL fMsgResult;
    DWORD dwRpcClientID;
    DWORD flags;
    BOOL fValidOpen;
    LC_CONFIG cfg;
    LC_CONFIG_ERRORINFO errorinfo;
} LEECHRPC_MSG_OPEN, * PLEECHRPC_MSG_OPEN;

typedef struct tdLEECHRPC_MSG_HDR {
    DWORD dwMagic;
    DWORD cbMsg;
    LEECHRPC_MSGTYPE tpMsg;
    BOOL fMsgResult;
    DWORD dwRpcClientID;
    DWORD flags;
} LEECHRPC_MSG_HDR, * PLEECHRPC_MSG_HDR, ** PPLEECHRPC_MSG_HDR;

typedef struct tdLEECHRPC_MSG_BIN {
    DWORD dwMagic;
    DWORD cbMsg;
    LEECHRPC_MSGTYPE tpMsg;
    BOOL fMsgResult;
    DWORD dwRpcClientID;
    DWORD flags;
    ULONG64 qwData[2];
    DWORD cbDecompress; 
    DWORD cb;
    BYTE pb[];
} LEECHRPC_MSG_BIN, * PLEECHRPC_MSG_BIN;

typedef struct tdLEECHRPC_MSG_DATA {
    DWORD dwMagic;
    DWORD cbMsg;
    LEECHRPC_MSGTYPE tpMsg;
    BOOL fMsgResult;
    DWORD dwRpcClientID;
    DWORD flags;
    ULONG64 qwData[2];
} LEECHRPC_MSG_DATA, * PLEECHRPC_MSG_DATA;

typedef struct td__VFS_FILELISTBLOB_ENTRY {
    ULONG64 ouszName;                       
    ULONG64 cbFileSize;                  
    BYTE pbExInfoOpaque[32];
} __VFS_FILELISTBLOB_ENTRY, * P__VFS_FILELISTBLOB_ENTRY;

typedef struct tdLC_CMD_AGENT_VFS_RSP {
    DWORD dwVersion;
    DWORD dwStatus;              
    DWORD cbReadWrite;             
    DWORD _FutureUse[2];
    DWORD cb;
    BYTE pb[0];
} LC_CMD_AGENT_VFS_RSP, * PLC_CMD_AGENT_VFS_RSP;

typedef struct td__VFS_FILELISTBLOB {
    DWORD dwVersion;                    
    DWORD cbStruct;
    DWORD cFileEntry;
    DWORD cbMultiText;
    union {
        LPSTR uszMultiText;
        ULONG64 _Reserved;
    };
    DWORD _FutureUse[8];
    __VFS_FILELISTBLOB_ENTRY FileEntry[0];
} __VFS_FILELISTBLOB, * P__VFS_FILELISTBLOB;


inline  LC_MAIN_CONTEXT g_ctx = { 0 };