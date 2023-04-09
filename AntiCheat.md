# 反作弊相关

## MemMon

内存监控

### WriteWatch

https://blog.csdn.net/yoie01/article/details/51751908

使用独立线程`GetWriteWatch`循环探测

读写时需要先关检测，写完后`ResetWriteWatch`清空记录并开检测

```c
#include <windows.h>
#include <stdio.h>
 
typedef struct _POSITION_{
  DWORD dx;
  DWORD dz;
  DWORD dy;
}POSITION,*PPOSITION;
 
#define BASE_SIZE 0x1000
#define WALK_ADD 10
DWORD WINAPI WatchThread(LPVOID Param){
  ULONG size = 0;
  DWORD p = 0;
  UINT  Ret = 0;
  PVOID px[0x400] = { 0 };
  while (1){
    size = 0x400;
    p = 0;
    Ret = GetWriteWatch(WRITE_WATCH_FLAG_RESET, Param, BASE_SIZE, px, &size, &p);
    if (Ret == 0 && size != 0){
      MessageBoxW(NULL, L"发现第三方软件修改关键内存", L"发现第三方软件", MB_OK);
    }
  }
  return 0;
}
int __cdecl main(int argc, char *argv[]){
  //申请Base基址内存
  PVOID Base = VirtualAlloc(NULL, BASE_SIZE, MEM_RESERVE | MEM_COMMIT | MEM_WRITE_WATCH, PAGE_READWRITE);
  PPOSITION pos = (PPOSITION)Base;
  //初始化坐标
  pos->dx = pos->dy = pos->dz = 1000;
  //清空监控记录
  ResetWriteWatch(Base, BASE_SIZE);
  //开启监控线程
  HANDLE hThread = CreateThread(NULL, 0, WatchThread, Base, 0, NULL);
  while (1){
    //暂停监控线程
    SuspendThread(hThread);//也可以通过其他方式进行！！！！暂停线程只是简单模型
    //模拟走路过程
    pos->dx += WALK_ADD;
    //继续监控
    ResetWriteWatch(Base, BASE_SIZE);
    ResumeThread(hThread);
    //输出X坐标
    printf("x = %d\n", pos->dx);
    Sleep(1000);
  }
  return 0;
}
```

### WorkingSet

https://blog.csdn.net/yoie01/article/details/80524412

```c
#include "stdafx.h"
#include <Psapi.h>
#pragma comment(lib, "psapi.lib")
#ifndef PAGE_SIZE
#define PAGE_SIZE 0x1000
#endif
#define NtCurrentProcess() ((HANDLE)-1)
#define PAGE_ALIGN(Va) ((PVOID)((ULONG_PTR)(Va) & ~(PAGE_SIZE - 1)))
#define INITIAL_RECORD_SIZE 1000
 
bool GetProcessDataFromThread(_In_ DWORD ThreadId, _Out_ DWORD& ProcessId, _Out_writes_z_(MAX_PATH) wchar_t ProcessPath[MAX_PATH]){
	bool status = false;
	ProcessId = 0;
	ProcessPath[0] = NULL;
	HANDLE Thread = OpenThread(THREAD_QUERY_LIMITED_INFORMATION, FALSE, ThreadId);
	if (Thread){
		ProcessId = GetProcessIdOfThread(Thread);
		if (ProcessId){
			HANDLE Process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, ProcessId);
			if (Process){
				status = (GetModuleFileNameExW(Process, NULL, ProcessPath, MAX_PATH) != 0);
				CloseHandle(Process);
			}
		}
		CloseHandle(Thread);
	}
	return status;
}
int check_ce(){
	int status = -1;
 
	PBYTE AllocatedBuffer = NULL;
	PPSAPI_WS_WATCH_INFORMATION_EX WatchInfoEx = NULL;
	const DWORD CurrentProcessId = GetCurrentProcessId();
	printf("[+] PID: %lu\n", CurrentProcessId);
#if defined(_M_IX86)
	// Can't run on Wow64 (32-bit on 64-bit OS).
	BOOL Wow64Process = FALSE;
	if (IsWow64Process(NtCurrentProcess(), &Wow64Process) && Wow64Process){
		fprintf(stderr, "[-] ERROR: This process cannot be run under Wow64.\n");
		goto Cleanup;
	}
#endif
	// Initiate monitoring of the working set for this process.
	if (!InitializeProcessForWsWatch(NtCurrentProcess())){
		fprintf(stderr, "[-] ERROR: Failed to initialize process for working set watch. InitializeProcessForWsWatch failed with error: %lu.\n", GetLastError());
		goto Cleanup;
	}
	AllocatedBuffer = (PBYTE)VirtualAlloc(NULL, PAGE_SIZE, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (!AllocatedBuffer){
		fprintf(stderr, "[-] ERROR: Failed to allocate %u bytes for page faulting test buffer.\n", PAGE_SIZE);
		goto Cleanup;
	}
	printf("[+] Allocated buffer at 0x%p.\n", AllocatedBuffer);
	DWORD WatchInfoSize = (sizeof(PSAPI_WS_WATCH_INFORMATION_EX) * INITIAL_RECORD_SIZE);
	WatchInfoEx = (PPSAPI_WS_WATCH_INFORMATION_EX)malloc(WatchInfoSize);
	if (!WatchInfoEx){
		fprintf(stderr, "[-] ERROR: Failed to allocate %lu bytes.\n", WatchInfoSize);
		goto Cleanup;
	}
	while (TRUE){
		memset(WatchInfoEx, 0, WatchInfoSize);
		if (!GetWsChangesEx(NtCurrentProcess(), WatchInfoEx, &WatchInfoSize)){
			DWORD ErrorCode = GetLastError();
			// This really isn't an error. This just means that no new pages
			// have been mapped into our process' VA since the last time
			// we called GetWsChangesEx.
			if (ErrorCode == ERROR_NO_MORE_ITEMS){
				// Wait a little bit before trying again.
				Sleep(1);
				continue;
			}
			// Any other error code is bad.
			if (ErrorCode != ERROR_INSUFFICIENT_BUFFER){
				fprintf(stderr, "[-] ERROR: GetWsChangesEx failed with error: %lu.\n", ErrorCode);
				goto Cleanup;
			}
			// If we get this far, we need to increase the buffer size. 
			WatchInfoSize *= 2;
			free(WatchInfoEx);
			WatchInfoEx = (PPSAPI_WS_WATCH_INFORMATION_EX)malloc(WatchInfoSize);
 
			if (!WatchInfoEx){
				fprintf(stderr, "[-] ERROR: Failed to allocate %lu bytes.\n", WatchInfoSize);
				goto Cleanup;
			}
			continue;
		}
		bool bFound = false;
		for (size_t i = 0;; ++i){
			PPSAPI_WS_WATCH_INFORMATION_EX info = &WatchInfoEx[i];
			if (info->BasicInfo.FaultingPc == NULL) break;
			PVOID FaultingPageVa = PAGE_ALIGN(info->BasicInfo.FaultingVa);
			if (FaultingPageVa == AllocatedBuffer){
				printf("[+] 0x%p (0x%p) was mapped by 0x%p (TID: %lu).\n", FaultingPageVa, info->BasicInfo.FaultingVa, info->BasicInfo.FaultingPc, (DWORD)info->FaultingThreadId);
				DWORD ProcessId;
				wchar_t ProcessPath[MAX_PATH];
				if (GetProcessDataFromThread((DWORD)info->FaultingThreadId, ProcessId, ProcessPath))
					printf("\t--> %S (PID: %lu).\n", ProcessPath, ProcessId);
				bFound = true;
				break;
			}
		}
		if (bFound){
			status = 1;
			break;
		}
	}
Cleanup:
	// 'free' the 'malloc's.
	if (WatchInfoEx){
		free(WatchInfoEx);
		WatchInfoEx = NULL;
	}
	if (AllocatedBuffer){
		VirtualFree(AllocatedBuffer, 0, MEM_RELEASE);
		AllocatedBuffer = NULL;
	}
    return status;
}
int main(){
	printf("Base = %llX\r\n",(UINT64)GetModuleHandle("WsWatch.exe"));
	while (1) {
		if (check_ce() > 0) printf("Find CE\r\n");
		Sleep(100);
	}
    return 0;
}

```



[c++实现Windows内存监视 - LightningStar - 博客园 (cnblogs.com)](https://www.cnblogs.com/harrypotterjackson/p/12113387.html)

反内存断点

[ReadProcessMemory As Anti-Memory Breakpoints - Pastebin.com](https://pastebin.com/RCkVDNXJ)



## AntiCE

通过对ReadDirectoryChangesW

https://github.com/gmh5225/Detection-CheatEngine

通过debugoutput

https://github.com/weak1337/CEDetector/blob/main/CEDetector/ce_detection.cpp



## AntiMemoryMoniter

https://bbs.kanxue.com/thread-153736.htm

ReadProcessMemory读取自进程 可以避免被监控

先map后读取



## 常见反作弊模块研究

### VAC

#### [VAC](https://github.com/danielkrupinski/VAC)

#### [VAC-Bypass-Loader](https://github.com/danielkrupinski/VAC-Bypass-Loader)

### EAC

KernelModeAntiCheat

https://github.com/mathisvickie/KMAC

## 研究性项目

### **[UltimateAntiCheat](https://github.com/AlSch092/UltimateAntiCheat)**

https://github.com/mq1n/NoMercy

https://github.com/chztbby/RebirthGuard/blob/master/RebirthGuard/RebirthGuard/verifying.cpp#L126