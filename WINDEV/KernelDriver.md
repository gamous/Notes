# Windows Kernel



## DSE

禁止加载不正确签名的驱动

### 签名

#### **DSignTools**

https://github.com/wanttobeno/FuckCertVerifyTime

通过Hook亚信签名工具的时间验证，使用**泄露的过期签名**对驱动进行双签

时间戳CA劫持已经失效，直接无时间戳签名然后配合下列设置 （需要确认Secure Boot关闭）

> 即使关闭了验证， 仍然需要驱动存在签名
>
> 即使在测试模式下，驱动仍然需要测试等级以上的签名存在
>
> VMware设置 -> 固件类型 -> UEFI -> 安全引导 
>
> WIN10以上系统需要双签

```cmd
;关闭签名验证
bcdedit.exe /set nointegritychecks on
;开启测试模式
bcdedit /set testsigning on
```

#### CSignTools

命令行快速签名

用PE工具对CSignTools添加导入项 `HookSigntool.dll!attach`

用ContextMenuManager添加项`D:\\Program Portable\\DSigntool\\CSignTool.exe sign /r single /f %1  /kp`

**Windows11**

绕过SecureBoot并关闭`VulnerableDriverBlocklistEnable`

```
reg add HKLM\SYSTEM\CurrentControlSet\CI\Config /v "VulnerableDriverBlocklistEnable" /t REG_DWORD /d 0 /f
reg add HKLM\SYSTEM\CurrentControlSet\Control\DeviceGuard\Scenarios\HypervisorEnforcedCodeIntegrity/v "Enabled" /t REG_DWORD /d 0 /f
```

#### Inf2Cat

使用WDK自带的测试签，也是微软提供给开发者的预期方案，设置起来较为麻烦

```
1. 安装WDK（Windows Driver kit）
2. 在WDK安装目录下找到Inf2Cat.exe
3.进入控制台（cmd）
4.输入以下命令：
Inf2Cat /driver:C:\MyDriver /os:2000,XP_X86,Server2003_X86,Vista_X86
注意：C:\MyDriver是我的驱动所在目录，请自行修改。
5. 正常情况会生成CAT文件，文件名由INF文件中的CatalogFile进行设置。
```

#### WHQL

正式签名 昂贵

### 利用漏洞驱动

#### KDU

https://github.com/hfiref0x/KDU

利用有漏洞的签名驱动来关闭DSE或加载任意用户驱动

```
kdu -list 列出目前可利用的漏洞驱动
kdu -ps 1234 #降级1234进程的任何保护
kdu -dse 0 #关闭DSE
kdu -dse 6 #开启DSE
kdu -map c:\driverless\mysuperhack.sys #加载mysuperhack.sys到内核
kdu -prv 6 -scv 3 -drvn edrv -drvr e3600bl -map c:\install\e3600bl.sys
#使用6号漏洞驱动 第3个版本的shellcode 将e3600bl.sys作为名称edrv的驱动加载并注册为e3600bl
kdu -pse "C:\Windows\System32\notepad.exe C:\TEMP\words.txt"
#将nodepad作为PPL反病毒软件运行
```

> PLL ( ProtectedProcessLight-AntiMalware ) 是一种进程权限，可以最大权限防止进程内存被读写以及防止注入
>
> 详细见以下链接
>
> https://paper.seebug.org/1892
>
> https://learn.microsoft.com/en-us/windows/win32/services/protecting-anti-malware-services-

#### KdMapper 

https://github.com/TheCruZ/kdmapper

利用Intel的驱动操作内核内存

https://forum.butian.net/share/1706

奇安信这篇share利用KdMapper 间接修改**`win11 CI.dll!g_CiOptions=0`**干掉了DSE

**映射物理内存到另一个虚拟地址然后修改后再进行恢复 可以绕过 MmProtectDriverSection 的保护**

#### Black-Angel-Rootkit

https://github.com/XaFF-XaFF/Black-Angel-Rootkit

使用kdmapper实现的内核rootkit

- 关闭DSE和KPP
- 隐藏进程和端口
- 进程提权
- 进程保护

#### VulDrv

https://github.com/alfarom256/CVE-2022-3699

[fengjixuchui/gdrv-loader](https://github.com/fengjixuchui/gdrv-loader)

一些可用的驱动

 https://www.unknowncheats.me/forum/anti-cheat-bypass/334557-vulnerable-driver-megathread.html

EAC也会检测部分被滥用的驱动作为外挂特征

### Bootkit

#### EfiGuard

https://github.com/Mattiwatti/EfiGuard

通过Bootkit技术，修改windows bootloader，从而在加载时禁用DSE和PG

#### SandboxBootkit

https://github.com/thesecretclub/SandboxBootkit

通过修改windows sandbox的`ntoskrnl.exe`，来关闭沙箱中的DSE和PG

### 直接加载工具

无需签名+超难检测 的 驱动加载工具 https://www.52pojie.cn/thread-1347349-1-1.html

在Win64系统上动态加载无签名驱动：WIN64LUD http://www.m5home.com/bbs/thread-7839-1-1.html

### 隐藏加载驱动

https://bbs.kanxue.com/thread-276912.htm

https://github.com/IcEy-999/Drv_Hide_And_Camouflage

## PatchGuard / KPP

循环检查几个重要驱动数据段和代码段释放被修改,如果被修改,则触发0x109蓝屏

### Shark

https://github.com/9176324/Shark

**运行时直接关闭PG的黑科技，配合其他工具非常好用**

### PatchGuardBypass

https://github.com/AdamOron/PatchGuardBypass

## WindowsDefender

在虚拟机里经常需要关掉WD防止乱删测试文件

联想支持中心刚好提供了一些小工具

关闭**Windows Defender Service** https://iknow.lenovo.com.cn/detail/dc_181036.html

关闭**Defender Smartscreen** https://iknow.lenovo.com.cn/detail/dc_194871.html

## VMware双机调试

一种VMware虚拟机的配置流程

### VMware安装

https://github.com/201853910/VMwareWorkstation

```
Win   Downloadlink：https://www.vmware.com/go/getworkstation-win
Linux Downloadlink：https://www.vmware.com/go/getworkstation-linux
VMware17 KEY: HC4XR-28185-4J8X0-1U3QK-A3RMF
```

VMware全局的几个优化点（Edit > Preferences...）

- Memory > Fit virtual machine memory into reserved host RAM （停用虚存，减少**硬盘IO**，硬盘IO是虚拟机最耗时的操作之一）
- Priority > Input grabbed > High 

### 系统安装

镜像见测试系统节

非windows官方镜像例如tiny系列必须要手动安装而不能使用自动

### 虚拟机优化

这里记录几个优化性能的点

- 虚拟机的内存需要适量，不能超过Host空余内存，按照推荐分配就行了
- Advanced > Input grabbed > High 
- Advanced > Memory page trimming is not allowd （停止物理内存到硬盘的同步，减少**硬盘IO**）

有关虚拟化引擎

- 勾选会降低性能，正常情况下别勾
- 勾选会导致被更多反虚拟化技术检测

vmx添加以下防止一些程序cpuid的检测

```
hypervisor.cpuid.v0 = "FALSE"
```

### 双机调试串口

先删除打印机避免端口占用

```
编辑虚拟机设置 -> 添加 -> 串行端口 -> 完成
使用命名管道 -> \\.\pipe\com_1 -> 该端是服务器，另一端是应用程序 -> 轮询时主动放弃CPU->确定
```

```
bcdedit /set testsigning on
bcdedit -debug on
bcdedit /bootdebug on
bcdedit /set testsigning on
bcdedit /dbgsettings SERIAL DEBUGPORT:1 BAUDRATE:115200    // 设置串口1为调试端口波特率为115200
bcdedit /copy "{current}" /d "Debug"                       // 将当前配置复制到Debug启动配置
bcdedit /debug "{<新建的启动配置的标识符>}" on               // 打开调试开关
```

```
bcdedit /dbgsettings
```

确认Serial 1 的baudrate为115200

开机的时候选择`Windows10 [启用调试程序]`

```
windbg.exe -b -k com:port=\\.\pipe\com_1,baud=115200,pipe
```

可以设置一个专门的快捷方式作为入口

配置好符号加载并启动系统

符号下载太慢可以用OpenArk的符号源镜像

```
kd> .sympath SRV*c:\mySymbols*http://msdl.microsoft.com/download/symbols
kd> .reload
kd> g
kd> g
kd> ed nt!Kd_SXS_Mask 0
kd> ed nt!Kd_FUSION_Mask 0
kd> u KiSystemServiceUser
```

### 调试

常用的调试函数

```c
DbgPrint("Test");
DbgBreakPoint();
```

内核态调试器重定向用户态调试器 https://bbs.kanxue.com/thread-260666.htm

**DbgPrint的信息还可以在DebugView 中查看** (勾选Capture Kernel)

更好用的DebugViewPP

https://github.com/CobaltFusion/DebugViewPP

## 其他双机调试

### VDK

https://github.com/4d61726b/VirtualKD-Redux

[VirtualKD-Redux/Tutorial.md](https://github.com/4d61726b/VirtualKD-Redux/blob/master/VirtualKD-Redux/Docs/Tutorial.md)

### BugChecker

https://github.com/vitoplantamura/BugChecker

## WDK

[下载 Windows 驱动程序工具包 (WDK) - Windows drivers | Microsoft Learn](https://learn.microsoft.com/zh-cn/windows-hardware/drivers/download-the-wdk)

[以前的 WDK 版本和其他下载 - Windows drivers | Microsoft Learn](https://learn.microsoft.com/zh-cn/windows-hardware/drivers/other-wdk-downloads)

[简单修改使WDK 22H2（VS2022）兼容旧版WDK10](https://www.52pojie.cn/thread-1674727-1-1.html)

一些坑：

- 可以装相对系统版本更旧的WDK编译使用
- **VS版本需要和WDK匹配**
- 高版本WDK可以编译兼容低版本的驱动
- 由于win7停止支持，WDK 22H2不兼容win7以下

## Mingw编译

低配电脑开VS实在太卡了，用mingw比较轻量，抄来一个编译方法

一定要使用64位版本的mingw-**w64**，否则winternl.h都没有

```
编译:gcc -o "OBJ文件名" -O3 -c "源文件名"
连接:ld "OBJ文件名" --subsystem=native --image-base=0x10000 --file-alignment=0x1000 --section-alignment=0x1000 --entry=_DriverEntry@8 -nostartfiles --nostdlib -shared -L "库路径" -l ntoskrnl -o "驱动文件名"
```

[基于MSYS的Windows Driver开发 - Martin 的博客](http://martin.is-programmer.com/posts/12654.html)

## 测试系统

测试驱动时往往不需要使用全功能的系统安装，许多不必要的服务拖慢了系统的速度

一些简化的系统更便于测试

### 原版镜像

https://next.itellyou.cn/

https://msdn.itellyou.cn/

### Tiny11

https://archive.org/details/tiny-11-NTDEV

https://github.com/ntdevlabs/tiny11builder

### Win7

https://www.ghxi.com/category/all/system/win7

### 可编译W2K3

从泄露的winxp源码编译windows 2003

[Windows Server 2003 (NT 5.2.3790.0) build guide (rentry.co)](https://rentry.co/build-win2k3)

### 简化工具

NtLite

Msys



## 驱动开发

理解windows内核——驱动开发 https://www.anquanke.com/post/id/262435

[MSR_HOOK - Note (gitbook.io)](https://b0ldfrev.gitbook.io/note/windows_kernel/msr_hook)

[allogic/KSU: The Kernel Script Utility is an x64 Kernel-Mode rootkit written in C. It was designed to disect and intercept process memory flow. (github.com)](https://github.com/allogic/KSU)

https://bbs.kanxue.com/thread-271211.htm

## WINDBG

https://rayanfam.com/topics/pykd-tutorial-part1/
