# AntiRootKits

## PCHunter

免费版本(v1.56) 支持到Win10(17763) http://www.xuetr.com/

破解版本(v1.57) 支持到**Win10(18363)** https://www.ghxi.com/pchunter.html

破解思路 https://bbs.kanxue.com/thread-271801-1.htm

功能：

1.进程、线程、进程模块、进程窗口、进程内存信息查看，杀进程、杀线程、卸载模块等功能
2.内核驱动模块查看，支持内核驱动模块的内存拷贝
3.SSDT、Shadow SSDT、FSD、KBD、TCPIP、Classpnp、Atapi、Acpi、SCSI、IDT、GDT信息查看，并能检测和恢复ssdt hook和inline hook
4.CreateProcess、CreateThread、LoadImage、CmpCallback、BugCheckCallback、Shutdown、Lego等Notify Routine信息查看，并支持对这些Notify Routine的删除
5.端口信息查看，目前不支持2000系统
6.查看消息钩子
7.内核模块的iat、eat、inline hook、patches检测和恢复
8.磁盘、卷、键盘、网络层等过滤驱动检测，并支持删除
9.注册表编辑
10.进程iat、eat、inline hook、patches检测和恢复
11.文件系统查看，支持基本的文件操作
12.查看（编辑）IE插件、SPI、启动项、服务、Host文件、映像劫持、文件关联、系统防火墙规则、IME
13.ObjectType Hook检测和恢复
14.DPC定时器检测和删除
15.MBR Rootkit检测和修复
16.内核对象劫持检测
17.WorkerThread枚举
18.Ndis中一些回调信息枚举
19.硬件调试寄存器、调试相关API检测
20.枚举SFilter/Fltmgr的回调
21.系统用户名检测

## OpenArk

带一套工具箱，但是没有inline hook检测

https://github.com/BlackINT3/OpenArk

## YDARK

进程检测可以防进程隐藏

https://github.com/ClownQq/YDArk/blob/master/README.md

## WKE

https://github.com/AxtMueller/Windows-Kernel-Explorer

## PYARK

有调试增强的功能

https://github.com/antiwar3/py

## KE64

https://github.com/alinml/ke64

1. 进程,线程,模块,窗口,内存,定时器,热键,(等显示 隐藏，欺骗，杀进程,杀线程，卸载，移除)
2. 用户钩子(消息钩子,事件钩子，inline,iat,eat,hook检测)
3. 驱动模块(卸载等操作)
4. FilterDriver(File,Disk,RAW,Volume,Keyboard,Mouse,I8042prt，Tdx,NDIS,PnpManager)(移除等操作)
5. CreateProcess,LoadImage,CreateThread,CmpCallback,Shutdown(移除等操作)
6. Callbak,ObjectType,ObjectTypeHook,DPC,WFPCallout,minifilter,WorkerThread(堆栈回溯)(移除等操作)
7. IRP(Keyboard,Mouse,I8042prt,ndis,nsiproxy,tcpip,partmgr,disk,ntfs,scsi,npfs,fltmgr)
8. GDI,IDT
9. 端口查看
10. 启动项(删除等操作)
11. 服务(启动,停止,暂定,恢复,重启,删除,启动类型,定位注册表,属性)
12. 注册表管理(删除,重命名,导出,新建(项，二进制，DWORD,QWORD,多字符串,可扩充字符串),修改(DWORD,QWORD,多字符串))
13. 文件管理(删除,文件快速定位,文件锁定,重命名,拷贝文件,去除只读隐藏属性,设置只读隐藏属性，属性)
14. 新增行为监视(行为包括: 文件(创建，读，写，删除，更名，设置属性，设置权限)， 注册表(打开，创建，删除项，删除值，读值，更名项，设置安全，查询值，设置值)， 进程(创建， 启动，销毁)， 线程(创建，销毁)， 模块(加载)， 网络(连接，监听，接收，发送) )等行为监控。
15. 应用层和驱动层(支持反汇编和汇编内存)
16. 功能(...)

## WKTools

https://github.com/AngleHony/WKTools

## WIN64AST

支持到Win10(16299)

http://www.m5home.com/bbs/thread-6975-1-1.html

https://bbs.kafan.cn/thread-1426416-1-1.html



## PPLcontrol

https://github.com/itm4n/PPLcontrol

检测PPL进程，或设置任意进程为PPL权限