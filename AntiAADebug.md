# Anti-AntiAntiDebuging

反 反反调试器的技术

即对反反调试的常见技术进行检测，从而达到检测调试的目的

## 实例

https://github.com/gamous/AmogusPlugin/



## Flag蜜罐

通过主动设置BeingDebug等flag，诱使反调试插件将其修改，若被清零则必然有反反调试器存在



## Hook检测

对反反调试常用的函数进行检测



## 可靠的系统版本获取

Ntdll的文件信息