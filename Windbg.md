## GBD2WINDBG

原文：https://github.com/wangray/WinDBG-for-GDB-users

从gdb到windbg的快速参照表

### Breakpoints

| GDB Command             | WinDBG Command | Description     | Usage/Examples           |
| ----------------------- | -------------- | --------------- | ------------------------ |
| `b/break`               | `bp`           | 设置断点        |                          |
| `disable`               | `bd #`         | 禁用断点        |                          |
| `enable`                | `be #`         | 启用断点        |                          |
| `info breakpoints`/`ib` | `bl`           | 列举断点        |                          |
| `watch`                 | `ba`           | 访存断点(读/写) | `ba [r|w|e] [Size] Addr` |

### Running/Stepping

| GDB Command | WinDBG Command | Description |
| ----------- | -------------- | ----------- |
| `r/run`     | `g` `.restart` | 运行        |
| `s/si`      | `p`            | 步过        |
| `n/ni`      | `t`            | 步进        |
| `finish`    | `pt`           | 跳出        |
| None        | `pc`           | 运行到调用  |
| `u`         | `pa`           | 运行到地址  |

### Variables, Symbols, and Memory

| GDB Command        | WinDBG Command | Description  | Usage/Example                                                |
| ------------------ | -------------- | ------------ | ------------------------------------------------------------ |
| `x*`               | `d*`           | 查看指定内存 | a = ascii chars u = Unicode chars b = byte + ascii w = word (2b) W = word (2b) + ascii d = dword (4b) c = dword (4b) + ascii q = qword (8b)  `dd 0x1000000` |
| `set {int}addr = ` | `e*`           | 编辑指定内存 | `ed 0x1000000 deadbeef`  a = ascii string za = ascii string (NULL-terminated) u = Unicode string zu = Unicode string (NULL-terminated) `e[a|u|za|zu] addr "String"` |
| `print`/`p`        | `dt/dv`        | 打印变量     | `dt ntdll!_PEB` `dt ntdll!_PEB @$peb`                        |
| `disasm`           | `u`            | 反汇编       | `u kernel32!CreateProcessAStub`                              |
| `* (deref)`        | `poi`          | 指针解引用   | `u poi(ebp+4)`                                               |
| None               | `x`            | 检查符号     | `x *!` `x /t /v MyDll!*` list symbols in MyDll with data type, symbol type, and size |

#### C++ Expression Syntax

| GDB Command                | WinDBG Command              | Description                                                  | Usage/Example                           |
| -------------------------- | --------------------------- | ------------------------------------------------------------ | --------------------------------------- |
| `p (Datatype *) &variable` | `dx (Datatype *) &variable` | displays a C++ expression                                    | `dx (nt!_EPROCESS *) &nt!PsIdleProcess` |
| `p [expression]`           | `??`                        | Evaluate C++ expressions. Used with the C++ expression parser - `@@c++()`, that supports operators, registers, macros. etc. See [docs](https://docs.microsoft.com/en-us/windows-hardware/drivers/debugger/c---numbers-and-operators) for a full list | `?? @@c++(1+2)`                         |

### Registers

使用 `@` 访问寄存器 , 例如 `@eip`.

| GDB Command      | WinDBG Command | Description | Usage/Example                                                |
| ---------------- | -------------- | ----------- | ------------------------------------------------------------ |
| `info registers` | `r`            | 查看寄存器  | `r Reg1  Reg2` `r Reg:Type` `Type` = data format in which to display the register (i.e.: `r eax:uw`) ib = Signed byte ub = Unsigned byte iw = Signed word (2b) uw = Unsigned word (2b) id = Signed dword (4b) ud = Unsigned dword (4b) iq = Signed qword (8b) uq = Unsigned qword (8b) f = 32-bit floating-point d = 64-bit floating-point |
| `set reg =`      | `r Reg=Value`  | 修改寄存器  |                                                              |

### Getting information

| GDB Command          | WinDBG Command | Description            | Usage/Example                |
| -------------------- | -------------- | ---------------------- | ---------------------------- |
| `info proc mappings` | `!address`     | 查看虚拟地址映射与权限 | `!address addr`              |
| `print`/`p`          | `x`            | 检验符号               | `x kernel32!*CreateProcess*` |
| None                 | `ln`           | 列出地址附近的符号     |                              |
| `backtrace`/`bt`     | `k`            | 栈回溯                 |                              |
| None                 | `!exchain`     | 查看 SEH 链            |                              |
|                      |                |                        |                              |

### Other useful commands

`!peb` – 显示 Process Environment Block 内容

`dt ntdll!_PEB @$peb` — 显示 PEB 的更多内容

### Tips

The WinDBG executable is installed in `C:\Program Files (x86)\Windows Kits\10\Debuggers\x86[64]/`. If it's not in your path, add it by going to the `Edit system environment variables` menu, and append to the `Path` variable.

`$peb` is a "pseudo-register", and there are [others](https://docs.microsoft.com/en-us/windows-hardware/drivers/debugger/pseudo-register-syntax) that hold useful values. Some are `$teb`, `$csp`, `$curprocess`.

### References

http://windbg.info/doc/1-common-cmds.html