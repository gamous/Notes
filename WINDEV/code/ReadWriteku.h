#pragma once
#include <windows.h>
#include <tlhelp32.h>
#include <string.h>
#include <atlconv.h>
//取指定进程句柄
HANDLE GetProcessHandle(DWORD pid) {
	return OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
}
//通过窗口句柄取进程ID
DWORD GetProcessID(HWND hwnd) {
	DWORD pid;
	GetWindowThreadProcessId(hwnd, &pid);
	return pid;
}
//取窗口句柄
HWND GetWindowHanlde(LPCSTR ProcessName) {
	return FindWindowA(NULL, ProcessName);
}

//取指定进程模块句柄 使用前请先转换为多字节
HMODULE GetModuleBaseAddress(DWORD dwProcID, const char* szModuleName)
{
	HMODULE ModuleBaseAddress = 0;
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, dwProcID);
	if (hSnapshot != INVALID_HANDLE_VALUE)
	{
		MODULEENTRY32 ModuleEntry32;
		ModuleEntry32.dwSize = sizeof(MODULEENTRY32);
		if (Module32First(hSnapshot, &ModuleEntry32))
		{
			do
			{
				if (strcmp(ModuleEntry32.szModule, szModuleName) == 0)
				{
					ModuleBaseAddress = (HMODULE)ModuleEntry32.modBaseAddr;
				}
			} while (Module32Next(hSnapshot, &ModuleEntry32));;
		}
		CloseHandle(hSnapshot);
	}
	return ModuleBaseAddress;
}

//======================R3_x86Read&Write=====================
//取指定进程内存_整数型 输出结果为0则失败
int ReadMemory(HANDLE ProcessID, DWORD MemoryAddr)
{
	int Buffers;
	ReadProcessMemory(ProcessID, (LPVOID)MemoryAddr, &Buffers, 4, NULL);
	return Buffers;
}
//取指定进程内存_文本型 输出结果为0则失败
char* ReadUnicode(HANDLE ProcessID, DWORD  addr, DWORD Len) //控制台程序不支持unicode打印输出
{
	char bootName[24];
	DWORD read_size = 0;
	ReadProcessMemory(ProcessID, (LPVOID)addr, &bootName, Len, nullptr);
	return bootName;
}
//取指定进程内存_小数型 输出结果为0则失败
float ReadMemory_F(HANDLE ProcessID, DWORD MemoryAddr)
{
	float Buffers;
	ReadProcessMemory(ProcessID, (LPVOID)MemoryAddr, &Buffers, 4, NULL);
	return Buffers;
}
//取指定进程内存_双精度小数型 输出结果为0则失败
double ReadMemory_LF(HANDLE ProcessID, DWORD MemoryAddr)
{
	double Buffers;
	ReadProcessMemory(ProcessID, (LPVOID)MemoryAddr, &Buffers, 4, NULL);
	return Buffers;
}
//写指定进程整数型 返回0则失败
bool WriteMemory(HANDLE ProcessID, DWORD MemoryAddr, int num) {
	return WriteProcessMemory(ProcessID, (LPVOID)MemoryAddr, &num, 4, NULL);
}
//写指定进程小数型 返回0则失败
bool WriteMemory_F(HANDLE ProcessID, DWORD MemoryAddr, float num) {
	return WriteProcessMemory(ProcessID, (LPVOID)MemoryAddr, &num, 4, NULL);
}
//写指定进程双精度小数型 返回0则失败
bool WriteMemory_LF(HANDLE ProcessID, DWORD MemoryAddr, double num) {
	return WriteProcessMemory(ProcessID, (LPVOID)MemoryAddr, &num, 8, NULL);
}

//======================R3_x64Read&Write=====================
//取指定进程内存_长整数型_X64 输出结果为0则失败
long long ReadMemory_64(HANDLE ProcessID, long long MemoryAddr)
{
	long long Buffers;
	ReadProcessMemory(ProcessID, (LPVOID)MemoryAddr, &Buffers, 8, NULL);
	return Buffers;
}
//取指定进程内存_小数型X64 输出结果为0则失败
float ReadMemory_F_64(HANDLE ProcessID, long long MemoryAddr)
{
	float Buffers;
	ReadProcessMemory(ProcessID, (LPVOID)MemoryAddr, &Buffers, 4, NULL);
	return Buffers;
}
//取指定进程内存_双精度小数型X64 输出结果为0则失败
double ReadMemory_LF_64(HANDLE ProcessID, long long MemoryAddr)
{
	double Buffers;
	ReadProcessMemory(ProcessID, (LPVOID)MemoryAddr, &Buffers, 8, NULL);
	return Buffers;
}
//写指定进程长整数型X64 返回0则失败
bool WriteMemory_64(HANDLE ProcessID, long long MemoryAddr, long long num) {
	return WriteProcessMemory(ProcessID, (LPVOID)MemoryAddr, &num, 8, NULL);
}
//写指定进程小数型X64 返回0则失败
bool WriteMemory_F_64(HANDLE ProcessID, long long MemoryAddr, float num) {
	return WriteProcessMemory(ProcessID, (LPVOID)MemoryAddr, &num, 8, NULL);
}
//写指定进程双精度小数型X64 返回0则失败
bool WriteMemory_LF_64(HANDLE ProcessID, long long MemoryAddr, double num) {
	return WriteProcessMemory(ProcessID, (LPVOID)MemoryAddr, &num, 8, NULL);
}