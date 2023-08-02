#include <stdio.h>
#include <windows.h>

/*  PML4 paging
  VA: (16) 9-9-9-9-12
  63:48 - 16bit signed ext 0000/FFFF
  47:39 -  9bit PXE_index/PML4I (PML4-PML4E)         512 (256TB) 1PXE->Mapping->all PTE
  38:30 -  9bit PPE_index/PDPTI (DirectoryPtr-PDPTE) 512 (512GB)
  29:21 -  9bit PDE_index/PDI   (Directory-PDE)      512 (1GB)
  20:12 -  9bit PTE_index/PTI   (Table-PTE)          512 (2MB) (MAX_VA/4K) * 8
  11:00 - 12bit in page offset                      4096 (4KB)

    sizeof(PTE)==8 (bytes) 4KBytePage 

  VA(LinearAddress) -> PA
   PML4  = cr3
   PML4E = PML4 + 8*PML4I
        
   PDPT  = *PML4E
   PDPTE = PDPT + 8*PDPTI
        
   PD    = *PDPTE
   PDE   = PD   + 8*PDI (with PS=0)
        
   PT    = *PDE
   PTE   = PT   + 8*PTI
        
   PA    = *PTE + offset

   PDPT=CR3[PML4I]    !dq
   PD=PDPT[PDPTI]     !dq
   PT=PD[PDI]         !dq
   PA=PT[PTI]+OFFSET  !dq

  SelfMapping
    

   Cr3
   

*/

ULONG64 g_PTE_BASE=0;
ULONG64 g_PDE_BASE=0;
ULONG64 g_PPE_BASE=0;
ULONG64 g_PXE_BASE=0;

#define CLS_LOWBIT(Value,Offset)\
    Value>>Offset<<Offset
#define GET_BIT(v,s_bit,e_bit)\
    v&(1<<(e_bit+1)-1)>>s_bit

#define GET_PTE(va)\
    ((va&0xffffffffffff)>>12<<3)+g_PTE_BASE

int main(){
    /*
    ULONG64 va=0;
    scanf("%llx",&va);
    UINT16 SEXT  = va>>48;
    UINT16 PML4I = ((va&(0x0000ffffffffffff))>>39)&0x1ff;
    UINT16 PDPTI = ((va&(0x0000ffffffffffff))>>30)&0x1ff;
    UINT16 PDI   = ((va&(0x0000ffffffffffff))>>21)&0x1ff;
    UINT16 PTI   = ((va&(0x0000ffffffffffff))>>12)&0x1ff;
    UINT16 offset = va&0b111111111111;
    printf("%x %x %x %x %x %x\n",SEXT,PML4I,PDPTI,PDI,PTI,offset);
    
    ULONG64 PML4,PDPT,PD,PT,PA;
    printf("r cr3\n");
    scanf("%llx",&PML4); //CR3
    PML4=CLS_LOWBIT(PML4,12);

    ULONG64 PML4E_PA = PML4+8*PML4I ,PML4E=0;
    printf("!dq %llx\nPML4E = ",PML4E_PA);
    scanf("%llx",&PML4E);
    PDPT=CLS_LOWBIT(PML4E,12);

    ULONG64 PDPTE_PA = PDPT + 8*PDPTI ,PDPTE=0;
    printf("!dq %llx\nPDPTE = ",PDPTE_PA);
    scanf("%llx",&PDPTE);
    PD=CLS_LOWBIT(PDPTE,12);

    ULONG64 PDE_PA = PD + 8*PDI ,PDE;
    printf("!dq %llx\nPDE = ",PDE_PA);
    scanf("%llx",&PDE);
    PT=CLS_LOWBIT(PDE,12);

    ULONG64 PTE_PA = PT + 8*PTI ,PTE;
    printf("!dq %llx\nPTE = ",PTE_PA);
    scanf("%llx",&PTE);
    PA=CLS_LOWBIT(PTE,12);

    printf("PA = %llx\n",PA+offset);

    //how access without !dq (PhyAddr access)?
    //use pte map pte self and get a special addr could be access pte by va
    //use pte map pde self and get a special addr could be access pde by va
    //...
    //which mean selfmapping
    

    while construct PTE
    GET_PTE(g_PTE_BASE) = g_PTE_BASE //
    GET_PTE(g_PDE_BASE) = g_PDE_BASE
    */
    //ULONG64 selfmap_index=0x300; // 
    //PTE->PXE
    ULONG64 random_index,cr3;

    random_index = 0x1ed;//default
    g_PTE_BASE=(random_index<<39)+0xffff000000000000;
    g_PDE_BASE=GET_PTE(g_PTE_BASE);
    g_PPE_BASE=GET_PTE(g_PDE_BASE);
    g_PXE_BASE=GET_PTE(g_PPE_BASE);
    printf("index:%llx PXE_BASE:%llx PPE_BASE:%llx PDE_BASE:%llx PTE_BASE:%llx\n",random_index,g_PXE_BASE,g_PPE_BASE,g_PDE_BASE,g_PTE_BASE);

    g_PTE_BASE=0xFFFF950000000000;
    g_PDE_BASE=GET_PTE(g_PTE_BASE);
    g_PPE_BASE=GET_PTE(g_PDE_BASE);
    g_PXE_BASE=GET_PTE(g_PPE_BASE);
    random_index = (g_PTE_BASE&0xffffffffffff)>>39;
    printf("index:%llx PXE_BASE:%llx PPE_BASE:%llx PDE_BASE:%llx PTE_BASE:%llx\n",random_index,g_PXE_BASE,g_PPE_BASE,g_PDE_BASE,g_PTE_BASE);
    //*(random_index*8+g_PXE_BASE) == *PXE == cr3 == ![cr3] == ![cr3+random_index*8]

//ULONG64 GetPml4Base()
//{
//    PHYSICAL_ADDRESS pCr3 = { 0 };
//    pCr3.QuadPart = __readcr3();
//    PULONG64 pCmpArr = MmGetVirtualForPhysical(pCr3);
// 
//    int count = 0;
//    while ((*pCmpArr & 0xFFFFFFFFF000) != pCr3.QuadPart)
//    {
//        if (++count >= 512)
//        {
//            return -1;
//        }
//        pCmpArr++;
//    }
//    return (ULONG64)pCmpArr & 0xFFFFFFFFFFFFF000;
//}

    cr3 = 0x11fa7b000;//PXE_PA

    //ULONG64 PXE=
    //ULONG64 PPE=
    //ULONG64 PDE=
    //ULONG64 PTE=
    
    
    
}

//VA 00000000001ad000 #PA
//PXE at FFFF FAFD 7EBF 5000    PPE at FFFF FAFD 7EA0 0000    PDE at FFFF FAFD 4000 0000    PTE at FFFF FA80 0000 0D68
