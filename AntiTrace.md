# AntiTrace

通过一些指令操作反调试器单步追踪



## INT 2E

int 2e 是系统调用向量 (Sysenter)



## Prefix+PUSHFD

通过前缀指令来制造一个实际不执行的 PUSHFD(0x9c)

### PUSHFD

Push EFLAGS

### EFLAGS Register

(program status and control)

```
00    CF   S Carry Flags
01    1
02    PF   S Parity Flags
03    0
04    AF   S Auxiliary Carry Flag
05    0
06    ZF   S Zero Flag
07    SF   S Sign Flag
08    TF   X Trap Flag 置1时允许单步调试模式，清零则禁用
09    IF   X Interrupt Enable Flag
10    DF   C Direction Flag 控制串指令的方向
11    OF   S Overflow Flag
12-13 TOPL X IO Privilege Level
14    MT   X Nested Task
15    0
16    RF   X Resume Flag 控制处理器对调试异常的响应
17    VM   X Virtual-8086Mode 置1时模拟8086，否则保护模式
18    AC   X Allignment Check
19    VIF  X Virtual Interrupt Flag
20    VIP  X Virtual Interrupt Pending
21    TD   X ID Falg
22-31 0
```



### 前缀指令

https://bbs.kanxue.com/thread-270359.htm

```c
bool IsDebugged(){
    __try{
        __asm{
            pushfd
            mov dword ptr [esp], 0x100
            popfd
            nop
        }
        return true;
    }
    __except(GetExceptionCode() == EXCEPTION_SINGLE_STEP
        ? EXCEPTION_EXECUTE_HANDLER
        : EXCEPTION_CONTINUE_EXECUTION)
    {
        return false;
    }
}
```





### 重复前缀

| **指令**    | **数值** |
| ----------- | -------- |
| REPNE/REPNZ | 0xF2     |
| REP/REPZ    | 0xF3     |

部分调试器处理**指令前缀**时存在问题，单步运行时pushfd会被实际执行，反之不会

```c++
int main(int argc, char* argv[]){
    unsigned long eflags=0;
    printf("Welcome\r\n");
    __asm{
        __emit 0xF3
        __emit 0xF3
        __emit 0xF3
        __emit 0xF3
        __emit 0xF3
        pushfd
        pop eax
        mov eflags,eax
    }
    if(eflags&0x100)    printf("Being traced\r\n");
    else                printf("Expected behavior\r\n");
    return 0;
}
```



```c++
bool IsDebugged(){
    __try{
        // 0xF3 0x64 disassembles as PREFIX REP:
        __asm __emit 0xF3
        __asm __emit 0x64
        // One byte INT 1
        __asm __emit 0xF1
        return true;
    }__except (EXCEPTION_EXECUTE_HANDLER){
        return false;
    }
}
```



### 宽度前缀

该前缀指令的作用是可以改变地址计算时候的宽度，来绕过部分调试器单步执行时对TF标志的处理

指令宽度发生变化，只压入eflags的低位

```c++
#include "windows.h"
#include "stdio.h"

int main(int argc, char* argv[]){
    unsigned long eflags=0;
    printf("Welcome\r\n");
    __asm{
        __emit 0x67
        __emit 0x67
        __emit 0x67
        __emit 0x67
        pushfd
        push bx
        pop eax
        shr eax, 0x10
        mov eflags, eax
        pop bx;
    }
    if(eflags&0x100)    printf("Being traced\r\n");
    else                printf("Expected behavior\r\n");
    return 0;
}
```





```c++
#include "windows.h"
#include "stdio.h"
 
int __cdecl Handler(void*,void*,void*,void*)
{
    printf("Incompatible System\r\n");
    ExitProcess(0);
    return ExceptionContinueSearch;
}
 
void main_2E()
{
    unsigned long realPC=0;
    __asm
    {
        push offset Handler
        push dword ptr fs:[0]
        mov dword ptr fs:[0],esp
        xor eax,eax
        xor edx,edx
        int 0x2E
        nop
        mov realPC,edx
        pop dword ptr fs:[0]
        pop ebx
    }
    printf("EIP is %x\r\n",realPC);
    if(realPC==-1) printf("Being Traced\r\n");
    return;
}
 
 
void main_2C()
{
    unsigned long realPC=0;
    __asm
    {
        push offset Handler
        push dword ptr fs:[0]
        mov dword ptr fs:[0],esp
        xor eax,eax
        xor edx,edx
        int 0x2C
        nop
        mov realPC,edx
        pop dword ptr fs:[0]
        pop ebx
    }
    printf("EIP is %x\r\n",realPC);
    if(realPC==-1) printf("Being Traced\r\n");
    return;
}
 
void Test_Trace()
{
    unsigned long EFlags=0;
    __asm
    {
        xor eax,eax
        xor edx,edx
        int 0x2E
        pushfd
        pop eax
        mov EFlags,eax
    }
    if(EFlags & 0x100 /* TF */) printf("Being Traced\r\n");
 
}
 
void main()
{
    main_2C();
    main_2E();
    Test_Trace();
}
```

[RE: 2 anti-tracing mechanisms specific to windows x64 (everdox.blogspot.com)](http://everdox.blogspot.com/2013/03/2-anti-tracing-mechanisms-specific-to.html)

[PspSetContext Anti-Tracing Trick - Pastebin.com](https://pastebin.com/5iRb3tBZ)



## x64 in x32

https://tttang.com/archive/1559

[使用32位64位交叉编码混淆来打败静态和动态分析工具 - 跳跳糖 (tttang.com)](https://tttang.com/archive/315/)