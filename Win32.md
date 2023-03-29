# WIN32

记录一点常用的win32api

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

