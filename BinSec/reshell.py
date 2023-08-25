
from capstone import *
shellcode=bytes([
0x48, 0xB8,  0xA4, 0x5F, 0x8F, 0x04, 0x80, 0xF8, 0xFF, 0xFF, #movabs  rax, 0xfffff880048f5fa4
0x33, 0xC9,                                                  #xor     ecx, ecx
0x48, 0x89, 0x44, 0x24, 0x08,                                #mov     qword ptr [rsp + 8], rax
0x48, 0x8D, 0x44, 0x24, 0x08,                                #lea     rax, [rsp + 8]
0x66, 0x66, 0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00,  #nop     word ptr [rax + rax]
0x30, 0x08,                                                  #xor     byte ptr [rax], cl
0xFF, 0xC1,                                                  #inc     ecx
0x48, 0x8D, 0x40, 0x01,                                      #lea     rax, [rax + 1]
0x83, 0xF9, 0x08,                                            #cmp     ecx, 8
0x7C, 0xF3,                                                  #jl      0x20
0x48, 0x8B, 0x44, 0x24, 0x08,                                #mov     rax, qword ptr [rsp + 8]
0x48, 0xFF, 0xE0])                                           #jmp     rax

def bytes2str(b):
    return " ".join([f"{i:02x}" for i in b])

md = Cs(CS_ARCH_X86, CS_MODE_64)
for i in md.disasm(shellcode, 0x00):
    print("0x%x:\t%30s\t%s\t%s" %(i.address, bytes2str(i.bytes),i.mnemonic, i.op_str)) 