## Notes

https://github.com/3had0w/Safety-Magazine

https://github.com/zhangjaycee/real_tech

https://github.com/Faran-17/Windows-Internals

主要是PEB、进程、服务相关的笔记

https://blog.can.ac/

windows-internal x86-inst exploit 

## PE

### 注入

https://github.com/guyHavia/reflective-dll-injection

反射注入，其实就是自己手动展开PE文件防止api调用留下特征 https://idiotc4t.com/defense-evasion/reflectivedllinjection-variation

https://github.com/NtRaiseHardError/NINA

基于栈劫持的无模块无线程创建的任意代码注入 shellcode被放在栈未使用的部分

https://github.com/DrNseven/SetWindowsHookEx-Injector

消息注入

https://github.com/can1357/ThePerfectInjector

基于PTE页表的完美隐藏的无痕注入 https://github.com/smallzhong/hide_execute_memory

https://github.com/VollRagm/PTView

https://github.com/cosine0/dllhook

dll注入python

### 保护

https://github.com/lem0nSec/ShellGhost

通过蜗牛式加密壳隐藏shellcode

https://github.com/igozdev/ObscuredItems

通过动态加密阻止内存值搜索

https://github.com/wanttobeno/DllProtect/

dll加密壳

https://github.com/JustasMasiulis/lazy_importer

懒加载导入

https://github.com/adamyaxley/Obfuscate

字符串混淆

https://github.com/weak1337/NO_ACCESS_Protection

https://github.com/noff2k/no-access-protection-x86

内存加密保护

https://github.com/charliewolfe/PointerGuard

使用VEH间接访问保护指针

### Hook

https://github.com/EasyHook/EasyHook

https://github.com/TsudaKageyu/minhook

最方便的两个um inlinehook

https://github.com/HoShiMin/HookLib

支持内核和用户态的inlinehook

https://github.com/stevemk14ebr/PolyHook_2_0

c++17 inlinehook

https://github.com/microsoft/Detours

inlinehook

https://github.com/SamuelTulach/LightHook

轻量inlinehook 可以用在efi、km、um

https://github.com/kubo/funchook

基于call替换的apihook

https://github.com/mrexodia/AppInitHook

基于AppInit_DLLs的全局hook

https://github.com/gfreivasc/VEHHook

https://github.com/hoangprod/LeoSpecial-VEH-Hook

https://github.com/codereversing/sehveh_hook

Vector Exception  Handling Hook

https://github.com/thamelodev/VShadow

虚表hook

https://github.com/weak1337/SkipHook

跳过一个指令的hook

https://github.com/apriorit/Wow64Hook

32位inlinehook64位

https://github.com/Oxygen1a1/InfinityHookClass

内核etwhook

https://github.com/1401199262/HookSwapContext/

etwhook swapcontext 枚举/隐藏进程

https://github.com/iPower/KasperskyHook

利用卡巴斯基的VT进行hook

https://github.com/Cr4sh/PTBypass-PoC

零环 通过PTE绕过hook检查

https://github.com/Rythorndoran/PageTableHook

通过pagetable绕过pg的hook

https://github.com/Fyyre/cmp_hook

### AntiHook

https://github.com/nickcano/ReloadLibrary

重载dll避免被hook

https://github.com/st4ckh0und/hook-buster

检查dll是否被hook

https://github.com/ch4ncellor/Inline-PatchFinder

导入表函数头检测

https://github.com/3gstudent/HiddenNtRegistry

https://github.com/mike1k/HookHunter

检查Hook

### 签名

https://github.com/Tylous/Limelighter

https://github.com/secretsquirrel/SigThief

自定义签名

### Trace

https://github.com/intelpt/WindowsIntelPT

intelPT



## RW

https://github.com/cristeigabriel/memscan

https://github.com/RadonCoding/mem-scraper

https://github.com/lucasvmx/libhack

https://github.com/kmgb/PatternScan

https://github.com/rdbo/libmem

https://github.com/nickcano/XenoScan

https://github.com/wanttobeno/x64_AOB_Search

https://github.com/ch4ncellor/chdr

https://github.com/DarthTon/Blackbone

https://github.com/DoranekoSystems/memory-server

https://github.com/CasualX/pelite

https://github.com/3nolan5/R5Apex-Driver



### linux

https://github.com/sleepy762/rwprocmem

https://github.com/kaiwan/device-memory-readwrite

https://github.com/scanmem/scanmem

## Rootkit

https://github.com/PHZ76/DesktopSharing

DXGI抓屏RTSP桌面转发

https://github.com/NVIDIA/video-sdk-samples/tree/master/nvEncDXGIOutputDuplicationSample

https://github.com/bmharper/WindowsDesktopDuplicationSample

桌面复制DDK抓屏

https://github.com/wanttobeno/BitBltScreenshotCleaner

HOOK BitBlt  简单的BitBlt钩子和屏幕快照清理器和记录器

https://github.com/bytecode77/r77-rootkit

三环rootkit

https://github.com/1401199262/AntiScreenCapture-r0

驱动反截图

https://github.com/SamuelTulach/windowhide

三环SetWindowDisplayAffinity反截图

https://github.com/M00nRise/ProcessHider

https://github.com/ryan-weil/HideProcessHook

三环hook目标进程Ntdll隐藏进程

https://github.com/TheD1rkMtr/BlockOpenHandle

三环提高进程权限阻止OpenHandle

通过nt api的rtl串来隐藏注册表项

https://github.com/oXis/GPUSleep

用GPU显存隐藏payload

https://github.com/mgeeky/ThreadStackSpoofer

伪装线程调用栈

https://github.com/dadas190/Heavens-Gate-2.0

从32位程序切换到64位来绕过杀软

https://github.com/rwfpl/rewolf-wow64ext

32位程序交互64位的辅助库

https://github.com/TheD1rkMtr/StackCrypt

https://github.com/TheD1rkMtr/HeapCrypt

挂起时加密堆栈



## ARK

https://github.com/zodiacon/DriverMon

https://github.com/hugsy/CFB

https://github.com/MellowNight/IOCTL-hook

监控IRP

https://github.com/namreeb/hacksniff/

hook Nt函数监控跨进程读写

https://github.com/1401199262/NMIStackWalk

检测无模块驱动

https://github.com/llkbkh/Anti-Rootkit

winark

https://github.com/zodiacon/AllTools

一堆工具

https://github.com/googleprojectzero/sandbox-attacksurface-analysis-tools

Windows系统工具 EditSection\NtApiDotNet\NtObjectManager\...

https://github.com/hfiref0x/WinObjEx64

winObj解析

https://github.com/horsicq/xntsv

系统结构实时解析

https://github.com/hzqst/Syscall-Monitor

syscall监控

https://github.com/FiYHer/system_trace_tool

内核驱动加载/卸载痕迹清理 PiDDBCacheTable and MmLastUnloadedDriver

https://github.com/can1357/ByePg

劫持HalPrivateDispatchTable的内核异常hook从而绕过PG

https://github.com/1401199262/Hook-KdTrap

直接hook kdtrap控制内核异常分发

https://github.com/hfiref0x/SyscallTables

NT全版本系统调用

https://github.com/daem0nc0re/PrivFu

windows内核令牌揭秘

https://github.com/liangfei44/VmWareThrough

host穿透读写vmware内存 解析windows系统内存

https://github.com/Spuckwaffel/Simple-MmcopyMemory-Hook

hook内核调用



## KM

### EFI

https://github.com/ekknod/KiSystemStartupMeme

EFI时劫持NT文件 执行受PG保护的任意代码，实例参考`ekknod/EC > EFI`

https://github.com/SamuelTulach/efi-memory

efi内存读写

### RW

https://github.com/si1kyyy/readwritedriver

https://github.com/hack-different/kext-kmem

https://github.com/nbqofficial/norsefire

### 按键

https://github.com/ViGEm/ViGEmBus

模拟输入

### 通讯

https://github.com/wbenny/KSOCKET

https://github.com/FiYHer/dxgkrnl_hook_ex

https://github.com/vmcall/dxgkrnl_hook

https://github.com/sondernextdoor/Poseidon

https://github.com/NullTerminatorr/NullHook

https://github.com/BerkanYildiz/Driver.NET csharp做客户端

### 其他

https://github.com/OSRDrivers/kmexts

内核callback注册实例

https://github.com/Ahora57/GetKernelBaseEx

获取ntoskrnl基地址

https://github.com/FaEryICE/MemScanner/

内核内存扫描

https://github.com/reservedcloud/open98

内核代码参考

https://github.com/Nou4r/HydraHook

hook ExAllocatePoolWithTag 过滤检测

### DWM

https://github.com/TKazer/Dwm

https://github.com/Rythorndoran/dwm-overlay

https://github.com/notr1ch/DWMCapture

https://github.com/ilovecsad/dwmhook

### Draw

https://github.com/Dy-Baby/CallMeWin32kDriver

https://github.com/BadPlayer555/KernelGDIDraw

https://github.com/rlybasic/DWM_Hook

https://github.com/armasm/dwmhook

## Bootkit

https://github.com/ldpreload/BlackLotus

通过板厂Leak的证书绕过SecureBoot的Bootkit



## RE

https://github.com/Shauren/protobuf-decompiler

protobuf二进制文件反编译工具

https://github.com/CodeCracker-Tools/MegaDumper

dotnet dumper



## Game-Re

### Re

https://github.com/TKazer/DrawAlgorithm

fps游戏逆向各功能绘制算法

https://github.com/guided-hacking/GH-Entity-List-Finder

实体列表搜索

### Source2

https://github.com/anarh1st47/Source2Dumps

https://github.com/neverlosecc/source2sdk

https://github.com/albertodemichelis/squirrel

起源2支持的脚本语言之一 apex中大量使用

https://github.com/sneakyevil/CS2-SchemaDumper

k32内存读取Csgo2的Schema数据

```c
m_SignatureAddress = Memory::FindSignature(reinterpret_cast<uintptr_t>(m_SchemaSystemBytes), m_SchemaModule[1], "48 89 05 ? ? ? ? 4C 8D 45 D0");
```

### Unity

https://github.com/sneakyevil/IL2CPP_Resolver

https://github.com/HTCheater/Il2CppExplorer

https://github.com/djkaty/Il2CppInspector

https://github.com/Perfare/Il2CppDumper

IL2cpp运行时解析

https://github.com/sinai-dev/UnityExplorer

运行时解析GUI工具

### UE

https://github.com/UE4SS-RE/RE-UE4SS

UE逆向套件

https://github.com/Jiang-Night/UE5Dumper

https://github.com/Chordp/UnrealDump

https://github.com/Spuckwaffel/UEDumper

https://github.com/3nolan5/Unreal-Engine---Internal-Hack

UEdumper

https://github.com/percpopper/UE-Freecam

UE相机cheat

https://github.com/OutTheShade/CPakParser

UE5 Pak文件解析

https://github.com/OutTheShade/UnrealMappingsDumper

生成usmap

https://github.com/OutTheShade/Unreal-Mappings-Archive

UE usmap集合 可用于判断游戏

## AntiCheat

https://github.com/0dayatday0/BattleFN-cheat-analysis



## CPPLib

https://github.com/hikogui/hikogui

https://github.com/adamhlt/ImGui-Standalone

GUI

https://github.com/nlohmann/json

JSON for Modern C++

https://github.com/aappleby/Matcheroni

正则

https://github.com/jermp/pthash

hash库

https://github.com/asmjit/asmtk

x86汇编工具库

https://github.com/Squalr/Self-Modifying-Code

使用asmtk进行自修改

https://github.com/ithewei/libhv

现代网络库

https://github.com/yhirose/cpp-httplib

http库

https://github.com/tezc/sc

linux通用库 （c）

https://github.com/GiovanniDicanio/WinReg

注册表库

https://github.com/hasherezade/libpeconv

PE开发库

## Cheat-Base

### Python

https://github.com/ekknod/scripthial

### Lua

https://github.com/ekknod/G37OBS

lua obs插件 外挂开发  利用OBS白名单权限绕过部分AC

### C++

https://github.com/ch4ncellor/chdr

进程操作库 很完整

https://github.com/bruhmoment21/UniversalHookX

通用overlay

https://github.com/YiCiqing/c-desktop-program-common-library

易++ 非常乐

https://github.com/TKazer/OS-ImGui

### Imgui

https://github.com/Blick1337/imgui-glitch-shader

### SpecGame

#### external

https://github.com/si1kyyy/valorant_external_cheat

https://github.com/TosoxDev/Destiny.ut

#### internal

https://github.com/papstuc/counterstrike2

https://github.com/nezu-cc/BakaWare4

https://github.com/bruhmoment21/cs2-sdk

https://github.com/jz0/source2-basehook

https://github.com/DragonQuestHero/PUBG-ESP

https://github.com/ch4ncellor/CSGO-P2C-Dumper

#### other

https://github.com/Lujiang0111/FFxivUisaveParser

https://github.com/ColombianGuy/r5_flowstate

https://github.com/TanukiSharp/MHArmory

https://github.com/Strackeror/MHW-QuestLoader

https://github.com/Synthlight/MHW-Editor

## ML

https://github.com/hiroi-sora/Umi-OCR



## LLVM

https://github.com/gmh5225/awesome-llvm-security

## PL

https://github.com/rswier/c4

简易C解释器实现

https://github.com/lhmouse/asteria

https://github.com/archibate/constl

https://github.com/ChaiScript/ChaiScript

https://github.com/modernizing/modernization

https://github.com/kokke/tiny-regex-c

## GameDev

https://github.com/facebook/igl

跨平台gl

https://github.com/terminal29/Simple-OpenVR-Driver-Tutorial

vr驱动



## Chromium

https://github.com/tomer8007/chromium-ipc-sniffer

IPC监控



## PWN

### Linux

https://github.com/lrh2000/StackRot

CVE-2023-3269: Linux kernel privilege escalation vulnerability

### android

https://github.com/saeidshirazi/awesome-android-security

### Toolkit

https://github.com/polyverse/disasm

自带反编译器的独立ropgadgets搜索器

https://github.com/ef4tless/xclibc

libc修改工具

## Misc

https://github.com/EricZimmerman/RegistryPlugins

注册表取证项

https://github.com/Dadido3/D3hex

二进制分析工具

## App

https://github.com/UnigramDev/Unigram

https://github.com/FurkanGozukara/Stable-Diffusion

https://github.com/comfyanonymous/ComfyUI

https://github.com/libin9iOak/ja-netfilter-all

jvm-proload hook 激活jetbrain