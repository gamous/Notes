#pragma once
#include <windows.h>
#include <tlhelp32.h>
#include <string.h>
#include <atlconv.h>
//ȡָ�����̾��
HANDLE GetProcessHandle(DWORD pid) {
	return OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
}
//ͨ�����ھ��ȡ����ID
DWORD GetProcessID(HWND hwnd) {
	DWORD pid;
	GetWindowThreadProcessId(hwnd, &pid);
	return pid;
}
//ȡ���ھ��
HWND GetWindowHanlde(LPCSTR ProcessName) {
	return FindWindowA(NULL, ProcessName);
}

//ȡָ������ģ���� ʹ��ǰ����ת��Ϊ���ֽ�
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
//ȡָ�������ڴ�_������ ������Ϊ0��ʧ��
int ReadMemory(HANDLE ProcessID, DWORD MemoryAddr)
{
	int Buffers;
	ReadProcessMemory(ProcessID, (LPVOID)MemoryAddr, &Buffers, 4, NULL);
	return Buffers;
}
//ȡָ�������ڴ�_�ı��� ������Ϊ0��ʧ��
char* ReadUnicode(HANDLE ProcessID, DWORD  addr, DWORD Len) //����̨����֧��unicode��ӡ���
{
	char bootName[24];
	DWORD read_size = 0;
	ReadProcessMemory(ProcessID, (LPVOID)addr, &bootName, Len, nullptr);
	return bootName;
}
//ȡָ�������ڴ�_С���� ������Ϊ0��ʧ��
float ReadMemory_F(HANDLE ProcessID, DWORD MemoryAddr)
{
	float Buffers;
	ReadProcessMemory(ProcessID, (LPVOID)MemoryAddr, &Buffers, 4, NULL);
	return Buffers;
}
//ȡָ�������ڴ�_˫����С���� ������Ϊ0��ʧ��
double ReadMemory_LF(HANDLE ProcessID, DWORD MemoryAddr)
{
	double Buffers;
	ReadProcessMemory(ProcessID, (LPVOID)MemoryAddr, &Buffers, 4, NULL);
	return Buffers;
}
//дָ������������ ����0��ʧ��
bool WriteMemory(HANDLE ProcessID, DWORD MemoryAddr, int num) {
	return WriteProcessMemory(ProcessID, (LPVOID)MemoryAddr, &num, 4, NULL);
}
//дָ������С���� ����0��ʧ��
bool WriteMemory_F(HANDLE ProcessID, DWORD MemoryAddr, float num) {
	return WriteProcessMemory(ProcessID, (LPVOID)MemoryAddr, &num, 4, NULL);
}
//дָ������˫����С���� ����0��ʧ��
bool WriteMemory_LF(HANDLE ProcessID, DWORD MemoryAddr, double num) {
	return WriteProcessMemory(ProcessID, (LPVOID)MemoryAddr, &num, 8, NULL);
}

//======================R3_x64Read&Write=====================
//ȡָ�������ڴ�_��������_X64 ������Ϊ0��ʧ��
long long ReadMemory_64(HANDLE ProcessID, long long MemoryAddr)
{
	long long Buffers;
	ReadProcessMemory(ProcessID, (LPVOID)MemoryAddr, &Buffers, 8, NULL);
	return Buffers;
}
//ȡָ�������ڴ�_С����X64 ������Ϊ0��ʧ��
float ReadMemory_F_64(HANDLE ProcessID, long long MemoryAddr)
{
	float Buffers;
	ReadProcessMemory(ProcessID, (LPVOID)MemoryAddr, &Buffers, 4, NULL);
	return Buffers;
}
//ȡָ�������ڴ�_˫����С����X64 ������Ϊ0��ʧ��
double ReadMemory_LF_64(HANDLE ProcessID, long long MemoryAddr)
{
	double Buffers;
	ReadProcessMemory(ProcessID, (LPVOID)MemoryAddr, &Buffers, 8, NULL);
	return Buffers;
}
//дָ�����̳�������X64 ����0��ʧ��
bool WriteMemory_64(HANDLE ProcessID, long long MemoryAddr, long long num) {
	return WriteProcessMemory(ProcessID, (LPVOID)MemoryAddr, &num, 8, NULL);
}
//дָ������С����X64 ����0��ʧ��
bool WriteMemory_F_64(HANDLE ProcessID, long long MemoryAddr, float num) {
	return WriteProcessMemory(ProcessID, (LPVOID)MemoryAddr, &num, 8, NULL);
}
//дָ������˫����С����X64 ����0��ʧ��
bool WriteMemory_LF_64(HANDLE ProcessID, long long MemoryAddr, double num) {
	return WriteProcessMemory(ProcessID, (LPVOID)MemoryAddr, &num, 8, NULL);
}