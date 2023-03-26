# AntiVM

## CPUID

原理：`cpuid 1`指令执行后，EAX的31位代表hypervisor

```c
bool isVM(){
    DWORD dw_ecx;
    bool bFlag = true;
    _asm{
        pushad;
        pushfd;
        mov eax,1;
        cpuid;
        mov dw_ecx,ecx;
        and ecx,0x80000000;
        test ecx,ecx;
        setz[bFlag];
        popfd;
        popad;
    }
    if(bFlag)
        return false;
    else
        return true;
}
```

绕过

```c
hypervisor.cpuid.v0 = "FALSE"
```

## VMwareIO

原理：VMware 的一个 "后门" I/O 端口, `0x5658 = "VX"`

程序在 VMware 内运行时可以使用`In`指令从`0x5658`端口读取数据,功能号为`0xa`时在`EBX`寄存器中返回版本号`0x564D5868`(`"VMXh"`)

`In`指令属于特权指令，在真机中会导致异常

```c
bool CheckVMWare1(){
    bool bResult = TRUE;
    __try {
        __asm{
            push   edx
            push   ecx
            push   ebx
            mov    eax, 'VMXh' 
            mov    ebx, 0
            mov    ecx, 10     //指定功能号，用于获取VMWare版本，为0x14时获取VM内存大小
            mov    edx, 'VX'   //端口号
            in     eax, dx     //从端口edx 读取VMware到eax
            cmp    ebx, 'VMXh'
            setz[bResult]      //为零 (ZF=1) 时设置字节
            pop    ebx
            pop    ecx
            pop    edx
        }
    } __except (EXCEPTION_EXECUTE_HANDLER){
        bResult = FALSE;
    }
    return bResult;
}
```



## RegOpenKey

```c
bool checkReg() {
	HKEY hkey;
	if (RegOpenKey(HKEY_CLASSES_ROOT, "\\Applications\\VMwareHostOpen.exe", &hkey) == ERROR_SUCCESS) {
		RegCloseKey(hKey);
         return true;
	} else return false;
}
```

```c
bool DetectVM(){
	HKEY hKey;
	char szBuffer[64];
	unsigned long hSize = sizeof(szBuffer)-1;
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\BIOS\\", 0, KEY_READ, &hKey) == ERROR_SUCCESS){
		RegQueryValueEx(hKey, "SystemManufacturer", NULL, NULL, (unsigned char *)szBuffer, &hSize);
		if (strstr(szBuffer, "VMWARE")){
			RegCloseKey(hKey);
			return true;
		}
		RegCloseKey(hKey);
	}
	return false;
}
```



## Hardware

通过硬件配置来检测

```c
bool checkMAC(){
    char buffer[128];
    char result[1024*50] = "";
    char MAC[] = "08-00-27";
    FILE *pipe = _popen("ipconfig /all","r");
    if(!pipe)
        return 0;
    
    while(!feof(pipe)){
        if(fgets(buffer,128,pipe))
            strcat(result,buffer);
    }
    _pclose(pipe);
    //08-00-27
    if(strstr(result,MAC))
        return false;
    else
        return true;
}
bool CheckCPU(){
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    int CoreNum = sysinfo.dwNumberOfProcessors;
    printf("Core:%d\n",CoreNum);
    if(CoreNum < 4)
        return false;
    else
        return true;
}

```

## 过检测

### VMProtect

```asm
cmp byte ptr ds:[eax+ecx+0x1],0x69
cmp byte ptr ds:[eax+ecx+0x2],0x72
cmp byte ptr ds:[eax+ecx+3],74
cmp byte ptr ds:[eax+ecx+4],75
cmp byte ptr ds:[eax+ecx+5],61
cmp byte ptr ds:[eax+ecx+6],6C
cmp byte ptr ds:[eax+ecx+7],42
cmp byte ptr ds:[eax+ecx+8],6F
cmp byte ptr ds:[eax+ecx+9],78
;VirtualBox

cmp byte ptr ds:[edx],56
cmp byte ptr ds:[eax+ecx+1],4D
cmp byte ptr ds:[eax+ecx+2],77
cmp byte ptr ds:[eax+ecx+3],61
cmp byte ptr ds:[eax+ecx+4],72
cmp byte ptr ds:[eax+ecx+5],65
;VMware

cmp byte ptr ds:[edx],50
cmp byte ptr ds:[eax+ecx+1],61
cmp byte ptr ds:[eax+ecx+2],72
cmp byte ptr ds:[eax+ecx+3],61
cmp byte ptr ds:[eax+ecx+4],6C
cmp byte ptr ds:[eax+ecx+5],6C
cmp byte ptr ds:[eax+ecx+6],65
cmp byte ptr ds:[eax+ecx+7],6C
cmp byte ptr ds:[eax+ecx+8],73
;Parallels
```

https://github.com/woxihuannisja/bypass_vmp_vm_detect



### SafeEngine

```
81 7D E4 68 58 4D 56
```

```asm
cmp dword ptr ss:[ebp-0x1Ch],0x564D5868h;修改掉
jnz L0069B0FF
push -0x4h
pop eax
jmp L0069B101
L0069B0FF:
xor eax,eax
L0069B101:
call L006B3C4C
retn
```

```
55 8B EC 83 EC 14
```

```asm
push ebp ;改成 mov eax,0  retn
mov ebp,esp
sub esp,0x14h
push ebx
push esi
jmp L0069AFA9
```





## 参考资料 Ref

[E-cards don?t like virtual environments](https://isc.sans.edu/diary/E-cards+don%3Ft+like+virtual+environments/3190)

https://github.com/nek0YanSu/CheckVM-Sandbox

https://www.52pojie.cn/thread-598022-1-1.html

https://bbs.kanxue.com/thread-119969.htm

https://bbs.kanxue.com/thread-225735.htm

https://github.com/wanttobeno/AntiVirtualMachine

https://www.freebuf.com/articles/system/202717.html

https://xz.aliyun.com/t/8922