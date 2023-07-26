# 反调试技术 Anti-Debug

## 调试标志 Debug Flags

系统表中的特殊标志（驻留在进程内存中并由操作系统设置）可用于指示正在调试进程。可以通过使用特定的 API 函数或检查内存中的系统表来验证这些标志的状态

这些技术是恶意软件最常用的技术

### IsDebuggerPresent

函数：`kernel32!IsDebuggerPresent`

原理：检查[进程环境块](https://www.nirsoft.net/kernel_struct/vista/PEB.html) (PEB)中的`BeingDebugged`标记

绕过：将PEB中`BeingDebugged`标志设为`0`即可

```c++
bool Check(){   
	return IsDebuggerPresent();
}
```

### CheckRemoteDebuggerPresent

函数：`kernel32!CheckRemoteDebuggerPresent`

```c
WINBASEAPI BOOL WINAPI CheckRemoteDebuggerPresent(
    _In_ HANDLE hProcess,
    _Out_ PBOOL pbDebuggerPresent
    );
```

原理：调用`NtQueryInformationProcess`查询`ProcessDebugPort`

绕过：通过对`NtQueryInformationProcess`的返回值进行修改

```c++
bool Check(){  
	BOOL bDebuggerPresent;
    return (TRUE == CheckRemoteDebuggerPresent(GetCurrentProcess(), &bDebuggerPresent) &&
    TRUE == bDebuggerPresent);
}
```

### NtQueryInformationProcess

函数：`ntdll!NtQueryInformationProcess` (MS未公开) [NTAPI Undoc](http://undocumented.ntinternals.net/index.html?page=UserMode%2FUndocumented Functions%2FNT Objects%2FThread%2FNtSetInformationThread.html)

原理：查询内核`EPROCESS`结构体的调试信息

```c++
typedef NTSTATUS (NTAPI *TNtQueryInformationProcess)(
    IN HANDLE           ProcessHandle,
    IN PROCESSINFOCLASS ProcessInformationClass,
    OUT PVOID           ProcessInformation,
    IN ULONG            ProcessInformationLength,
    OUT PULONG          ReturnLength
    );
HMODULE hNtdll = LoadLibraryA("ntdll.dll");
if (hNtdll){
    auto pfnNtQueryInformationProcess = (TNtQueryInformationProcess)GetProcAddress(
        hNtdll, "NtQueryInformationProcess");
}
```

```
#define ThreadHideFromDebugger 0x11
#define ProcessDebugPort   0x7
#define ProcessDebugObjectHandle 0x1E
#define ProcessDebugFlags 0x1F
```



#### ProcessDebugPort (7)

当进程正在被调试时, 返回值为`0xffffffff`

```c++
bool Check(){  
    if (pfnNtQueryInformationProcess){
        DWORD dwProcessDebugPort, dwReturned;
        NTSTATUS status = pfnNtQueryInformationProcess(
            GetCurrentProcess(),
            ProcessDebugPort,
            &dwProcessDebugPort,
            sizeof(DWORD),
            &dwReturned);
        return NT_SUCCESS(status) && (-1 == dwProcessDebugPort);
    }
    return false;
}
```

#### ProcessDebugObjectHandle (1e)

当调试会话启动, 会同时创建一个`Debug Object`以及对应句柄，可以被查询

```c++
bool Check(){ 
    if (pfnNtQueryInformationProcess){
        DWORD dwProcessDebugFlags, dwReturned;
        const DWORD ProcessDebugFlags = 0x1f;
        NTSTATUS status = pfnNtQueryInformationProcess(
            GetCurrentProcess(),
            ProcessDebugFlags,
            &dwProcessDebugFlags,
            sizeof(DWORD),
            &dwReturned);
        return NT_SUCCESS(status) && (0 == dwProcessDebugFlags);
    }
    return false;
}
```

#### ProcessDebugFlags (1f)

返回`EPROCESS`结构体的`NoDebugInherit`的相反数

```c++
bool Check(){ 
    if (pfnNtQueryInformationProcess)
    {
        DWORD dwReturned;
        HANDLE hProcessDebugObject = 0;
        const DWORD ProcessDebugObjectHandle = 0x1e;
        NTSTATUS status = pfnNtQueryInformationProcess(
            GetCurrentProcess(),
            ProcessDebugObjectHandle,
            &hProcessDebugObject,
            sizeof(HANDLE),
            &dwReturned);
        return NT_SUCCESS(status) && (0 != hProcessDebugObject);
    }
    return false;
}
```

#### ProcessBasicInformation

通过父进程检查是否被调试器启动

```c
typedef struct _PROCESS_BASIC_INFORMATION
{
    NTSTATUS ExitStatus;
    PPEB PebBaseAddress;
    ULONG_PTR AffinityMask;
    KPRIORITY BasePriority;
    HANDLE UniqueProcessId;
    HANDLE InheritedFromUniqueProcessId; //父进程PID
} PROCESS_BASIC_INFORMATION, *PPROCESS_BASIC_INFORMATION;
```

```c++
bool Check(){ 
    if (pfnNtQueryInformationProcess)
    {
        DWORD dwReturned;
        PROCESS_BASIC_INFORMATION Basic;
        const DWORD ProcessDebugObjectHandle = 0;
        NTSTATUS status = pfnNtQueryInformationProcess(
            GetCurrentProcess(),
            ProcessBasicInformation,
            &&Basic,
            sizeof(Basic),
            &dwReturned);
        return Basic.InheritedFromUniqueProcessId != ULongToHandle(PID("explorer.exe"));
    }
    return false;
}
```



### NtQuerySystemInformation

函数：`ntdll!NtQuerySystemInformation` 

```c
NTSYSCALLAPI NTSTATUS NTAPI NtQuerySystemInformation(
    _In_ SYSTEM_INFORMATION_CLASS SystemInformationClass,
    _Out_opt_ PVOID SystemInformation,
    _In_ ULONG SystemInformationLength,
    _Out_opt_ PULONG ReturnLength
    );
```

原理：查询`SystemKernelDebuggerInformation`的信息

检测当前系统是否正在调试状态

这个函数可以判断当前这个系统是否被内核调试器给附加或者是给调试状态、比如双机调试

```c++
enum { SystemKernelDebuggerInformation = 0x23 };
typedef struct _SYSTEM_KERNEL_DEBUGGER_INFORMATION { 
    BOOLEAN DebuggerEnabled; 
    BOOLEAN DebuggerNotPresent; 
} SYSTEM_KERNEL_DEBUGGER_INFORMATION, *PSYSTEM_KERNEL_DEBUGGER_INFORMATION; 

bool Check(){
    NTSTATUS status;
    SYSTEM_KERNEL_DEBUGGER_INFORMATION SystemInfo;
    
    status = NtQuerySystemInformation(
        (SYSTEM_INFORMATION_CLASS)SystemKernelDebuggerInformation,
        &SystemInfo,
        sizeof(SystemInfo),
        NULL);
    return SUCCEEDED(status)
        ? (SystemInfo.DebuggerEnabled && !SystemInfo.DebuggerNotPresent)
        : false;
}
```

#### ZwQueryInformationProcess

```c++
extern "C" int __stdcall ZwQueryInformationProcess(HANDLE,int,unsigned long*,unsigned long,unsigned long*);
bool Check(){
	unsigned long _port_=0;
	ZwQueryInformationProcess(GetCurrentProcess(),ProcessDebugPort,&_port_,0x4,0);
	if(_port_)return true;
    
	unsigned long DbgObjHand=0;
	int ret=ZwQueryInformationProcess(GetCurrentProcess(),ProcessDebugObjectHandle,&DbgObjHand,0x4,0);
	if(ret>=0 || DbgObjHand)return true;
    
    unsigned long DbgFlags=0;
    ZwQueryInformationProcess(GetCurrentProcess(),ProcessDebugFlags,&DbgFlags,0x4,0);
    if(DbgFlags==0)return true;
}
```



### ThreadNtSetInformationThread 

 `_ETHREAD->HideFromDebugger & ThreadHideFromDebugger `

设置了ThreadHideFromDebugger的线程不会发送调试事件，相当于主动分离调试器

#### NtSetInformationThread

```c++
typedef NTSTATUS (NTAPI *pfnNtSetInformationThread)(
    _In_ HANDLE ThreadHandle,
    _In_ ULONG  ThreadInformationClass,
    _In_ PVOID  ThreadInformation,
    _In_ ULONG  ThreadInformationLength
    );
const ULONG ThreadHideFromDebugger = 0x11;
void HideFromDebugger()
{
    HMODULE hNtDll = LoadLibrary(TEXT("ntdll.dll"));
    pfnNtSetInformationThread NtSetInformationThread = (pfnNtSetInformationThread)
        GetProcAddress(hNtDll, "NtSetInformationThread");
    NTSTATUS status = NtSetInformationThread(GetCurrentThread(), 
        ThreadHideFromDebugger, NULL, 0);
}	
```

#### ZwSetInformationThread

```c++
#define ThreadHideFromDebugger 0x11
extern "C"
{
       int __stdcall ZwSetInformationThread(HANDLE,int,unsigned long*,unsigned long);
}
ZwSetInformationThread(GetCurrentThread(),ThreadHideFromDebugger,0,0);
```

#### ZwCreateThreadEx

https://pastebin.com/jAv5GYUd

直接使用`ZwCreateThreadEx`创建调试器不可见进程

```c
struct UNICODE_S{
    unsigned short len;
    unsigned short max;
    wchar_t* pStr;
};
struct OBJECT_ATTRIBUTES{
  unsigned long           Length;
  HANDLE                  RootDirectory;
  UNICODE_S*              ObjectName;
  unsigned long           Attributes;
  void*           SecurityDescriptor;
  void*           SecurityQualityOfService;
};
 
typedef int(__stdcall *pfnZwCreateThreadEx)(HANDLE* hThread,int DesiredAccess,OBJECT_ATTRIBUTES* ObjectAttributes,HANDLE ProcessHandle,void* lpStartAddress,void* lpParameter,unsigned long CreateSuspended_Flags,unsigned long StackZeroBits,unsigned long SizeOfStackCommit,unsigned long SizeOfStackReserve,void* lpBytesBuffer);
pfnZwCreateThreadEx ZwCreateThreadEx=(pfnZwCreateThreadEx)GetProcAddress(GetModuleHandle("ntdll.dll"),"ZwCreateThreadEx");
    if(ZwCreateThreadEx){
        HANDLE hThread=0;
        ZwCreateThreadEx(&hThread,0x1FFFFF,0,GetCurrentProcess(),&dummy,0,0x4/*HiddenFromDebugger*/,0,0x1000,0x10000,0);
        if(hThread)WaitForSingleObject(hThread,INFINITE);
    }
```





### RtlIsAnyDebuggerPresent

函数：`ntdll!RtlIsAnyDebuggerPresent`

检查`PEB->IsBeingDebugged`和`KUSER_SHARED_DATA->KdDebuggerEnabled`



### RtlQueryProcessHeapInformation

函数：`ntdll!RtlQueryProcessHeapInformation`

原理：从当前进程中的堆标识判断

```c++
bool Check()
{
    ntdll::PDEBUG_BUFFER pDebugBuffer = ntdll::RtlCreateQueryDebugBuffer(0, FALSE);
    if (!SUCCEEDED(ntdll::RtlQueryProcessHeapInformation((ntdll::PRTL_DEBUG_INFORMATION)pDebugBuffer)))
        return false;
    ULONG dwFlags = ((ntdll::PRTL_PROCESS_HEAPS)pDebugBuffer->HeapInformation)->Heaps[0].Flags;
    return dwFlags & ~HEAP_GROWABLE;
}
```

### RtlQueryProcessDebugInformation

原理：从进程中的堆标识判断

```c++
bool Check()
{
    ntdll::PDEBUG_BUFFER pDebugBuffer = ntdll::RtlCreateQueryDebugBuffer(0, FALSE);
    if (!SUCCEEDED(ntdll::RtlQueryProcessDebugInformation(GetCurrentProcessId(), ntdll::PDI_HEAPS | ntdll::PDI_HEAP_BLOCKS, pDebugBuffer)))
        return false;
    ULONG dwFlags = ((ntdll::PRTL_PROCESS_HEAPS)pDebugBuffer->HeapInformation)->Heaps[0].Flags;
    return dwFlags & ~HEAP_GROWABLE;
}
```



### PEB checks

直接检测PEB中的标识

#### 结构体

```c
typedef struct _PEB
{
    BOOLEAN InheritedAddressSpace;
    BOOLEAN ReadImageFileExecOptions;
    BOOLEAN BeingDebugged;
    union
    {
        BOOLEAN BitField;
        struct
        {
            BOOLEAN ImageUsesLargePages : 1;
            BOOLEAN IsProtectedProcess : 1;
            BOOLEAN IsImageDynamicallyRelocated : 1;
            BOOLEAN SkipPatchingUser32Forwarders : 1;
            BOOLEAN IsPackagedProcess : 1;
            BOOLEAN IsAppContainer : 1;
            BOOLEAN IsProtectedProcessLight : 1;
            BOOLEAN IsLongPathAwareProcess : 1;
        } s1;
    } u1;
 
    HANDLE Mutant;
 
    PVOID ImageBaseAddress;
    PPEB_LDR_DATA Ldr;
    PRTL_USER_PROCESS_PARAMETERS ProcessParameters;
    PVOID SubSystemData;
    PVOID ProcessHeap;
    PRTL_CRITICAL_SECTION FastPebLock;
    PVOID AtlThunkSListPtr;
    PVOID IFEOKey;
    union
    {
        ULONG CrossProcessFlags;
        struct
        {
            ULONG ProcessInJob : 1;
            ULONG ProcessInitializing : 1;
            ULONG ProcessUsingVEH : 1;
            ULONG ProcessUsingVCH : 1;
            ULONG ProcessUsingFTH : 1;
            ULONG ProcessPreviouslyThrottled : 1;
            ULONG ProcessCurrentlyThrottled : 1;
            ULONG ReservedBits0 : 25;
        } s2;
    } u2;
    union
    {
        PVOID KernelCallbackTable;
        PVOID UserSharedInfoPtr;
    } u3;
    ULONG SystemReserved[1];
    ULONG AtlThunkSListPtr32;
    PVOID ApiSetMap;
    ULONG TlsExpansionCounter;
    PVOID TlsBitmap;
    ULONG TlsBitmapBits[2];
    PVOID ReadOnlySharedMemoryBase;
    PVOID HotpatchInformation;
    PVOID *ReadOnlyStaticServerData;
    PVOID AnsiCodePageData;
    PVOID OemCodePageData;
    PVOID UnicodeCaseTableData;
 
    ULONG NumberOfProcessors;
    ULONG NtGlobalFlag;
 
    LARGE_INTEGER CriticalSectionTimeout;
    SIZE_T HeapSegmentReserve;
    SIZE_T HeapSegmentCommit;
    SIZE_T HeapDeCommitTotalFreeThreshold;
    SIZE_T HeapDeCommitFreeBlockThreshold;
 
    ULONG NumberOfHeaps;
    ULONG MaximumNumberOfHeaps;
    PVOID *ProcessHeaps;
 
    PVOID GdiSharedHandleTable;
    PVOID ProcessStarterHelper;
    ULONG GdiDCAttributeList;
 
    PRTL_CRITICAL_SECTION LoaderLock;
 
    ULONG OSMajorVersion;
    ULONG OSMinorVersion;
    USHORT OSBuildNumber;
    USHORT OSCSDVersion;
    ULONG OSPlatformId;
    ULONG ImageSubsystem;
    ULONG ImageSubsystemMajorVersion;
    ULONG ImageSubsystemMinorVersion;
    ULONG_PTR ActiveProcessAffinityMask;
    GDI_HANDLE_BUFFER GdiHandleBuffer;
    PVOID PostProcessInitRoutine;
 
    PVOID TlsExpansionBitmap;
    ULONG TlsExpansionBitmapBits[32];
 
    ULONG SessionId;
 
    ULARGE_INTEGER AppCompatFlags;
    ULARGE_INTEGER AppCompatFlagsUser;
    PVOID pShimData;
    PVOID AppCompatInfo;
 
    UNICODE_STRING CSDVersion;
 
    PVOID ActivationContextData;
    PVOID ProcessAssemblyStorageMap;
    PVOID SystemDefaultActivationContextData;
    PVOID SystemAssemblyStorageMap;
 
    SIZE_T MinimumStackCommit;
 
    PVOID *FlsCallback;
    LIST_ENTRY FlsListHead;
    PVOID FlsBitmap;
    ULONG FlsBitmapBits[FLS_MAXIMUM_AVAILABLE / (sizeof(ULONG) * 8)];
    ULONG FlsHighIndex;
 
    PVOID WerRegistrationData;
    PVOID WerShipAssertPtr;
    PVOID pContextData;
    PVOID pImageHeaderHash;
    union
    {
        ULONG TracingFlags;
        struct
        {
            ULONG HeapTracingEnabled : 1;
            ULONG CritSecTracingEnabled : 1;
            ULONG LibLoaderTracingEnabled : 1;
            ULONG SpareTracingBits : 29;
        } s3;
    } u4;
    ULONGLONG CsrServerReadOnlySharedMemoryBase;
    PVOID TppWorkerpListLock;
    LIST_ENTRY TppWorkerpList;
    PVOID WaitOnAddressHashTable[128];
} PEB, *PPEB;
```

##### 获取进程PEB地址 x32/x64

```c++
PVOID GetPEB(){
#ifdef _WIN64
    return (PVOID)__readgsqword(0x0C * sizeof(PVOID)); //gs:0x60
#else
    return (PVOID)__readfsdword(0x0C * sizeof(PVOID)); //fs:0x30
#endif
}
```

##### 获取进程PEB地址WOW64

windows系统通过wow64机制在x64上运行x32程序，这会导致两个PEB结构产生，一个是32位的，一个是64位的，通过下面的方法可以获取到WOW64下64位的PEB

```c++
PVOID GetPEB64(){
    PVOID pPeb = 0;
#ifndef _WIN64
    // 1. WOW64下的进程存在两个 PEB - PEB64 和 PEB32 
    // 2. PEB64 在 PEB32 之后
    //如果版本大于 Windows8 才需要获取真实 PEB64
    if (IsWin8OrHigher()){
        BOOL isWow64 = FALSE;
        typedef BOOL(WINAPI *pfnIsWow64Process)(HANDLE hProcess, PBOOL isWow64);
        pfnIsWow64Process fnIsWow64Process = (pfnIsWow64Process)GetProcAddress(GetModuleHandleA("Kernel32.dll"), "IsWow64Process");
        //通过 Kernel32.dll!IsWow64Process 判断是否wow64
        if (fnIsWow64Process(GetCurrentProcess(), &isWow64)){
            if (isWow64){
                //定位到PEB32
                pPeb = (PVOID)__readfsdword(0x0C * sizeof(PVOID));
                //定位到PEB32后的PEB64
                pPeb = (PVOID)((PBYTE)pPeb + 0x1000);
            }
        }
    }
#endif
    return pPeb;
}
```

##### 获得其他进程PEB地址

```c++
GetPebAddress(HANDLE hProcess){
::PROCESS_BASIC_INFORMATION pbi = { 0 };
auto status = NtQueryInformationProcess(hProcess, ProcessBasicInformation, &pbi, sizeof(pbi), nullptr);
return NT_SUCCESS(status) ? (PEB *)pbi.PebBaseAddress : nullptr;
}
```

##### 检查系统信息

```c++
WORD GetVersionWord()
{
    OSVERSIONINFO verInfo = { sizeof(OSVERSIONINFO) };
    GetVersionEx(&verInfo);
    return MAKEWORD(verInfo.dwMinorVersion, verInfo.dwMajorVersion);
}
BOOL IsWin8OrHigher() { return GetVersionWord() >= _WIN32_WINNT_WIN8; }
BOOL IsVistaOrHigher() { return GetVersionWord() >= _WIN32_WINNT_VISTA; }	
```

#### BeingDebugged

和IsDebuggerPresent等价

```c++
bool Check() {
    pPeb=GetPEB();
	if (pPeb->BeingDebugged)return true;
    return false;
}
```

#### NtGlobalFlag

被调试时，NtGlobalFlag的这三个标志被设置

```c
#define FLG_HEAP_ENABLE_TAIL_CHECK   0x10
#define FLG_HEAP_ENABLE_FREE_CHECK   0x20
#define FLG_HEAP_VALIDATE_PARAMETERS 0x40
#define NT_GLOBAL_FLAG_DEBUGGED (FLG_HEAP_ENABLE_TAIL_CHECK | FLG_HEAP_ENABLE_FREE_CHECK | FLG_HEAP_VALIDATE_PARAMETERS)

#ifndef _WIN64
PPEB pPeb = (PPEB)__readfsdword(0x30);
DWORD dwNtGlobalFlag = *(PDWORD)((PBYTE)pPeb + 0x68);
#else
PPEB pPeb = (PPEB)__readgsqword(0x60);
DWORD dwNtGlobalFlag = *(PDWORD)((PBYTE)pPeb + 0xBC);
#endif // _WIN64
 
if (dwNtGlobalFlag & NT_GLOBAL_FLAG_DEBUGGED)
    goto being_debugged;
```

```c++
if (pPeb->NtGlobalFlag & 0x70)return true;
```

#### Heap Flags

PEB中的下述字段包含了堆信息

```c++
    PVOID ProcessHeap;
	ULONG NumberOfHeaps;
    ULONG MaximumNumberOfHeaps;
    PVOID *ProcessHeaps;
```

堆首对应结构体`_HEAP`，其中包含有两个受调试器影响的字段 `flags`和`force_flags`

当进程被调试的时候，flags的HEAP_GROWABLE位被清除；ForceFlags被设置为非0值

32位系统上的Flags 

```
HEAP_GROWABLE (2)
HEAP_TAIL_CHECKING_ENABLED (0x20)
HEAP_FREE_CHECKING_ENABLED (0x40)
HEAP_SKIP_VALIDATION_CHECKS (0x10000000)
HEAP_VALIDATE_PARAMETERS_ENABLED (0x40000000)
```

64位系统上Flags 

```
HEAP_GROWABLE (2)
HEAP_TAIL_CHECKING_ENABLED (0x20)
HEAP_FREE_CHECKING_ENABLED (0x40)
HEAP_VALIDATE_PARAMETERS_ENABLED (0x40000000)
```

ForceFlags

```
HEAP_GROWABLE (2)
HEAP_TAIL_CHECKING_ENABLED (0x20)
HEAP_FREE_CHECKING_ENABLED (0x40)
HEAP_VALIDATE_PARAMETERS_ENABLED (0x40000000)
```

```c++
int GetHeapFlagsOffset(bool x64)
{
    return x64 ?
        IsVistaOrHigher() ? 0x70 : 0x14: //x64 offsets
        IsVistaOrHigher() ? 0x40 : 0x0C; //x86 offsets
}
int GetHeapForceFlagsOffset(bool x64)
{
    return x64 ?
        IsVistaOrHigher() ? 0x74 : 0x18: //x64 offsets
        IsVistaOrHigher() ? 0x44 : 0x10; //x86 offsets
}
void CheckHeap()
{
    PVOID pPeb = GetPEB();
    PVOID pPeb64 = GetPEB64();
    PVOID heap = 0;
    DWORD offsetProcessHeap = 0;
    PDWORD heapFlagsPtr = 0, heapForceFlagsPtr = 0;
    BOOL x64 = FALSE;
#ifdef _WIN64
    x64 = TRUE;
    offsetProcessHeap = 0x30;
#else
    offsetProcessHeap = 0x18;
#endif
    heap = (PVOID)*(PDWORD_PTR)((PBYTE)pPeb + offsetProcessHeap);
    heapFlagsPtr = (PDWORD)((PBYTE)heap + GetHeapFlagsOffset(x64));
    heapForceFlagsPtr = (PDWORD)((PBYTE)heap + GetHeapForceFlagsOffset(x64));
    if (*heapFlagsPtr & ~HEAP_GROWABLE || *heapForceFlagsPtr != 0)return true;
    if (pPeb64)
    {
        heap = (PVOID)*(PDWORD_PTR)((PBYTE)pPeb64 + 0x30);
        heapFlagsPtr = (PDWORD)((PBYTE)heap + GetHeapFlagsOffset(true));
        heapForceFlagsPtr = (PDWORD)((PBYTE)heap + GetHeapForceFlagsOffset(true));
        if (*heapFlagsPtr & ~HEAP_GROWABLE || *heapForceFlagsPtr != 0)return true;
    }
    return false;
}
```

#### Heap Protection

根据Heap Flags，堆分配会有一些额外的行为

如果 HEAP_TAIL_CHECKING_ENABLED (0x20) 被设置，堆块尾部会追加`0xABABABAB ABABABAB` (两个PVOID的长度)

如果 HEAP_FREE_CHECKING_ENABLED (0x40) 被设置，堆块的对齐字节填充追加`0xFEEEFEEE`

```c++
bool Check(){
    PROCESS_HEAP_ENTRY HeapEntry = { 0 };
    do{
        if (!HeapWalk(GetProcessHeap(), &HeapEntry))
            return false;
    } while (HeapEntry.wFlags != PROCESS_HEAP_ENTRY_BUSY);
    PVOID pOverlapped = (PBYTE)HeapEntry.lpData + HeapEntry.cbData;
    return ((DWORD)(*(PDWORD)pOverlapped) == 0xABABABAB);
}
```

#### Back BeingDebugged

PEB中的BeingDebugged被设为True时导致了后续的变化

- NtGlobalFlag
- RtlCreateHeap中的堆创建方法变为RtlDebugCreateHeap
  -  Heap Flags
  - Heap Protection

如果在上述变化发生前就修改BeingDebugged即可避免这些特征产生并被检测

**但是如果把BeingDebugged设为False，就不会在系统断点处触发中断了**

因此需要在特定的时机修改BeingDebugged

| DebugEventCode        | Count | PEB BeingDebugged | Note                 |
| --------------------- | ----- | ----------------- | -------------------- |
| LOAD_DLL_DEBUG_EVENT  | 0     | FALSE             |                      |
| LOAD_DLL_DEBUG_EVENT  | 1     | TRUE              |                      |
| EXCEPTION_DEBUG_EVENT | 0     | FALSE             | EXCEPTION_BREAKPOINT |

#### KUSER_SHARED_DATA

kuser_shared_data是一个用户态和内核态共享的结构体，用户态只有读的权限

该结构和内核相关，具有稳定的地址和结构体偏移（x64和x86相同）

其中和debug相关的值为`kuser_shared_data->KdDebuggerEnabled`，该值存在时说明内核调试器存在，**3环的调试器并不会导致这个值变化**

由于SoftICE的内核调试机制是自己实现的，未使用Windows原生的内核调试机制，因此也不会导致这个值变化

| Offset | Definition                   | Versions       | Remarks            |
| :----- | :--------------------------- | :------------- | :----------------- |
| 0x02D4 | `BOOLEAN KdDebuggerEnabled;` | 5.0 and higher | last member in 5.0 |

```c++
bool check_kuser_shared_data_structure(){
    unsigned char b = *(unsigned char*)0x7ffe0000 + 0x2d4;
    return ((b & 0x01) || (b & 0x02));
}
```





## 对象句柄 Object Handles

接受内核对象句柄作为参数的WinAPI函数在调试时可能会有不同的行为，或者会**因为调试器的实现而产生副作用**

此外，在调试开始时，操作系统会创建特定的内核对象

### OpenProcess

函数：`kernel32!OpenProcess`

原理： 判断**进程是否有SeDebugPrivilege权限**

反反调试：拦截`ntdll!OpenProcess`，如果参数是`csrss.exe`的句柄则返回Null

一般进程而言，如果用OpenProcess打开`csrss.exe`程序则会返回无权限访问，如果以管理员身份登录并且进程被调试器调试的话，调试器拥有SeDebugPrivilege权限，被调试进程会继承SeDebugPrivilege权限，有了此权限被调试程序就可以打开csrss.exe程序了

win 8.1后因为有PPL，被调试进程不再能打开了

```c++
typedef DWORD (WINAPI *TCsrGetProcessId)(VOID);
bool Check(){   
    HMODULE hNtdll = LoadLibraryA("ntdll.dll");
    if (!hNtdll)
        return false;
    TCsrGetProcessId pfnCsrGetProcessId = (TCsrGetProcessId)GetProcAddress(hNtdll, "CsrGetProcessId");
    if (!pfnCsrGetProcessId)
        return false;
    HANDLE hCsr = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pfnCsrGetProcessId());
    if (hCsr != NULL)
    {
        CloseHandle(hCsr);
        return true;
    }        
    else
        return false;
}
```



### CreateFile

函数：`kernel32!CreateFileW` 或 `Kernel32!CreateFileA`

原理：通过**被调试文件的占用情况**

反反调试：关闭被调试文件句柄

`CREATE_PROCESS_DEBUG_EVENT`发生时，被调试文件句柄存储在[`CREATEPROCESS_DEBUG_INFO`](https://learn.microsoft.com/en-us/windows/win32/api/minwinbase/ns-minwinbase-create_process_debug_info)结构中

如果调试器未关闭此句柄，则无法以**独占访问方式**打开文件。

使用`kernel32!CreateFileW`或`Kernel32!CreateFileA`以独占方式打开当前进程文件，如果调用失败，则可以认为当前进程存在调试器。

```c++
bool Check(){
    CHAR szFileName[MAX_PATH];
    if (0 == GetModuleFileNameA(NULL, szFileName, sizeof(szFileName)))
        return false;
    return INVALID_HANDLE_VALUE == CreateFileA(szFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, 0);
}
```



**LoadLibrary**

函数：`kernel32!LoadLibraryW`或`kernel32!LoadLibraryA`

原理：通过**被加载文件的占用情况**

反反调试：没有

`LOAD_DLL_DEBUG_EVENT`发生时，被加载文件句柄存储在[`LOAD_DLL_DEBUG_INFO`](https://learn.microsoft.com/en-us/windows/win32/api/minwinbase/ns-minwinbase-load_dll_debug_info)结构中

如果调试器未关闭此句柄，则无法以**独占访问方式**打开文件。

使用 `kernel32!LoadLibraryA`加载任何文件 然后 使用`kernel32!CreateFileW`或`Kernel32!CreateFileA`以独占方式打开该文件，如果调用失败，则可以认为当前进程存在调试器。

```c++
bool Check(){
    CHAR szBuffer[] = { "C:\\Windows\\System32\\calc.exe" };
    LoadLibraryA(szBuffer);
    return INVALID_HANDLE_VALUE == CreateFileA(szBuffer, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
}
```



### NtClose

函数：`ntdll!NtClose`  或 `kernel32!CloseHandle`

原理：通过调试器的**异常接管处理**来判断

反反调试：拦截`ntdll!NtClose` 使用`ntdll!NtQueryObject`检查句柄是否有效

使用`ntdll!NtClose`  或 `kernel32!CloseHandle`关闭无效句柄时会产生`EXCEPTION_INVALID_HANDLE`（0xC0000008）异常

由于Windows内核对象句柄跟踪机制，这个异常正常情况下是被缓存的，但在被调试的情况下，这个异常**会被传递**给异常处理程序

```c++
bool Check(){
    __try{
        NtClose((HANDLE)0xDEADBEEF);
        return false;
    }__except (EXCEPTION_INVALID_HANDLE == GetExceptionCode()
                ? EXCEPTION_EXECUTE_HANDLER 
                : EXCEPTION_CONTINUE_SEARCH){
        return true;
    }
}
```

### NtDuplicateObject

函数：`ntdll!NtDuplicateObject`

原理：NtClose的变种 通过调试器的**异常接管处理**来判断 但是构造句柄异常的方式更加隐蔽 是SafeEngine商业版使用的反调试之一

反反调试：拦截`ntdll!NtDuplicateObject`  可以使用 SharpOD *ZwFunctions

使用`NtDuplicateObject`获取当前进程句柄并设置为禁止关闭，之后再使用`NtDuplicateObject`并关闭原句柄，则必然产生句柄异常

````c++
bool Check(){
    __try{
        HANDLE hTarget, hNewTarget;
        DuplicateHandle((HANDLE)-1, (HANDLE)-1, (HANDLE)-1, &hTarget, 0, 0, DUPLICATE_SAME_ACCESS);
        SetHandleInformation(hTarget, HANDLE_FLAG_PROTECT_FROM_CLOSE, HANDLE_FLAG_PROTECT_FROM_CLOSE);
        DuplicateHandle((HANDLE)-1, (HANDLE)hTarget, (HANDLE)-1, &hNewTarget, 0, 0, DUPLICATE_CLOSE_SOURCE);
        return false;
    }__except (EXCEPTION_INVALID_HANDLE == GetExceptionCode()
                ? EXCEPTION_EXECUTE_HANDLER 
                : EXCEPTION_CONTINUE_SEARCH){
        return true;
    }
}
````

### NtQueryObject

函数：`ntdll!NtQueryObject`

原理：调试会话会产生`Debug Object`的内核对象并存在句柄

反反调试：拦截`ntdll!NtQueryObject`，如果ObjectAllTypesInformation类被查询，从结果中过滤调试对象

这个技术不能确定当前进程是否正在被调试，只能确定是否有调试器运行

```c++
typedef struct _OBJECT_TYPE_INFORMATION{
    UNICODE_STRING TypeName;
    ULONG TotalNumberOfHandles;
    ULONG TotalNumberOfObjects;
} OBJECT_TYPE_INFORMATION, *POBJECT_TYPE_INFORMATION;
typedef struct _OBJECT_ALL_INFORMATION{
    ULONG NumberOfObjects;
    OBJECT_TYPE_INFORMATION ObjectTypeInformation[1];
} OBJECT_ALL_INFORMATION, *POBJECT_ALL_INFORMATION;
typedef NTSTATUS (WINAPI *TNtQueryObject)(
    HANDLE                   Handle,
    OBJECT_INFORMATION_CLASS ObjectInformationClass,
    PVOID                    ObjectInformation,
    ULONG                    ObjectInformationLength,
    PULONG                   ReturnLength
);
enum { ObjectAllTypesInformation = 3 };
#define STATUS_INFO_LENGTH_MISMATCH 0xC0000004

bool Check(){
    bool bDebugged = false;
    NTSTATUS status;
    LPVOID pMem = nullptr;
    ULONG dwMemSize;
    POBJECT_ALL_INFORMATION pObjectAllInfo;
    PBYTE pObjInfoLocation;
    HMODULE hNtdll;
    TNtQueryObject pfnNtQueryObject;
     
    hNtdll = LoadLibraryA("ntdll.dll");
    if (!hNtdll) return false;
    pfnNtQueryObject = (TNtQueryObject)GetProcAddress(hNtdll, "NtQueryObject");
    if (!pfnNtQueryObject) return false;
    status = pfnNtQueryObject(
        NULL,
        (OBJECT_INFORMATION_CLASS)ObjectAllTypesInformation,
        &dwMemSize, sizeof(dwMemSize), &dwMemSize);
    if (STATUS_INFO_LENGTH_MISMATCH != status) goto NtQueryObject_Cleanup;
    pMem = VirtualAlloc(NULL, dwMemSize, MEM_COMMIT, PAGE_READWRITE);
    if (!pMem) goto NtQueryObject_Cleanup;
    status = pfnNtQueryObject(
        (HANDLE)-1,
        (OBJECT_INFORMATION_CLASS)ObjectAllTypesInformation,
        pMem, dwMemSize, &dwMemSize);
    if (!SUCCEEDED(status)) goto NtQueryObject_Cleanup;
    pObjectAllInfo = (POBJECT_ALL_INFORMATION)pMem;
    pObjInfoLocation = (PBYTE)pObjectAllInfo->ObjectTypeInformation;
    for(UINT i = 0; i < pObjectAllInfo->NumberOfObjects; i++) {
        POBJECT_TYPE_INFORMATION pObjectTypeInfo =
            (POBJECT_TYPE_INFORMATION)pObjInfoLocation;
        if (wcscmp(L"DebugObject", pObjectTypeInfo->TypeName.Buffer) == 0){
            if (pObjectTypeInfo->TotalNumberOfObjects > 0)
                bDebugged = true;
            break;
        }
        // Get the address of the current entries
        // string so we can find the end
        pObjInfoLocation = (PBYTE)pObjectTypeInfo->TypeName.Buffer;
        // Add the size
        pObjInfoLocation += pObjectTypeInfo->TypeName.Length;
        // Skip the trailing null and alignment bytes
        ULONG tmp = ((ULONG)pObjInfoLocation) & -4;
        // Not pretty but it works
        pObjInfoLocation = ((PBYTE)tmp) + sizeof(DWORD);
    }
 
NtQueryObject_Cleanup:
    if (pMem) VirtualFree(pMem, 0, MEM_RELEASE);
    return bDebugged;
}
```



## 异常处理

### INT 2D

int2d是系统调试服务使用的向量，在ring3下使用时会产生异常，但如果存在调试器则不会进入异常

同时，可以用于检测ring0下的SoftICE

32位

```c
bool IsDebugged() {
    __try {
        __asm {
            xor eax, eax
            int 0x2d
            nop
        }
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}
```

#### DbgPrint/DbgPrompt

仅限于**Wow64**下的一个方法

可以隐藏直接的指令

```c
typedef ULONG(WINAPI* pfnNtDbgPrint)(PCSTR Format, ...);
typedef ULONG(WINAPI* pfnNtDbgPrompt)(PCCH Prompt, PCH Response, ULONG Length);
pfnNtDbgPrint      DbgPrint;
pfnNtDbgPrompt     DbgPrompt;
BOOL InitNtFuncs() {
    HMODULE h_module = GetModuleHandle(TEXT("ntdll.dll"));
    if (!h_module)return FALSE;
    DbgPrint  = (pfnNtDbgPrint)GetProcAddress(h_module, "DbgPrint");
    DbgPrompt = (pfnNtDbgPrompt)GetProcAddress(h_module, "DbgPrompt");
    return TRUE;
}
bool IsDebugged1() {
    __try {
        DbgPrint("Hello");
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}
bool IsDebugged2() {
    unsigned char* resp=(unsigned char*)LocalAlloc(LMEM_ZEROINIT,0x100);
    __try {
        DbgPrompt(L"World",resp,0x100);
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}
```

### INT  01

#### TrapFlag

通过主动设置TrapFlag的值 触发单步异常，如果调试器捕获异常则被调试

```c++
BOOL isDebugged = TRUE;
__try
{
    __asm
    {
        pushfd
        or dword ptr[esp], 0x100 // set the Trap Flag 
        popfd                    // Load the value into EFLAGS register
        nop
    }
}
__except (EXCEPTION_EXECUTE_HANDLER)
{
    // If an exception has been raised – debugger is not present
    isDebugged = FALSE;
}
if (isDebugged)
{
    std::cout << "Stop debugging program!" << std::endl;
    exit(-1);
}
```





## 进程内存

### 断点检测

```c++
IMAGE_DOS_HEADER *dos_head=(IMAGE_DOS_HEADER *)GetModuleHandle(NULL);
PIMAGE_NT_HEADERS32 nt_head=(PIMAGE_NT_HEADERS32)((DWORD)dos_head+(DWORD)dos_head->e_lfanew);
BYTE*OEP=(BYTE*)(nt_head->OptionalHeader.AddressOfEntryPoint+(DWORD)dos_head);
//扫描程序入口点的20字节是否存在调试断点
for(unsigned long index=0;index<20;index++){
	if(OEP[index]==0xcc)ExitProcess(0);
}
```



### 硬断检测

从线程上下文中检查DR寄存器是否设置硬件断点

```cpp
bool IsDebugged(){
    CONTEXT ctx;
    ZeroMemory(&ctx, sizeof(CONTEXT)); 
    ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS; 
    if(!GetThreadContext(GetCurrentThread(), &ctx))
        return false;
    return ctx.Dr0 || ctx.Dr1 || ctx.Dr2 || ctx.Dr3;
}
```

### CRC检测

可以检查到inlinehook和软件断点

通常只能对少数关键位置做检查

特征明显，修改内存失败后进行访存追踪即可发现子线程持续访问

```c
// 从checkpoints直接复制来的
PVOID g_pFuncAddr;
DWORD g_dwFuncSize;
DWORD g_dwOriginalChecksum;

static void VeryImportantFunction(){
    // ...
}

static DWORD WINAPI ThreadFuncCRC32(LPVOID lpThreadParameter){
    //子线程实时检查crc
    while (true){
        if (CRC32((PBYTE)g_pFuncAddr, g_dwFuncSize) != g_dwOriginalChecksum)
            ExitProcess(0);
        Sleep(10000);
    }
    return 0;
}

size_t DetectFunctionSize(PVOID pFunc){
    PBYTE pMem = (PBYTE)pFunc;
    size_t nFuncSize = 0;
    do {
        ++nFuncSize;
    } while (*(pMem++) != 0xC3);
    return nFuncSize;
}

int main(){
    g_pFuncAddr = (PVOID)&VeryImportantFunction;
    g_dwFuncSize = DetectFunctionSize(g_pFuncAddr);
    g_dwOriginalChecksum = CRC32((PBYTE)g_pFuncAddr, g_dwFuncSize);
    
    HANDLE hChecksumThread = CreateThread(NULL, NULL, ThreadFuncCRC32, NULL, NULL, NULL);
    
    // ...
    
    return 0;
}
```





## 反附加

### DbgBreakPoint

hook `ntdll!DbgBreakPoint`

```c++
void Patch_DbgBreakPoint(){
    HMODULE hNtdll = GetModuleHandleA("ntdll.dll");
    if (!hNtdll) return;
    FARPROC pDbgBreakPoint = GetProcAddress(hNtdll, "DbgBreakPoint");
    if (!pDbgBreakPoint) return;
    DWORD dwOldProtect;
    if (!VirtualProtect(pDbgBreakPoint, 1, PAGE_EXECUTE_READWRITE, &dwOldProtect)) return;
    *(PBYTE)pDbgBreakPoint = (BYTE)0xC3; // ret
}
```



### ObjectCallback

内核句柄降权



[waliedassar: ShareCount As Anti-Debugging Trick (waleedassar.blogspot.com)](http://waleedassar.blogspot.com/2014/06/sharecount-as-anti-debugging-trick.html)



## 反调试器

### 窗口检测

```c++
HWND hd_od=FindWindow("ollydbg",NULL);
SetWindowLong(hd_od,GWL_STYLE,WS_DISABLED);
```

### 阻塞输入

```c++
BYTE *address=(BYTE *)GetProcAddress(LoadLibrary("user32.dll"),"BlockInput");;
bool modify=true;
for(int x=0;x<20;x++){
	if(address[x]==0xff&&address[x+1]!=0xff){
		modify=false;
		break;
	}
}
if(modify) ExitProcess(0);
//main
BlockInput(TRUE);
//...
__try{
       __asm{
              xor eax,eax
             div eax,eax
            xor eax,eax
       }
      ExitProcess(0);
}
__except(1,1){
       BlockInput(FALSE);
}
```





## CSRSS

[Microsoft Windows System Call Table (XP/2003/Vista/2008/7/2012/8/10) (vexillium.org)](https://j00ru.vexillium.org/syscalls/nt/64/)

[Windows CSRSS API Function List (NT/2000/XP/2003/Vista/2008/7/2012/8) (vexillium.org)](https://j00ru.vexillium.org/csrss_list/api_list.html)



## 相关符号

### WindowsSyscallsEx

https://github.com/DragonQuestHero/WindowsSyscallsEx

从pdb获取 NT任意符号偏移

### EasyPdb

https://github.com/Kwansy98/EasyPdb

下载并解析

## 

## 相关项目

### wubbabooMark

https://github.com/hfiref0x/WubbabooMark

### al-khaser

https://github.com/LordNoteworthy/al-khaser

### AntiCrack-DotNet

https://github.com/AdvDebug/AntiCrack-DotNet

## 实例参考

### GenshinDebuggerBypass 

https://github.com/xTaiwanPingLord/GenshinDebuggerBypass

### Genshin-Bypass

https://github.com/notmarek/Genshin-Bypass

## 调试工具

### SharpOD

https://bbs.kanxue.com/thread-218988.htm

### TitanHide

https://github.com/mrexodia/TitanHide

通过SSDT hook 内核函数

支持系统：windows XP - windows 10

支持调试器：任意

### HyperHide

https://github.com/Air14/HyperHide

使用VT-x和EPT技术 综合实现的内核反调试大全

### AADebug

https://github.com/DragonQuestHero/Kernel-Anit-Anit-Debug-Plugins

只能在特定win7下运行

通过多种方法hook内核函数

并且重写了调试链函数

```
NtDebugActiveProcess
DbgkpQueueMessage
KiDispatchException
DebugActiveProcess
DbgUixxx
```

### vt-ReloadDbg

https://github.com/xyddnljydd/vt-ReloadDbg

部分重写调试

### Mirage

幻境 VT反调试

1. win7 x64 ( *`6.1.7600`*)
2. win10 19h1 x64 (*`10.0.18362.XXXX`*)

https://github.com/stonedreamforest/Mirage

### driver_callback_bypass_1909

https://github.com/FiYHer/driver_callback_bypass_1909

运行在1909系统

移除各种内核回调



## 调试原理

了解调试原理才能做检测或构建自己的调试链

从0开始编写简易调试器 https://bbs.kanxue.com/thread-276162.htm

一个调试器的实现 https://www.cnblogs.com/zplutor/archive/2011/03/04/1971279.html

```c
DebugActiveProcess(pid)
DEBUG_EVENT debug_event;
WaitForDebugEvent(&debug_event, INFINITE);
typedef struct _DEBUG_EVENT {
    DWORD dwDebugEventCode;
    DWORD dwProcessId;
    DWORD dwThreadId;
    union {
        EXCEPTION_DEBUG_INFO Exception;
        CREATE_THREAD_DEBUG_INFO CreateThread;
        CREATE_PROCESS_DEBUG_INFO CreateProcessInfo;
        EXIT_THREAD_DEBUG_INFO ExitThread;
        EXIT_PROCESS_DEBUG_INFO ExitProcess;
        LOAD_DLL_DEBUG_INFO LoadDll;
        UNLOAD_DLL_DEBUG_INFO UnloadDll;
        OUTPUT_DEBUG_STRING_INFO DebugString;
        RIP_INFO RipInfo;
    } u;
} DEBUG_EVENT, *LPDEBUG_EVENT;
```



## 参考资料 Ref

[waliedassar: Wow64-Specific Anti-Debug Trick](http://waleedassar.blogspot.com/2013/01/wow64-specific-anti-debug-trick.html)

[Waliedassar's Pastebin](https://pastebin.com/u/waliedassar)

[CHECK POINT RESEARCH](https://anti-debug.checkpoint.com/)

[CTF-Wiki: 反调试技术](https://ctf-wiki.org/reverse/windows/anti-debug/)

[Revercc's Blog: Windows反调试技术](https://www.cnblogs.com/revercc/p/13721197.html)

[LordNoteworthy/al-khaser](https://github.com/LordNoteworthy/al-khaser/tree/master/al-khaser)

https://www.anquanke.com/post/id/176532

https://song-10.gitee.io/2021/08/08/Reverse-2021-08-08-anti-debug

[Windows下反（反）调试技术汇总 | 天融信阿尔法实验室 (topsec.com.cn)](http://blog.topsec.com.cn/windows下反（反）调试技术汇总/)

[Anti-Debug Protection Techniques: Implementation and Neutralization - CodeProject](https://www.codeproject.com/Articles/1090943/Anti-Debug-Protection-Techniques-Implementation-an)

[Windows 平台反调试相关的技术方法总结—part 2 - 先知社区 (aliyun.com)](https://xz.aliyun.com/t/5339#toc-14)

[Windows 平台反调试相关的技术方法总结—part 3 - 先知社区 (aliyun.com)](https://xz.aliyun.com/t/5408)

[利用异常实现反调试 - 活着的虫子 - 博客园 (cnblogs.com)](https://www.cnblogs.com/yilang/p/12039606.html)

[详解反调试技术](https://blog.csdn.net/qq_32400847/article/details/52798050)

[whklhhhh反调试](https://so.csdn.net/so/search?q=反调试&t=blog&u=whklhhhh)

https://github.com/Hipepper/anti_all_in_one
