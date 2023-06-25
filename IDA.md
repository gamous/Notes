# Better IDA Better Life







## 中文逆向

### 中文函数名

```c++
//ida.dll!calc_c_cpp_name
//0f 85 ? ? ? ? 80 7b ? ? 48 ? ? ? 0f 85 - 0xa2
//75 ? 8b ? 48 ? ? ? ? ? ? e8 ? ? ? ? 48 ? ? 75 ? 40 + 0x2d
      if ( !v8 || !strchr(" [](),*&", v11) )
      {
        if ( v10 - v9 < 8 || strncmp(v10 - 8, "operator", 8ui64) )
        {
LABEL_23:
          *v10 = '_'; //此处导致中文函数名显示异常
          goto LABEL_24;
        }
        if ( strchr(" +-*/%&!^", v11) )
```

https://www.52pojie.cn/thread-1414525-1-1.html



## Idapython

https://github.com/inforion/idapython-cheatsheet

https://github.com/maiyao1988/IDAScripts

https://github.com/cra0/ida-scripts

https://github.com/patois/IDAPyHelper

https://github.com/TakahiroHaruyama/ida_haru

https://github.com/a1ext/auto_re

https://github.com/wangwei39120157028/IDAPythonScripts

https://github.com/veritas501/PltResolver

https://github.com/OhItsDiiTz/idc-scripts

https://github.com/RenardDev/DeDumpVAC/blob/main/DeDumpVAC.py#

https://github.com/NyaMisty/ida_kern

https://github.com/Mizari/scripts

https://github.com/Strforexc/IDApython/blob/master/protobuf-finder.py

## 插件

https://github.com/HACK-THE-WORLD/IDAPluginList

https://github.com/vmallet/ida-plugins

### 代码美化

https://github.com/oopsmishap/HexRaysPyTools

https://github.com/P4nda0s/IDABeautify

https://github.com/JustasMasiulis/ida_bitfields

https://github.com/eternalklaus/RefHunter

https://github.com/mefistotelis/ida-pro-loadmap

https://github.com/hasherezade/ida_ifl

https://github.com/1111joe1111/ida_ea

https://github.com/gaasedelen/lighthouse

https://github.com/PShocker/de-ollvm

https://github.com/YanStar/FixVmpDump

https://github.com/mcdulltii/obfDetect

https://github.com/obpo-project/obpo-plugin

https://github.com/teapotd/xdeobf

https://github.com/0xeb/ida-strikeout

https://github.com/WPeace-HcH/WPeChatGPT

https://github.com/JusticeRage/Gepetto

https://github.com/FBLeee/IDA_ChatGpt

https://github.com/H4lo/Away_From_Sub_Function_IN_IDA

https://github.com/kweatherman/ida_missinglink

https://github.com/binaryai/plugins

https://github.com/patois/HRDevHelper

https://github.com/archercreat/ida_names/

### 数据导出

https://github.com/Mixaill/FakePDB

https://github.com/NyaMisty/idatil2c

### 调试辅助

https://github.com/36hours/idaemu

https://github.com/alexhude/uEmu

https://github.com/P4nda0s/IDAFrida

https://github.com/airbus-cert/ttddbg

https://github.com/x64dbg/x64dbgida

https://github.com/cseagle/sk3wldbg

https://github.com/anic/ida2pwntools

https://github.com/danigargu/heap-viewer

https://github.com/danigargu/deREferencing

https://github.com/AntoineBlaud/EasyRe

https://github.com/AntoineBlaud/frida-ultimap

### CXX

https://github.com/MlsDmitry/better-rtti-parser

https://github.com/rcx/classinformer-ida7

https://github.com/herosi/classinformer-ida8

https://github.com/BlueAmulet/Virtuailor

https://github.com/nccgroup/SusanRTTI

https://github.com/RicBent/Classy

https://github.com/medigateio/ida_medigate

https://github.com/CyanSquid/vtable-namer

https://github.com/CyanSquid/IDA-Python-FixVtables

### 易语言

https://github.com/fjqisba/E-Decompiler

### QT

https://github.com/fjqisba/QtMetaParser

### Rust

https://github.com/cxiao/ida-rust-untangler

### NT驱动

https://github.com/can1357/NtRays

https://github.com/VoidSec/DriverBuddyReloaded

### 密码

https://github.com/polymorf/findcrypt-yara

### 安卓

https://github.com/evilpan/jni_helper

### CSharp

https://github.com/tacesrever/Il2CppParser

## Sig

https://github.com/Reverier-Xu/Rizzo-IDA/

https://github.com/ajkhoury/SigMaker-x64

https://github.com/kweatherman/sigmakerex

https://github.com/senator715/IDA-Fusion

## 特征库

https://github.com/push0ebp/sig-database

https://github.com/Maktm/FLIRTDB

## CI

### Docker

https://github.com/NyaMisty/docker-wine-ida

https://github.com/blacktop/docker-idapro

https://github.com/acbocai/run_idat

https://github.com/chnzzh/batch-ida