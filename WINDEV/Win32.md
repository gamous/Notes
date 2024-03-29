# WIN32





[POP POP RET: SEH Exploiting process (marcoramilli.com)](https://marcoramilli.com/2011/11/28/pop-pop-ret-seh-exploiting-process/)

记录一点常用的win32api

https://www.geoffchappell.com/studies/windows/km/ntoskrnl

[TEB (geoffchappell.com)](https://www.geoffchappell.com/studies/windows/km/ntoskrnl/inc/api/pebteb/teb/index.htm)

[WinAPI data types and constants (renenyffenegger.ch)](https://renenyffenegger.ch/notes/Windows/development/WinAPI/data-types/index#winapi-datatype-NT_TIB)

[基于断链的DLL隐藏 - idiotc4t's blog](https://idiotc4t.com/defense-evasion/unlink-module-hide)

[深入理解WIN32](https://www.anquanke.com/member.html?memberId=156165)

https://cloud.tencent.com/developer/article/1036207

https://github.com/MicrosoftDocs/win32

https://github.com/ranni0225/WRK/tree/main/WRK-V1.2

## 层次结构

```
--ring3--
kernel32.dll
ntdll.dll
--ring0--
ntoskrnl.exe
```





## 进程相关

### PEB结构

**P**rocess **E**nvirorment **B**lock Structure

在当前`TEB+0x30`的位置

```asm
__asm{
	mov eax,fs:[0x30]
	mov PEB,eax
}
```

```c
typedef struct _PEB {
  BYTE                          Reserved1[2];
  BYTE                          BeingDebugged;
  BYTE                          Reserved2[1];
  PVOID                         Reserved3[2];
  PPEB_LDR_DATA                 Ldr;
  PRTL_USER_PROCESS_PARAMETERS  ProcessParameters;
  BYTE                          Reserved4[104];
  PVOID                         Reserved5[52];
  PPS_POST_PROCESS_INIT_ROUTINE PostProcessInitRoutine;
  BYTE                          Reserved6[128];
  PVOID                         Reserved7[1];
  ULONG                         SessionId;
} PEB, *PPEB;
```

### 进程句柄

从pid操作其他进程，需要先获得对应权限的进程句柄

```c
HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);
if(!hProcess)return;
CloseHandle(hProcess);
```

### 跨进程内存操作

```c
BOOL ReadProcessMemory(
  [in]  HANDLE  hProcess,
  [in]  LPCVOID lpBaseAddress,
  [out] LPVOID  lpBuffer,
  [in]  SIZE_T  nSize,
  [out] SIZE_T  *lpNumberOfBytesRead
);
BOOL WriteProcessMemory(
  [in]  HANDLE  hProcess,
  [in]  LPVOID  lpBaseAddress,
  [in]  LPCVOID lpBuffer,
  [in]  SIZE_T  nSize,
  [out] SIZE_T  *lpNumberOfBytesWritten
);
```

### 查询进程信息

```c
__kernel_entry NTSTATUS NtQueryInformationProcess(
  [in]            HANDLE           ProcessHandle,
  [in]            PROCESSINFOCLASS ProcessInformationClass,
  [out]           PVOID            ProcessInformation,
  [in]            ULONG            ProcessInformationLength,
  [out, optional] PULONG           ReturnLength
);
```

### 修改进程信息

```c
BOOL SetProcessInformation(
  [in] HANDLE                    hProcess,
  [in] PROCESS_INFORMATION_CLASS ProcessInformationClass,
       LPVOID                    ProcessInformation,
  [in] DWORD                     ProcessInformationSize
);
```



```c
NtQueryInformationProcess(
    ...
	ProcessInformationClass=ProcessBasicInformation
	...)
typedef struct _PROCESS_BASIC_INFORMATION {
    PVOID Reserved1;
    PPEB PebBaseAddress;
    PVOID Reserved2[2];
    ULONG_PTR UniqueProcessId;
    PVOID Reserved3;
} PROCESS_BASIC_INFORMATION;
```



## Syscall

https://github.com/joshfinley/SyscallDumper

https://github.com/hfiref0x/SyscallTables

Microsoft Windows System Call Table (XP/2003/Vista/2008/7/2012/8/10)  https://j00ru.vexillium.org/syscalls/nt/64/

Windows x86-64 WIN32K.SYS System Call Table (XP/2003/Vista/2008/7/2012/8/10) https://j00ru.vexillium.org/syscalls/win32k/64/

https://github.com/VeroFess/X64SystemCall

单asm syscall

https://tttang.com/archive/1464/



windows镜像下载

https://files.rg-adguard.net/
