# HyperVisor

## INTEL VT-X

### MyVT学习

https://bbs.kanxue.com/thread-271854.htm

#### 导览

```c
int VmxInit(){
	PVMXCPUPCB pVcpu = VmxGetCurrentCPUPCB();
    //vmon区域初始化 VmxInitVmOn()
    ...
    	int error = __vmx_on(&pVcpu->VmxOnAddrPhys.QuadPart);//开机柜
    ...
    //vmcs区域初始化 VmxInitVmcs()
    ...
		__vmx_vmclear(&pVcpu->VmxcsAddrPhys.QuadPart);//拔电源
		__vmx_vmptrld(&pVcpu->VmxcsAddrPhys.QuadPart);//选中机器
    	//VmxInitGuest(GuestEip, GuestEsp);
		//VmxInitHost(hostEip);
		//VmxInitEntry();
		//VmxInitExit();
		//VmxInitControls();
    		__vmx_vmwrite...//配置vmcs
             //VmxInitEpt();
    ...
	error = __vmx_vmlaunch();//开机
}	
```

#### 开锁：支持检测

Intel开发手册-31.5  VMM SETUP & TEATDOWN

```c
//检测Bios是否开启VT
BOOLEAN VmxIsCheckSupportVTBIOS(){
    ULONG64 value = __readmsr(IA32_FEATURE_CONTROL);
    return (value & 0x5) == 0x5;
}
```

```c
//检测CPU是否支持VT
BOOLEAN VmxIsCheckSupportVTCPUID(){
    int cpuidinfo[4];
    __cpuidex(cpuidinfo, 1, 0);
    //CPUID 是否支持VT ecx.vmx第6位 如果为1，支持VT，否则不支持
    return (cpuidinfo[2] >> 5) & 1;
}
```

```c
//检测CR4VT是否开启，如果为1 代表已经开启过了，否则没有开启
BOOLEAN VmxIsCheckSupportVTCr4(){
    ULONG64 mcr4 = __readcr4();
    //检测CR4 VT是否开启，cr4.vmxe如果第14位为1，那么VT已经被开启，否则可以开启
    return ((mcr4 >> 13) & 1) == 0;
}
```

#### 开机柜：vmxon

Intel开发手册-24.11.5 VMXON Region

```c
//申请一块4KB对齐的非分页内存
PHYSICAL_ADDRESS lowphys,heiPhy;
lowphys.QuadPart = 0;
heiPhy.QuadPart = -1;
pVcpu->VmxOnAddr = MmAllocateContiguousMemorySpecifyCache(PAGE_SIZE, lowphys, heiPhy, lowphys, MmCached);
//rtlzeromemory确保挂页
memset(pVcpu->VmxOnAddr, 0, PAGE_SIZE);
pVcpu->VmxOnAddrPhys = MmGetPhysicalAddress(pVcpu->VmxOnAddr);
//低四字节填入vmxon
ULONG64 vmxBasic = __readmsr(IA32_VMX_BASIC);
*(PULONG)pVcpu->VmxOnAddr = (ULONG)vmxBasic;
```

Intel开发手册-26.3.1.1 Checks on Guest Control Register,Debug Registers, and MSRs

```c
ULONG64 vcr00 = __readmsr(IA32_VMX_CR0_FIXED0);
ULONG64 vcr01 = __readmsr(IA32_VMX_CR0_FIXED1);
ULONG64 vcr04 = __readmsr(IA32_VMX_CR4_FIXED0);
ULONG64 vcr14 = __readmsr(IA32_VMX_CR4_FIXED1);
 
ULONG64 mcr4 = __readcr4();
ULONG64 mcr0 = __readcr0();
 
mcr4 |= vcr04;
mcr4 &= vcr14;
mcr0 |= vcr00;
mcr0 &= vcr01;
 
__writecr0(mcr0);
__writecr4(mcr4);
```

```c
int error = __vmx_on(&pVcpu->VmxOnAddrPhys.QuadPart);
```

#### 拔电源：vmclear

拔电源前先申请vmcs内存用于保存虚拟机上下文，用于"装机"，与vmxon类似

```c
//申请一块4KB对齐的非分页内存(vmcs)
PHYSICAL_ADDRESS lowphys, heiPhy;
lowphys.QuadPart = 0;
heiPhy.QuadPart = -1;
pVcpu->VmxcsAddr = MmAllocateContiguousMemorySpecifyCache(PAGE_SIZE, lowphys, heiPhy, lowphys, MmCached);
//申请内存失败
if (!pVcpu->VmxcsAddr)return -1;
//挂页 获得物理地址
memset(pVcpu->VmxcsAddr, 0, PAGE_SIZE);
pVcpu->VmxcsAddrPhys = MmGetPhysicalAddress(pVcpu->VmxcsAddr);
//申请一块4KB对齐的非分页内存(vm stack)
pVcpu->VmxHostStackTop = MmAllocateContiguousMemorySpecifyCache(PAGE_SIZE * 36, lowphys, heiPhy, lowphys, MmCached);
//申请内存失败
if (!pVcpu->VmxHostStackTop)return -1;
//
memset(pVcpu->VmxHostStackTop, 0, PAGE_SIZE * 36);
pVcpu->VmxHostStackBase = (ULONG64)pVcpu->VmxHostStackTop + PAGE_SIZE * 36 - 0x200;
//低四字节(id)填入vmcs
ULONG64 vmxBasic = __readmsr(IA32_VMX_BASIC);
*(PULONG)pVcpu->VmxcsAddr = (ULONG)vmxBasic;
```

初始化

```c
__vmx_vmclear(&pVcpu->VmxcsAddrPhys.QuadPart);
```

#### 选中机器：vmptrload

```c
__vmx_vmptrld(&pVcpu->VmxcsAddrPhys.QuadPart);
```

#### 装机：init vmcs (vmwrite)

##### 上下文保存

24.4 Guest-State Area

Guest 各寄存器存档的位置，vmlaunch时从该区域加载，vmexit时保存到该区域

24.5 Host-State Area

Host 各寄存器存档的位置，vmexit时从该区域加载，vmlaunch时保存到该区域

参考《处理器虚拟化技术》3.4章节 表格 与 图3.8

```c
//段寄存器重组拼接格式
void FullGdtDataItem(int index,short selector){
	GdtTable gdtTable = {0};
	AsmGetGdtTable(&gdtTable);
	selector &= 0xFFF8;

	ULONG limit = __segmentlimit(selector);
	PULONG item = (PULONG)(gdtTable.Base + selector);
	LARGE_INTEGER itemBase = {0};
	if(7==index){
		//读TR
		itemBase.LowPart = ((item[0] >> 16) & 0xFFFF) | ((item[1] & 0xFF) << 16) | ((item[1] & 0xFF000000));
		itemBase.HighPart = item[2];
		//属性
		ULONG attr = (item[1] & 0x00F0FF00) >> 8;
	}else{
		itemBase.LowPart = (*item & 0xFFFF0000) >> 16;
		item += 1;
		itemBase.LowPart |= (*item & 0xFF000000) | ((*item & 0xFF) << 16);
		//属性
		ULONG attr = (*item & 0x00F0FF00) >> 8;
		if (selector == 0) attr |= 1 << 16;
	}
	//index 0 -> GUEST_ES_BASE(0x00006806)
    //index 1 -> GUEST_CS_BASE(0x00006808)
    //...
	__vmx_vmwrite(GUEST_ES_BASE + index * 2, itemBase.QuadPart);
	__vmx_vmwrite(GUEST_ES_LIMIT + index * 2, limit);
	__vmx_vmwrite(GUEST_ES_AR_BYTES + index * 2, attr);
	__vmx_vmwrite(GUEST_ES_SELECTOR + index * 2, selector);
}

```





##### 控制字段

24.6 VM-Execution Control Fields

24.7 VM-Exit Control Fields

24.8 VM-Entry Control Fields

##### 异常处理

24.9 VM-Exit Information Fields

只读区域 存储vmx失败代码



	//VmxInitGuest(GuestEip, GuestEsp);
	//VmxInitHost(hostEip);
	//VmxInitEntry();
	//VmxInitExit();
	//VmxInitControls();



#### 开机：vmlaunch

#### vmexit

#### vmresume

#### 拔电源：vmclear

#### 关柜门：vmxoff

### 相关项目

https://github.com/changeofpace/VivienneVMM



### 参考





## AMD  SVM

### SimpleSvmHook

https://github.com/tandasat/SimpleSvmHook

### AetherVisor

https://github.com/MellowNight/AetherVisor

