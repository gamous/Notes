#include <windows.h>
#include <stdio.h>


typedef int (*pfn_testgo_2)(DWORD32,DWORD32,DWORD32);
int testgogp(DWORD32 a,DWORD32 b,DWORD32 c)
{
	printf("Shellcode 0x%x 0x%x 0x%x\r\n",a,b,c);
    return 0x666;
}

//constexpr DWORD64 desrc(PVOID function)
//{
//	return (DWORD64)function^0x0706050403020100;
//}
DWORD64 desrc(PVOID funtion)
{
	DWORD64 buffer[1];
	buffer[0] = reinterpret_cast<DWORD64>(funtion);
	for (int i = 0; i < 8; i++) ((PUCHAR)buffer)[i] = ((PUCHAR)buffer)[i]^i;
	return buffer[0];
}

static byte Shellcode[0x43] = {
        0x48, 0xB8,  0xA4, 0x5F, 0x8F, 0x04, 0x80, 0xF8, 0xFF, 0xFF, //movabs  rax, 0xfffff880048f5fa4
        0x48, 0x89,  0x4C, 0x24, 0xE0,                                   //mov [rsp-0x20],rcx
        0x33, 0xC9,                                                  //xor     ecx, ecx
        0x48, 0x89, 0x44, 0x24, 0x08,                                //mov     qword ptr [rsp + 8], rax
        0x48, 0x8D, 0x44, 0x24, 0x08,                                //lea     rax, [rsp + 8]
        0x66, 0x66, 0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00,  //nop     word ptr [rax + rax]
        0x30, 0x08,                                                  //xor     byte ptr [rax], cl
        0xFF, 0xC1,                                                  //inc     ecx
        0x48, 0x8D, 0x40, 0x01,                                      //lea     rax, [rax + 1]
        0x83, 0xF9, 0x08,                                            //cmp     ecx, 8
        0x7C, 0xF3,                                                  //jl      0x20
        0x48, 0x8B, 0x4C, 0x24, 0xE0,                                    //mov rcx,[rsp-0x20]
        0x48, 0x8B, 0x44, 0x24, 0x08,                                //mov     rax, qword ptr [rsp + 8]
        0x48, 0xFF, 0xE0                                             //jmp     rax
};

void preparecall(DWORD64 function)
{
	DWORD old;
	*(PDWORD64)(Shellcode+ 2) = function;
	VirtualProtect(Shellcode,0x100,PAGE_EXECUTE_READWRITE,&old);
	
}
int main(){
    preparecall(desrc(testgogp));
    int res=((pfn_testgo_2)(&Shellcode))(1,2,3);
    printf("res=%03x\r\n",res);
}