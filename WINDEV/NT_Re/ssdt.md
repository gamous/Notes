## Syscall调用路径 

EnumDeviceDrivers为例

psapi.dll内只有一个stub，实际上直接转发到`K32EnumDeviceDrivers	api-ms-win-core-psapi-l1-1-0`

```asm
.text:0000000180001090                               ; Exported entry   2. EnumDeviceDrivers
.text:0000000180001090
.text:0000000180001090                               ; BOOL __stdcall EnumDeviceDriversStub(LPVOID *lpImageBase, DWORD cb, LPDWORD lpcbNeeded)
.text:0000000180001090                               public EnumDeviceDriversStub
.text:0000000180001090                               EnumDeviceDriversStub proc near         ; DATA XREF: .rdata:00000001800022D4↓o
.text:0000000180001090                                                                       ; .rdata:off_180002688↓o
.text:0000000180001090 48 FF 25 71 11 00 00          jmp     cs:__imp_K32EnumDeviceDrivers   ;(0x180001097+0x00001171)
.text:0000000180001090
.text:0000000180001090                               EnumDeviceDriversStub endp

...
.idata:0000000180002208                               ; BOOL (__stdcall *K32EnumDeviceDrivers)(LPVOID *lpImageBase, DWORD cb, LPDWORD lpcbNeeded)
.idata:0000000180002208 ?? ?? ?? ?? ?? ?? ?? ??       extrn __imp_K32EnumDeviceDrivers:qword  ; DATA XREF: EnumDeviceDriversStub↑r
```



`api-ms-win-core-psapi-l1-1-0	` 是[Windows API 集 ](https://learn.microsoft.com/zh-cn/windows/win32/apiindex/windows-apisets)的格式

```
.\apiset.exe api-ms-win-core-psapi-l1-1-0
[!] Api set library resolved : api-ms-win-core-psapi-l1-1-0 -> kernelbase.dll
```



`kernelbase.dll: K32EnumDeviceDrivers` 

```asm
.text:000000018006E030                               ; BOOL __stdcall K32EnumDeviceDrivers(LPVOID *lpImageBase, DWORD cb, LPDWORD lpcbNeeded)
.text:000000018006E030                               public K32EnumDeviceDrivers
.text:000000018006E030                               K32EnumDeviceDrivers proc near          ; DATA XREF: 
.text:000000018006E030 48 89 5C 24 10                mov     [rsp+arg_8], rbx                ; EnumDeviceDrivers
.text:000000018006E035 48 89 74 24 18                mov     [rsp+arg_10], rsi
.text:000000018006E03A 48 89 4C 24 08                mov     [rsp+arg_0], rcx
.text:000000018006E03F 57                            push    rdi
.text:000000018006E040 41 54                         push    r12
.text:000000018006E042 41 55                         push    r13
.text:000000018006E044 41 56                         push    r14
.text:000000018006E046 41 57                         push    r15
.text:000000018006E048 48 83 EC 30                   sub     rsp, 30h
.text:000000018006E04C 4D 8B E8                      mov     r13, r8
.text:000000018006E04F 44 8B E2                      mov     r12d, edx
.text:000000018006E052 BE 30 05 00 00                mov     esi, 530h
.text:000000018006E057 33 DB                         xor     ebx, ebx
.text:000000018006E059                               loc_18006E059:                          ; CODE XREF: K32EnumDeviceDrivers+148↓j
.text:000000018006E059 8B D6                         mov     edx, esi                        ; uBytes
.text:000000018006E05B 33 C9                         xor     ecx, ecx                        ; uFlags
.text:000000018006E05D E8 2E B4 FC FF                call    LocalAlloc
.text:000000018006E05D
.text:000000018006E062 48 8B F8                      mov     rdi, rax
.text:000000018006E065 48 89 44 24 20                mov     [rsp+58h+hMem], rax
.text:000000018006E06A 48 85 C0                      test    rax, rax
.text:000000018006E06D 0F 84 2D 98 04 00             jz      loc_1800B78A0
.text:000000018006E06D
.text:000000018006E073 4C 8D 4C 24 78                lea     r9, [rsp+58h+ReturnLength]      ; ReturnLength
.text:000000018006E078 44 8B C6                      mov     r8d, esi                        ; SystemInformationLength
.text:000000018006E07B 48 8B D0                      mov     rdx, rax                        ; SystemInformation
.text:000000018006E07E B9 0B 00 00 00                mov     ecx, 0Bh                        ; SystemInformationClass
.text:000000018006E083 48 FF 15 AE 82 15 00          call    cs:__imp_NtQuerySystemInformation                                     ...
```

