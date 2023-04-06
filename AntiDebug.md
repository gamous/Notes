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

函数：`ntdll!NtQueryInformationProcess` (MS未公开)

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



### PEB

直接检测PEB中的标识

#### BeingDebugged

和IsDebuggerPresent等价

```c++
bool Check() {
    bool bDebugged = false;
    __asm {
        MOV EAX, DWORD PTR FS : [0x30]
            MOV AL, BYTE PTR DS : [EAX + 2]
            MOV bDebugged, AL
    }
    return bDebugged;
}
```

https://zhuanlan.zhihu.com/p/57329235

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



### CloseHandle

函数：`ntdll!NtClose`  或 `kernel32!CloseHandle`

原理：通过调试器的**异常接管处理**来判断

反反调试：拦截`ntdll!NtClose` 使用`ntdll!NtQueryObject`检查句柄是否有效

使用`ntdll!NtClose`  或 `kernel32!CloseHandle`关闭无效句柄时会产生`EXCEPTION_INVALID_HANDLE`（0xC0000008）异常

由于Windows内核对象句柄跟踪机制，这个异常正常情况下是被缓存的，但在被调试的情况下，这个异常**会被传递**给异常处理程序

```c++
bool Check(){
    __try{
        CloseHandle((HANDLE)0xDEADBEEF);
        return false;
    }__except (EXCEPTION_INVALID_HANDLE == GetExceptionCode()
                ? EXCEPTION_EXECUTE_HANDLER 
                : EXCEPTION_CONTINUE_SEARCH){
        return true;
    }
}
```


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

### int 0x2d

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

另一种形式，隐藏Int指令

```c
typedef ULONG(WINAPI* NtDbgPrint)(PCSTR Format, ...);
NtDbgPrint      m_NtDbgPrint;
BOOL InitNtFuncs() {
    HMODULE h_module = GetModuleHandle(TEXT("ntdll.dll"));
    if (!h_module)return FALSE;
    m_NtDbgPrint = (NtDbgPrint)GetProcAddress(h_module, "DbgPrint");
    return TRUE;
}
bool IsDebugged2() {
    __try {
        m_NtDbgPrint("Hello");
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}
```





## 线程操作  Thread

### ZwCreateThreadEx

https://pastebin.com/jAv5GYUd

直接使用`ZwCreateThreadEx`创建调试器不可见进程

```c
ZwCreateThreadEx(&hThread,0x1FFFFF,0,GetCurrentProcess(),&dummy,0, 0x4/*HiddenFromDebugger*/,0,0x1000,0x10000,0);
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

## 相关结构体

### PEB

调试

```
PEB
{
	BeingDebugger:1
	NtGlobalFlag:0x70
	HeapFlags:=2 offset (x32) 0x40!=2 则被调试状态 0x70!=0则被调试状态
	ForceFlags:=0 offset (x32)0x44!=2 则被调试状态 0x74
}
```



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

## 相关项目

### al-khaser

https://github.com/LordNoteworthy/al-khaser

### AntiCrack-DotNet

https://github.com/AdvDebug/AntiCrack-DotNet

## 



## 反反调试工具

### TitanHide

https://github.com/mrexodia/TitanHide

通过SSDT hook 内核函数

支持系统：windows XP - windows 10

支持调试器：任意

### HyperHide

https://github.com/Air14/HyperHide

使用VT-x和EPT技术

### AADebug

https://github.com/DragonQuestHero/Kernel-Anit-Anit-Debug-Plugins

通过多种方法hook内核函数

并且重写了调试链函数

```
NtDebugActiveProcess
DbgkpQueueMessage
KiDispatchException
DebugActiveProcess
DbgUixxx
```

## 调试原理

了解调试原理才能做检测或构建自己的调试链

从0开始编写简易调试器 https://bbs.kanxue.com/thread-276162.htm

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
